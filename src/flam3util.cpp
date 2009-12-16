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
#include <QMap>
#include <math.h>
#include <locale.h>

#include "logger.h"
#include "flam3util.h"

namespace Util
{

	static QMap<QString, int> variations;


	const QMap<QString, int>& flam3_variations()
	{
		if (variations.isEmpty())
			for (int n = 0 ; n < flam3_nvariations ; n++)
				variations.insert(flam3_variation_names[n], n);
		return variations;
	}

	int variation_number(const char* var)
	{
		QString s(var);
		return variation_number( s );
	}

	int variation_number(const QString& s)
	{
		// reverse map variations,
		if (variations.isEmpty())
			flam3_variations();
		if (!variations.contains(s))
			return -1;
		return variations.value( s );
	}

	QColor get_xform_color(flam3_genome* g, flam3_xform* xform)
	{
		int idx = (int)(xform->color * 255.0);
		flam3_palette_entry p = g->palette[idx];
		return QColor::fromRgbF(p.color[0], p.color[1], p.color[2]);
	}

	QTextStream& operator<<( QTextStream& os, flam3_genome& g )
	{
		os  << "symmetry: "  << g.symmetry
			<< "\ntime: " << g.time
			<< "\nwidth, height: " << g.width << ", " << g.height
			<< "\ngenome has " << g.num_xforms << " xforms"
			<< "\nsample_density: " << g.sample_density
			<< "\nspatial_oversample: " << g.spatial_oversample
			<< "\nspatial_filter_radius: " << g.spatial_filter_radius
			<< "\ncenter: " << g.center[0] << ", " << g.center[1]
			<< "\nzoom: " << g.zoom;
		for (int n = 0 ; n < g.num_xforms ; n++)
			os << "\nxform " << n << " density: " << g.xform[n].density;
		os << endl;
		return os;
	}

	QTextStream& operator<<( QTextStream& os, flam3_frame& f )
	{
		os << "ngenomes: " << f.ngenomes
			<< "\nbits: " << f.bits
			<< "\npixel_aspect_ratio: " << f.pixel_aspect_ratio
			<< "\nverbose: " << f.verbose << "\n--genome--\n" << f.genomes;
		return os;
	}


