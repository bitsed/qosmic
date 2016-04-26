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

#include "qosmic.h"
#include "flam3filestream.h"
#include "logger.h"

Flam3FileStream::Flam3FileStream(QFile* f)
: m_file(f)
{
}

QFile* Flam3FileStream::file() const
{
	return m_file;
}

void Flam3FileStream::setFile(QFile* f)
{
	m_file = f;
}

bool Flam3FileStream::read(GenomeVector* genomes)
{
	int ncps(0);
	flam3_genome* in;
	if (read(&in, &ncps))
	{
		genomes->setData(in, ncps);
		return true;
	}
	return false;
}

Flam3FileStream& Flam3FileStream::operator>>(GenomeVector* genomes)
{
	int ncps(0);
	flam3_genome* in;
	if (read(&in, &ncps))
		genomes->insertRows(genomes->size(), ncps, in);
	return *this;
}

bool Flam3FileStream::read(flam3_genome** in, int* ncps)
{
	if (!m_file->open(QIODevice::ReadOnly))
		return false;

	FILE* fd = fdopen(m_file->handle(), "r");
	// 'in' points to a static defined in flam3.c
	*in = Util::read_from_file(fd, m_file->fileName().toLatin1().data(), 1, ncps);
	fclose(fd);
	m_file->close();
	if (*ncps < 1)
		return false;

	// sanitize the genomes to avoid strange behavior and problems with libflam3
	for (int n = 0 ; n < *ncps ; n++)
	{
		flam3_genome* g = *in + n;
		g->symmetry = 1;           // clear genome symmetry flag
		g->ntemporal_samples = 1;  // temporal_samples is only for animations
		g->interpolation = flam3_interpolation_linear; // animation interp
	}

	return true;
}

bool Flam3FileStream::write(GenomeVector* genomes)
{
	return write(genomes->data(), genomes->size());
}

Flam3FileStream& Flam3FileStream::operator<<(GenomeVector* genomes)
{
	write(genomes);
	return *this;
}

bool Flam3FileStream::write(flam3_genome* genomes, int ngenomes)
{
	if (ngenomes < 1)
	{
		logWarn("Flam3FileStream::write : cannot write < 1 genome");
		return false;
	}

	if (!m_file->open(QIODevice::WriteOnly))
		return false;

	logInfo(QString("Flam3FileStream::write : writing %1 genomes to '%2'")
			.arg(ngenomes).arg(m_file->fileName()));
	FILE* fd = fdopen(m_file->handle(), "w"); // check for error again?
	if (ngenomes > 1)
	{
		char attrs[] = "";
		fprintf(fd, "<qstack>\n");
		for (int i = 0 ; i < ngenomes ; i++)
		{
			flam3_genome* g = genomes + i;
			int n = g->symmetry;
			g->symmetry = 0;
			Util::write_to_file(fd, g, attrs, 0);
			g->symmetry = n;
		}
		fprintf(fd, "</qstack>\n");
	}
	else
	{
		// always clear the genome symmetry since the parser modifies the
		// genome when reading this tag.
		char attrs[] = "";
		int n = genomes[0].symmetry;
		genomes[0].symmetry = 0;
		Util::write_to_file(fd, genomes, attrs, 0);
		genomes[0].symmetry = n;
	}

	int rv = fclose(fd);
	m_file->close();
	return rv == 0;
}

/**
 * A static method that saves the given genome to the autosave file.
 * The type argument gives the conditions for performing a save, and they
 * must match the current settings for the GenomeVector as set by the user.
 */
void Flam3FileStream::autoSave(GenomeVector* genomes, int type)
{
	if (genomes->autoSave() & type)
	{
		QFile file(QOSMIC_AUTOSAVE);
		Flam3FileStream s(&file);
		s.write(genomes);
	}
}
