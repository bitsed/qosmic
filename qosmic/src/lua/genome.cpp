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
#include "genome.h"
#include "xform.h"
#include "luathreadadapter.h"

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
	{ "interpolation_type", &Genome::interpolation_type },
	{ "palette_interpolation", &Genome::palette_interpolation },
	{ "num_xforms", &Genome::num_xforms },
	{ "final_xform_index", &Genome::final_xform_index },
	{ "final_xform_enable", &Genome::final_xform_enable },
	{ "get_final_xform", &Genome::get_final_xform },
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
	{ "xform", &Genome::xform },
	{ "xforms", &Genome::xforms },
	{ "clear_xforms", &Genome::clear_xforms },
	{ "load_palette", &Genome::load_palette },
	{ "palette", &Genome::palette },
	{ "chaos", &Genome::chaos },
	{ 0, 0 }
};

Genome::Genome(lua_State* L)
: m_idx(-1), m_genome()
{
	/* retrieve a context */
	lua_pushlightuserdata(L, (void*)&LuaThreadAdapter::RegKey);  /* push address */
	lua_gettable(L, LUA_REGISTRYINDEX);  /* retrieve value */
	LuaType::setContext(static_cast<LuaThreadAdapter*>(lua_touserdata(L, -1)));
	lua_pop(L, 1);

	if (lua_gettop(L) > 0 && lua_isuserdata(L, 1))
	{
		// check for a Genome type to copy
		Genome* g = Lunar<Genome>::check(L, 1);
		flam3_copy(&m_genome, g->get_genome_ptr(L));
		lua_pop(L, 1);
	}
	else
		Util::init_genome(&m_genome);

	get_genome_ptr(L);
}

flam3_genome* Genome::get_genome_ptr(lua_State* L)
{
	if (m_idx >= 0)
	{
		GenomeVector* vec = m_adapter->genomeVector();
		if (m_idx < vec->size())
			genome_ptr = vec->data() + m_idx;
		else
			luaL_error(L, "index out of bounds: Genome[%d]", m_idx + 1);
	}
	else
		genome_ptr = &m_genome;

	return genome_ptr;
}

#define intAcc(name) int Genome::name(lua_State* L)\
{\
	get_genome_ptr(L); \
	if (lua_gettop(L) == 1)\
		genome_ptr->name = luaL_checkint(L, 1);\
	else\
	{\
		lua_settop(L, 0);\
		lua_pushinteger(L, genome_ptr->name);\
	}\
	return 1;\
}\

#define intAccRO(name) int Genome::name(lua_State* L)\
{\
	get_genome_ptr(L); \
	lua_settop(L, 0);\
	lua_pushinteger(L, genome_ptr->name);\
	return 1;\
}\

#define realAcc(name) int Genome::name(lua_State* L)\
{\
	get_genome_ptr(L); \
	if (lua_gettop(L) == 1)\
		genome_ptr->name = luaL_checknumber(L, 1);\
	else\
	{\
		lua_settop(L, 0);\
		lua_pushnumber(L, genome_ptr->name);\
	}\
	return 1;\
}\

