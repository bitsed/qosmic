/***************************************************************************
 *   Copyright (C) 2007-2024 by David Bitseff                              *
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
#include "xform.h"
#include "luathreadadapter.h"

#include <cmath>
#include <QCoreApplication>
#define tr(msg) QCoreApplication::translate("Lua::XForm", msg).toLatin1().constData()

namespace Lua
{
const char XForm::className[] = "XForm";

Lunar<XForm>::RegType XForm::methods[] =
{
	{ "index", &XForm::index },
	{ "density", &XForm::density },
	{ "color_speed", &XForm::color_speed },
	{ "color", &XForm::color },
	{ "opacity", &XForm::opacity },
	{ "animate", &XForm::animate },
	{ "var", &XForm::var },
	{ "param", &XForm::var },

	// xform coordinates
	{ "coords", &XForm::coords },
	{ "o", &XForm::a },
	{ "x", &XForm::b },
	{ "y", &XForm::c },
	{ "a", &XForm::a },
	{ "b", &XForm::b },
	{ "c", &XForm::c },
	{ "coefs", &XForm::coefs },
	{ "xa", &XForm::xa },
	{ "xb", &XForm::xb },
	{ "xc", &XForm::xc },
	{ "xd", &XForm::xd },
	{ "xe", &XForm::xe },
	{ "xf", &XForm::xf },
	{ "pos", &XForm::pos },
	{ "translate", &XForm::translate },
	{ "rotate", &XForm::rotate },
	{ "scale", &XForm::scale },
	{ "shear", &XForm::shear },

	// post xform coordinates
	{ "coordsp", &XForm::coordsp },
	{ "op", &XForm::ap },
	{ "xp", &XForm::bp },
	{ "yp", &XForm::cp },
	{ "coefsp", &XForm::coefsp },
	{ "ap", &XForm::ap },
	{ "bp", &XForm::bp },
	{ "cp", &XForm::cp },
	{ "xap", &XForm::xap },
	{ "xbp", &XForm::xbp },
	{ "xcp", &XForm::xcp },
	{ "xdp", &XForm::xdp },
	{ "xep", &XForm::xep },
	{ "xfp", &XForm::xfp },
	{ "posp", &XForm::posp },
	{ "translatep", &XForm::translatep },
	{ "rotatep", &XForm::rotatep },
	{ "scalep", &XForm::scalep },
	{ "shearp", &XForm::shearp },

	{ 0, 0 }
};

XForm::XForm(lua_State* L)
:  m_xfidx(-1), m_gidx(-1), m_genome(0), m_xform()
{
	/* retrieve a context */
	lua_pushlightuserdata(L, (void*)&LuaThreadAdapter::RegKey);  /* push address */
	lua_gettable(L, LUA_REGISTRYINDEX);  /* retrieve value */
	LuaType::setContext(static_cast<LuaThreadAdapter*>(lua_touserdata(L, -1)));
	basisTriangle = m_adapter->basisTriangle();
	lua_pop(L, 1);

	if (lua_gettop(L) > 0 && lua_isuserdata(L, 1))
	{
		// check for a XForm type to copy
		XForm* xf = Lunar<XForm>::check(L, 1);
		flam3_copy_xform(&m_xform, xf->get_xform_ptr(L));
		lua_pop(L, 1);
	}
	else
		Util::init_xform(&m_xform);

	get_xform_ptr(L);
	xf2c(); // load Ax,Ay, Bx,By, Cx,Cy
	xfp2c();
}

flam3_xform* XForm::get_xform_ptr(lua_State* L)
{
	if (m_genome != 0) // XForm associated with some Genome
	{
		flam3_genome* g = m_genome->get_genome_ptr(L);
		if (m_xfidx >= 0 && m_xfidx < g->num_xforms) // has an xform index
			xform_ptr = g->xform + m_xfidx;
		else
			luaL_error(L, tr("index out of bounds: Genome[%d].XForm[%d]"), m_gidx + 1, m_xfidx + 1);
	}
	else
	{
		// XForm not associated with a Genome
		xform_ptr = &m_xform;
	}

	return xform_ptr;
}

XForm::~XForm()
{
}

#define intAcc(name) int XForm::name(lua_State* L)\
{\
	get_xform_ptr(L); \
	if (lua_gettop(L) == 1)\
	{\
		xform_ptr->name = luaL_checkint(L, 1);\
		setModified();\
	}\
	else\
	{\
		lua_settop(L, 0);\
		lua_pushinteger(L, xform_ptr->name);\
	}\
	return 1;\
}\

#define intAccRO(name) int XForm::name(lua_State* L)\
{\
	get_xform_ptr(L); \
	lua_settop(L, 0);\
	lua_pushinteger(L, xform_ptr->name);\
	return 1;\
}\

#define realAcc(name) int XForm::name(lua_State* L)\
{\
	get_xform_ptr(L); \
	if (lua_gettop(L) == 1)\
	{\
		xform_ptr->name = luaL_checknumber(L, 1);\
		setModified();\
	}\
	else\
	{\
		lua_settop(L, 0);\
		lua_pushnumber(L, xform_ptr->name);\
	}\
	return 1;\
}\

#define realAccRO(name) int XForm::name(lua_State* L)\
{\
	get_xform_ptr(L); \
	lua_settop(L, 0);\
	lua_pushnumber(L, xform_ptr->name);\
	return 1;\
}\

int XForm::index(lua_State* L)
{
	get_xform_ptr(L);
	lua_settop(L, 0);
	lua_pushinteger(L, m_xfidx + 1);
	return 1;
}

realAcc(density);
realAcc(color_speed);

int XForm::color(lua_State* L)
{
	get_xform_ptr(L);
	if (lua_gettop(L) == 1)
	{
		xform_ptr->color = luaL_checknumber(L, 1);
		setModified();
	}
	else
	{
		lua_settop(L, 0);
		lua_pushnumber(L, xform_ptr->color);
	}
	return 1;
}

