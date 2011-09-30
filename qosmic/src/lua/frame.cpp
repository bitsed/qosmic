/***************************************************************************
 *   Copyright (C) 2007 - 2011 by David Bitseff                            *
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
#include "luathreadadapter.h"

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
	method(add_genome),
	method(genome),
	method(genomes),
	{ 0, 0 }
};

Frame::Frame(lua_State* L) : LuaType()
{
	/* retrieve a context */
	lua_pushlightuserdata(L, (void*)&LuaThreadAdapter::RegKey);  /* push address */
	lua_gettable(L, LUA_REGISTRYINDEX);  /* retrieve value */
	setContext(static_cast<LuaThreadAdapter*>(lua_touserdata(L, -1)));
	lua_pop(L, 1);
}

Frame::~Frame()
{
}

int Frame::get_genome(lua_State* L)
{
	logFine(QString("Frame::get_genome, ptr: 0x%1")
			.arg((long)genome_vec->data(),0,16));
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
		flam3_genome g = flam3_genome();
		Util::init_genome(&g);
		for (int n = 0 ; n < idx - lsize + 1 ; n++)
			genome_vec->append(g);
	}
	luaL_getmetatable(L, Genome::className);
	Lunar<Genome>::new_T(L);
	Genome* g = Lunar<Genome>::check(L, 1);
	g->setContext(L, idx);

	return 1;
}

int Frame::bitdepth(lua_State* L)
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
	else
	{
		const char* fname = luaL_checkstring(L, 1);
		lua_settop(L, 0);
		lua_pushboolean(L, m_adapter->loadFile(fname));
		m_adapter->resetModified(true);
	}
	return 1;
}

int Frame::save(lua_State* L)
{
	int args = lua_gettop(L);
	if (args != 1)
	{
		luaL_error(L, "save requires one string argument", "");
		lua_pushboolean(L, false);
	}
	else
	{
		const char* fname = luaL_checkstring(L, 1);
		lua_pushboolean(L, m_adapter->saveFile(QString(fname)));
	}
	return 1;
}

int Frame::copy_genome(lua_State* L)
{
	int to = genome_vec->size();
	if (lua_isnumber(L, 2))
		to = luaL_checkint(L, 2) - 1;

	if (to < 0)
		luaL_error(L, "index out of range: Genome[%d] is null", to + 1);

	if (lua_isuserdata(L, 1))
	{
		// copy Genome 'a' to 'b'
		Lua::Genome* lg = Lunar<Genome>::check(L, 1);
		flam3_genome* g = lg->get_genome_ptr(L);
		int from = lg->index();	// a non-attached temporary Lua genome has index < 0
		flam3_genome* gd = genome_vec->data();
		if (g != gd + to)
		{
			if (to < genome_vec->size())
			{
				flam3_copy(gd + to, g); // copy 'from' into 'to'
				m_adapter->setModified(to);
			}
			else
			{
				// increase the genome vector size if needed
				while (genome_vec->size() <= to)
				{
					flam3_genome gt = flam3_genome();
					if (from < 0)
						flam3_copy(&gt, g);
					else
						flam3_copy(&gt, genome_vec->data() + from);
					genome_vec->append(gt);
					m_adapter->insertModified(genome_vec->size());
				}
			}
		}
		else
			logWarn("Lua::Frame::copy_genome : attempted to copy over same instance");
	}
	else
	{
		// copy index 'a' to 'b'
		int from = luaL_checkint(L, 1) - 1;
		if (from == to)
			return 0;

		if (from >= genome_vec->size() || from < 0)
			luaL_error(L, "index out of range: Genome[%d] is null", from + 1);

		flam3_genome* gd = genome_vec->data();
		if (gd + from != gd + to)
		{
			if (to < genome_vec->size())
			{
				flam3_copy(gd + to, gd + from); // copy 'from' into 'to'
				m_adapter->setModified(to);
			}
			else
			{
				// increase the genome vector size if needed
				while (genome_vec->size() <= to)
				{
					flam3_genome gt = flam3_genome();
					flam3_copy(&gt, genome_vec->data() + from);
					genome_vec->append(gt);
					m_adapter->insertModified(genome_vec->size());
				}
			}
		}
		else
			logWarn("Lua::Frame::copy_genome : attempted to copy over same instance");
	}

	lua_settop(L, 0);
	luaL_getmetatable(L, Genome::className);
	Lunar<Genome>::new_T(L);
	Lunar<Genome>::check(L, 1)->setContext(L, to);
	return 1;
}


