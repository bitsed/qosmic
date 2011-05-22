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
#ifndef GENOME_LUA_H
#define GENOME_LUA_H

#include "lunar.h"
#include "flam3util.h"
#include "luatype.h"
#include "basistriangle.h"

#define intAcc_H(name) int name(lua_State *L);
#define intAcc_HRO(name) int name(lua_State *L);
#define realAcc_H(name) int name(lua_State *L);
#define realAcc_HRO(name) int name(lua_State *L);


namespace Lua
{

class Genome : public LuaType
{
	int idx;
	flam3_genome* genome_ptr;
	BasisTriangle* basis;

	public:
	Genome(lua_State *L);
	~Genome();
	int index(lua_State *L);
	int index() const;
	int center(lua_State *L);
	int rot_center(lua_State *L);
	int background(lua_State *L);

	intAcc_HRO(num_xforms);
	intAcc_HRO(genome_index);

	intAcc_H(width);
	intAcc_H(height);
	realAcc_H(rotate);
	realAcc_H(vibrancy);
	realAcc_H(hue_rotation);
	realAcc_H(zoom);
	realAcc_H(pixels_per_unit);
	realAcc_H(spatial_filter_radius);
	intAcc_H(spatial_filter_select);
	realAcc_H(sample_density);
	realAcc_H(brightness);
	realAcc_H(contrast);
	realAcc_H(gamma);
	realAcc_H(time);
	realAcc_H(estimator);
	realAcc_H(estimator_curve);
	realAcc_H(estimator_minimum);
	realAcc_H(gam_lin_thresh);
	realAcc_H(highlight_power);
	intAcc_H(interpolation);
	intAcc_H(interpolation_type);
	intAcc_H(palette_interpolation);
	intAcc_H(final_xform_enable);
	intAcc_H(final_xform_index);
	intAcc_H(symmetry);
	intAcc_H(palette_index);
	intAcc_H(spatial_oversample);
	intAcc_H(nbatches);
	intAcc_H(ntemporal_samples);

	int get_xform(lua_State*);
	int add_xform(lua_State*);
	int del_xform(lua_State*);
	int copy_xform(lua_State*);
	int clear_xforms(lua_State*);
	int load_palette(lua_State*);
	int palette(lua_State*);
	int chaos(lua_State*);

	void setContext(LuaThreadAdapter*, int);

	static const char className[];
	static Lunar<Genome>::RegType methods[];
};
}

#endif
