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
#ifndef XFORM_LUA_H
#define XFORM_LUA_H

#include "lunar.h"
#include "basistriangle.h"
#include "luatype.h"

namespace Lua
{
class XForm : public LuaType
{
	int idx;
	TriangleCoords coords;
	TriangleCoords coordsp;
	BasisTriangle* basisTriangle;
	flam3_xform* xform_ptr;

	void c2xf();
	void c2xfp();
	void xf2c();
	void xfp2c();

	void get_variables_from_table(lua_State*, int);
	void set_variables_to_table(lua_State*, int);

	public:
		XForm(lua_State* L);
		~XForm();
		int index(lua_State* L);
		int density(lua_State* L);
		int color_speed(lua_State* L);
		int color(lua_State* L);
		int opacity(lua_State* L);
		int animate(lua_State* L);
		int var(lua_State* L);
		int a(lua_State* L);
		int b(lua_State* L);
		int c(lua_State* L);
		int pos(lua_State* L);
		int translate(lua_State* L);
		int rotate(lua_State* L);
		int scale(lua_State* L);
		int shear(lua_State* L);

		int xa(lua_State* L);
		int xb(lua_State* L);
		int xc(lua_State* L);
		int xd(lua_State* L);
		int xe(lua_State* L);
		int xf(lua_State* L);

		int ap(lua_State* L);
		int bp(lua_State* L);
		int cp(lua_State* L);
		int posp(lua_State* L);
		int translatep(lua_State* L);
		int rotatep(lua_State* L);
		int scalep(lua_State* L);
		int shearp(lua_State* L);

		int xap(lua_State* L);
		int xbp(lua_State* L);
		int xcp(lua_State* L);
		int xdp(lua_State* L);
		int xep(lua_State* L);
		int xfp(lua_State* L);

		void setContext(LuaThreadAdapter*, int, int);
		flam3_xform* data();

		static const char className[];
		static Lunar<XForm>::RegType methods[];
};
}
#endif