int XForm::opacity(lua_State* L)
{
	get_xform_ptr(L);
	if (lua_gettop(L) == 1)
	{
		xform_ptr->opacity = qBound(0.0, luaL_checknumber(L, 1), 1.0);
		setModified();
	}
	else
	{
		lua_settop(L, 0);
		lua_pushnumber(L, xform_ptr->opacity);
	}
	return 1;
}

int XForm::animate(lua_State* L)
{
	get_xform_ptr(L);
	if (lua_gettop(L) == 1)
	{
		xform_ptr->animate = qMax(0.0, luaL_checknumber(L, 1));
		setModified();
	}
	else
	{
		lua_settop(L, 0);
		lua_pushnumber(L, xform_ptr->animate);
	}
	return 1;
}

int XForm::var(lua_State* L)
{
	get_xform_ptr(L);
	if (lua_gettop(L) < 1)
	{
		// return the entire set of variations
		lua_newtable(L);
		for (int i = 0 ; i < flam3_nvariations ; i++)
		{
			lua_pushinteger(L, i + 1);
			lua_newtable(L);
			lua_pushinteger(L, 1);
			lua_pushnumber(L, xform_ptr->var[i]);
			lua_rawset(L, -3);
			lua_pushinteger(L, 2);
			lua_newtable(L);
			set_variables_to_table(L, i);
			lua_rawset(L, -3);

			lua_getglobal(L, "_xform_var_variables_metatable");
			if (!lua_istable(L, -1))
			{
				lua_pop(L, 1);
				int error = luaL_loadstring(L,
					"if _xform_var_variables_metatable == nil then "
					"_xform_var_variables_metatable = { __index = function (table, key) "
					"if key == \"value\" then return rawget(table, 1) end "
					"if key == \"variables\" then return rawget(table, 2) end "
					"local v = rawget(rawget(table, 2), key) "
					"if v ~= nil then return v end "
					"return rawget(table, key) end, "
					"__newindex = function(table, key, value) "
					"if key == \"value\" then return rawset(table, 1, value) end "
					"if key == \"variables\" then return rawset(table, 2, value) end "
					"local v = rawget(rawget(table, 2), key) "
					"if v ~= nil then return rawset(rawget(table, 2), key, value) end "
					"return rawset(table, key, value) "
					"end } end") || lua_pcall(L, 0, 0, 0);
				if (error)
				{
					QString s(tr("couldn't build metatables for var.variables: %1"));
					luaL_error(L, "%s", s.arg(lua_tostring(L, -1)).toLatin1().constData());
				}

				lua_getglobal(L, "_xform_var_variables_metatable");
			}
			lua_setmetatable(L, -2);
			lua_rawset(L, -3);
		}

		lua_getglobal(L, "_xform_var_metatable");
		if (!lua_istable(L, -1))
		{
			lua_pop(L, 1);
			int error = luaL_loadstring(L,
				"if _xform_var_metatable == nil then "
				"_xform_var_metatable = { __index = function(table, key) "
				"local k = string.upper(key) "
				"return rawget(table, _G[k]) "
				"end } end") || lua_pcall(L, 0, 0, 0);
			if (error)
			{
				QString s(tr("couldn't build metatables for var: %1"));
				luaL_error(L, "%s", s.arg(lua_tostring(L, -1)).toLatin1().constData());
			}
			lua_getglobal(L, "_xform_var_metatable");
		}
		lua_setmetatable(L, -2);
	}
	else if (lua_type(L, 1) == LUA_TTABLE)
	{
		// setting the all xform variations from a table
		for (int i = 0 ; i < flam3_nvariations ; i++)
		{
			lua_pushinteger(L, i + 1);
			lua_rawget(L, -2);
			luaL_checktype(L, -1, LUA_TTABLE);

			lua_pushinteger(L, 1);
			lua_rawget(L, -2);
			xform_ptr->var[i] = luaL_checknumber(L, -1);
			lua_pop(L, 1);

			lua_pushinteger(L, 2);
			lua_rawget(L, -2);
			luaL_checktype(L, -1, LUA_TTABLE);
			get_variables_from_table(L, i);
			lua_pop(L, 2);
		}
		setModified();
	}
	else
	{
		int var_num;
		if (lua_isnumber(L, 1) == 1)
		{
			var_num = lua_tointeger(L, 1) - 1;
			if (var_num < 0 || var_num >= flam3_nvariations)
				luaL_error(L, tr("invalid variation index %d"), var_num + 1);
		}
		else
		{
			const char* name = luaL_checkstring(L, 1);
			var_num = Util::variation_number(QString(name).toLower());
			if (var_num == -1)
				luaL_error(L, tr("variation %s not found"), name);
		}

		int args = lua_gettop(L);
		if (args > 1)
		{
			// grab the variation value
			xform_ptr->var[var_num] = luaL_checknumber(L, 2);
			setModified();
			if (args == 3)
			{
				if (!lua_istable(L, 3))
					luaL_error(L, tr("variables argument is not a valid table"));
				get_variables_from_table(L, var_num);
			}
			else
				return 1;
		}
		else
		{
			lua_settop(L, 0);
			lua_pushnumber(L, xform_ptr->var[var_num]);
			lua_newtable(L);
			set_variables_to_table(L, var_num);
		}
		return 2;
	}
	return 1;
}

/** this macro is used in get_variables_from_table() to copy the lua table
  * of variables into the xform structure
  */
