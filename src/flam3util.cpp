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
#include <QMap>
#include <QHash>
#include <cmath>
#include <ctime>
#include <clocale>

#include "logger.h"
#include "flam3util.h"


namespace Util
{

	static QMap<QString, int> variation_map;
	const QMap<QString, int>& flam3_variations()
	{
		return variation_map;
	}

	static QStringList variation_name_list;
	const QStringList& variation_names()
	{
		return variation_name_list;
	}

	int variation_number(const char* var)
	{
		QString s(var);
		return variation_number( s );
	}

	int variation_number(const QString& s)
	{
		// reverse map variations,
		if (!variation_map.contains(s))
			return -1;
		return variation_map.value( s );
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

	struct xform_variable_accessor
	{
		xform_variable_accessor() {}
		virtual double get_var(flam3_xform*)=0;
		virtual void set_var(flam3_xform*, double)=0;
	};

	static QHash<QString, xform_variable_accessor*> xform_variable_accessors;

#define create_xform_variable_accessor(name) \
	struct xform_variable_accessor_##name : public xform_variable_accessor \
	{\
		xform_variable_accessor_##name() {} \
		double get_var(flam3_xform* xform) { return xform->name; } \
		void set_var(flam3_xform* xform, double value) { xform->name = value; } \
	};

	create_xform_variable_accessor(blob_low)
	create_xform_variable_accessor(blob_high)
	create_xform_variable_accessor(blob_waves)

	create_xform_variable_accessor(pdj_a)
	create_xform_variable_accessor(pdj_b)
	create_xform_variable_accessor(pdj_c)
	create_xform_variable_accessor(pdj_d)

	create_xform_variable_accessor(fan2_x)
	create_xform_variable_accessor(fan2_y)

	create_xform_variable_accessor(rings2_val)

	create_xform_variable_accessor(perspective_angle)
	create_xform_variable_accessor(perspective_dist)

	create_xform_variable_accessor(julian_power)
	create_xform_variable_accessor(julian_dist)

	create_xform_variable_accessor(juliascope_power)
	create_xform_variable_accessor(juliascope_dist)

	create_xform_variable_accessor(radial_blur_angle)

	create_xform_variable_accessor(pie_slices)
	create_xform_variable_accessor(pie_rotation)
	create_xform_variable_accessor(pie_thickness)

	create_xform_variable_accessor(ngon_sides)
	create_xform_variable_accessor(ngon_power)
	create_xform_variable_accessor(ngon_circle)
	create_xform_variable_accessor(ngon_corners)

	create_xform_variable_accessor(curl_c1)
	create_xform_variable_accessor(curl_c2)

	create_xform_variable_accessor(rectangles_x)
	create_xform_variable_accessor(rectangles_y)

	create_xform_variable_accessor(amw_amp)

	create_xform_variable_accessor(disc2_rot)
	create_xform_variable_accessor(disc2_twist)

	create_xform_variable_accessor(super_shape_rnd)
	create_xform_variable_accessor(super_shape_m)
	create_xform_variable_accessor(super_shape_n1)
	create_xform_variable_accessor(super_shape_n2)
	create_xform_variable_accessor(super_shape_n3)
	create_xform_variable_accessor(super_shape_holes)

	create_xform_variable_accessor(flower_petals)
	create_xform_variable_accessor(flower_holes)

	create_xform_variable_accessor(conic_eccentricity)
	create_xform_variable_accessor(conic_holes)

	create_xform_variable_accessor(parabola_height)
	create_xform_variable_accessor(parabola_width)

	create_xform_variable_accessor(bent2_x)
	create_xform_variable_accessor(bent2_y)

	create_xform_variable_accessor(bipolar_shift)

	create_xform_variable_accessor(cell_size)

	create_xform_variable_accessor(cpow_r)
	create_xform_variable_accessor(cpow_i)
	create_xform_variable_accessor(cpow_power)

	create_xform_variable_accessor(curve_xamp)
	create_xform_variable_accessor(curve_yamp)
	create_xform_variable_accessor(curve_xlength)
	create_xform_variable_accessor(curve_ylength)

	create_xform_variable_accessor(escher_beta)

	create_xform_variable_accessor(lazysusan_spin)
	create_xform_variable_accessor(lazysusan_space)
	create_xform_variable_accessor(lazysusan_twist)
	create_xform_variable_accessor(lazysusan_x)
	create_xform_variable_accessor(lazysusan_y)

	create_xform_variable_accessor(modulus_x)
	create_xform_variable_accessor(modulus_y)

	create_xform_variable_accessor(oscope_separation)
	create_xform_variable_accessor(oscope_frequency)
	create_xform_variable_accessor(oscope_amplitude)
	create_xform_variable_accessor(oscope_damping)

	create_xform_variable_accessor(popcorn2_x)
	create_xform_variable_accessor(popcorn2_y)
	create_xform_variable_accessor(popcorn2_c)

	create_xform_variable_accessor(separation_x)
	create_xform_variable_accessor(separation_xinside)
	create_xform_variable_accessor(separation_y)
	create_xform_variable_accessor(separation_yinside)

	create_xform_variable_accessor(split_xsize)
	create_xform_variable_accessor(split_ysize)

	create_xform_variable_accessor(splits_x)
	create_xform_variable_accessor(splits_y)

	create_xform_variable_accessor(stripes_space)
	create_xform_variable_accessor(stripes_warp)

	create_xform_variable_accessor(wedge_angle)
	create_xform_variable_accessor(wedge_hole)
	create_xform_variable_accessor(wedge_count)
	create_xform_variable_accessor(wedge_swirl)

	create_xform_variable_accessor(wedge_julia_angle)
	create_xform_variable_accessor(wedge_julia_count)
	create_xform_variable_accessor(wedge_julia_power)
	create_xform_variable_accessor(wedge_julia_dist)

	create_xform_variable_accessor(wedge_sph_angle)
	create_xform_variable_accessor(wedge_sph_count)
	create_xform_variable_accessor(wedge_sph_hole)
	create_xform_variable_accessor(wedge_sph_swirl)

	create_xform_variable_accessor(whorl_inside)
	create_xform_variable_accessor(whorl_outside)

	create_xform_variable_accessor(waves2_freqx)
	create_xform_variable_accessor(waves2_scalex)
	create_xform_variable_accessor(waves2_freqy)
	create_xform_variable_accessor(waves2_scaley)

	create_xform_variable_accessor(auger_sym)
	create_xform_variable_accessor(auger_weight)
	create_xform_variable_accessor(auger_freq)
	create_xform_variable_accessor(auger_scale)

	create_xform_variable_accessor(flux_spread)

	create_xform_variable_accessor(mobius_re_a)
	create_xform_variable_accessor(mobius_im_a)
	create_xform_variable_accessor(mobius_re_b)
	create_xform_variable_accessor(mobius_im_b)
	create_xform_variable_accessor(mobius_re_c)
	create_xform_variable_accessor(mobius_im_c)
	create_xform_variable_accessor(mobius_re_d)
	create_xform_variable_accessor(mobius_im_d)


#define add_xform_variable_accessor(name) \
		xform_variable_accessors.insert(QString(#name), new xform_variable_accessor_##name)

	void init_xform_variable_accessors()
	{
		add_xform_variable_accessor(blob_low);
		add_xform_variable_accessor(blob_high);
		add_xform_variable_accessor(blob_waves);

		add_xform_variable_accessor(pdj_a);
		add_xform_variable_accessor(pdj_b);
		add_xform_variable_accessor(pdj_c);
		add_xform_variable_accessor(pdj_d);

		add_xform_variable_accessor(fan2_x);
		add_xform_variable_accessor(fan2_y);

		add_xform_variable_accessor(rings2_val);

		add_xform_variable_accessor(perspective_angle);
		add_xform_variable_accessor(perspective_dist);

		add_xform_variable_accessor(julian_power);
		add_xform_variable_accessor(julian_dist);

		add_xform_variable_accessor(juliascope_power);
		add_xform_variable_accessor(juliascope_dist);

		add_xform_variable_accessor(radial_blur_angle);

		add_xform_variable_accessor(pie_slices);
		add_xform_variable_accessor(pie_rotation);
		add_xform_variable_accessor(pie_thickness);

		add_xform_variable_accessor(ngon_sides);
		add_xform_variable_accessor(ngon_power);
		add_xform_variable_accessor(ngon_circle);
		add_xform_variable_accessor(ngon_corners);

		add_xform_variable_accessor(curl_c1);
		add_xform_variable_accessor(curl_c2);

		add_xform_variable_accessor(rectangles_x);
		add_xform_variable_accessor(rectangles_y);

		add_xform_variable_accessor(amw_amp);

		add_xform_variable_accessor(disc2_rot);
		add_xform_variable_accessor(disc2_twist);

		add_xform_variable_accessor(super_shape_rnd);
		add_xform_variable_accessor(super_shape_m);
		add_xform_variable_accessor(super_shape_n1);
		add_xform_variable_accessor(super_shape_n2);
		add_xform_variable_accessor(super_shape_n3);
		add_xform_variable_accessor(super_shape_holes);

		add_xform_variable_accessor(flower_petals);
		add_xform_variable_accessor(flower_holes);

		add_xform_variable_accessor(conic_eccentricity);
		add_xform_variable_accessor(conic_holes);

		add_xform_variable_accessor(parabola_height);
		add_xform_variable_accessor(parabola_width);

		add_xform_variable_accessor(bent2_x);
		add_xform_variable_accessor(bent2_y);

		add_xform_variable_accessor(bipolar_shift);

		add_xform_variable_accessor(cell_size);

		add_xform_variable_accessor(cpow_r);
		add_xform_variable_accessor(cpow_i);
		add_xform_variable_accessor(cpow_power);

		add_xform_variable_accessor(curve_xamp);
		add_xform_variable_accessor(curve_yamp);
		add_xform_variable_accessor(curve_xlength);
		add_xform_variable_accessor(curve_ylength);

		add_xform_variable_accessor(escher_beta);

		add_xform_variable_accessor(lazysusan_spin);
		add_xform_variable_accessor(lazysusan_space);
		add_xform_variable_accessor(lazysusan_twist);
		add_xform_variable_accessor(lazysusan_x);
		add_xform_variable_accessor(lazysusan_y);

		add_xform_variable_accessor(modulus_x);
		add_xform_variable_accessor(modulus_y);

		add_xform_variable_accessor(oscope_separation);
		add_xform_variable_accessor(oscope_frequency);
		add_xform_variable_accessor(oscope_amplitude);
		add_xform_variable_accessor(oscope_damping);

		add_xform_variable_accessor(popcorn2_x);
		add_xform_variable_accessor(popcorn2_y);
		add_xform_variable_accessor(popcorn2_c);

		add_xform_variable_accessor(separation_x);
		add_xform_variable_accessor(separation_xinside);
		add_xform_variable_accessor(separation_y);
		add_xform_variable_accessor(separation_yinside);

		add_xform_variable_accessor(split_xsize);
		add_xform_variable_accessor(split_ysize);

		add_xform_variable_accessor(splits_x);
		add_xform_variable_accessor(splits_y);

		add_xform_variable_accessor(stripes_space);
		add_xform_variable_accessor(stripes_warp);

		add_xform_variable_accessor(wedge_angle);
		add_xform_variable_accessor(wedge_hole);
		add_xform_variable_accessor(wedge_count);
		add_xform_variable_accessor(wedge_swirl);

		add_xform_variable_accessor(wedge_julia_angle);
		add_xform_variable_accessor(wedge_julia_count);
		add_xform_variable_accessor(wedge_julia_power);
		add_xform_variable_accessor(wedge_julia_dist);

		add_xform_variable_accessor(wedge_sph_angle);
		add_xform_variable_accessor(wedge_sph_count);
		add_xform_variable_accessor(wedge_sph_hole);
		add_xform_variable_accessor(wedge_sph_swirl);

		add_xform_variable_accessor(whorl_inside);
		add_xform_variable_accessor(whorl_outside);

		add_xform_variable_accessor(waves2_freqx);
		add_xform_variable_accessor(waves2_scalex);
		add_xform_variable_accessor(waves2_freqy);
		add_xform_variable_accessor(waves2_scaley);

		add_xform_variable_accessor(auger_sym);
		add_xform_variable_accessor(auger_weight);
		add_xform_variable_accessor(auger_freq);
		add_xform_variable_accessor(auger_scale);

		add_xform_variable_accessor(flux_spread);

		add_xform_variable_accessor(mobius_re_a);
		add_xform_variable_accessor(mobius_im_a);
		add_xform_variable_accessor(mobius_re_b);
		add_xform_variable_accessor(mobius_im_b);
		add_xform_variable_accessor(mobius_re_c);
		add_xform_variable_accessor(mobius_im_c);
		add_xform_variable_accessor(mobius_re_d);
		add_xform_variable_accessor(mobius_im_d);
	}

	double get_xform_variable ( flam3_xform* xform, QString name )
	{
		QString lookup(name.replace(QChar(' '),QString("_")).toLower());
		xform_variable_accessor* accessor = xform_variable_accessors.value(lookup);
		if (accessor)
			return accessor->get_var(xform);
		else
			logError(QString("Util::get_xform_variable : Unknown variable '%1'").arg(lookup));

		return 0.;
	}


	void set_xform_variable ( flam3_xform* xform, QString name, double value )
	{
		QString lookup(name.replace(QChar(' '),QString("_")).toLower());
		xform_variable_accessor* accessor = xform_variable_accessors.value(lookup);
		if (accessor)
			accessor->set_var(xform, value);
		else
			logError(QString("Util::set_xform_variable : Unknown variable '%1'").arg(lookup));
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

		<< "flux_spread"

		<< "mobius_Re a" << "mobius_Im a"
		<< "mobius_Re b" << "mobius_Im b"
		<< "mobius_Re c" << "mobius_Im c"
		<< "mobius_Re d" << "mobius_Im d"

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
			= flam3_parse_xml2(xml.toLatin1().data(),
				QString("stdin").toLatin1().data(), 1, ncps);
		replace_C_locale(locale);
		return g;
	}

	// set reasonable defaults for a new flam3_genome
	void init_genome(flam3_genome* g)
	{
		clear_cp(g, flam3_defaults_on);

		// set palette to all white and alpha = 1.0
		for (int i = 0 ; i < 256 ; i++)
		{
			flam3_palette_entry* c = g->palette + i;
			c->index = i;
			c->color[0] = 1.0;
			c->color[1] = 1.0;
			c->color[2] = 1.0;
			c->color[3] = 1.0;
		}

		// disable symmetry (flam3.c says 0 means none, but that's not true)
		g->symmetry = 1;
		g->ntemporal_samples = 1;

	}

	// set reasonable defaults for a new flam3_xform
	void init_xform(flam3_xform* xform)
	{
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

	void add_default_xforms(flam3_genome* g, int num)
	{
		int new_idx = g->num_xforms;
		if (g->final_xform_enable == 1)
			new_idx -= 1;

		flam3_add_xforms(g, num, 0, 0);

		int last_idx = g->num_xforms - 1;
		if (g->final_xform_enable == 1)
			last_idx -= 1;

		for (int n = new_idx ; n <= last_idx ; n++)
			init_xform(g->xform + n);
	}

	void rectToPolar(double x, double y, double* r, double* p)
	{
		*r = sqrt(pow(x, 2.) + pow(y, 2.));
		if (x >= 0. && y >= 0.) *p = atan( y / x );
		else if (x < 0. && y >= 0.) *p = M_PI - atan( y / ((-1.)*x) );
		else if (x < 0. && y < 0.) *p = M_PI + atan( y / x );
		else *p = 2.*M_PI - atan( ((-1.)*y) / x );
	}

	void rectToPolarDeg(double x, double y, double* r, double* d)
	{
		rectToPolar(x, y, r, d);
		*d = *d * 180. / M_PI;
	}

	void polarToRect(double r, double p, double* x, double* y)
	{
		*x = r*cos(p);
		*y = r*sin(p);
	}

	void polarDegToRect(double r, double p, double* x, double* y)
	{
		polarToRect( r, p * M_PI / 180., x, y);
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


	// The static initializer routine to populate the static variables used
	// by the functions defined above.
	static const struct util_initializer
	{
		util_initializer()
		{
			for (int n = 0 ; n < flam3_nvariations ; n++)
			{
				char* name = flam3_variation_names[n];

				// populate the variation name->number map
				variation_map.insert(name, n);

				// and add the variation to a list of available names
				variation_name_list.append(name);
			}

			// create the xform variable name accessors
			init_xform_variable_accessors();
		}
	} init;
}


/***************************************************************************
 * Any resemblance of functions, variables, or algorithms described below
 * here to code listed in flam3 source is more than a coincidence.  Most
 * everything was taken from flam3-genome.c and modified to fit our own
 * needs.
 ***************************************************************************/

void Util::spin(flam3_genome* parent, flam3_genome* dest, int frame, double blend)
{
	// Spin the parent blend*360 degrees
	flam3_genome* result = sheep_loop(parent, blend);

	// Set genome parameters
	result->time = (double)frame;
	result->interpolation = flam3_interpolation_linear;
	result->palette_interpolation = flam3_palette_interpolation_hsv;
	flam3_copy(dest, result);
	// Free the cp allocated in flam3_sheep_loop
	clear_cp(result, flam3_defaults_on);
	free(result);
}

void Util::spin_inter(flam3_genome* parents, flam3_genome* dest, int frame, double blend, bool seqflag, double stagger)
{
   // Interpolate between rotated parents
   flam3_genome* result = sheep_edge(parents, blend, (int)seqflag, stagger);

   // Why check for random palettes on both ends?
   if ((parents[0].palette_index != flam3_palette_random) &&
	  (parents[1].palette_index != flam3_palette_random))
	  {
		  result->palette_index = flam3_palette_interpolated;
		  result->palette_index0 = parents[0].palette_index;
		  result->hue_rotation0 = parents[0].hue_rotation;
		  result->palette_index1 = parents[1].palette_index;
		  result->hue_rotation1 = parents[1].hue_rotation;
		  result->palette_blend = blend;
	  }

   // Set genome attributes
   result->time = (double)frame;
   flam3_copy(dest, result);
   // Free genome storage
   clear_cp(result, flam3_defaults_on);
   free(result);
}

flam3_genome* Util::create_genome_sequence(flam3_genome* cp, int ncp, int* dncp, int nframes, int loops, double stagger)
{
	if (nframes <= 0)
	{
		nframes = qMax(nframes, 1);
		logWarn(QString("Util::create_genome_sequence : Setting non-positive value for nframes = %1").arg(nframes));
	}

	int tframes(0);
	tframes = ncp * nframes * loops;
	tframes += (ncp - 1) * nframes + 1;

	flam3_genome* dcp = (flam3_genome*)calloc(tframes, sizeof(flam3_genome));
	if (dcp == NULL)
	{
		logError(QString("Util::create_genome_sequence : Couldn't calloc %1 genome structures").arg(tframes));
		return 0;
	}

	int framecount(0);
	int seqflag;
	double blend;
	for (int i = 0 ; i < ncp ; i++)
	{
		for (int n = 0 ; n < loops ; n++)
			for (int frame = 0; frame < nframes; frame++)
			{
				blend = frame / (double)nframes;
				spin(&cp[i], &dcp[framecount], framecount, blend);
				framecount++;
			}

		if (i < ncp - 1)
			for (int frame = 0; frame < nframes; frame++)
			{
				if (0 == frame || (( nframes - 1 ) == frame))
					seqflag = true;
				else
					seqflag = false;
				blend = frame / (double)nframes;
				spin_inter(&cp[i], &dcp[framecount], framecount, blend, seqflag, stagger);
				framecount++;
			}
	}
	spin(&cp[ncp - 1], &dcp[framecount], framecount, 0.0);
	*dncp = framecount + 1;
	return dcp;
}

flam3_genome* Util::create_genome_interpolation(flam3_genome* cp, int ncp, int* dncp, double stagger)
{
	for (int i = 0 ; i < ncp ; i++)
	{
		if (i > 0 && cp[i].time <= cp[i-1].time)
		{
			logWarn(QString("error: control points must be sorted by time, but %1 <= %2, index %3")
					.arg(cp[i].time).arg(cp[i-1].time).arg(i));
			return NULL;
		}
		/* Strip out all motion elements here.  Why? */
//		for (int j = 0 ; j < cp[i].num_xforms ; j++)
//			flam3_delete_motion_elements(&cp[i].xform[j]);
	}

	int first_frame = (int) cp[0].time;
	int last_frame  = (int) cp[ncp-1].time;
	if (last_frame < first_frame)
		last_frame = first_frame;
	int tframes = last_frame - first_frame + 1;

	flam3_genome* dcp = (flam3_genome*)calloc(tframes, sizeof(flam3_genome));
	if (dcp == NULL)
	{
		logError(QString("Util::create_genome_interp : Couldn't calloc %1 genome structures").arg(tframes));
		return 0;
	}

	int framecount = 0;
	for (int ftime = first_frame ; ftime <= last_frame ; ftime += 1)
	{
		int iscp = 0;
		for (int i = 0; i < ncp ; i++)
		{
			if (ftime == cp[i].time)
			{
				flam3_copy(dcp + (framecount++), cp + i);
				iscp = 1;
			}
		}
		if (iscp == 0)
		{
			flam3_interpolate(cp, ncp, (double)ftime, stagger, dcp + (framecount++));
			for (int i = 0 ; i < ncp ; i++)
			{
				if ( ftime == cp[i].time - 1 )
					iscp = 1;
			}
			if (iscp == 0)
				(dcp + framecount)->interpolation_type = flam3_inttype_linear;
		}

	}

	// reset the time for each frame so flam3_render can find each frame
	for (int n = 0 ; n < framecount ; n++)
		(dcp + n)->time = n;
	*dncp = framecount;
	return dcp;
}
