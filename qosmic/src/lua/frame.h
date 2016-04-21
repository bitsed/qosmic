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
#ifndef FRAME_LUA_H
#define FRAME_LUA_H

#include "genomevector.h"
#include "luatype.h"
#include "lunar.h"


namespace Lua
{
class LuaThreadAdapter;
class Frame : public LuaType
{
	GenomeVector* genome_vec;

	public:
		Frame(lua_State*);
		~Frame();

		// lua interface
		int get_genome(lua_State*);
		int selected(lua_State*);
		int bitdepth(lua_State*);
		int earlyclip(lua_State*);
		int num_genomes(lua_State*);
		int update(lua_State*);
		int render(lua_State*);
		int load(lua_State*);
		int save(lua_State*);
		int copy_genome(lua_State*);
		int del_genome(lua_State*);
		int add_genome(lua_State*);
		int genomes(lua_State*);
		int genome(lua_State*);

		void setContext(LuaThreadAdapter*);
		static const char className[];
		static Lunar<Frame>::RegType methods[];
};
}

#endif
