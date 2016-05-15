/***************************************************************************
 *   Copyright (C) 2007-2016 by David Bitseff                              *
 *   bitsed@gmail.com                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/
#include <QDir>

#include "logger.h"
#include "luathread.h"
#include "genome.h"
#include "xform.h"
#include "qosmic.h"
#include "mainwindow.h"
#include "luathreadadapter.h"

namespace Lua
{
LuaThread::LuaThread(MainWindow* m, QObject* parent)
: QThread(parent), lua_errstr(), lua_paths()
{
	lua_paths.append(QOSMIC_SCRIPTSDIR + "/?.lua");
	lua_paths.append(";" + QOSMIC_USERDIR  + "/scripts/?.lua");
	thread_adapter = new LuaThreadAdapter(m, this);
	irandinit(&ctx, 0);
	connect(this, SIGNAL(scriptStopped()),
		thread_adapter, SLOT(mainWindowChangedSlot()), Qt::DirectConnection);
}

LuaThread::~LuaThread()
{
	delete thread_adapter;
}


void LuaThread::run()
{
	lua_State* L = luaL_newstate();
	luaL_openlibs(L);
	lua_load_environment(L);
	GenomeVector* genomes = thread_adapter->genomeVector();
	int selected = genomes->selected();
	thread_adapter->resetModified();
	thread_adapter->window()->setDialogsEnabled(false);
	thread_adapter->listen(true);
	lua_stopluathread_script = false;
	lua_errstr.clear();

	int error = luaL_loadstring(L, lua_text.toLatin1().constData())
					|| lua_pcall(L, 0, LUA_MULTRET, 0);
	if (error)
	{
		lua_errstr = tr("error: %1").arg(lua_tostring(L, -1));
		if (lua_errstr.contains(QRegExp(tr("stopping$")))) // error says stopping if stopped
		{
			lua_errstr = tr("script stopped");
			logInfo("LuaThread::run : script stopped");
		}
		else
		{
			logError(QString("LuaThread::run : %1").arg(lua_errstr));
			emitScriptOutput(lua_errstr);
		}
		lua_pop(L, 1);  /* pop error message from the stack */
	}
	else
		lua_errstr = tr("ok");

	lua_close(L);
	thread_adapter->window()->setDialogsEnabled(true);

	// signal the genomevector watchers of updates made with Lua calls
	QList<bool> modified  = thread_adapter->modifiedList();
	genomes->dataModified(modified);
	int current = genomes->selected();
	if ((0 <= current) &&
	((current < modified.size() && modified.at(current)) || (selected != current)))
	{
		logFine("LuaThread::run : rendering preview for %d", current);
		thread_adapter->renderPreview(current);
	}
	thread_adapter->listen(false);
	emit scriptFinished();
}

/**
 * The is the c++ LuaThread method to signal a script that it should stop
 * executing.
 */
void LuaThread::stopScript()
{
	lua_stopluathread_script = true;
	emit scriptStopped();  // mainly for signaling the mainwindowadapter
}

/**
 * This function is bound to the lua environment function stopped().  It allows
 * the user to insert checkpoints for stopping a script from outside of the
 * lua thread.  This is handy for stopping a script that has as infinite loop.
 */
int LuaThread::lua_stopluathread(lua_State* L)
{
	LuaThread* p = static_cast<LuaThread*>(lua_touserdata(L, lua_upvalueindex(1)));
	if (p)
	{
		if (lua_gettop(L) > 0 && lua_isboolean(L, 1))
			p->lua_stopluathread_script = lua_toboolean(L, 1);
		else
			lua_pushboolean(L, p->lua_stopluathread_script);
	}
	else
		return luaL_error(L, tr("stack has no thread ref").toLatin1().constData());

	return 1;
}

/**
 * Used by the lua_print function to signal output.
 */
void LuaThread::emitScriptOutput(const QString& output)
{
	emit scriptHasOutput(output);
}

