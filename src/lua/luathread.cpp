/***************************************************************************
 *   Copyright (C) 2007 by David Bitseff                                   *
 *   dbitsef@zipcon.net                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include <QDir>

#include "logger.h"
#include "luathread.h"
#include "genome.h"
#include "xform.h"
#include "qosmic.h"

#include "luathreadadapter.h"

namespace Lua
{
LuaThread::LuaThread(MainWindow* m, QObject* parent)
	: QThread(parent), mw(m), lua_error()
{
	f = 0;
	irandinit(&ctx, 0);
}

LuaThread::~LuaThread()
{
}


void LuaThread::run()
{
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);

	Lunar<Frame>::Register(L);
	Lunar<Genome>::Register(L);
	Lunar<XForm>::Register(L);

	luaL_getmetatable(L, Frame::className);
	Lunar<Frame>::new_T(L);
	f = Lunar<Frame>::check(L,1);
	LuaThreadAdapter ctx(mw, this);
	f->setContext(&ctx);
	lua_setglobal(L, "frame");

	lua_pushcfunction(L, &LuaThread::lua_stopluathread);
	lua_pushlightuserdata(L, (void*)this);
	lua_pushcclosure(L, &LuaThread::lua_stopluathread, 1);
	lua_setglobal(L, "stopped");

	lua_pushcfunction(L, &LuaThread::lua_irand);
	lua_pushlightuserdata(L, (void*)this);
	lua_pushcclosure(L, &LuaThread::lua_irand, 1);
	lua_setglobal(L, "irand");

	lua_pushcfunction(L, &LuaThread::lua_msleep);
	lua_pushlightuserdata(L, (void*)this);
	lua_pushcclosure(L, &LuaThread::lua_msleep, 1);
	lua_setglobal(L, "msleep");

	lua_load_environment(L);
	mw->setDialogsEnabled(false);
	lua_stopluathread_script = false;
	lua_error.clear();
	int error = luaL_loadstring(L, lua_text.toAscii().constData())
					|| lua_pcall(L, 0, LUA_MULTRET, 0);
	if (error)
	{
		lua_error = tr("error: %1").arg(lua_tostring(L, -1));
		if (lua_error.contains(QRegExp(tr("stopping$")))) // error says stopping if stopped
			logInfo("LuaThread::run : script stopped");
		else
			logError(QString("LuaThread::run : %1").arg(lua_error));
		lua_pop(L, 1);  /* pop error message from the stack */
	}
	else
		lua_error = tr("ok");

	lua_close(L);
	f = 0;
	mw->setDialogsEnabled(true);
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
		return luaL_error(L, "stack has no thread ref", "");

	return 1;
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
		return luaL_error(L, "stack has no thread ref", "");

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
			return luaL_error(L, "stopping", "");
		else
			lua_pushboolean(L, 1);
	}
	else
		return luaL_error(L, "stack has no thread ref", "");

	return 1;
}

// expose the sleep function for the thread
void LuaThread::msleep(unsigned long msecs)
{
	QThread::msleep(msecs);
}

