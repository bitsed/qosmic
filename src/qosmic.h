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
#ifndef QOSMIC_H
#define QOSMIC_H

#include <QString>
#include <QDir>

#ifndef VERSION
#define VERSION "unknown"
#endif

#ifndef SCRIPTSDIR
#define SCRIPTSDIR ":/unknown"
#endif

static const QString QOSMIC_VERSION( VERSION );
static const QString QOSMIC_SCRIPTSDIR( SCRIPTSDIR );
static const QString QOSMIC_USERDIR( QDir::home().absoluteFilePath(".qosmic") );
static const QString QOSMIC_AUTOSAVE( QOSMIC_USERDIR + "/autosave.flam3" );

static const QString DEFAULT_FLAME_XML(
"<flame time=\"0\" palette=\"27\" size=\"1280 960\" center=\"0.0 0.0\" "
	"scale=\"100.0\" oversample=\"1\" filter=\"0.0\" quality=\"40\" "
	"passes=\"1\" temporal_samples=\"1\" estimator_radius=\"0\" "
	"brightness=\"8.0\" gamma=\"4\" interpolation_type=\"linear\">\n"
	"<xform weight=\"1.0\" color=\"1.0\" linear=\"1\" coefs=\"1 0 0 1 0 0\"/>\n"
"</flame>");

#endif
