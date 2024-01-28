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
#include "undoring.h"
#include "logger.h"

UndoRing::UndoRing()
{
	logFiner("UndoRing::UndoRing() : enter");
	current = head = -1;
}

UndoRing::UndoRing(const UndoRing& in)
{
	logFiner("UndoRing::UndoRing(&copy) : enter");
	current = head = -1;
	*this = in;
}

UndoRing& UndoRing::operator=(const UndoRing& in)
{
	for (int n = 0 ; n < UNDORING_SIZE ; n++)
		ring[n] = in.ring[n];
	current = in.current;
	head = in.head;
	return *this;
}

UndoRing::~UndoRing()
{
	clear();
}

UndoState* UndoRing::next()
{
	if (current < head)
		current++;
	logFine(QString("UndoRing::next : current %1 / [%2,%3]")
			.arg(current)
			.arg(head - UNDORING_SIZE)
			.arg(head));
	return ring + (current % UNDORING_SIZE);
}

UndoState* UndoRing::prev()
{
	if (current > 0)
	{
		if (current > (head - UNDORING_SIZE + 1))
			current--;
		logFine(QString("UndoRing::prev : current %1 / [%2,%3]")
			.arg(current)
			.arg(head - UNDORING_SIZE)
			.arg(head));
		return ring + (current % UNDORING_SIZE);
	}
	return ring;
}

UndoState* UndoRing::advance()
{
	current++;
	head = current;
	logFine(QString("UndoRing::advance : current %1").arg(current));
	return ring + (current % UNDORING_SIZE);
}

UndoState* UndoRing::currentState()
{
	return ring + (current % UNDORING_SIZE);
}

void UndoRing::clear()
{
	logFiner(QString("UndoRing::clear : enter"));
	for (int n = 0 ; n < UNDORING_SIZE ; n++)
		(ring + n)->clear();
	current = head = -1;
}

bool UndoRing::atHead()
{
	return current == head;
}

bool UndoRing::atTail()
{
	return current == (head - UNDORING_SIZE + 1) || current <= 0;
}

int UndoRing::index()
{
	return (head - current + 1);
}

int UndoRing::size()
{
	return UNDORING_SIZE;
}


UndoState::UndoState() : Genome()
{
	clear();
}

UndoState::UndoState(const UndoState& in) : Genome()
{
	clear();
	*this = in;
}

UndoState& UndoState::operator=(const UndoState& in)
{
	flam3_copy(&Genome, (flam3_genome*)&(in.Genome)); // danger?
	SelectionRect = in.SelectionRect;
	SelectedType  = in.SelectedType;
	NodesO = in.NodesO;
	NodesX = in.NodesX;
	NodesY = in.NodesY;
	Triangles = in.Triangles;
	MarkPos   = in.MarkPos;
	return *this;
}

void UndoState::clear()
{
	clear_cp(&Genome, flam3_defaults_on);
	SelectionRect = QPolygonF();
	MarkPos = QPointF();
	NodesO.clear();
	NodesX.clear();
	NodesY.clear();
}