#define realAccRO(name) int Genome::name(lua_State* L)\
{\
	get_genome_ptr(L); \
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


int Genome::index(lua_State* L)
{
	get_genome_ptr(L);
	lua_settop(L, 0);
	lua_pushinteger(L, m_idx + 1);
	return 1;
}

// this 'should be' the same as index(lua_State)
int Genome::genome_index(lua_State* L)
{
	get_genome_ptr(L);
	lua_settop(L, 0);
	lua_pushinteger(L, genome_ptr->genome_index + 1);
	return 1;
}

int Genome::final_xform_index(lua_State* L)
{
	get_genome_ptr(L);
	lua_pushinteger(L, genome_ptr->final_xform_index + 1);
	return 1;
}

int Genome::final_xform_enable(lua_State* L)
{
	get_genome_ptr(L);
	if (lua_gettop(L) > 0)
	{
		luaL_checktype(L, 1, LUA_TBOOLEAN);
		bool flag = lua_toboolean(L, 1) > 0;
		if (flag)
		{
			if (genome_ptr->final_xform_enable == 0)
				flam3_add_xforms(genome_ptr, 1, 0, 1);
		}
		else
		{
			if (genome_ptr->final_xform_enable == 1)
				flam3_delete_xform(genome_ptr, genome_ptr->final_xform_index);
		}
	}
	else
		lua_pushboolean(L, genome_ptr->final_xform_enable);

	return 1;
}

int Genome::center(lua_State* L)
{
	get_genome_ptr(L);
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

int Genome::rot_center(lua_State* L)
{
	get_genome_ptr(L);
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

int Genome::background(lua_State* L)
{
	get_genome_ptr(L);
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

int Genome::xform(lua_State* L)
{
	get_genome_ptr(L);
	switch (lua_gettop(L))
	{
		case 2:
		{
			int idx = luaL_checkint(L, 1) - 1;
			flam3_xform* xf = Lunar<XForm>::check(L, 2)->get_xform_ptr(L);
			flam3_xform* to = genome_ptr->xform + idx;
			if (to == xf)
				return 1;

			// increase the xforms size if needed
			if (genome_ptr->num_xforms <= idx)
				Util::add_default_xforms(genome_ptr, 1 + idx - genome_ptr->num_xforms);
			flam3_copy_xform(genome_ptr->xform + idx, xf);

			lua_settop(L, 0);
			luaL_getmetatable(L, XForm::className);
			Lunar<XForm>::new_T(L);
			Lunar<XForm>::check(L, 1)->setContext(L, this, idx);
			break;
		}
		case 1:
		{
			int idx = luaL_checkint(L, 1) - 1;
			if (idx > genome_ptr->num_xforms || idx < 0)
				luaL_error(L, "index out of range: Genome.XForm[%d] is null", idx + 1);
			lua_pop(L, 1);
			luaL_getmetatable(L, XForm::className);
			Lunar<XForm>::new_T(L);
			Lunar<XForm>::check(L, 1)->setContext(L, this, idx);
			break;
		}
		default:
		{
			lua_settop(L, 0);
			luaL_getmetatable(L, XForm::className);
			Lunar<XForm>::new_T(L);
			Lunar<XForm>::check(L, 1)->setContext(L, this, 0);
		}
	}
	return 1;
}

int Genome::xforms(lua_State* L)
{
	get_genome_ptr(L);
	lua_settop(L, 0);
	lua_newtable(L);
	for (int n = 0 ; n < genome_ptr->num_xforms ; n++)
	{
		lua_pushinteger(L, n + 1);
		XForm* obj = new XForm(L);
		Lunar<XForm>::push(L, obj, true); // gc_T will delete this object
		obj->setContext(L, this, n);
		lua_settable(L, -3);
	}
	return 1;
}

int Genome::get_xform(lua_State* L)
{
	get_genome_ptr(L);
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
		Lunar<XForm>::check(L, 1)->setContext(L, this, xfn);
	}
	else
	{
		luaL_error(L, "genome has no xform at index %d", xfn + 1);
		return 0;
	}
	return 1;
}

int Genome::get_final_xform(lua_State* L)
{
	get_genome_ptr(L);
	lua_settop(L, 0);
	if (genome_ptr->final_xform_enable == 1)
	{
		luaL_getmetatable(L, XForm::className);
		Lunar<XForm>::new_T(L);
		Lunar<XForm>::check(L, 1)->setContext(L, this, genome_ptr->final_xform_index);
		return 1;
	}
	return 0;
}

int Genome::add_xform(lua_State* L)
{
	get_genome_ptr(L);
	int num = 1;
	XForm* from = 0;
	if (lua_gettop(L) > 1) // two arguments num, XForm
	{
		num  = luaL_checkint(L, 1);
		from = Lunar<XForm>::check(L, 2);
	}
	else if (lua_gettop(L) > 0)  // one argument num or XForm
	{
		if (lua_isuserdata(L, 1))
			from = Lunar<XForm>::check(L, 1);
		else
			num = luaL_checkint(L, 1);
	}

	int last_idx = qMax(0, genome_ptr->num_xforms - 1);
	if (genome_ptr->final_xform_enable == 1)
		last_idx -= 1;

	Util::add_default_xforms(genome_ptr, num);

	int dest_idx = genome_ptr->num_xforms - 1;
	if (genome_ptr->final_xform_enable == 1)
		dest_idx -= 1;

	if (from != 0)
	{
		flam3_xform* xf = from->get_xform_ptr(L);
		for (int i = last_idx ; i <= dest_idx ; i++)
			flam3_copy_xform(genome_ptr->xform + i, xf);
	}

	lua_settop(L, 0);
	luaL_getmetatable(L, XForm::className);
	Lunar<XForm>::new_T(L);
	Lunar<XForm>::check(L, 1)->setContext(L, this, dest_idx);
	return 1;
}

int Genome::copy_xform(lua_State* L)
{
	get_genome_ptr(L);
	flam3_xform* from = 0;
	int src_idx = 0;
	int dest_idx = luaL_checkint(L, 2) - 1;

	if (dest_idx < 0)
		luaL_error(L, "genome has no xform at index %d", dest_idx + 1);

	if (lua_isuserdata(L, 1))
		from = Lunar<XForm>::check(L, 1)->get_xform_ptr(L);

	else
	{
		src_idx = luaL_checkint(L, 1) - 1;
		if (src_idx >= genome_ptr->num_xforms || src_idx < 0)
			luaL_error(L, "genome has no xform at index %d", src_idx + 1);
	}

	if (genome_ptr->num_xforms <= dest_idx)
		Util::add_default_xforms(genome_ptr, dest_idx - genome_ptr->num_xforms + 1);

	if (from == 0)
		from = genome_ptr->xform + src_idx;

	flam3_xform* to = genome_ptr->xform + dest_idx;
	flam3_copy_xform(to, from);
	lua_settop(L, 0);
	luaL_getmetatable(L, XForm::className);
	Lunar<XForm>::new_T(L);
	Lunar<XForm>::check(L, 1)->setContext(L, this, dest_idx);
	return 1;
}

int Genome::del_xform(lua_State* L)
{
	get_genome_ptr(L);
	int idx = luaL_checkint(L, 1) - 1;
	if (idx >= genome_ptr->num_xforms || idx < 0)
		luaL_error(L, "genome has no xform at index %d", idx + 1);
	else
		flam3_delete_xform(genome_ptr, idx);
	return 0;
}

int Genome::clear_xforms(lua_State* L)
{
	get_genome_ptr(L);
	for (int n = genome_ptr->num_xforms - 1 ; n >= 0 ; n--)
		flam3_delete_xform(genome_ptr, n);
	return 0;
}

int Genome::load_palette(lua_State* L)
{
	get_genome_ptr(L);
	int idx = qMax(0, luaL_checkint(L, 1) - 1);
	double hue_rotation = 0;
	if (lua_gettop(L) > 1)
		hue_rotation = luaL_checknumber(L, 2);
	flam3_get_palette(idx, genome_ptr->palette, hue_rotation);
	return 0;
}

int Genome::palette(lua_State* L)
{
	get_genome_ptr(L);
	if (lua_gettop(L) < 1)
	{
		// return the entire palette
		lua_newtable(L);
		for (int i = 0 ; i < 256 ; i++)
		{
			lua_pushinteger(L, i + 1);
			lua_newtable(L);
			lua_pushstring(L, "a");
			lua_pushnumber(L, genome_ptr->palette[i].color[3]);
			lua_settable(L, -3);
			lua_pushstring(L, "b");
			lua_pushnumber(L, genome_ptr->palette[i].color[2]);
			lua_settable(L, -3);
			lua_pushstring(L, "g");
			lua_pushnumber(L, genome_ptr->palette[i].color[1]);
			lua_settable(L, -3);
			lua_pushstring(L, "r");
			lua_pushnumber(L, genome_ptr->palette[i].color[0]);
			lua_settable(L, -3);
			lua_settable(L, -3);
		}
	}
	else if (lua_type(L, 1) == LUA_TTABLE)
	{
		// setting the whole palette from a table
		for (int i = 0 ; i < 256 ; i++)
		{
			double val;
			lua_pushinteger(L, i + 1);
			lua_gettable(L, -2);
			luaL_checktype(L, -1, LUA_TTABLE);

			lua_pushstring(L, "r");
			lua_gettable(L, -2);
			val = luaL_checknumber(L, -1);
			lua_pop(L, 1);
			genome_ptr->palette[i].color[0] = val;

			lua_pushstring(L, "g");
			lua_gettable(L, -2);
			val = luaL_checknumber(L, -1);
			lua_pop(L, 1);
			genome_ptr->palette[i].color[1] = val;

			lua_pushstring(L, "b");
			lua_gettable(L, -2);
			val = luaL_checknumber(L, -1);
			lua_pop(L, 1);
			genome_ptr->palette[i].color[2] = val;

			lua_pushstring(L, "a");
			lua_gettable(L, -2);
			if (lua_isnil(L, -1))
				val = 1.0;
			else
				val = luaL_checknumber(L, -1);
			lua_pop(L, 1);
			genome_ptr->palette[i].color[3] = val;
			lua_pop(L, 1);
		}
	}
	else
	{
		int idx = qMax(0, luaL_checkint(L, 1) - 1);
		if (idx > 255)
			luaL_error(L, "index %d out of range", idx + 1);
		double r, g, b, a;
		if (lua_gettop(L) > 1)
		{
			r = luaL_checknumber(L, 2);
			g = luaL_checknumber(L, 3);
			b = luaL_checknumber(L, 4);
			if (lua_gettop(L) > 4)
				a = luaL_checknumber(L, 5);
			else
				a = 1.0;
			genome_ptr->palette[idx].color[0] = r;
			genome_ptr->palette[idx].color[1] = g;
			genome_ptr->palette[idx].color[2] = b;
			genome_ptr->palette[idx].color[3] = a;
		}
		else
		{
			r = genome_ptr->palette[idx].color[0];
			g = genome_ptr->palette[idx].color[1];
			b = genome_ptr->palette[idx].color[2];
			a = genome_ptr->palette[idx].color[3];
		}
		lua_settop(L, 0);
		lua_pushnumber(L, r);
		lua_pushnumber(L, g);
		lua_pushnumber(L, b);
		lua_pushnumber(L, a);
		return 4;
	}
	return 1;
}

int Genome::chaos(lua_State* L)
{
	get_genome_ptr(L);
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
	get_genome_ptr(L);
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
void Genome::setContext(lua_State* L, int index)
{
	m_idx = index;
	get_genome_ptr(L);
}

flam3_genome* Genome::data()
{
	return genome_ptr;
}

int Genome::index() const
{
	return m_idx;
}

Genome::~Genome()
{
	clear_cp(&m_genome, flam3_defaults_on);
}

}


