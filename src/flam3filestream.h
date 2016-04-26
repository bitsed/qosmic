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
#ifndef FLAM3FILESTREAM_H
#define FLAM3FILESTREAM_H

#include <QFile>

#include "genomevector.h"

class Flam3FileStream
{
	QFile* m_file;

	public:
		Flam3FileStream(QFile*);
		bool read(GenomeVector*);
		bool read(flam3_genome**, int*);
		bool write(GenomeVector*);
		bool write(flam3_genome*, int);
		void setFile(QFile*);
		QFile* file() const;
		Flam3FileStream& operator>>(GenomeVector*);
		Flam3FileStream& operator<<(GenomeVector*);

		static void autoSave(GenomeVector*, int =GenomeVector::AlwaysSave);
};

#endif // FLAM3FILESTREAM_H