	double get_xform_variable ( flam3_xform* xform, QString name )
	{
		if ( name == "blob_low" ) { return xform->blob_low ; }
		else if ( name == "blob_high" ) { return xform->blob_high ; }
		else if ( name == "blob_waves" ) { return xform->blob_waves ; }


		else if ( name == "pdj_a" ) { return xform->pdj_a ; }
		else if ( name == "pdj_b" ) { return xform->pdj_b ; }
		else if ( name == "pdj_c" ) { return xform->pdj_c ; }
		else if ( name == "pdj_d" ) { return xform->pdj_d ; }


		else if ( name == "fan2_x" ) { return xform->fan2_x ; }
		else if ( name == "fan2_y" ) { return xform->fan2_y ; }


		else if ( name == "rings2_val" ) { return xform->rings2_val ; }


		else if ( name == "perspective_angle" ) { return xform->perspective_angle ; }
		else if ( name == "perspective_dist" ) { return xform->perspective_dist ; }


		else if ( name == "julian_power" ) { return xform->julian_power ; }
		else if ( name == "julian_dist" ) { return xform->julian_dist ; }


		else if ( name == "juliascope_power" ) { return xform->juliascope_power ; }
		else if ( name == "juliascope_dist" ) { return xform->juliascope_dist ; }


		else if ( name == "radial_blur_angle" ) { return xform->radial_blur_angle ; }


		else if ( name == "pie_slices" ) { return xform->pie_slices ; }
		else if ( name == "pie_rotation" ) { return xform->pie_rotation ; }
		else if ( name == "pie_thickness" ) { return xform->pie_thickness ; }


		else if ( name == "ngon_sides" ) { return xform->ngon_sides ; }
		else if ( name == "ngon_power" ) { return xform->ngon_power ; }
		else if ( name == "ngon_circle" ) { return xform->ngon_circle ; }
		else if ( name == "ngon_corners" ) { return xform->ngon_corners ; }

		else if ( name == "curl_c1" ) { return xform->curl_c1 ; }
		else if ( name == "curl_c2" ) { return xform->curl_c2 ; }

		else if ( name == "rectangles_x" ) { return xform->rectangles_x ; }
		else if ( name == "rectangles_y" ) { return xform->rectangles_y ; }


		else if ( name == "amw_amp" ) { return xform->amw_amp ; }


		else if ( name == "disc2_rot" ) { return xform->disc2_rot ; }
		else if ( name == "disc2_twist" ) { return xform->disc2_twist ; }


		else if ( name == "super_shape_rnd" ) { return xform->super_shape_rnd ; }
		else if ( name == "super_shape_m" ) { return xform->super_shape_m ; }
		else if ( name == "super_shape_n1" ) { return xform->super_shape_n1 ; }
		else if ( name == "super_shape_n2" ) { return xform->super_shape_n2 ; }
		else if ( name == "super_shape_n3" ) { return xform->super_shape_n3 ; }
		else if ( name == "super_shape_holes" ) { return xform->super_shape_holes ; }


		else if ( name == "flower_petals" ) { return xform->flower_petals ; }
		else if ( name == "flower_holes" ) { return xform->flower_holes ; }


		else if ( name == "conic_eccentricity" ) { return xform->conic_eccentricity ; }
		else if ( name == "conic_holes" ) { return xform->conic_holes ; }


		else if ( name == "parabola_height" ) { return xform->parabola_height ; }
		else if ( name == "parabola_width" ) { return xform->parabola_width ; }


		else if ( name == "bent2_x" ) { return xform->bent2_x ; }
		else if ( name == "bent2_y" ) { return xform->bent2_y ; }


		else if ( name == "bipolar_shift" ) { return xform->bipolar_shift ; }


		else if ( name == "cell_size" ) { return xform->cell_size ; }


		else if ( name == "cpow_r" ) { return xform->cpow_r ; }
		else if ( name == "cpow_i" ) { return xform->cpow_i ; }
		else if ( name == "cpow_power" ) { return xform->cpow_power ; }


		else if ( name == "curve_xamp" ) { return xform->curve_xamp ; }
		else if ( name == "curve_yamp" ) { return xform->curve_yamp ; }
		else if ( name == "curve_xlength" ) { return xform->curve_xlength ; }
		else if ( name == "curve_ylength" ) { return xform->curve_ylength ; }


		else if ( name == "escher_beta" ) { return xform->escher_beta ; }


		else if ( name == "lazysusan_spin" ) { return xform->lazysusan_spin ; }
		else if ( name == "lazysusan_space" ) { return xform->lazysusan_space ; }
		else if ( name == "lazysusan_twist" ) { return xform->lazysusan_twist ; }
		else if ( name == "lazysusan_x" ) { return xform->lazysusan_x ; }
		else if ( name == "lazysusan_y" ) { return xform->lazysusan_y ; }


		else if ( name == "modulus_x" ) { return xform->modulus_x ; }
		else if ( name == "modulus_y" ) { return xform->modulus_y ; }


		else if ( name == "oscope_separation" ) { return xform->oscope_separation ; }
		else if ( name == "oscope_frequency" ) { return xform->oscope_frequency ; }
		else if ( name == "oscope_amplitude" ) { return xform->oscope_amplitude ; }
		else if ( name == "oscope_damping" ) { return xform->oscope_damping ; }


		else if ( name == "popcorn2_x" ) { return xform->popcorn2_x ; }
		else if ( name == "popcorn2_y" ) { return xform->popcorn2_y ; }
		else if ( name == "popcorn2_c" ) { return xform->popcorn2_c ; }


		else if ( name == "separation_x" ) { return xform->separation_x ; }
		else if ( name == "separation_xinside" ) { return xform->separation_xinside ; }
		else if ( name == "separation_y" ) { return xform->separation_y ; }
		else if ( name == "separation_yinside" ) { return xform->separation_yinside ; }


		else if ( name == "split_xsize" ) { return xform->split_xsize ; }
		else if ( name == "split_ysize" ) { return xform->split_ysize ; }


		else if ( name == "splits_x" ) { return xform->splits_x ; }
		else if ( name == "splits_y" ) { return xform->splits_y ; }


		else if ( name == "stripes_space" ) { return xform->stripes_space ; }
		else if ( name == "stripes_warp" ) { return xform->stripes_warp ; }


		else if ( name == "wedge_angle" ) { return xform->wedge_angle ; }
		else if ( name == "wedge_hole" ) { return xform->wedge_hole ; }
		else if ( name == "wedge_count" ) { return xform->wedge_count ; }
		else if ( name == "wedge_swirl" ) { return xform->wedge_swirl ; }


		else if ( name == "wedge_julia_angle" ) { return xform->wedge_julia_angle ; }
		else if ( name == "wedge_julia_count" ) { return xform->wedge_julia_count ; }
		else if ( name == "wedge_julia_power" ) { return xform->wedge_julia_power ; }
		else if ( name == "wedge_julia_dist" ) { return xform->wedge_julia_dist ; }


		else if ( name == "wedge_sph_angle" ) { return xform->wedge_sph_angle ; }
		else if ( name == "wedge_sph_count" ) { return xform->wedge_sph_count ; }
		else if ( name == "wedge_sph_hole" ) { return xform->wedge_sph_hole ; }
		else if ( name == "wedge_sph_swirl" ) { return xform->wedge_sph_swirl ; }


		else if ( name == "whorl_inside" ) { return xform->whorl_inside ; }
		else if ( name == "whorl_outside" ) { return xform->whorl_outside ; }


		else if ( name == "waves2_freqx" ) { return xform->waves2_freqx ; }
		else if ( name == "waves2_scalex" ) { return xform->waves2_scalex ; }
		else if ( name == "waves2_freqy" ) { return xform->waves2_freqy ; }
		else if ( name == "waves2_scaley" ) { return xform->waves2_scaley ; }


		else if ( name == "auger_sym" ) { return xform->auger_sym ; }
		else if ( name == "auger_weight" ) { return xform->auger_weight ; }
		else if ( name == "auger_freq" ) { return xform->auger_freq ; }
		else if ( name == "auger_scale" ) { return xform->auger_scale ; }

		else
			logWarn(QString("Util::get_xform_variable : unknown variable '%1'").arg(name));

		return 0.;
	}

