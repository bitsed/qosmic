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

#ifndef FLAM3_UTIL
#define FLAM3_UTIL

#include <QTextStream>
#include <QColor>

#undef VERSION
extern "C" {
#include "flam3.h"
}


namespace Util
{
	QColor get_xform_color(flam3_genome*, flam3_xform*);
	void set_xform_variable(flam3_xform*, QString, double);
	double get_xform_variable(flam3_xform*, QString);
	QStringList& get_variable_names();
	int variation_number(const char*);
	int variation_number(const QString&);
	void write_to_file(FILE*, flam3_genome*, char*, int);
	flam3_genome* read_from_file(FILE*, char*, int, int*);
	flam3_genome* read_xml_string(QString, int*);
	void init_genome(flam3_genome*);
	void init_xform(flam3_xform*);
	void add_default_xforms(flam3_genome*, int num=1);
	const QMap<QString, int>& flam3_variations();
	const QStringList& variation_names();
	char* setup_C_locale();
	void replace_C_locale(char*);

	flam3_genome* create_genome_sequence(flam3_genome* cp, int ncp, int* dncp, int nframes=100, int loops=1, double stagger=0.0);
	flam3_genome* create_genome_interpolation(flam3_genome* cp, int ncp, int* dncp, double stagger=0.0);
	void spin(flam3_genome* cp, flam3_genome* dcp, int framecount, double blend);
	void spin_inter(flam3_genome* cp, flam3_genome* dcp, int framecount, double blend, bool seqflag, double stagger);

	void rectToPolar(double, double, double*, double*);
	void rectToPolarDeg(double, double, double*, double*);
	void polarToRect(double, double, double*, double*);
	void polarDegToRect(double, double, double*, double*);

	randctx* get_isaac_randctx();
}

/**
 * These are some nice output operators for the flam3_frame
 * and flam3_genome types.
 */
QTextStream& operator<<( QTextStream&, flam3_genome& );
QTextStream& operator<<( QTextStream&, flam3_frame& );

#endif

