/***************************************************************************
 *   Copyright (C) 2007, 2008, 2009 by David Bitseff                       *
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
#include "frame.h"
#include "genome.h"

#define method(name) {#name, &Frame::name}
namespace Lua
{

const char Frame::className[] = "Frame";
Lunar<Frame>::RegType Frame::methods[] =
{
	method(get_genome),
	method(bitdepth),
	method(earlyclip),
	method(render),
	method(update),
	method(num_genomes),
	method(load),
	method(save),
	method(copy_genome),
	method(del_genome),
	{ 0, 0 }
};

Frame::Frame(lua_State* /*L*/) : LuaType()
{
}

Frame::~Frame()
{
}

int Frame::get_genome(lua_State *L)
{
	logFine(QString("Frame::get_genome, ptr: 0x%1")
			.arg((long)genome_vec->data(),0,16));
//	Lunar<Frame>::dumpstack(L);
	int idx = 0;
	if (lua_gettop(L) == 1)
	{
		idx = qMax(0, luaL_checkint(L, 1) - 1);
		lua_pop(L, 1);
	}

	int lsize = genome_vec->size();
	if ( idx >= lsize )
	{
		logInfo(QString("Frame::get_genome, adding %1 genomes")
			.arg(idx - lsize + 1));
		flam3_genome g;
		Util::init_genome(&g);
		for (int n = 0 ; n < idx - lsize + 1 ; n++)
			genome_vec->append(g);
	}
	luaL_getmetatable(L, Genome::className);
	Lunar<Genome>::new_T(L);
	Genome* g = Lunar<Genome>::check(L, 1);
	g->setContext(m_adapter, idx);

	return 1;
}

int Frame::bitdepth(lua_State *L)
{
	lua_settop(L, 0);
	lua_pushinteger(L, 64);
	return 1;
}

int Frame::earlyclip(lua_State* L)
{
	logWarn("Frame::earlyclip : function not implemented");
	lua_settop(L, 0);
	return 0;
}

int Frame::num_genomes(lua_State* L)
{
	lua_pushinteger(L, genome_vec->size());
	return 1;
}

int Frame::render(lua_State* L)
{
	int args = lua_gettop(L);
	bool saved = true;
	if (args == 2)
	{
		int idx = qMax(0, luaL_checkint(L, 1) - 1);
		const char* fname = luaL_checkstring(L, 2);
		saved = m_adapter->saveImage(QString(fname), idx);
	}
	else if (args == 1)
	{
		if (lua_isnumber(L, 1) == 1)
		{
			int idx = qMax(0, luaL_checkint(L, 1) - 1);
			m_adapter->renderPreview(idx);
		}
		else if (lua_isuserdata(L, 1) == 1)
		{
			Genome* g = Lunar<Genome>::check(L, 1);
			m_adapter->renderPreview(g->index());
		}
		else
		{
			const char* fname = luaL_checkstring(L, 1);
			saved = m_adapter->saveImage(QString(fname));
		}
	}
	else
		// call render and then block lua until the image is finished
		m_adapter->renderPreview();

	lua_settop(L, 0);
	lua_pushboolean(L, saved);
	return 1;
}

// the same as render(), but it calls m_adapter->update() instead
int Frame::update(lua_State* L)
{
	int args = lua_gettop(L);
	bool saved = true;
	if (args == 2)
	{
		int idx = qMax(0, luaL_checkint(L, 1) - 1);
		const char* fname = luaL_checkstring(L, 2);
		saved = m_adapter->saveImage(QString(fname), idx);
	}
	else if (args == 1)
	{
		if (lua_isnumber(L, 1) == 1)
		{
			int idx = qMax(0, luaL_checkint(L, 1) - 1);
			m_adapter->update(idx);
		}
		else if (lua_isuserdata(L, 1) == 1)
		{
			Genome* g = Lunar<Genome>::check(L, 1);
			m_adapter->update(g->index());
		}
		else
		{
			const char* fname = luaL_checkstring(L, 1);
			saved = m_adapter->saveImage(QString(fname));
		}
	}
	else
		// call update() and then block lua until the image is finished
		m_adapter->update();

	lua_settop(L, 0);
	lua_pushboolean(L, saved);
	return 1;
}

int Frame::load(lua_State* L)
{
	int args = lua_gettop(L);
	if (args != 1)
	{
		luaL_error(L, "load requires one string argument", "");
		lua_pushboolean(L, false);
	}
	const char* fname = luaL_checkstring(L, 1);
	lua_settop(L, 0);
	lua_pushboolean(L, m_adapter->loadFile(fname));
	return 1;
}

int Frame::save(lua_State* L)
{
	int args = lua_gettop(L);
	const char* fname;
	if (args > 1)
	{
		luaL_error(L, "save requires at most one string argument", "");
		lua_pushboolean(L, false);
	}
	if (args == 1)
	{
		fname = luaL_checkstring(L, 1);
		lua_pushboolean(L, m_adapter->saveFile(QString(fname)));
	}
	else
		lua_pushboolean(L, m_adapter->save());

	return 1;
}


// c++ interface
void Frame::setContext(LuaThreadAdapter* m)
{
	LuaType::setContext(m);
	genome_vec = m_adapter->genomeVector();
}

int Frame::copy_genome(lua_State* L)
{
	int from = qMax(0, luaL_checkint(L, 1) - 1);
	int to = qMax(0, luaL_checkint(L, 2) - 1);
	if (from == to)
		return 0;
	if (genome_vec->size() <= from )
		luaL_error(L, "genome %d is null", from);
	if (genome_vec->size() <= to)
	{
		flam3_genome g;
		Util::init_genome(&g);
		int lsize = genome_vec->size();
		for (int n = 0 ; n < to - lsize + 1 ; n++)
			genome_vec->append(g);
		to = genome_vec->size() - 1;
	}
	flam3_copy(genome_vec->data() + to, genome_vec->data() + from);
	return 0;
}


int Frame::del_genome(lua_State* L)
{
	int idx = qMax(0, luaL_checkint(L, 1) - 1);
	if (idx >= genome_vec->size())
		return 0;
	genome_vec->remove(idx);
	return 0;
}

}