	void set_xform_variable ( flam3_xform* xform, QString name, double value )
	{
		if ( name == "blob_low" ) { xform->blob_low = value ; }
		else if ( name == "blob_high" ) { xform->blob_high = value ; }
		else if ( name == "blob_waves" ) { xform->blob_waves = value ; }


		else if ( name == "pdj_a" ) { xform->pdj_a = value ; }
		else if ( name == "pdj_b" ) { xform->pdj_b = value ; }
		else if ( name == "pdj_c" ) { xform->pdj_c = value ; }
		else if ( name == "pdj_d" ) { xform->pdj_d = value ; }


		else if ( name == "fan2_x" ) { xform->fan2_x = value ; }
		else if ( name == "fan2_y" ) { xform->fan2_y = value ; }


		else if ( name == "rings2_val" ) { xform->rings2_val = value ; }


		else if ( name == "perspective_angle" ) { xform->perspective_angle = value ; }
		else if ( name == "perspective_dist" ) { xform->perspective_dist = value ; }


		else if ( name == "julian_power" ) { xform->julian_power = value ; }
		else if ( name == "julian_dist" ) { xform->julian_dist = value ; }


		else if ( name == "juliascope_power" ) { xform->juliascope_power = value ; }
		else if ( name == "juliascope_dist" ) { xform->juliascope_dist = value ; }


		else if ( name == "radial_blur_angle" ) { xform->radial_blur_angle = value ; }


		else if ( name == "pie_slices" ) { xform->pie_slices = value ; }
		else if ( name == "pie_rotation" ) { xform->pie_rotation = value ; }
		else if ( name == "pie_thickness" ) { xform->pie_thickness = value ; }


		else if ( name == "ngon_sides" ) { xform->ngon_sides = value ; }
		else if ( name == "ngon_power" ) { xform->ngon_power = value ; }
		else if ( name == "ngon_circle" ) { xform->ngon_circle = value ; }
		else if ( name == "ngon_corners" ) { xform->ngon_corners = value ; }

		else if ( name == "curl_c1" ) { xform->curl_c1 = value ; }
		else if ( name == "curl_c2" ) { xform->curl_c2 = value ; }

		else if ( name == "rectangles_x" ) { xform->rectangles_x = value ; }
		else if ( name == "rectangles_y" ) { xform->rectangles_y = value ; }

		else if ( name == "amw_amp" ) { xform->amw_amp = value ; }

		else if ( name == "disc2_rot" ) { xform->disc2_rot = value ; }
		else if ( name == "disc2_twist" ) { xform->disc2_twist = value ; }


		else if ( name == "super_shape_rnd" ) { xform->super_shape_rnd = value ; }
		else if ( name == "super_shape_m" ) { xform->super_shape_m = value ; }
		else if ( name == "super_shape_n1" ) { xform->super_shape_n1 = value ; }
		else if ( name == "super_shape_n2" ) { xform->super_shape_n2 = value ; }
		else if ( name == "super_shape_n3" ) { xform->super_shape_n3 = value ; }
		else if ( name == "super_shape_holes" ) { xform->super_shape_holes = value ; }


		else if ( name == "flower_petals" ) { xform->flower_petals = value ; }
		else if ( name == "flower_holes" ) { xform->flower_holes = value ; }


		else if ( name == "conic_eccentricity" ) { xform->conic_eccentricity = value ; }
		else if ( name == "conic_holes" ) { xform->conic_holes = value ; }


		else if ( name == "parabola_height" ) { xform->parabola_height = value ; }
		else if ( name == "parabola_width" ) { xform->parabola_width = value ; }


		else if ( name == "bent2_x" ) { xform->bent2_x = value ; }
		else if ( name == "bent2_y" ) { xform->bent2_y = value ; }


		else if ( name == "bipolar_shift" ) { xform->bipolar_shift = value ; }


		else if ( name == "cell_size" ) { xform->cell_size = value ; }


		else if ( name == "cpow_r" ) { xform->cpow_r = value ; }
		else if ( name == "cpow_i" ) { xform->cpow_i = value ; }
		else if ( name == "cpow_power" ) { xform->cpow_power = value ; }


		else if ( name == "curve_xamp" ) { xform->curve_xamp = value ; }
		else if ( name == "curve_yamp" ) { xform->curve_yamp = value ; }
		else if ( name == "curve_xlength" ) { xform->curve_xlength = value ; }
		else if ( name == "curve_ylength" ) { xform->curve_ylength = value ; }


		else if ( name == "escher_beta" ) { xform->escher_beta = value ; }


		else if ( name == "lazysusan_spin" ) { xform->lazysusan_spin = value ; }
		else if ( name == "lazysusan_space" ) { xform->lazysusan_space = value ; }
		else if ( name == "lazysusan_twist" ) { xform->lazysusan_twist = value ; }
		else if ( name == "lazysusan_x" ) { xform->lazysusan_x = value ; }
		else if ( name == "lazysusan_y" ) { xform->lazysusan_y = value ; }


		else if ( name == "modulus_x" ) { xform->modulus_x = value ; }
		else if ( name == "modulus_y" ) { xform->modulus_y = value ; }


		else if ( name == "oscope_separation" ) { xform->oscope_separation = value ; }
		else if ( name == "oscope_frequency" ) { xform->oscope_frequency = value ; }
		else if ( name == "oscope_amplitude" ) { xform->oscope_amplitude = value ; }
		else if ( name == "oscope_damping" ) { xform->oscope_damping = value ; }


		else if ( name == "popcorn2_x" ) { xform->popcorn2_x = value ; }
		else if ( name == "popcorn2_y" ) { xform->popcorn2_y = value ; }
		else if ( name == "popcorn2_c" ) { xform->popcorn2_c = value ; }


		else if ( name == "separation_x" ) { xform->separation_x = value ; }
		else if ( name == "separation_xinside" ) { xform->separation_xinside = value ; }
		else if ( name == "separation_y" ) { xform->separation_y = value ; }
		else if ( name == "separation_yinside" ) { xform->separation_yinside = value ; }


		else if ( name == "split_xsize" ) { xform->split_xsize = value ; }
		else if ( name == "split_ysize" ) { xform->split_ysize = value ; }


		else if ( name == "splits_x" ) { xform->splits_x = value ; }
		else if ( name == "splits_y" ) { xform->splits_y = value ; }


		else if ( name == "stripes_space" ) { xform->stripes_space = value ; }
		else if ( name == "stripes_warp" ) { xform->stripes_warp = value ; }


		else if ( name == "wedge_angle" ) { xform->wedge_angle = value ; }
		else if ( name == "wedge_hole" ) { xform->wedge_hole = value ; }
		else if ( name == "wedge_count" ) { xform->wedge_count = value ; }
		else if ( name == "wedge_swirl" ) { xform->wedge_swirl = value ; }


		else if ( name == "wedge_julia_angle" ) { xform->wedge_julia_angle = value ; }
		else if ( name == "wedge_julia_count" ) { xform->wedge_julia_count = value ; }
		else if ( name == "wedge_julia_power" ) { xform->wedge_julia_power = value ; }
		else if ( name == "wedge_julia_dist" ) { xform->wedge_julia_dist = value ; }


		else if ( name == "wedge_sph_angle" ) { xform->wedge_sph_angle = value ; }
		else if ( name == "wedge_sph_count" ) { xform->wedge_sph_count = value ; }
		else if ( name == "wedge_sph_hole" ) { xform->wedge_sph_hole = value ; }
		else if ( name == "wedge_sph_swirl" ) { xform->wedge_sph_swirl = value ; }


		else if ( name == "whorl_inside" ) { xform->whorl_inside = value ; }
		else if ( name == "whorl_outside" ) { xform->whorl_outside = value ; }


		else if ( name == "waves2_freqx" ) { xform->waves2_freqx = value ; }
		else if ( name == "waves2_scalex" ) { xform->waves2_scalex = value ; }
		else if ( name == "waves2_freqy" ) { xform->waves2_freqy = value ; }
		else if ( name == "waves2_scaley" ) { xform->waves2_scaley = value ; }


		else if ( name == "auger_sym" ) { xform->auger_sym = value ; }
		else if ( name == "auger_weight" ) { xform->auger_weight = value ; }
		else if ( name == "auger_freq" ) { xform->auger_freq = value ; }
		else if ( name == "auger_scale" ) { xform->auger_scale = value ; }

		else
			logWarn(QString("Util::set_xform_variable : unknown variable '%1'").arg(name));

	}

