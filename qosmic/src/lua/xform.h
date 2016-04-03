/***************************************************************************
 *   Copyright (C) 2007-2016 by David Bitseff                              *
 *   bitsed@gmail.com                                                      *
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
#include "genome.h"

namespace Lua
{
class XForm : public LuaType
{
	int m_xfidx;
	int m_gidx;
	Genome* m_genome;
	flam3_xform* xform_ptr;
	flam3_xform m_xform;
	BasisTriangle* basisTriangle;
	TriangleCoords triangleCoords;
	TriangleCoords triangleCoordsP;

	void c2xf();
	void c2xfp();
	void xf2c();
	void xfp2c();

	void get_variables_from_table(lua_State*, int);
	void set_variables_to_table(lua_State*, int);

	public:
		XForm(lua_State*);
		~XForm();
		int index(lua_State*);
		int density(lua_State*);
		int color_speed(lua_State*);
		int color(lua_State*);
		int opacity(lua_State*);
		int animate(lua_State*);
		int var(lua_State*);
		int coords(lua_State*);
		int a(lua_State*);
		int b(lua_State*);
		int c(lua_State*);
		int pos(lua_State*);
		int translate(lua_State*);
		int rotate(lua_State*);
		int scale(lua_State*);
		int shear(lua_State*);

		int coefs(lua_State*);
		int xa(lua_State*);
		int xb(lua_State*);
		int xc(lua_State*);
		int xd(lua_State*);
		int xe(lua_State*);
		int xf(lua_State*);

		int coordsp(lua_State*);
		int ap(lua_State*);
		int bp(lua_State*);
		int cp(lua_State*);
		int posp(lua_State*);
		int translatep(lua_State*);
		int rotatep(lua_State*);
		int scalep(lua_State*);
		int shearp(lua_State*);

		int coefsp(lua_State*);
		int xap(lua_State*);
		int xbp(lua_State*);
		int xcp(lua_State*);
		int xdp(lua_State*);
		int xep(lua_State*);
		int xfp(lua_State*);


		void setContext(lua_State*, Genome*, int);
		void setModified();
		flam3_xform* get_xform_ptr(lua_State*);
		flam3_xform* data();

		static const char className[];
		static Lunar<XForm>::RegType methods[];
};
}
#endif
