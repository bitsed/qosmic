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
#ifndef UNDORING_H
#define UNDORING_H

#include <QPolygonF>

#include "flam3util.h"

#define UNDORING_SIZE 50

class UndoState
{
	public:
		flam3_genome Genome;
		QPolygonF SelectionRect;
		int SelectedType;
		QList<int> NodesO;
		QList<int> NodesX;
		QList<int> NodesY;
		QList<int> Triangles;
		QPointF MarkPos;

		UndoState();
		UndoState(const UndoState&);
		UndoState& operator=(const UndoState&);
		void clear();
};

class UndoStateProvider
{
	public:
		virtual void provideState(UndoState*) =0;
		virtual void restoreState(UndoState*) =0;
};

class UndoRing
{
	qint64 head;
	qint64 current;

	UndoState ring[UNDORING_SIZE];

	public:
		UndoRing();
		UndoRing(const UndoRing&);
		UndoRing& operator=(const UndoRing&);
		~UndoRing();
		UndoState* prev();
		UndoState* next();
		UndoState* advance();
		UndoState* currentState();
		void clear();
		int index();
		int size();
		bool atHead();
		bool atTail();
};

#endif