/**
 * Define my own `print' function, following the luaB_print
 * model but changing `fputs' to put the strings at a proper place
 * (a console window or a log file, for instance).
 */
int LuaThread::lua_print(lua_State* L)
{
	LuaThread* p = static_cast<LuaThread*>(lua_touserdata(L, lua_upvalueindex(1)));
	if (p)
	{
		int n = lua_gettop(L);  /* number of arguments */
		int i;
		lua_getglobal(L, "tostring");
		for (i=1; i<=n; i++)
		{
			const char *s;
			lua_pushvalue(L, -1);  /* function to be called */
			lua_pushvalue(L, i);   /* value to print */
			lua_call(L, 1, 1);
			s = lua_tostring(L, -1);  /* get result */
			if (s == NULL) {
				luaL_Buffer b;
				luaL_buffinit(L, &b);
				luaL_addstring(&b, LUA_QL("tostring"));
				luaL_addstring(&b, tr(" must return a string to ").toLatin1().constData());
				luaL_addstring(&b, LUA_QL("print"));
				luaL_pushresult(&b);
				return lua_error(L);
			}
			if (i>1) p->emitScriptOutput("\t");
			p->emitScriptOutput(s);
			lua_pop(L, 1);  /* pop result */
		}
		p->emitScriptOutput("\n");
	}
	else
		return luaL_error(L, tr("stack has no thread ref").toLatin1().constData());

	return 0;
}


void LuaThread::scriptInputResponse(bool ok, QString& response)
{
	input_response_text = response;
	input_response_ok = ok;
	input_response = true;
}

void LuaThread::emitScriptInputRequest(const QString& prompt, const QString& text)
{
	input_response = false;
	input_response_ok = false;
	input_response_text = QString();
	emit scriptInputRequest(prompt, text);
}

int LuaThread::lua_dialog(lua_State* L)
{
	LuaThread* p = static_cast<LuaThread*>(lua_touserdata(L, lua_upvalueindex(1)));
	if (p)
	{
		const QString prompt(luaL_optstring(L, 1, "?"));
		const QString   text(luaL_optstring(L, 2, ""));
		p->emitScriptInputRequest(prompt, text);

		while (!p->input_response)
			p->msleep(100);

		lua_pushboolean(L, p->input_response_ok);
		lua_pushstring(L, p->input_response_text.toLatin1().constData());
	}
	else
		return luaL_error(L, tr("stack has no thread ref").toLatin1().constData());

	return 2;
}


int LuaThread::lua_irand(lua_State* L)
{
	LuaThread* p = static_cast<LuaThread*>(lua_touserdata(L, lua_upvalueindex(1)));
	if (p)
	{
		lua_settop(L, 0);
		lua_pushnumber(L, (double)irand(&(p->ctx)) / 0xffffffff );
	}
	else
		return luaL_error(L, tr("stack has no thread ref").toLatin1().constData());

	return 1;
}

int LuaThread::lua_msleep(lua_State* L)
{
	LuaThread* p = static_cast<LuaThread*>(lua_touserdata(L, lua_upvalueindex(1)));
	if (p)
	{
		long msecs = luaL_checklong(L, 1);
		QThread::msleep(msecs);
		lua_settop(L, 0);
		// check if stopped
		if (p->lua_stopluathread_script)
			return luaL_error(L, tr("stopping").toLatin1().constData());
		else
			lua_pushboolean(L, 1);
	}
	else
		return luaL_error(L, tr("stack has no thread ref").toLatin1().constData());

	return 1;
}

// expose the sleep function for the thread
void LuaThread::msleep(unsigned long msecs)
{
	QThread::msleep(msecs);
}