	QStringList& get_variable_names()
	{
		static QStringList var_names = (QStringList()
		<< "blob_low"
		<< "blob_high"
		<< "blob_waves"

		<< "pdj_a"
		<< "pdj_b"
		<< "pdj_c"
		<< "pdj_d"

		<< "fan2_x"
		<< "fan2_y"

		<< "rings2_val"

		<< "perspective_angle"
		<< "perspective_dist"

		<< "julian_power"
		<< "julian_dist"

		<< "juliascope_power"
		<< "juliascope_dist"

		<< "radial_blur_angle"

		<< "pie_slices"
		<< "pie_rotation"
		<< "pie_thickness"

		<< "ngon_sides"
		<< "ngon_power"
		<< "ngon_circle"
		<< "ngon_corners"

		<< "curl_c1"
		<< "curl_c2"

		<< "rectangles_x"
		<< "rectangles_y"

		<< "amw_amp"

		<< "disc2_rot"
		<< "disc2_twist"

		<< "super_shape_rnd"
		<< "super_shape_m"
		<< "super_shape_n1"
		<< "super_shape_n2"
		<< "super_shape_n3"
		<< "super_shape_holes"

		<< "flower_petals"
		<< "flower_holes"

		<< "conic_eccentricity"
		<< "conic_holes"

		<< "parabola_height"
		<< "parabola_width"

		<< "bent2_x"
		<< "bent2_y"

		<< "bipolar_shift"

		<< "cell_size"

		<< "cpow_r"
		<< "cpow_i"
		<< "cpow_power"

		<< "curve_xamp"
		<< "curve_yamp"
		<< "curve_xlength"
		<< "curve_ylength"

		<< "escher_beta"

		<< "lazysusan_spin"
		<< "lazysusan_space"
		<< "lazysusan_twist"
		<< "lazysusan_x"
		<< "lazysusan_y"

		<< "modulus_x"
		<< "modulus_y"

		<< "oscope_separation"
		<< "oscope_frequency"
		<< "oscope_amplitude"
		<< "oscope_damping"

		<< "popcorn2_x" << "popcorn2_y" << "popcorn2_c"

		<< "separation_x" << "separation_xinside"
		<< "separation_y" << "separation_yinside"

		<< "split_xsize"
		<< "split_ysize"

		<< "splits_x" << "splits_y"

		<< "stripes_space"
		<< "stripes_warp"

		<< "wedge_angle" << "wedge_hole"
		<< "wedge_count" << "wedge_swirl"


		<< "wedge_julia_angle"
		<< "wedge_julia_count"
		<< "wedge_julia_power"
		<< "wedge_julia_dist"


		<< "wedge_sph_angle" << "wedge_sph_count"
		<< "wedge_sph_hole" << "wedge_sph_swirl"


		<< "whorl_inside" << "whorl_outside"


		<< "waves2_freqx" << "waves2_scalex"
		<< "waves2_freqy" << "waves2_scaley"


		<< "auger_sym" << "auger_weight"
		<< "auger_freq" << "auger_scale"

		);

		return var_names;
	}