void LuaThread::lua_load_environment(lua_State* L)
{
	lua_pushinteger(L, VAR_LINEAR); lua_setglobal(L, "LINEAR");
	lua_pushinteger(L, VAR_SINUSOIDAL); lua_setglobal(L, "SINUSOIDAL");
	lua_pushinteger(L, VAR_SPHERICAL); lua_setglobal(L, "SPHERICAL");
	lua_pushinteger(L, VAR_SWIRL); lua_setglobal(L, "SWIRL");
	lua_pushinteger(L, VAR_HORSESHOE); lua_setglobal(L, "HORSESHOE");
	lua_pushinteger(L, VAR_POLAR); lua_setglobal(L, "POLAR");
	lua_pushinteger(L, VAR_HANDKERCHIEF); lua_setglobal(L, "HANDKERCHIEF");
	lua_pushinteger(L, VAR_HEART); lua_setglobal(L, "HEART");
	lua_pushinteger(L, VAR_DISC); lua_setglobal(L, "DISC");
	lua_pushinteger(L, VAR_SPIRAL); lua_setglobal(L, "SPIRAL");
	lua_pushinteger(L, VAR_HYPERBOLIC); lua_setglobal(L, "HYPERBOLIC");
	lua_pushinteger(L, VAR_DIAMOND); lua_setglobal(L, "DIAMOND");
	lua_pushinteger(L, VAR_EX); lua_setglobal(L, "EX");
	lua_pushinteger(L, VAR_JULIA); lua_setglobal(L, "JULIA");
	lua_pushinteger(L, VAR_BENT); lua_setglobal(L, "BENT");
	lua_pushinteger(L, VAR_WAVES); lua_setglobal(L, "WAVES");
	lua_pushinteger(L, VAR_FISHEYE); lua_setglobal(L, "FISHEYE");
	lua_pushinteger(L, VAR_POPCORN); lua_setglobal(L, "POPCORN");
	lua_pushinteger(L, VAR_EXPONENTIAL); lua_setglobal(L, "EXPONENTIAL");
	lua_pushinteger(L, VAR_POWER); lua_setglobal(L, "POWER");
	lua_pushinteger(L, VAR_COSINE); lua_setglobal(L, "COSINE");
	lua_pushinteger(L, VAR_RINGS); lua_setglobal(L, "RINGS");
	lua_pushinteger(L, VAR_FAN); lua_setglobal(L, "FAN");
	lua_pushinteger(L, VAR_BLOB); lua_setglobal(L, "BLOB");
	lua_pushinteger(L, VAR_PDJ); lua_setglobal(L, "PDJ");
	lua_pushinteger(L, VAR_FAN2); lua_setglobal(L, "FAN2");
	lua_pushinteger(L, VAR_RINGS2); lua_setglobal(L, "RINGS2");
	lua_pushinteger(L, VAR_EYEFISH); lua_setglobal(L, "EYEFISH");
	lua_pushinteger(L, VAR_BUBBLE); lua_setglobal(L, "BUBBLE");
	lua_pushinteger(L, VAR_CYLINDER); lua_setglobal(L, "CYLINDER");
	lua_pushinteger(L, VAR_PERSPECTIVE); lua_setglobal(L, "PERSPECTIVE");
	lua_pushinteger(L, VAR_NOISE); lua_setglobal(L, "NOISE");
	lua_pushinteger(L, VAR_JULIAN); lua_setglobal(L, "JULIAN");
	lua_pushinteger(L, VAR_JULIASCOPE); lua_setglobal(L, "JULIASCOPE");
	lua_pushinteger(L, VAR_BLUR); lua_setglobal(L, "BLUR");
	lua_pushinteger(L, VAR_GAUSSIAN_BLUR); lua_setglobal(L, "GAUSSIAN_BLUR");
	lua_pushinteger(L, VAR_RADIAL_BLUR); lua_setglobal(L, "RADIAL_BLUR");
	lua_pushinteger(L, VAR_PIE); lua_setglobal(L, "PIE");
	lua_pushinteger(L, VAR_NGON); lua_setglobal(L, "NGON");
	lua_pushinteger(L, VAR_CURL); lua_setglobal(L, "CURL");
	lua_pushinteger(L, VAR_RECTANGLES); lua_setglobal(L, "RECTANGLES");
	lua_pushinteger(L, VAR_ARCH); lua_setglobal(L, "ARCH");
	lua_pushinteger(L, VAR_TANGENT); lua_setglobal(L, "TANGENT");
	lua_pushinteger(L, VAR_SQUARE); lua_setglobal(L, "SQUARE");
	lua_pushinteger(L, VAR_RAYS); lua_setglobal(L, "RAYS");
	lua_pushinteger(L, VAR_BLADE); lua_setglobal(L, "BLADE");
	lua_pushinteger(L, VAR_SECANT2); lua_setglobal(L, "SECANT");
	lua_pushinteger(L, VAR_TWINTRIAN); lua_setglobal(L, "TWINTRIAN");
	lua_pushinteger(L, VAR_CROSS); lua_setglobal(L, "CROSS");
	lua_pushinteger(L, VAR_DISC2); lua_setglobal(L, "DISC2");
	lua_pushinteger(L, VAR_SUPER_SHAPE); lua_setglobal(L, "SUPER_SHAPE");
	lua_pushinteger(L, VAR_FLOWER); lua_setglobal(L, "FLOWER");
	lua_pushinteger(L, VAR_CONIC); lua_setglobal(L, "CONIC");
	lua_pushinteger(L, VAR_PARABOLA); lua_setglobal(L, "PARABOLA");
	lua_pushinteger(L, flam3_nvariations); lua_setglobal(L, "NUM_VARS");
	lua_pushinteger(L, VAR_BENT2); lua_setglobal(L, "BENT2");
	lua_pushinteger(L, VAR_BIPOLAR); lua_setglobal(L, "BIPOLAR");
	lua_pushinteger(L, VAR_BOARDERS); lua_setglobal(L, "BOARDERS");
	lua_pushinteger(L, VAR_BUTTERFLY); lua_setglobal(L, "BUTTERFLY");
	lua_pushinteger(L, VAR_CELL); lua_setglobal(L, "CELL");
	lua_pushinteger(L, VAR_CPOW); lua_setglobal(L, "CPOW");
	lua_pushinteger(L, VAR_CURVE); lua_setglobal(L, "CURVE");
	lua_pushinteger(L, VAR_EDISC); lua_setglobal(L, "EDISC");
	lua_pushinteger(L, VAR_ELLIPTIC); lua_setglobal(L, "ELLIPTIC");
	lua_pushinteger(L, VAR_ESCHER); lua_setglobal(L, "ESCHER");
	lua_pushinteger(L, VAR_FOCI); lua_setglobal(L, "FOCI");
	lua_pushinteger(L, VAR_LAZYSUSAN); lua_setglobal(L, "LAZYSUSAN");
	lua_pushinteger(L, VAR_LOONIE); lua_setglobal(L, "LOONIE");
	lua_pushinteger(L, VAR_PRE_BLUR); lua_setglobal(L, "PRE_BLUR");
	lua_pushinteger(L, VAR_MODULUS); lua_setglobal(L, "MODULUS");
	lua_pushinteger(L, VAR_OSCILLOSCOPE); lua_setglobal(L, "OSCILLOSCOPE");
	lua_pushinteger(L, VAR_POLAR2); lua_setglobal(L, "POLAR2");
	lua_pushinteger(L, VAR_POPCORN2); lua_setglobal(L, "POPCORN2");
	lua_pushinteger(L, VAR_SCRY); lua_setglobal(L, "SCRY");
	lua_pushinteger(L, VAR_SEPARATION); lua_setglobal(L, "SEPARATION");
	lua_pushinteger(L, VAR_SPLIT); lua_setglobal(L, "SPLIT");
	lua_pushinteger(L, VAR_SPLITS); lua_setglobal(L, "SPLITS");
	lua_pushinteger(L, VAR_STRIPES); lua_setglobal(L, "STRIPES");
	lua_pushinteger(L, VAR_WEDGE); lua_setglobal(L, "WEDGE");
	lua_pushinteger(L, VAR_WEDGE_JULIA); lua_setglobal(L, "WEDGE_JULIA");
	lua_pushinteger(L, VAR_WEDGE_SPH); lua_setglobal(L, "WEDGE_SPH");
	lua_pushinteger(L, VAR_WHORL); lua_setglobal(L, "WHORL");
	lua_pushinteger(L, VAR_WAVES2); lua_setglobal(L, "WAVES2");
	lua_pushinteger(L, VAR_EXP); lua_setglobal(L, "EXP");
	lua_pushinteger(L, VAR_LOG); lua_setglobal(L, "LOG");
	lua_pushinteger(L, VAR_SIN); lua_setglobal(L, "SIN");
	lua_pushinteger(L, VAR_COS); lua_setglobal(L, "COS");
	lua_pushinteger(L, VAR_TAN); lua_setglobal(L, "TAN");
	lua_pushinteger(L, VAR_SEC); lua_setglobal(L, "SEC");
	lua_pushinteger(L, VAR_CSC); lua_setglobal(L, "CSC");
	lua_pushinteger(L, VAR_COT); lua_setglobal(L, "COT");
	lua_pushinteger(L, VAR_SINH); lua_setglobal(L, "SINH");
	lua_pushinteger(L, VAR_COSH); lua_setglobal(L, "COSH");
	lua_pushinteger(L, VAR_TANH); lua_setglobal(L, "TANH");
	lua_pushinteger(L, VAR_SECH); lua_setglobal(L, "SECH");
	lua_pushinteger(L, VAR_CSCH); lua_setglobal(L, "CSCH");
	lua_pushinteger(L, VAR_COTH); lua_setglobal(L, "COTH");
	lua_pushinteger(L, VAR_AUGER); lua_setglobal(L, "AUGER");
	lua_pushinteger(L, -1); lua_setglobal(L, "RANDOM");

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

	// adjust the package.path to include user/app paths for require()
	lua_getglobal(L, "package");
	lua_pushstring(L, "path");
	lua_gettable(L, -2);
	QString path( lua_tostring(L, -1) );
	path.append(";" + QOSMIC_SCRIPTSDIR + "/?.lua");
	path.append(";" + QDir::homePath()  + "/.qosmic/scripts/?.lua");
	lua_pop(L, 1);
	lua_pushstring(L, "path");
	lua_pushstring(L, path.toAscii().data());
	lua_settable(L, -3);
	lua_pop(L, 1);
}

void LuaThread::setLuaText(QString str)
{
	lua_text = str;
}

const QString& LuaThread::luaText()
{
	return lua_text;
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
	return lua_error;
}

}