void LuaThread::lua_load_environment(lua_State* L)
{
	// register methods for flam3 types
	Lunar<Frame>::Register(L);
	Lunar<Genome>::Register(L);
	Lunar<XForm>::Register(L);

	/* registry["thread_adapter"] = &thread_adapter */
	lua_pushlightuserdata(L, (void*)&LuaThreadAdapter::RegKey);  /* push address */
	lua_pushlightuserdata(L, (void*)thread_adapter);
	lua_settable(L, LUA_REGISTRYINDEX);

	// created and bind a global Frame object
	luaL_getmetatable(L, Frame::className);
	Lunar<Frame>::new_T(L);
	Lunar<Frame>::check(L, 1);
	lua_setglobal(L, "frame");

	// create and bind global support functions
	lua_pushlightuserdata(L, (void*)this);
	lua_pushcclosure(L, &LuaThread::lua_stopluathread, 1);
	lua_setglobal(L, "stopped");

	lua_pushlightuserdata(L, (void*)this);
	lua_pushcclosure(L, &LuaThread::lua_irand, 1);
	lua_setglobal(L, "irand");

	lua_pushlightuserdata(L, (void*)this);
	lua_pushcclosure(L, &LuaThread::lua_msleep, 1);
	lua_setglobal(L, "msleep");

	lua_pushlightuserdata(L, (void*)this);
	lua_pushcclosure(L, &LuaThread::lua_print, 1);
	lua_setglobal(L, "print");

	lua_pushlightuserdata(L, (void*)this);
	lua_pushcclosure(L, &LuaThread::lua_dialog, 1);
	lua_setglobal(L, "dialog");

	// global flam3 variation constants
	lua_pushinteger(L, VAR_LINEAR + 1); lua_setglobal(L, "LINEAR");
	lua_pushinteger(L, VAR_SINUSOIDAL + 1); lua_setglobal(L, "SINUSOIDAL");
	lua_pushinteger(L, VAR_SPHERICAL + 1); lua_setglobal(L, "SPHERICAL");
	lua_pushinteger(L, VAR_SWIRL + 1); lua_setglobal(L, "SWIRL");
	lua_pushinteger(L, VAR_HORSESHOE + 1); lua_setglobal(L, "HORSESHOE");
	lua_pushinteger(L, VAR_POLAR + 1); lua_setglobal(L, "POLAR");
	lua_pushinteger(L, VAR_HANDKERCHIEF + 1); lua_setglobal(L, "HANDKERCHIEF");
	lua_pushinteger(L, VAR_HEART + 1); lua_setglobal(L, "HEART");
	lua_pushinteger(L, VAR_DISC + 1); lua_setglobal(L, "DISC");
	lua_pushinteger(L, VAR_SPIRAL + 1); lua_setglobal(L, "SPIRAL");
	lua_pushinteger(L, VAR_HYPERBOLIC + 1); lua_setglobal(L, "HYPERBOLIC");
	lua_pushinteger(L, VAR_DIAMOND + 1); lua_setglobal(L, "DIAMOND");
	lua_pushinteger(L, VAR_EX + 1); lua_setglobal(L, "EX");
	lua_pushinteger(L, VAR_JULIA + 1); lua_setglobal(L, "JULIA");
	lua_pushinteger(L, VAR_BENT + 1); lua_setglobal(L, "BENT");
	lua_pushinteger(L, VAR_WAVES + 1); lua_setglobal(L, "WAVES");
	lua_pushinteger(L, VAR_FISHEYE + 1); lua_setglobal(L, "FISHEYE");
	lua_pushinteger(L, VAR_POPCORN + 1); lua_setglobal(L, "POPCORN");
	lua_pushinteger(L, VAR_EXPONENTIAL + 1); lua_setglobal(L, "EXPONENTIAL");
	lua_pushinteger(L, VAR_POWER + 1); lua_setglobal(L, "POWER");
	lua_pushinteger(L, VAR_COSINE + 1); lua_setglobal(L, "COSINE");
	lua_pushinteger(L, VAR_RINGS + 1); lua_setglobal(L, "RINGS");
	lua_pushinteger(L, VAR_FAN + 1); lua_setglobal(L, "FAN");
	lua_pushinteger(L, VAR_BLOB + 1); lua_setglobal(L, "BLOB");
	lua_pushinteger(L, VAR_PDJ + 1); lua_setglobal(L, "PDJ");
	lua_pushinteger(L, VAR_FAN2 + 1); lua_setglobal(L, "FAN2");
	lua_pushinteger(L, VAR_RINGS2 + 1); lua_setglobal(L, "RINGS2");
	lua_pushinteger(L, VAR_EYEFISH + 1); lua_setglobal(L, "EYEFISH");
	lua_pushinteger(L, VAR_BUBBLE + 1); lua_setglobal(L, "BUBBLE");
	lua_pushinteger(L, VAR_CYLINDER + 1); lua_setglobal(L, "CYLINDER");
	lua_pushinteger(L, VAR_PERSPECTIVE + 1); lua_setglobal(L, "PERSPECTIVE");
	lua_pushinteger(L, VAR_NOISE + 1); lua_setglobal(L, "NOISE");
	lua_pushinteger(L, VAR_JULIAN + 1); lua_setglobal(L, "JULIAN");
	lua_pushinteger(L, VAR_JULIASCOPE + 1); lua_setglobal(L, "JULIASCOPE");
	lua_pushinteger(L, VAR_BLUR + 1); lua_setglobal(L, "BLUR");
	lua_pushinteger(L, VAR_GAUSSIAN_BLUR + 1); lua_setglobal(L, "GAUSSIAN_BLUR");
	lua_pushinteger(L, VAR_RADIAL_BLUR + 1); lua_setglobal(L, "RADIAL_BLUR");
	lua_pushinteger(L, VAR_PIE + 1); lua_setglobal(L, "PIE");
	lua_pushinteger(L, VAR_NGON + 1); lua_setglobal(L, "NGON");
	lua_pushinteger(L, VAR_CURL + 1); lua_setglobal(L, "CURL");
	lua_pushinteger(L, VAR_RECTANGLES + 1); lua_setglobal(L, "RECTANGLES");
	lua_pushinteger(L, VAR_ARCH + 1); lua_setglobal(L, "ARCH");
	lua_pushinteger(L, VAR_TANGENT + 1); lua_setglobal(L, "TANGENT");
	lua_pushinteger(L, VAR_SQUARE + 1); lua_setglobal(L, "SQUARE");
	lua_pushinteger(L, VAR_RAYS + 1); lua_setglobal(L, "RAYS");
	lua_pushinteger(L, VAR_BLADE + 1); lua_setglobal(L, "BLADE");
	lua_pushinteger(L, VAR_SECANT2 + 1); lua_setglobal(L, "SECANT");
	lua_pushinteger(L, VAR_TWINTRIAN + 1); lua_setglobal(L, "TWINTRIAN");
	lua_pushinteger(L, VAR_CROSS + 1); lua_setglobal(L, "CROSS");
	lua_pushinteger(L, VAR_DISC2 + 1); lua_setglobal(L, "DISC2");
	lua_pushinteger(L, VAR_SUPER_SHAPE + 1); lua_setglobal(L, "SUPER_SHAPE");
	lua_pushinteger(L, VAR_FLOWER + 1); lua_setglobal(L, "FLOWER");
	lua_pushinteger(L, VAR_CONIC + 1); lua_setglobal(L, "CONIC");
	lua_pushinteger(L, VAR_PARABOLA + 1); lua_setglobal(L, "PARABOLA");
	lua_pushinteger(L, VAR_BENT2 + 1); lua_setglobal(L, "BENT2");
	lua_pushinteger(L, VAR_BIPOLAR + 1); lua_setglobal(L, "BIPOLAR");
	lua_pushinteger(L, VAR_BOARDERS + 1); lua_setglobal(L, "BOARDERS");
	lua_pushinteger(L, VAR_BUTTERFLY + 1); lua_setglobal(L, "BUTTERFLY");
	lua_pushinteger(L, VAR_CELL + 1); lua_setglobal(L, "CELL");
	lua_pushinteger(L, VAR_CPOW + 1); lua_setglobal(L, "CPOW");
	lua_pushinteger(L, VAR_CURVE + 1); lua_setglobal(L, "CURVE");
	lua_pushinteger(L, VAR_EDISC + 1); lua_setglobal(L, "EDISC");
	lua_pushinteger(L, VAR_ELLIPTIC + 1); lua_setglobal(L, "ELLIPTIC");
	lua_pushinteger(L, VAR_ESCHER + 1); lua_setglobal(L, "ESCHER");
	lua_pushinteger(L, VAR_FOCI + 1); lua_setglobal(L, "FOCI");
	lua_pushinteger(L, VAR_LAZYSUSAN + 1); lua_setglobal(L, "LAZYSUSAN");
	lua_pushinteger(L, VAR_LOONIE + 1); lua_setglobal(L, "LOONIE");
	lua_pushinteger(L, VAR_PRE_BLUR + 1); lua_setglobal(L, "PRE_BLUR");
	lua_pushinteger(L, VAR_MODULUS + 1); lua_setglobal(L, "MODULUS");
	lua_pushinteger(L, VAR_OSCILLOSCOPE + 1); lua_setglobal(L, "OSCILLOSCOPE");
	lua_pushinteger(L, VAR_POLAR2 + 1); lua_setglobal(L, "POLAR2");
	lua_pushinteger(L, VAR_POPCORN2 + 1); lua_setglobal(L, "POPCORN2");
	lua_pushinteger(L, VAR_SCRY + 1); lua_setglobal(L, "SCRY");
	lua_pushinteger(L, VAR_SEPARATION + 1); lua_setglobal(L, "SEPARATION");
	lua_pushinteger(L, VAR_SPLIT + 1); lua_setglobal(L, "SPLIT");
	lua_pushinteger(L, VAR_SPLITS + 1); lua_setglobal(L, "SPLITS");
	lua_pushinteger(L, VAR_STRIPES + 1); lua_setglobal(L, "STRIPES");
	lua_pushinteger(L, VAR_WEDGE + 1); lua_setglobal(L, "WEDGE");
	lua_pushinteger(L, VAR_WEDGE_JULIA + 1); lua_setglobal(L, "WEDGE_JULIA");
	lua_pushinteger(L, VAR_WEDGE_SPH + 1); lua_setglobal(L, "WEDGE_SPH");
	lua_pushinteger(L, VAR_WHORL + 1); lua_setglobal(L, "WHORL");
	lua_pushinteger(L, VAR_WAVES2 + 1); lua_setglobal(L, "WAVES2");
	lua_pushinteger(L, VAR_EXP + 1); lua_setglobal(L, "EXP");
	lua_pushinteger(L, VAR_LOG + 1); lua_setglobal(L, "LOG");
	lua_pushinteger(L, VAR_SIN + 1); lua_setglobal(L, "SIN");
	lua_pushinteger(L, VAR_COS + 1); lua_setglobal(L, "COS");
	lua_pushinteger(L, VAR_TAN + 1); lua_setglobal(L, "TAN");
	lua_pushinteger(L, VAR_SEC + 1); lua_setglobal(L, "SEC");
	lua_pushinteger(L, VAR_CSC + 1); lua_setglobal(L, "CSC");
	lua_pushinteger(L, VAR_COT + 1); lua_setglobal(L, "COT");
	lua_pushinteger(L, VAR_SINH + 1); lua_setglobal(L, "SINH");
	lua_pushinteger(L, VAR_COSH + 1); lua_setglobal(L, "COSH");
	lua_pushinteger(L, VAR_TANH + 1); lua_setglobal(L, "TANH");
	lua_pushinteger(L, VAR_SECH + 1); lua_setglobal(L, "SECH");
	lua_pushinteger(L, VAR_CSCH + 1); lua_setglobal(L, "CSCH");
	lua_pushinteger(L, VAR_COTH + 1); lua_setglobal(L, "COTH");
	lua_pushinteger(L, VAR_AUGER + 1); lua_setglobal(L, "AUGER");
	lua_pushinteger(L, VAR_FLUX + 1); lua_setglobal(L, "FLUX");
	lua_pushinteger(L, VAR_MOBIUS + 1); lua_setglobal(L, "MOBIUS");
	lua_pushinteger(L, -1); lua_setglobal(L, "RANDOM");
	lua_pushinteger(L, flam3_nvariations); lua_setglobal(L, "NUM_VARS");

	// spatial filter kernel types
	lua_pushinteger(L, flam3_gaussian_kernel); lua_setglobal(L, "GAUSSIAN_KERNEL");
	lua_pushinteger(L, flam3_hermite_kernel); lua_setglobal(L, "HERMITE_KERNEL");
	lua_pushinteger(L, flam3_box_kernel); lua_setglobal(L, "BOX_KERNEL");
	lua_pushinteger(L, flam3_triangle_kernel); lua_setglobal(L, "TRIANGLE_KERNEL");
	lua_pushinteger(L, flam3_bell_kernel); lua_setglobal(L, "BELL_KERNEL");
	lua_pushinteger(L, flam3_b_spline_kernel); lua_setglobal(L, "B_SPLINE_KERNEL");
	lua_pushinteger(L, flam3_lanczos3_kernel); lua_setglobal(L, "LANCZOS3_KERNEL");
	lua_pushinteger(L, flam3_lanczos2_kernel); lua_setglobal(L, "LANCZOS2_KERNEL");
	lua_pushinteger(L, flam3_mitchell_kernel); lua_setglobal(L, "MITCHELL_KERNEL");
	lua_pushinteger(L, flam3_blackman_kernel); lua_setglobal(L, "BLACKMAN_KERNEL");
	lua_pushinteger(L, flam3_catrom_kernel); lua_setglobal(L, "CATROM_KERNEL");
	lua_pushinteger(L, flam3_hamming_kernel); lua_setglobal(L, "HAMMING_KERNEL");
	lua_pushinteger(L, flam3_hanning_kernel); lua_setglobal(L, "HANNING_KERNEL");
	lua_pushinteger(L, flam3_quadratic_kernel); lua_setglobal(L, "QUADRATIC_KERNEL");

	// motion interpolation curves and functions
	lua_pushinteger(L, flam3_interpolation_linear); lua_setglobal(L, "INTERPOLATION_LINEAR");
	lua_pushinteger(L, flam3_interpolation_smooth); lua_setglobal(L, "INTERPOLATION_SMOOTH");
	lua_pushinteger(L, flam3_inttype_linear); lua_setglobal(L, "INTTYPE_LINEAR");
	lua_pushinteger(L, flam3_inttype_log); lua_setglobal(L, "INTTYPE_LOG");
	lua_pushinteger(L, flam3_inttype_compat); lua_setglobal(L, "INTTYPE_COMPAT");
	lua_pushinteger(L, flam3_inttype_older); lua_setglobal(L, "INTTYPE_OLDER");

	// create an array of variation names
	lua_createtable(L, flam3_nvariations, 0);
	lua_pushinteger(L, VAR_LINEAR + 1); lua_pushstring(L, "LINEAR"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_SINUSOIDAL + 1); lua_pushstring(L, "SINUSOIDAL"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_SPHERICAL + 1); lua_pushstring(L, "SPHERICAL"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_SWIRL + 1); lua_pushstring(L, "SWIRL"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_HORSESHOE + 1); lua_pushstring(L, "HORSESHOE"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_POLAR + 1); lua_pushstring(L, "POLAR"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_HANDKERCHIEF + 1); lua_pushstring(L, "HANDKERCHIEF"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_HEART + 1); lua_pushstring(L, "HEART"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_DISC + 1); lua_pushstring(L, "DISC"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_SPIRAL + 1); lua_pushstring(L, "SPIRAL"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_HYPERBOLIC + 1); lua_pushstring(L, "HYPERBOLIC"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_DIAMOND + 1); lua_pushstring(L, "DIAMOND"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_EX + 1); lua_pushstring(L, "EX"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_JULIA + 1); lua_pushstring(L, "JULIA"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_BENT + 1); lua_pushstring(L, "BENT"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_WAVES + 1); lua_pushstring(L, "WAVES"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_FISHEYE + 1); lua_pushstring(L, "FISHEYE"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_POPCORN + 1); lua_pushstring(L, "POPCORN"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_EXPONENTIAL + 1); lua_pushstring(L, "EXPONENTIAL"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_POWER + 1); lua_pushstring(L, "POWER"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_COSINE + 1); lua_pushstring(L, "COSINE"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_RINGS + 1); lua_pushstring(L, "RINGS"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_FAN + 1); lua_pushstring(L, "FAN"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_BLOB + 1); lua_pushstring(L, "BLOB"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_PDJ + 1); lua_pushstring(L, "PDJ"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_FAN2 + 1); lua_pushstring(L, "FAN2"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_RINGS2 + 1); lua_pushstring(L, "RINGS2"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_EYEFISH + 1); lua_pushstring(L, "EYEFISH"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_BUBBLE + 1); lua_pushstring(L, "BUBBLE"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_CYLINDER + 1); lua_pushstring(L, "CYLINDER"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_PERSPECTIVE + 1); lua_pushstring(L, "PERSPECTIVE"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_NOISE + 1); lua_pushstring(L, "NOISE"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_JULIAN + 1); lua_pushstring(L, "JULIAN"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_JULIASCOPE + 1); lua_pushstring(L, "JULIASCOPE"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_BLUR + 1); lua_pushstring(L, "BLUR"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_GAUSSIAN_BLUR + 1); lua_pushstring(L, "GAUSSIAN_BLUR"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_RADIAL_BLUR + 1); lua_pushstring(L, "RADIAL_BLUR"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_PIE + 1); lua_pushstring(L, "PIE"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_NGON + 1); lua_pushstring(L, "NGON"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_CURL + 1); lua_pushstring(L, "CURL"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_RECTANGLES + 1); lua_pushstring(L, "RECTANGLES"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_ARCH + 1); lua_pushstring(L, "ARCH"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_TANGENT + 1); lua_pushstring(L, "TANGENT"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_SQUARE + 1); lua_pushstring(L, "SQUARE"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_RAYS + 1); lua_pushstring(L, "RAYS"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_BLADE + 1); lua_pushstring(L, "BLADE"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_SECANT2 + 1); lua_pushstring(L, "SECANT"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_TWINTRIAN + 1); lua_pushstring(L, "TWINTRIAN"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_CROSS + 1); lua_pushstring(L, "CROSS"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_DISC2 + 1); lua_pushstring(L, "DISC2"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_SUPER_SHAPE + 1); lua_pushstring(L, "SUPER_SHAPE"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_FLOWER + 1); lua_pushstring(L, "FLOWER"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_CONIC + 1); lua_pushstring(L, "CONIC"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_PARABOLA + 1); lua_pushstring(L, "PARABOLA"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_BENT2 + 1); lua_pushstring(L, "BENT2"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_BIPOLAR + 1); lua_pushstring(L, "BIPOLAR"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_BOARDERS + 1); lua_pushstring(L, "BOARDERS"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_BUTTERFLY + 1); lua_pushstring(L, "BUTTERFLY"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_CELL + 1); lua_pushstring(L, "CELL"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_CPOW + 1); lua_pushstring(L, "CPOW"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_CURVE + 1); lua_pushstring(L, "CURVE"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_EDISC + 1); lua_pushstring(L, "EDISC"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_ELLIPTIC + 1); lua_pushstring(L, "ELLIPTIC"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_ESCHER + 1); lua_pushstring(L, "ESCHER"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_FOCI + 1); lua_pushstring(L, "FOCI"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_LAZYSUSAN + 1); lua_pushstring(L, "LAZYSUSAN"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_LOONIE + 1); lua_pushstring(L, "LOONIE"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_PRE_BLUR + 1); lua_pushstring(L, "PRE_BLUR"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_MODULUS + 1); lua_pushstring(L, "MODULUS"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_OSCILLOSCOPE + 1); lua_pushstring(L, "OSCILLOSCOPE"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_POLAR2 + 1); lua_pushstring(L, "POLAR2"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_POPCORN2 + 1); lua_pushstring(L, "POPCORN2"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_SCRY + 1); lua_pushstring(L, "SCRY"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_SEPARATION + 1); lua_pushstring(L, "SEPARATION"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_SPLIT + 1); lua_pushstring(L, "SPLIT"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_SPLITS + 1); lua_pushstring(L, "SPLITS"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_STRIPES + 1); lua_pushstring(L, "STRIPES"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_WEDGE + 1); lua_pushstring(L, "WEDGE"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_WEDGE_JULIA + 1); lua_pushstring(L, "WEDGE_JULIA"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_WEDGE_SPH + 1); lua_pushstring(L, "WEDGE_SPH"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_WHORL + 1); lua_pushstring(L, "WHORL"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_WAVES2 + 1); lua_pushstring(L, "WAVES2"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_EXP + 1); lua_pushstring(L, "EXP"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_LOG + 1); lua_pushstring(L, "LOG"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_SIN + 1); lua_pushstring(L, "SIN"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_COS + 1); lua_pushstring(L, "COS"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_TAN + 1); lua_pushstring(L, "TAN"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_SEC + 1); lua_pushstring(L, "SEC"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_CSC + 1); lua_pushstring(L, "CSC"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_COT + 1); lua_pushstring(L, "COT"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_SINH + 1); lua_pushstring(L, "SINH"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_COSH + 1); lua_pushstring(L, "COSH"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_TANH + 1); lua_pushstring(L, "TANH"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_SECH + 1); lua_pushstring(L, "SECH"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_CSCH + 1); lua_pushstring(L, "CSCH"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_COTH + 1); lua_pushstring(L, "COTH"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_AUGER + 1); lua_pushstring(L, "AUGER"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_FLUX + 1); lua_pushstring(L, "FLUX"); lua_settable(L, -3);
	lua_pushinteger(L, VAR_MOBIUS + 1); lua_pushstring(L, "MOBIUS"); lua_settable(L, -3);
	lua_setglobal(L, "VARIATIONS");

	// adjust the package.path to include user/app paths for require()
	if (!lua_paths.isEmpty())
	{
		lua_getglobal(L, "package");
		lua_pushstring(L, "path");
		lua_gettable(L, -2);
		QString path( lua_tostring(L, -1) );
		path.append(";" + lua_paths);
		lua_pop(L, 1);
		lua_pushstring(L, "path");
		lua_pushstring(L, path.toLatin1().data());
		lua_settable(L, -3);
		lua_pop(L, 1);
	}
}

void LuaThread::setLuaText(QString str)
{
	lua_text = str;
}

const QString& LuaThread::luaText()
{
	return lua_text;
}

void LuaThread::setLuaPaths(const QString& paths)
{
	lua_paths = paths;
}

QString LuaThread::luaPaths() const
{
	return lua_paths;
}

bool LuaThread::stopping() const
{
	return lua_stopluathread_script;
}


QString LuaThread::getMessage()
{
	static int pos = 0;
	if (isRunning())
	{
		QString msg(tr("Running ........"));
		msg.chop(8 - (pos++ % 8));
		return msg;
	}
	return lua_errstr;
}

}