	char* setup_C_locale()
	{
		// force use of "C" locale when reading/writing reals.
		// first save away the current settings.
		char* locale = NULL;
		char* lorig  = setlocale(LC_NUMERIC, NULL);
		if (lorig == NULL)
			logError("Util::setup_C_locale : couldn't get current locale");
		else
		{
			int slen = strlen(lorig) + 1;
			locale = (char*)malloc(slen);
			if (locale != NULL)
				memcpy(locale, lorig, slen);
		}
		if (setlocale(LC_NUMERIC, "C") == NULL)
			logError("Util::setup_C_locale : couldn't set C locale");

		return locale;
	}

	void replace_C_locale(char* locale)
	{
		if (locale != NULL)
		{
			if (setlocale(LC_NUMERIC, locale) == NULL)
				logError("Util::replace_Ct_locale : couldn't replace locale settings");
			free(locale);
		}
	}

	void write_to_file(FILE* fd, flam3_genome* genome, char* attrs, int edits)
	{
		char* locale = setup_C_locale();
		flam3_print(fd, genome, attrs, edits);
		replace_C_locale(locale);
	}

	flam3_genome* read_from_file(FILE* fd, char* fn, int default_flag, int* ncps)
	{
		char* locale = setup_C_locale();
		flam3_genome* g = flam3_parse_from_file(fd, fn, default_flag, ncps);
		replace_C_locale(locale);
		return g;
	}