#define get_table_var(VARIATION, VARIABLE)\
	lua_pushstring(L, #VARIABLE);\
	lua_rawget(L, -2);\
	if (!lua_isnil(L, -1))\
	{\
		if (!lua_isnumber(L, -1))\
			luaL_error(L, tr("number expected for variable " #VARIABLE));\
		xform_ptr->VARIATION##_##VARIABLE = lua_tonumber(L, -1);\
	}\
	lua_pop(L, 1);

/** Set the variables for the given variation from the table.  A string
  * indexed table containing the variables is on the top of the stack.
  */
void XForm::get_variables_from_table(lua_State* L, int variation)
{
	switch (variation)
	{
		case VAR_BLOB:
		{
			get_table_var(blob, low)
			get_table_var(blob, high)
			get_table_var(blob, waves)
			break;
		}
		case VAR_PDJ:
		{
			get_table_var(pdj, a)
			get_table_var(pdj, b)
			get_table_var(pdj, c)
			get_table_var(pdj, d)
			break;
		}
		case VAR_FAN2:
		{
			get_table_var(fan2, x)
			get_table_var(fan2, y)
			break;
		}
		case VAR_RINGS2:
		{
			get_table_var(rings2, val)
			break;
		}
		case VAR_PERSPECTIVE:
		{
			get_table_var(perspective, angle)
			get_table_var(perspective, dist)
			break;
		}
		case VAR_JULIAN:
		{
			get_table_var(julian, power)
			get_table_var(julian, dist)
			break;
		}
		case VAR_JULIASCOPE:
		{
			get_table_var(juliascope, power)
			get_table_var(juliascope, dist)
			break;
		}
		case VAR_RADIAL_BLUR:
		{
			get_table_var(radial_blur, angle)
			break;
		}
		case VAR_PIE:
		{
			get_table_var(pie, rotation)
			get_table_var(pie, slices)
			get_table_var(pie, thickness)
			break;
		}
		case VAR_NGON:
		{
			get_table_var(ngon, circle)
			get_table_var(ngon, corners)
			get_table_var(ngon, sides)
			get_table_var(ngon, power)
			break;
		}
		case VAR_CURL:
		{
			get_table_var(curl, c1)
			get_table_var(curl, c2)
			break;
		}
		case VAR_RECTANGLES:
		{
			get_table_var(rectangles, x)
			get_table_var(rectangles, y)
			break;
		}
		case VAR_DISC2:
		{
			get_table_var(disc2, rot)
			get_table_var(disc2, twist)
			break;
		}
		case VAR_SUPER_SHAPE:
		{
			get_table_var(super_shape, rnd)
			get_table_var(super_shape, m)
			get_table_var(super_shape, n1)
			get_table_var(super_shape, n2)
			get_table_var(super_shape, n3)
			get_table_var(super_shape, holes)
			break;
		}
		case VAR_FLOWER:
		{
			get_table_var(flower, holes)
			get_table_var(flower, petals)
			break;
		}
		case VAR_CONIC:
		{
			get_table_var(conic, eccentricity)
			get_table_var(conic, holes)
			break;
		}
		case VAR_PARABOLA:
		{
			get_table_var(parabola, height)
			get_table_var(parabola, width)
			break;
		}
		case VAR_BENT2:
		{
			get_table_var(bent2, x)
			get_table_var(bent2, y)
			break;
		}
		case VAR_BIPOLAR:
		{
			get_table_var(bipolar, shift)
			break;
		}
		case VAR_CELL:
		{
			get_table_var(cell, size)
			break;
		}
		case VAR_CPOW:
		{
			get_table_var(cpow, r)
			get_table_var(cpow, i)
			get_table_var(cpow, power)
			break;
		}
		case VAR_CURVE:
		{
			get_table_var(curve, xamp)
			get_table_var(curve, yamp)
			get_table_var(curve, xlength)
			get_table_var(curve, ylength)
			break;
		}
		case VAR_ESCHER:
		{
			get_table_var(escher, beta)
			break;
		}
		case VAR_LAZYSUSAN:
		{
			get_table_var(lazysusan, spin)
			get_table_var(lazysusan, space)
			get_table_var(lazysusan, twist)
			get_table_var(lazysusan, x)
			get_table_var(lazysusan, y)
			break;
		}
		case VAR_MODULUS:
		{
			get_table_var(modulus, x)
			get_table_var(modulus, y)
			break;
		}
		case VAR_OSCILLOSCOPE:
		{
			get_table_var(oscope, separation)
			get_table_var(oscope, frequency)
			get_table_var(oscope, amplitude)
			get_table_var(oscope, damping)
			break;
		}
		case VAR_POPCORN2:
		{
			get_table_var(popcorn2, x)
			get_table_var(popcorn2, y)
			get_table_var(popcorn2, c)
			break;
		}
		case VAR_SEPARATION:
		{
			get_table_var(separation, x)
			get_table_var(separation, xinside)
			get_table_var(separation, y)
			get_table_var(separation, yinside)
			break;
		}
		case VAR_SPLIT:
		{
			get_table_var(split, xsize)
			get_table_var(split, ysize)
			break;
		}
		case VAR_SPLITS:
		{
			get_table_var(splits, x)
			get_table_var(splits, y)
			break;
		}
		case VAR_STRIPES:
		{
			get_table_var(stripes, space)
			get_table_var(stripes, warp)
			break;
		}
		case VAR_WEDGE:
		{
			get_table_var(wedge, angle)
			get_table_var(wedge, hole)
			get_table_var(wedge, count)
			get_table_var(wedge, swirl)
			break;
		}
		case VAR_WEDGE_JULIA:
		{
			get_table_var(wedge_julia, angle)
			get_table_var(wedge_julia, count)
			get_table_var(wedge_julia, power)
			get_table_var(wedge_julia, dist)
			break;
		}
		case VAR_WEDGE_SPH:
		{
			get_table_var(wedge_sph, angle)
			get_table_var(wedge_sph, count)
			get_table_var(wedge_sph, hole)
			get_table_var(wedge_sph, swirl)
			break;
		}
		case VAR_WHORL:
		{
			get_table_var(whorl, inside)
			get_table_var(whorl, outside)
			break;
		}
		case VAR_WAVES2:
		{
			get_table_var(waves2, freqx)
			get_table_var(waves2, scalex)
			get_table_var(waves2, freqy)
			get_table_var(waves2, scaley)
			break;
		}
		case VAR_AUGER:
		{
			get_table_var(auger, sym)
			get_table_var(auger, weight)
			get_table_var(auger, freq)
			get_table_var(auger, scale)
			break;
		}
		case VAR_FLUX:
		{
			get_table_var(flux, spread)
			break;
		}
		case VAR_MOBIUS:
		{
			get_table_var(mobius, re_a)
			get_table_var(mobius, im_a)
			get_table_var(mobius, re_b)
			get_table_var(mobius, im_b)
			get_table_var(mobius, re_c)
			get_table_var(mobius, im_c)
			get_table_var(mobius, re_d)
			get_table_var(mobius, im_d)
			break;
		}
		default:
			break;
	}
}

/** this macro is used in set_variables_to_table() to copy the xform variables
  * into the lua table
  */
#define set_table_var(VARIATION, VARIABLE)\
	lua_pushstring(L, #VARIABLE);\
	lua_pushnumber(L, xform_ptr->VARIATION##_##VARIABLE);\
	lua_rawset(L, -3);

/** Set the variables for the given variation to the table
  * on the top of the stack.
  */
void XForm::set_variables_to_table(lua_State* L, int variation)
{
	switch (variation)
	{
		case VAR_BLOB:
		{
			set_table_var(blob, low)
			set_table_var(blob, high)
			set_table_var(blob, waves)
			break;
		}
		case VAR_PDJ:
		{
			set_table_var(pdj, a)
			set_table_var(pdj, b)
			set_table_var(pdj, c)
			set_table_var(pdj, d)
			break;
		}
		case VAR_FAN2:
		{
			set_table_var(fan2, x)
			set_table_var(fan2, y)
			break;
		}
		case VAR_RINGS2:
		{
			set_table_var(rings2, val)
			break;
		}
		case VAR_PERSPECTIVE:
		{
			set_table_var(perspective, angle)
			set_table_var(perspective, dist)
			break;
		}
		case VAR_JULIAN:
		{
			set_table_var(julian, power)
			set_table_var(julian, dist)
			break;
		}
		case VAR_JULIASCOPE:
		{
			set_table_var(juliascope, power)
			set_table_var(juliascope, dist)
			break;
		}
		case VAR_RADIAL_BLUR:
		{
			set_table_var(radial_blur, angle)
			break;
		}
		case VAR_PIE:
		{
			set_table_var(pie, rotation)
			set_table_var(pie, slices)
			set_table_var(pie, thickness)
			break;
		}
		case VAR_NGON:
		{
			set_table_var(ngon, circle)
			set_table_var(ngon, corners)
			set_table_var(ngon, sides)
			set_table_var(ngon, power)
			break;
		}
		case VAR_CURL:
		{
			set_table_var(curl, c1)
			set_table_var(curl, c2)
			break;
		}
		case VAR_RECTANGLES:
		{
			set_table_var(rectangles, x)
			set_table_var(rectangles, y)
			break;
		}
		case VAR_DISC2:
		{
			set_table_var(disc2, rot)
			set_table_var(disc2, twist)
			break;
		}
		case VAR_SUPER_SHAPE:
		{
			set_table_var(super_shape, rnd)
			set_table_var(super_shape, m)
			set_table_var(super_shape, n1)
			set_table_var(super_shape, n2)
			set_table_var(super_shape, n3)
			set_table_var(super_shape, holes)
			break;
		}
		case VAR_FLOWER:
		{
			set_table_var(flower, holes)
			set_table_var(flower, petals)
			break;
		}
		case VAR_CONIC:
		{
			set_table_var(conic, eccentricity)
			set_table_var(conic, holes)
			break;
		}
		case VAR_PARABOLA:
		{
			set_table_var(parabola, height)
			set_table_var(parabola, width)
			break;
		}
		case VAR_BENT2:
		{
			set_table_var(bent2, x)
			set_table_var(bent2, y)
			break;
		}
		case VAR_BIPOLAR:
		{
			set_table_var(bipolar, shift)
			break;
		}
		case VAR_CELL:
		{
			set_table_var(cell, size)
			break;
		}
		case VAR_CPOW:
		{
			set_table_var(cpow, r)
			set_table_var(cpow, i)
			set_table_var(cpow, power)
			break;
		}
		case VAR_CURVE:
		{
			set_table_var(curve, xamp)
			set_table_var(curve, yamp)
			set_table_var(curve, xlength)
			set_table_var(curve, ylength)
			break;
		}
		case VAR_ESCHER:
		{
			set_table_var(escher, beta)
			break;
		}
		case VAR_LAZYSUSAN:
		{
			set_table_var(lazysusan, spin)
			set_table_var(lazysusan, space)
			set_table_var(lazysusan, twist)
			set_table_var(lazysusan, x)
			set_table_var(lazysusan, y)
			break;
		}
		case VAR_MODULUS:
		{
			set_table_var(modulus, x)
			set_table_var(modulus, y)
			break;
		}
		case VAR_OSCILLOSCOPE:
		{
			set_table_var(oscope, separation)
			set_table_var(oscope, frequency)
			set_table_var(oscope, amplitude)
			set_table_var(oscope, damping)
			break;
		}
		case VAR_POPCORN2:
		{
			set_table_var(popcorn2, x)
			set_table_var(popcorn2, y)
			set_table_var(popcorn2, c)
			break;
		}
		case VAR_SEPARATION:
		{
			set_table_var(separation, x)
			set_table_var(separation, xinside)
			set_table_var(separation, y)
			set_table_var(separation, yinside)
			break;
		}
		case VAR_SPLIT:
		{
			set_table_var(split, xsize)
			set_table_var(split, ysize)
			break;
		}
		case VAR_SPLITS:
		{
			set_table_var(splits, x)
			set_table_var(splits, y)
			break;
		}
		case VAR_STRIPES:
		{
			set_table_var(stripes, space)
			set_table_var(stripes, warp)
			break;
		}
		case VAR_WEDGE:
		{
			set_table_var(wedge, angle)
			set_table_var(wedge, hole)
			set_table_var(wedge, count)
			set_table_var(wedge, swirl)
			break;
		}
		case VAR_WEDGE_JULIA:
		{
			set_table_var(wedge_julia, angle)
			set_table_var(wedge_julia, count)
			set_table_var(wedge_julia, power)
			set_table_var(wedge_julia, dist)
			break;
		}
		case VAR_WEDGE_SPH:
		{
			set_table_var(wedge_sph, angle)
			set_table_var(wedge_sph, count)
			set_table_var(wedge_sph, hole)
			set_table_var(wedge_sph, swirl)
			break;
		}
		case VAR_WHORL:
		{
			set_table_var(whorl, inside)
			set_table_var(whorl, outside)
			break;
		}
		case VAR_WAVES2:
		{
			set_table_var(waves2, freqx)
			set_table_var(waves2, scalex)
			set_table_var(waves2, freqy)
			set_table_var(waves2, scaley)
			break;
		}
		case VAR_AUGER:
		{
			set_table_var(auger, sym)
			set_table_var(auger, weight)
			set_table_var(auger, freq)
			set_table_var(auger, scale)
			break;
		}
		case VAR_FLUX:
		{
			set_table_var(flux, spread)
			break;
		}
		case VAR_MOBIUS:
		{
			set_table_var(mobius, re_a)
			set_table_var(mobius, im_a)
			set_table_var(mobius, re_b)
			set_table_var(mobius, im_b)
			set_table_var(mobius, re_c)
			set_table_var(mobius, im_c)
			set_table_var(mobius, re_d)
			set_table_var(mobius, im_d)
			break;
		}
		default:
			break;
	}
}

int XForm::coords(lua_State* L)
{
	get_xform_ptr(L);
	const char* labels[] = { "o", "x", "y" };
	if (lua_gettop(L) < 1)
	{
		lua_newtable(L);
		for (int i = 0 ; i < 3 ; i++)
		{
			lua_pushstring(L, labels[i]);
			lua_newtable(L);
			lua_pushstring(L, "x");
			lua_pushnumber(L, triangleCoords[i].x());
			lua_settable(L, -3);
			lua_pushstring(L, "y");
			lua_pushnumber(L, triangleCoords[i].y());
			lua_settable(L, -3);
			lua_settable(L, -3);
		}
	}
	else
	{
		luaL_checktype(L, 1, LUA_TTABLE);
		for (int i = 0 ; i < 3 ; i++)
		{
			lua_pushstring(L, labels[i]);
			lua_gettable(L, -2);
			lua_pushstring(L, "x");
			lua_gettable(L, -2);
			triangleCoords[i].rx() = luaL_checknumber(L, -1);
			lua_pop(L, 1);
			lua_pushstring(L, "y");
			lua_gettable(L, -2);
			triangleCoords[i].ry() = luaL_checknumber(L, -1);
			lua_pop(L, 2);
		}
		c2xf();
		setModified();
	}
	return 1;
}

int XForm::a(lua_State* L)
{
	get_xform_ptr(L);
	if (lua_gettop(L) == 2)
	{
		triangleCoords[0].rx() = luaL_checknumber(L, 1);
		triangleCoords[0].ry() = luaL_checknumber(L, 2);
		c2xf();
		setModified();
	}
	else
	{
		lua_settop(L, 0);
		lua_pushnumber(L, triangleCoords[0].x());
		lua_pushnumber(L, triangleCoords[0].y());
	}
	return 2;
}

int XForm::b(lua_State* L)
{
	get_xform_ptr(L);
	if (lua_gettop(L) == 2)
	{
		triangleCoords[1].rx() = luaL_checknumber(L, 1);
		triangleCoords[1].ry() = luaL_checknumber(L, 2);
		c2xf();
		setModified();
	}
	else
	{
		lua_settop(L, 0);
		lua_pushnumber(L, triangleCoords[1].x());
		lua_pushnumber(L, triangleCoords[1].y());
	}
	return 2;
}

int XForm::c(lua_State* L)
{
	get_xform_ptr(L);
	if (lua_gettop(L) == 2)
	{
		triangleCoords[2].rx() = luaL_checknumber(L, 1);
		triangleCoords[2].ry() = luaL_checknumber(L, 2);
		c2xf();
		setModified();
	}
	else
	{
		lua_settop(L, 0);
		lua_pushnumber(L, triangleCoords[2].x());
		lua_pushnumber(L, triangleCoords[2].y());
	}
	return 2;
}

int XForm::coefs(lua_State* L)
{
	get_xform_ptr(L);
	if (lua_gettop(L) < 1)
	{
		lua_newtable(L);
		lua_pushstring(L, "a");
		lua_pushnumber(L, xform_ptr->c[0][0]);
		lua_settable(L, -3);
		lua_pushstring(L, "b");
		lua_pushnumber(L, xform_ptr->c[1][0]);
		lua_settable(L, -3);
		lua_pushstring(L, "c");
		lua_pushnumber(L, xform_ptr->c[2][0]);
		lua_settable(L, -3);
		lua_pushstring(L, "d");
		lua_pushnumber(L, xform_ptr->c[0][1]);
		lua_settable(L, -3);
		lua_pushstring(L, "e");
		lua_pushnumber(L, xform_ptr->c[1][1]);
		lua_settable(L, -3);
		lua_pushstring(L, "f");
		lua_pushnumber(L, xform_ptr->c[2][1]);
		lua_settable(L, -3);
	}
	else
	{
		luaL_checktype(L, 1, LUA_TTABLE);
		lua_pushstring(L, "a");
		lua_gettable(L, -2);
		xform_ptr->c[0][0] = luaL_checknumber(L, -1);
		lua_pop(L, 1);
		lua_pushstring(L, "b");
		lua_gettable(L, -2);
		xform_ptr->c[1][0] = luaL_checknumber(L, -1);
		lua_pop(L, 1);
		lua_pushstring(L, "c");
		lua_gettable(L, -2);
		xform_ptr->c[2][0] = luaL_checknumber(L, -1);
		lua_pop(L, 1);
		lua_pushstring(L, "d");
		lua_gettable(L, -2);
		xform_ptr->c[0][1] = luaL_checknumber(L, -1);
		lua_pop(L, 1);
		lua_pushstring(L, "e");
		lua_gettable(L, -2);
		xform_ptr->c[1][1] = luaL_checknumber(L, -1);
		lua_pop(L, 1);
		lua_pushstring(L, "f");
		lua_gettable(L, -2);
		xform_ptr->c[2][1] = luaL_checknumber(L, -1);
		lua_pop(L, 1);
		xf2c();
		setModified();
	}
	return 1;
}

int XForm::xa(lua_State* L)
{
	get_xform_ptr(L);
	if (lua_gettop(L) == 1)
	{
		xform_ptr->c[0][0] = luaL_checknumber(L, 1);
		xf2c();
		setModified();
	}
	lua_settop(L, 0);
	lua_pushnumber(L, xform_ptr->c[0][0]);
	return 1;
}

int XForm::xd(lua_State* L)
{
	get_xform_ptr(L);
	if (lua_gettop(L) == 1)
	{
		xform_ptr->c[0][1] = luaL_checknumber(L, 1);
		xf2c();
		setModified();
	}
	lua_settop(L, 0);
	lua_pushnumber(L, xform_ptr->c[0][1]);
	return 1;
}

int XForm::xb(lua_State* L)
{
	get_xform_ptr(L);
	if (lua_gettop(L) == 1)
	{
		xform_ptr->c[1][0] = luaL_checknumber(L, 1);
		xf2c();
		setModified();
	}
	lua_settop(L, 0);
	lua_pushnumber(L, xform_ptr->c[1][0]);
	return 1;
}

int XForm::xe(lua_State* L)
{
	get_xform_ptr(L);
	if (lua_gettop(L) == 1)
	{
		xform_ptr->c[1][1] = luaL_checknumber(L, 1);
		xf2c();
		setModified();
	}
	lua_settop(L, 0);
	lua_pushnumber(L, xform_ptr->c[1][1]);
	return 1;
}

int XForm::xc(lua_State* L)
{
	get_xform_ptr(L);
	if (lua_gettop(L) == 1)
	{
		xform_ptr->c[2][0] = luaL_checknumber(L, 1);
		xf2c();
		setModified();
	}
	lua_settop(L, 0);
	lua_pushnumber(L, xform_ptr->c[2][0]);
	return 1;
}

int XForm::xf(lua_State* L)
{
	get_xform_ptr(L);
	if (lua_gettop(L) == 1)
	{
		xform_ptr->c[2][1] = luaL_checknumber(L, 1);
		xf2c();
		setModified();
	}
	lua_settop(L, 0);
	lua_pushnumber(L, xform_ptr->c[2][1]);
	return 1;
}


// post xform accessors
int XForm::coordsp(lua_State* L)
{
	get_xform_ptr(L);
	const char* labels[] = { "o", "x", "y" };
	if (lua_gettop(L) < 1)
	{
		lua_newtable(L);
		for (int i = 0 ; i < 3 ; i++)
		{
			lua_pushstring(L, labels[i]);
			lua_newtable(L);
			lua_pushstring(L, "x");
			lua_pushnumber(L, triangleCoordsP[i].x());
			lua_settable(L, -3);
			lua_pushstring(L, "y");
			lua_pushnumber(L, triangleCoordsP[i].y());
			lua_settable(L, -3);
			lua_settable(L, -3);
		}
	}
	else
	{
		luaL_checktype(L, 1, LUA_TTABLE);
		for (int i = 0 ; i < 3 ; i++)
		{
			lua_pushstring(L, labels[i]);
			lua_gettable(L, -2);
			lua_pushstring(L, "x");
			lua_gettable(L, -2);
			triangleCoordsP[i].rx() = luaL_checknumber(L, -1);
			lua_pop(L, 1);
			lua_pushstring(L, "y");
			lua_gettable(L, -2);
			triangleCoordsP[i].ry() = luaL_checknumber(L, -1);
			lua_pop(L, 2);
		}
		c2xfp();
		setModified();
	}
	return 1;
}


int XForm::ap(lua_State* L)
{
	get_xform_ptr(L);
	if (lua_gettop(L) == 2)
	{
		triangleCoordsP[0].rx() = luaL_checknumber(L, 1);
		triangleCoordsP[0].ry() = luaL_checknumber(L, 2);
		c2xfp();
		setModified();
	}
	else
	{
		lua_settop(L, 0);
		lua_pushnumber(L, triangleCoordsP[0].x());
		lua_pushnumber(L, triangleCoordsP[0].y());
	}
	return 2;
}

int XForm::bp(lua_State* L)
{
	get_xform_ptr(L);
	if (lua_gettop(L) == 2)
	{
		triangleCoordsP[1].rx() = luaL_checknumber(L, 1);
		triangleCoordsP[1].ry() = luaL_checknumber(L, 2);
		c2xfp();
		setModified();
	}
	else
	{
		lua_settop(L, 0);
		lua_pushnumber(L, triangleCoordsP[1].x());
		lua_pushnumber(L, triangleCoordsP[1].y());
	}
	return 2;
}

int XForm::cp(lua_State* L)
{
	get_xform_ptr(L);
	if (lua_gettop(L) == 2)
	{
		triangleCoordsP[2].rx() = luaL_checknumber(L, 1);
		triangleCoordsP[2].ry() = luaL_checknumber(L, 2);
		c2xfp();
		setModified();
	}
	else
	{
		lua_settop(L, 0);
		lua_pushnumber(L, triangleCoordsP[2].x());
		lua_pushnumber(L, triangleCoordsP[2].y());
	}
	return 2;
}

int XForm::coefsp(lua_State* L)
{
	get_xform_ptr(L);
	if (lua_gettop(L) < 1)
	{
		lua_newtable(L);
		lua_pushstring(L, "a");
		lua_pushnumber(L, xform_ptr->post[0][0]);
		lua_settable(L, -3);
		lua_pushstring(L, "b");
		lua_pushnumber(L, xform_ptr->post[1][0]);
		lua_settable(L, -3);
		lua_pushstring(L, "c");
		lua_pushnumber(L, xform_ptr->post[2][0]);
		lua_settable(L, -3);
		lua_pushstring(L, "d");
		lua_pushnumber(L, xform_ptr->post[0][1]);
		lua_settable(L, -3);
		lua_pushstring(L, "e");
		lua_pushnumber(L, xform_ptr->post[1][1]);
		lua_settable(L, -3);
		lua_pushstring(L, "f");
		lua_pushnumber(L, xform_ptr->post[2][1]);
		lua_settable(L, -3);
	}
	else
	{
		luaL_checktype(L, 1, LUA_TTABLE);
		lua_pushstring(L, "a");
		lua_gettable(L, -2);
		xform_ptr->post[0][0] = luaL_checknumber(L, -1);
		lua_pop(L, 1);
		lua_pushstring(L, "b");
		lua_gettable(L, -2);
		xform_ptr->post[1][0] = luaL_checknumber(L, -1);
		lua_pop(L, 1);
		lua_pushstring(L, "c");
		lua_gettable(L, -2);
		xform_ptr->post[2][0] = luaL_checknumber(L, -1);
		lua_pop(L, 1);
		lua_pushstring(L, "d");
		lua_gettable(L, -2);
		xform_ptr->post[0][1] = luaL_checknumber(L, -1);
		lua_pop(L, 1);
		lua_pushstring(L, "e");
		lua_gettable(L, -2);
		xform_ptr->post[1][1] = luaL_checknumber(L, -1);
		lua_pop(L, 1);
		lua_pushstring(L, "f");
		lua_gettable(L, -2);
		xform_ptr->post[2][1] = luaL_checknumber(L, -1);
		lua_pop(L, 1);
		xfp2c();
		setModified();
	}
	return 1;
}

int XForm::xap(lua_State* L)
{
	get_xform_ptr(L);
	if (lua_gettop(L) == 1)
	{
		xform_ptr->post[0][0] = luaL_checknumber(L, 1);
		xfp2c();
		setModified();
	}
	lua_settop(L, 0);
	lua_pushnumber(L, xform_ptr->post[0][0]);
	return 1;
}

int XForm::xdp(lua_State* L)
{
	get_xform_ptr(L);
	if (lua_gettop(L) == 1)
	{
		xform_ptr->post[0][1] = luaL_checknumber(L, 1);
		xfp2c();
		setModified();
	}
	lua_settop(L, 0);
	lua_pushnumber(L, xform_ptr->post[0][1]);
	return 1;
}

int XForm::xbp(lua_State* L)
{
	get_xform_ptr(L);
	if (lua_gettop(L) == 1)
	{
		xform_ptr->post[1][0] = luaL_checknumber(L, 1);
		xfp2c();
		setModified();
	}
	lua_settop(L, 0);
	lua_pushnumber(L, xform_ptr->post[1][0]);
	return 1;
}

int XForm::xep(lua_State* L)
{
	get_xform_ptr(L);
	if (lua_gettop(L) == 1)
	{
		xform_ptr->post[1][1] = luaL_checknumber(L, 1);
		xfp2c();
		setModified();
	}
	lua_settop(L, 0);
	lua_pushnumber(L, xform_ptr->post[1][1]);
	return 1;
}

int XForm::xcp(lua_State* L)
{
	get_xform_ptr(L);
	if (lua_gettop(L) == 1)
	{
		xform_ptr->post[2][0] = luaL_checknumber(L, 1);
		xfp2c();
		setModified();
	}
	lua_settop(L, 0);
	lua_pushnumber(L, xform_ptr->post[2][0]);
	return 1;
}

int XForm::xfp(lua_State* L)
{
	get_xform_ptr(L);
	if (lua_gettop(L) == 1)
	{
		xform_ptr->post[2][1] = luaL_checknumber(L, 1);
		xfp2c();
		setModified();
	}
	lua_settop(L, 0);
	lua_pushnumber(L, xform_ptr->post[2][1]);
	return 1;
}



// c++ interface
void XForm::xf2c()
{
	triangleCoords = basisTriangle->getCoords(xform_ptr->c);
}

void XForm::c2xf()
{
	basisTriangle->applyTransform(triangleCoords, xform_ptr->c);
}

void XForm::xfp2c()
{
	triangleCoordsP = basisTriangle->getCoords(xform_ptr->post);
}

void XForm::c2xfp()
{
	basisTriangle->applyTransform(triangleCoordsP, xform_ptr->post);
}

void XForm::setContext(lua_State* L, Genome* g, int x_idx)
{
	m_genome = g;
	m_gidx   = m_genome->index();
	m_xfidx  = x_idx;
	get_xform_ptr(L);
	xf2c(); // load Ax,Ay, Bx,By, Cx,Cy
	xfp2c();
}

void XForm::setModified()
{
	if (m_gidx > -1) // XForm associated with some Genome
		m_adapter->setModified(m_gidx);
}

flam3_xform* XForm::data()
{
	return xform_ptr;
}

int XForm::translate(lua_State* L)
{
	get_xform_ptr(L);
	double dx = luaL_checknumber(L, 1);
	double dy = luaL_checknumber(L, 2);
	triangleCoords.translate(dx, dy);
	c2xf();
	setModified();
	return 0;
}

int XForm::pos(lua_State* L)
{
	get_xform_ptr(L);
	if (lua_gettop(L) == 2)
	{
		double x = luaL_checknumber(L, 1);
		double y = luaL_checknumber(L, 2);
		QPointF f = QPointF(x,y) - triangleCoords.boundingRect().center();
		triangleCoords.translate(f.x(), f.y());
		c2xf();
		setModified();
	}
	QPointF c = triangleCoords.boundingRect().center();
	lua_settop(L,0);
	lua_pushnumber(L, c.x());
	lua_pushnumber(L, c.y());
	return 2;
}

int XForm::rotate(lua_State* L)
{
	get_xform_ptr(L);
	double deg = luaL_checknumber(L, 1);
	QPointF c(triangleCoords.boundingRect().center());
	if (lua_gettop(L) > 1)
	{
		double x = luaL_checknumber(L, 2);
		double y = luaL_checknumber(L, 3);
		c = QPointF(x, y);
		setModified();
	}
	QTransform trans;
	trans.translate(c.x(), c.y()).rotate(deg).translate(-c.x(), -c.y());
	triangleCoords = trans.map(QPolygonF(triangleCoords));
	c2xf();
	return 0;
}

int XForm::scale(lua_State* L)
{
	get_xform_ptr(L);
	double dx = luaL_checknumber(L, 1);
	double dy = dx;
	if (lua_gettop(L) > 1)
		dy = luaL_checknumber(L, 2);
	QPointF c = triangleCoords.boundingRect().center();
	if (lua_gettop(L) > 2)
	{
		double x = luaL_checknumber(L, 3);
		double y = luaL_checknumber(L, 4);
		c = QPointF(x, y);
		setModified();
	}
	QTransform trans;
	trans.translate(c.x(), c.y()).scale(dx, dy).translate(-c.x(), -c.y());
	triangleCoords = trans.map(QPolygonF(triangleCoords));
	c2xf();
	return 0;
}

int XForm::shear(lua_State* L)
{
	get_xform_ptr(L);
	double dx = luaL_checknumber(L, 1);
	double dy = luaL_checknumber(L, 2);
	QPointF c = triangleCoords.boundingRect().center();
	if (lua_gettop(L) > 2)
	{
		double x = luaL_checknumber(L, 3);
		double y = luaL_checknumber(L, 4);
		c = QPointF(x, y);
		setModified();
	}
	QTransform trans;
	trans.translate(c.x(), c.y()).shear(dx, dy).translate(-c.x(), -c.y());
	triangleCoords = trans.map(QPolygonF(triangleCoords));
	c2xf();
	return 0;
}

// post transform 'transforms'
int XForm::translatep(lua_State* L)
{
	get_xform_ptr(L);
	double dx = luaL_checknumber(L, 1);
	double dy = luaL_checknumber(L, 2);
	triangleCoordsP.translate(dx, dy);
	c2xfp();
	setModified();
	return 0;
}

int XForm::posp(lua_State* L)
{
	get_xform_ptr(L);
	if (lua_gettop(L) == 2)
	{
		double x = luaL_checknumber(L, 1);
		double y = luaL_checknumber(L, 2);
		QPointF f = QPointF(x,y) - triangleCoordsP.boundingRect().center();
		triangleCoordsP.translate(f.x(), f.y());
		c2xfp();
		setModified();
	}
	QPointF c = triangleCoordsP.boundingRect().center();
	lua_settop(L,0);
	lua_pushnumber(L, c.x());
	lua_pushnumber(L, c.y());
	return 2;
}

int XForm::rotatep(lua_State* L)
{
	get_xform_ptr(L);
	double deg = luaL_checknumber(L, 1);
	QPointF c(triangleCoordsP.boundingRect().center());
	if (lua_gettop(L) > 1)
	{
		double x = luaL_checknumber(L, 2);
		double y = luaL_checknumber(L, 3);
		c = QPointF(x, y);
		setModified();
	}
	QTransform trans;
	trans.translate(c.x(), c.y()).rotate(deg).translate(-c.x(), -c.y());
	triangleCoordsP = trans.map(QPolygonF(triangleCoordsP));
	c2xfp();
	return 0;
}

int XForm::scalep(lua_State* L)
{
	get_xform_ptr(L);
	double dx = luaL_checknumber(L, 1);
	double dy = dx;
	if (lua_gettop(L) > 1)
		dy = luaL_checknumber(L, 2);
	QPointF c = triangleCoordsP.boundingRect().center();
	if (lua_gettop(L) > 2)
	{
		double x = luaL_checknumber(L, 3);
		double y = luaL_checknumber(L, 4);
		c = QPointF(x, y);
		setModified();
	}
	QTransform trans;
	trans.translate(c.x(), c.y()).scale(dx, dy).translate(-c.x(), -c.y());
	triangleCoordsP = trans.map(QPolygonF(triangleCoordsP));
	c2xfp();
	return 0;
}

int XForm::shearp(lua_State* L)
{
	get_xform_ptr(L);
	double dx = luaL_checknumber(L, 1);
	double dy = luaL_checknumber(L, 2);
	QPointF c = triangleCoordsP.boundingRect().center();
	if (lua_gettop(L) > 2)
	{
		double x = luaL_checknumber(L, 3);
		double y = luaL_checknumber(L, 4);
		c = QPointF(x, y);
		setModified();
	}
	QTransform trans;
	trans.translate(c.x(), c.y()).shear(dx, dy).translate(-c.x(), -c.y());
	triangleCoordsP = trans.map(QPolygonF(triangleCoordsP));
	c2xfp();
	return 0;
}

}

