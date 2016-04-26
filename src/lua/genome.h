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
#ifndef GENOME_LUA_H
#define GENOME_LUA_H

#include "lunar.h"
#include "flam3util.h"
#include "luatype.h"
#include "basistriangle.h"

#define intAcc_H(name) int name(lua_State*);
#define intAcc_HRO(name) int name(lua_State*);
#define realAcc_H(name) int name(lua_State*);
#define realAcc_HRO(name) int name(lua_State*);


namespace Lua
{

class Genome : public LuaType
{
	int m_idx;
	flam3_genome m_genome;
	flam3_genome* genome_ptr;

	public:
	Genome(lua_State*);
	~Genome();
	int index(lua_State*);
	int index() const;
	int center(lua_State*);
	int rot_center(lua_State*);
	int background(lua_State*);
	flam3_genome* get_genome_ptr(lua_State*);

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
	int get_final_xform(lua_State*);
	int xform(lua_State*);
	int xforms(lua_State*);

	void setContext(lua_State*, int);
	void setModified();
	flam3_genome* data();

	static const char className[];
	static Lunar<Genome>::RegType methods[];
};
}

#endif