	flam3_genome* read_xml_string(QString xml, int* ncps)
	{
		char* locale = setup_C_locale();
		flam3_genome* g
			= flam3_parse_xml2(xml.toAscii().data(),
				QString("stdin").toAscii().data(), 1, ncps);
		replace_C_locale(locale);
		return g;
	}

	void init_genome(flam3_genome* g)
	{
		// Clear and initialize a newly created and unused genome. Copied from
		// clear_cp() in flam.c in the libflam3 source.
		//
		// todo: replace by clear_cp() which is now exported from libflam3-2.8
		//
		g->palette_index = flam3_palette_random;
		flam3_get_palette(flam3_palette_random, g->palette, 0.0);
		g->num_xforms = 0;
		g->xform = 0;
		g->chaos = 0;
		g->time = 0.0;
		g->center[0] = 0.0;
		g->center[1] = 0.0;
		g->rot_center[0] = 0.0;
		g->rot_center[1] = 0.0;
		g->gamma = 4.0;
		g->vibrancy = 1.0;
		g->contrast = 1.0;
		g->brightness = 4.0;
		g->symmetry = 1;
		g->hue_rotation = 0.0;
		g->rotate = 0.0;
		g->edits = NULL;
		g->pixels_per_unit = 50;
		g->final_xform_enable = 0;
		g->final_xform_index = -1;
		g->interpolation = flam3_interpolation_linear;
		g->palette_interpolation = flam3_palette_interpolation_hsv;
		g->genome_index = 0;
		memset(g->parent_fname,0,flam3_parent_fn_len);
		memset(g->flame_name, 0, flam3_name_len+1);
		g->background[0] = 0.0;
		g->background[1] = 0.0;
		g->background[2] = 0.0;
		g->width = 100;
		g->height = 100;
		g->spatial_oversample = 1;
		g->spatial_filter_radius = 0.5;
		g->zoom = 0.0;
		g->sample_density = 1;
		/* Density estimation stuff defaulting to ON */
		g->estimator = 9.0;
		g->estimator_minimum = 0.0;
		g->estimator_curve = 0.4;
		g->gam_lin_thresh = 0.01;
		g->nbatches = 1;
		g->ntemporal_samples = 60;
		g->spatial_filter_select = flam3_gaussian_kernel;
	}

