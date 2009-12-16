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
#include "genome.h"
#include "xform.h"

namespace Lua
{
const char Genome::className[] = "Genome";
Lunar<Genome>::RegType Genome::methods[] =
{
	{ "width",  &Genome::width },
	{ "height", &Genome::height },
	{ "index",  &Genome::index },
	{ "center", &Genome::center },
	{ "rot_center", &Genome::rot_center },
	{ "rotate", &Genome::rotate },
	{ "vibrancy", &Genome::vibrancy },
	{ "hue_rotation", &Genome::hue_rotation },
	{ "background", &Genome::background },
	{ "zoom", &Genome::zoom },
	{ "pixels_per_unit", &Genome::pixels_per_unit },
	{ "scale", &Genome::pixels_per_unit },
	{ "spatial_filter_radius", &Genome::spatial_filter_radius },
	{ "spatial_filter_select", &Genome::spatial_filter_select },
	{ "sample_density", &Genome::sample_density },
	{ "brightness", &Genome::brightness },
	{ "contrast", &Genome::contrast },
	{ "gamma", &Genome::gamma },
	{ "time", &Genome::time },
	{ "interpolation", &Genome::interpolation },
	{ "interpolation", &Genome::interpolation_type },
	{ "palette_interpolation", &Genome::palette_interpolation },
	{ "num_xforms", &Genome::num_xforms },
	{ "final_xform_index", &Genome::final_xform_index },
	{ "final_xform_enable", &Genome::final_xform_enable },
	{ "genome_index", &Genome::genome_index },
	{ "symmetry", &Genome::symmetry },
	{ "palette_index", &Genome::palette_index },
	{ "spatial_oversample", &Genome::spatial_oversample },
	{ "nbatches", &Genome::nbatches },
	{ "ntemporal_samples", &Genome::ntemporal_samples },
	{ "estimator", &Genome::estimator },
	{ "estimator_curve", &Genome::estimator_curve },
	{ "estimator_minimum", &Genome::estimator_minimum },
	{ "highlight_power", &Genome::highlight_power },
	{ "gam_lin_thresh", &Genome::gam_lin_thresh },
	{ "get_xform", &Genome::get_xform },
	{ "add_xform", &Genome::add_xform },
	{ "del_xform", &Genome::del_xform },
	{ "copy_xform", &Genome::copy_xform },
	{ "clear_xforms", &Genome::clear_xforms },
	{ "load_palette", &Genome::load_palette },
	{ "palette", &Genome::palette },
	{ "chaos", &Genome::chaos },
	{ 0, 0 }
};

Genome::Genome(lua_State* /*L*/)
{
}


#define intAcc(name) int Genome::name(lua_State *L)\
{\
	if (lua_gettop(L) == 1)\
		genome_ptr->name = luaL_checkint(L, 1);\
	else\
	{\
		lua_settop(L, 0);\
		lua_pushinteger(L, genome_ptr->name);\
	}\
	return 1;\
}\

#define intAccRO(name) int Genome::name(lua_State *L)\
{\
	lua_settop(L, 0);\
	lua_pushinteger(L, genome_ptr->name);\
	return 1;\
}\

#define realAcc(name) int Genome::name(lua_State *L)\
{\
	if (lua_gettop(L) == 1)\
		genome_ptr->name = luaL_checknumber(L, 1);\
	else\
	{\
		lua_settop(L, 0);\
		lua_pushnumber(L, genome_ptr->name);\
	}\
	return 1;\
}\

#define realAccRO(name) int Genome::name(lua_State *L)\
{\
	lua_settop(L, 0);\
	lua_pushnumber(L, genome_ptr->name);\
	return 1;\
}\

intAccRO(num_xforms);

intAcc(width);
intAcc(height);
realAcc(rotate);
realAcc(vibrancy);
realAcc(hue_rotation);
realAcc(zoom);
realAcc(pixels_per_unit);
realAcc(spatial_filter_radius);
intAcc(spatial_filter_select);
realAcc(sample_density);
realAcc(brightness);
realAcc(contrast);
realAcc(gamma);
realAcc(time);
realAcc(estimator);
realAcc(estimator_curve);
realAcc(estimator_minimum);
realAcc(gam_lin_thresh);
intAcc(interpolation);
intAcc(interpolation_type);
intAcc(palette_interpolation);
intAcc(symmetry);
intAcc(palette_index);
intAcc(spatial_oversample);
intAcc(nbatches);
intAcc(ntemporal_samples);


int Genome::index(lua_State *L)
{
	lua_settop(L, 0);
	lua_pushinteger(L, idx + 1);
	return 1;
}

// this 'should be' the same as index(lua_State)
int Genome::genome_index(lua_State *L)
{
	lua_settop(L, 0);
	lua_pushinteger(L, genome_ptr->genome_index + 1);
	return 1;
}

int Genome::final_xform_index(lua_State *L)
{
	if (lua_gettop(L) == 1)
	{
		int xfn = qMax(-1, luaL_checkint(L, 1) - 1);
		if (xfn < 0)
		{
			genome_ptr->final_xform_index = -1;
			genome_ptr->final_xform_enable = 0;
		}
		else
			if (xfn < genome_ptr->num_xforms)
			{
				genome_ptr->final_xform_index = xfn;
				genome_ptr->final_xform_enable = 1;
			}
			else
				luaL_error(L, "genome has no xform at index %d", xfn + 1);
	}
	else
	{
		lua_settop(L, 0);
		lua_pushinteger(L, genome_ptr->final_xform_index + 1);
	}
	return 1;
}

int Genome::final_xform_enable(lua_State *L)
{
	if (lua_gettop(L) == 1)
	{
		luaL_checktype(L, 1, LUA_TBOOLEAN);
		bool flag = lua_toboolean(L, 1) > 0;
		if (flag)
		{
			if (genome_ptr->final_xform_index < 0)
				genome_ptr->final_xform_index = genome_ptr->num_xforms - 1;
			genome_ptr->final_xform_enable = 1;
		}
		else
		{
			genome_ptr->final_xform_index = -1;
			genome_ptr->final_xform_enable = 0;
		}
	}
	else
	{
		lua_settop(L, 0);
		lua_pushboolean(L, genome_ptr->final_xform_enable);
	}
	return 1;
}

int Genome::center(lua_State *L)
{
	double* x = &genome_ptr->center[0];
	double* y = &genome_ptr->center[1];
	if (lua_gettop(L) == 2)
	{
		*x = luaL_checknumber(L, 1);
		*y = luaL_checknumber(L, 2);
	}
	else
	{
		lua_settop(L, 0);
		lua_pushnumber(L, *x);
		lua_pushnumber(L, *y);
	}
	return 2;
}

int Genome::rot_center(lua_State *L)
{
	double* x = &genome_ptr->rot_center[0];
	double* y = &genome_ptr->rot_center[1];
	if (lua_gettop(L) == 2)
	{
		*x = luaL_checknumber(L, 1);
		*y = luaL_checknumber(L, 2);
	}
	else
	{
		lua_settop(L, 0);
		lua_pushnumber(L, *x);
		lua_pushnumber(L, *y);
	}
	return 2;
}

int Genome::background(lua_State *L)
{
	double* r = &genome_ptr->background[0];
	double* g = &genome_ptr->background[1];
	double* b = &genome_ptr->background[2];
	if (lua_gettop(L) == 3)
	{
		*r = luaL_checknumber(L, 1);
		*g = luaL_checknumber(L, 2);
		*b = luaL_checknumber(L, 3);
	}
	else
	{
		lua_settop(L, 0);
		lua_pushnumber(L, *r);
		lua_pushnumber(L, *g);
		lua_pushnumber(L, *b);
	}
	return 3;
}

int Genome::get_xform(lua_State *L)
{
	int xfn = 0;
	if (lua_gettop(L) == 1)
	{
		xfn = qMax(0, luaL_checkint(L, 1) - 1);
		lua_pop(L, 1);
	}
	if (xfn < genome_ptr->num_xforms)
	{
		luaL_getmetatable(L, XForm::className);
		Lunar<XForm>::new_T(L);
		XForm* xf = Lunar<XForm>::check(L, 1);
		xf->setContext(m_adapter, idx, xfn);
	}
	else
	{
		luaL_error(L, "genome has no xform at index %d", xfn + 1);
		return 0;
	}
	return 1;
}

int Genome::add_xform(lua_State *L)
{
	int n = genome_ptr->num_xforms;
	int num = 1;
	if (lua_gettop(L) > 0)
		num = luaL_checkint(L, 1);

	for (int i = 0 ; i < num ; i++)
		Util::add_default_xform(genome_ptr);

	lua_settop(L, 0);
	luaL_getmetatable(L, XForm::className);
	Lunar<XForm>::new_T(L);
	XForm*  xf = Lunar<XForm>::check(L, 1);
	if (genome_ptr->final_xform_enable == 1)
		xf->setContext(m_adapter, idx, n - 1);
	else
		xf->setContext(m_adapter, idx, n);
	return 1;
}

int Genome::copy_xform(lua_State *L)
{
	if (lua_isuserdata(L, 1))
	{
		XForm* xf = Lunar<XForm>::check(L, 1);
		int nxforms = genome_ptr->num_xforms;
		int dest_idx = nxforms - 1;
		if (lua_isnumber(L, 2))
			dest_idx = qMax(0, luaL_checkint(L, 2) - 1);
		if (dest_idx >= nxforms)
			luaL_error(L, "genome has no xform at index %d", dest_idx + 1);
		lua_settop(L, 0);
		flam3_copy_xform(genome_ptr->xform + dest_idx, xf->data());
		luaL_getmetatable(L, XForm::className);
		Lunar<XForm>::new_T(L);
		xf = Lunar<XForm>::check(L, 1);
		xf->setContext(m_adapter, idx, dest_idx);
	}
	else
	{
		int sidx = qMax(0, luaL_checkint(L, 1) - 1);
		int nxforms = genome_ptr->num_xforms;
		if (sidx >= nxforms)
			luaL_error(L, "genome has no xform at index %d", sidx + 1);
		else
		{
			flam3_xform* from = genome_ptr->xform + sidx;
			flam3_add_xforms(genome_ptr, 1, 0, 0);
			flam3_xform* to = genome_ptr->xform + nxforms;
			flam3_copy_xform(to, from);
			lua_settop(L, 0);
			luaL_getmetatable(L, XForm::className);
			Lunar<XForm>::new_T(L);
			XForm*  xf = Lunar<XForm>::check(L, 1);
			xf->setContext(m_adapter, idx, nxforms);
		}
	}
	return 1;
}

int Genome::del_xform(lua_State *L)
{
	int idx = qMax(0, luaL_checkint(L, 1) - 1);
	if (idx >= genome_ptr->num_xforms)
		luaL_error(L, "genome has no xform at index %d", idx + 1);
	else
		flam3_delete_xform(genome_ptr, idx);
	return 0;
}

int Genome::clear_xforms(lua_State* /*L*/)
{
	for (int n = genome_ptr->num_xforms - 1 ; n >= 0 ; n--)
		flam3_delete_xform(genome_ptr, n);
	return 0;
}

int Genome::load_palette(lua_State* L)
{
	int idx = qMax(0, luaL_checkint(L, 1) - 1);
	double hue_rotation = 0;
	if (lua_gettop(L) > 1)
		hue_rotation = luaL_checknumber(L, 2);
	flam3_get_palette(idx, genome_ptr->palette, hue_rotation);
	return 0;
}

int Genome::palette(lua_State* L)
{
	int idx = qMax(0, luaL_checkint(L, 1) - 1);
	if (idx > 255)
		luaL_error(L, "index %d out of range", idx + 1);
	double r, g, b;
	if (lua_gettop(L) > 1)
	{
		r = luaL_checknumber(L, 2);
		g = luaL_checknumber(L, 3);
		b = luaL_checknumber(L, 4);
		genome_ptr->palette[idx].color[0] = r;
		genome_ptr->palette[idx].color[1] = g;
		genome_ptr->palette[idx].color[2] = b;
	}
	else
	{
		r = genome_ptr->palette[idx].color[0];
		g = genome_ptr->palette[idx].color[1];
		b = genome_ptr->palette[idx].color[2];
	}
	lua_settop(L, 0);
	lua_pushnumber(L, r);
	lua_pushnumber(L, g);
	lua_pushnumber(L, b);
	return 3;
}

int Genome::chaos(lua_State* L)
{
	if (lua_gettop(L) < 1)
	{
		// return the entire chaos array
		lua_newtable(L);
		for (int i = 0 ; i < genome_ptr->num_xforms ; i++)
		{
			lua_pushinteger(L, i + 1);
			lua_newtable(L);
			for (int j = 0 ; j < genome_ptr->num_xforms ; j++)
			{
				lua_pushinteger(L, j + 1);
				lua_pushnumber(L, genome_ptr->chaos[i][j]);
				lua_settable(L, -3);
			}
			lua_settable(L, -3);
		}
	}
	else
	{
		if (lua_type(L, 1) == LUA_TTABLE)
		{
			// setting the whole table
			for (int i = 0 ; i < genome_ptr->num_xforms ; i++)
			{
				lua_pushinteger(L, i + 1);
				lua_gettable(L, -2);
				luaL_checktype(L, -1, LUA_TTABLE);
				for (int j = 0 ; j < genome_ptr->num_xforms ; j++)
				{
					lua_pushinteger(L, j + 1);
					lua_gettable(L, -2);
					double val = luaL_checknumber(L, -1);
					lua_pop(L, 1);
					genome_ptr->chaos[i][j] = val;
				}
				lua_pop(L, 1);
			}
		}
		else
		{
			int idx = qMax(0, luaL_checkint(L, 1) - 1);
			if (idx >= genome_ptr->num_xforms)
				luaL_error(L, "no index %d in chaos array", idx + 1);
			if (lua_type(L, 2) == LUA_TTABLE)
			{
				// setting chaos array for one xform index
				for (int j = 0 ; j < genome_ptr->num_xforms ; j++)
				{
					lua_pushinteger(L, j + 1);
					lua_gettable(L, -2);
					double val = luaL_checknumber(L, -1);
					lua_pop(L, 1);
					genome_ptr->chaos[idx][j] = val;
				}
			}
			else
			{
				if (lua_gettop(L) > 2)
				{
					// setting chaos array for one table entry
					int rdx = qMax(0, luaL_checkint(L, 2) - 1);
					if (rdx >= genome_ptr->num_xforms)
						luaL_error(L, "no index %d,%d in chaos array", idx + 1, rdx + 1);
					double val = luaL_checknumber(L, 3);
					genome_ptr->chaos[idx][rdx] = val;
				}
				else
				{
					if (lua_gettop(L) == 2)
					{
						// return chaos array for one chaos entry
						int rdx = qMax(0, luaL_checkint(L, 2) - 1);
						if (rdx >= genome_ptr->num_xforms)
							luaL_error(L, "no index %d,%d in chaos array", idx + 1, rdx + 1);
						lua_pushnumber(L, genome_ptr->chaos[idx][rdx]);
					}
					else
					{
						// return chaos array for one xform index
						lua_newtable(L);
						for (int j = 0 ; j < genome_ptr->num_xforms ; j++)
						{
							lua_pushinteger(L, j + 1);
							lua_pushnumber(L, genome_ptr->chaos[idx][j]);
							lua_settable(L, -3);
						}
					}
				}
			}
		}
	}
	return 1;
}

int Genome::highlight_power(lua_State* L)
{
	if (lua_gettop(L) > 0)
	{
		double val = luaL_checknumber(L, 1);
		if (val < 0.0) val = -1.0;
		genome_ptr->highlight_power = val;
	}
	else
		lua_pushnumber(L, genome_ptr->highlight_power);
	return 1;
}

// c++ interface
void Genome::setContext(LuaThreadAdapter* ctx, int index)
{
	LuaType::setContext(ctx);
	idx = index;
	genome_ptr = m_adapter->genomeVector()->data() + idx;
	basis      = m_adapter->basisTriangle();
}

int Genome::index() const
{
	return idx;
}

Genome::~Genome()
{
}

}