int Frame::del_genome(lua_State* L)
{
	int idx = qMax(0, luaL_checkint(L, 1) - 1);
	if (idx >= genome_vec->size())
		return 0;

	lua_settop(L, 0);
	lua_pushboolean(L, genome_vec->remove(idx));
	m_adapter->removeModified(idx);
	if (idx == 0 && genome_vec->size() > 0)
		m_adapter->setModified(idx);

	return 1;
}

int Frame::add_genome(lua_State* L)
{
	int nargs = lua_gettop(L);
	if (nargs > 1)
	{
		int num_to_add = qMax(1, luaL_checkint(L, 1));
		Lua::Genome* lg = Lunar<Genome>::check(L, 2);
		flam3_genome* g = lg->get_genome_ptr(L);
		int idx = lg->index();

		for (int n = 0 ; n < num_to_add ; n++)
		{
			flam3_genome gt = flam3_genome();
			if (idx < 0)
				flam3_copy(&gt, g);
			else
				flam3_copy(&gt, genome_vec->data() + idx);
			genome_vec->append(gt);
			m_adapter->insertModified(genome_vec->size());
		}
	}
	else
	{
		int num_to_add = 1;
		flam3_genome* ga = 0;
		int idx = 0;
		if (nargs > 0)
		{
			if (lua_isuserdata(L, 1))
			{
				Lua::Genome* lg = Lunar<Genome>::check(L, 1);
				ga  = lg->get_genome_ptr(L);
				idx = lg->index();
			}
			else
				num_to_add = qMax(1, luaL_checkint(L, 1));
		}

		// increase the genome vector size
		for (int n = 0 ; n < num_to_add ; n++)
		{
			flam3_genome g = flam3_genome();
			if (ga != 0)
			{
				if (idx < 0)
					flam3_copy(&g, ga); // append the temp genome given by the user
				else
					flam3_copy(&g, genome_vec->data() + idx);
			}
			else
				Util::init_genome(&g);
			genome_vec->append(g);
			m_adapter->insertModified(genome_vec->size());
		}
	}

	lua_settop(L, 0);
	luaL_getmetatable(L, Genome::className);
	Lunar<Genome>::new_T(L);
	Lunar<Genome>::check(L, 1)->setContext(L, genome_vec->size() - 1);
	return 1;
}


int Frame::genome(lua_State* L)
{
	switch (lua_gettop(L))
	{
		case 2:
		{
			int idx = luaL_checkint(L, 1) - 1;
			if (idx < 0)
				luaL_error(L, "index out of range: Genome[%d] is null", idx + 1);
			Lua::Genome* lg = Lunar<Genome>::check(L, 2);
			flam3_genome* g = lg->get_genome_ptr(L);
			flam3_genome* to = genome_vec->data() + idx;
			if (to == g)
				break;

			if (idx < genome_vec->size())
			{
				flam3_copy(to, g);
				m_adapter->setModified(idx);
			}
			else
			{
				// increase the genome vector size if needed
				int from = lg->index();
				while (genome_vec->size() <= idx)
				{
					flam3_genome gt = flam3_genome();
					if (from < 0)
						flam3_copy(&gt, g);
					else
						flam3_copy(&gt, genome_vec->data() + from);
					genome_vec->append(gt);
					m_adapter->insertModified(genome_vec->size());
				}
			}

			break;
		}
		case 1:
		{
			int idx = luaL_checkint(L, 1) - 1;
			if (idx >= genome_vec->size() || idx < 0)
				luaL_error(L, "index out of range: Genome[%d] is null", idx + 1);
			lua_pop(L, 1);
			luaL_getmetatable(L, Genome::className);
			Lunar<Genome>::new_T(L);
			Lunar<Genome>::check(L, 1)->setContext(L, idx);
			break;
		}
		default:
		{
			lua_settop(L, 0);
			luaL_getmetatable(L, Genome::className);
			Lunar<Genome>::new_T(L);
			Lunar<Genome>::check(L, 1)->setContext(L, 0);
		}
	}
	return 1;
}

int Frame::genomes(lua_State* L)
{
	lua_settop(L, 0);
	lua_newtable(L);
	for (int n = 0 ; n < genome_vec->size() ; n++)
	{
		lua_pushinteger(L, n + 1);
		Genome* obj = new Genome(L);
		Lunar<Genome>::push(L, obj, true); // gc_T will delete this object
		obj->setContext(L, n);
		lua_settable(L, -3);
	}
	return 1;
}

// c++ interface
void Frame::setContext(LuaThreadAdapter* m)
{
	LuaType::setContext(m);
	genome_vec = m_adapter->genomeVector();
}

}