	void add_default_xform(flam3_genome* g)
	{
		flam3_xform* xform;
		flam3_add_xforms(g, 1, 0, 0);

		if (g->final_xform_enable == 1)
			xform = g->xform + (g->num_xforms - 2);
		else
			xform = g->xform + (g->num_xforms - 1);

		xform->c[0][0] = 1.0;
		xform->c[0][1] = 0.0;
		xform->c[1][0] = 0.0;
		xform->c[1][1] = 1.0;
		xform->c[2][0] = 0.0;
		xform->c[2][1] = 0.0;

		xform->post[0][0] = 1.0;
		xform->post[0][1] = 0.0;
		xform->post[1][0] = 0.0;
		xform->post[1][1] = 1.0;
		xform->post[2][0] = 0.0;
		xform->post[2][1] = 0.0;

		for (int j = 0 ; j < flam3_nvariations ; j++)
			xform->var[j] = 0.0;
		xform->var[VAR_LINEAR] = 1.0;
		xform->color = 0.0;
		xform->density = 0.5;
	}

#define PI 3.14159265358979323846

	void rectToPolar(double x, double y, double* r, double* p)
	{
		*r = sqrt(pow(x, 2.) + pow(y, 2.));
		if (x >= 0. && y >= 0.) *p = atan( y / x );
		else if (x < 0. && y >= 0.) *p = PI - atan( y / ((-1.)*x) );
		else if (x < 0. && y < 0.) *p = PI + atan( y / x );
		else *p = 2.*PI - atan( ((-1.)*y) / x );
	}

	void rectToPolarDeg(double x, double y, double* r, double* d)
	{
		rectToPolar(x, y, r, d);
		*d = *d * 180. / PI;
	}

	void polarToRect(double r, double p, double* x, double* y)
	{
		*x = r*cos(p);
		*y = r*sin(p);
	}

	void polarDegToRect(double r, double p, double* x, double* y)
	{
		polarToRect( r, p * PI / 180., x, y);
	}

	randctx* get_isaac_randctx()
	{
		static randctx r;
		static bool init = false;
		if (init)
			return &r;

		/* Initialize the issac random number generator used by libflam3 */
		long int default_isaac_seed = (long int)time(0);
		memset(r.randrsl, 0, RANDSIZ*sizeof(ub4));
		for (int lp = 0; lp < RANDSIZ; lp++)
			r.randrsl[lp] = default_isaac_seed;
		irandinit(&r, 1);
		init = true;
		return &r;
	}

}
