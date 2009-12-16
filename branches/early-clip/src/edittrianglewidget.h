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

#ifndef EDITTRIANGLEWIDGET_H
#define EDITTRIANGLEWIDGET_H


#include "ui_edittrianglewidget.h"
#include "qosmicwidget.h"

class Triangle;
class FigureEditor;
class GenomeVector;

class EditTriangleWidget : public QWidget, public QosmicWidget,
	private Ui::EditTriangleWidget
{
	Q_OBJECT

	public:
		EditTriangleWidget(GenomeVector*, FigureEditor*, QWidget* parent=0);
		QPointF triangleTransformPos();
		QPointF selectionTransformPos();
		void reset();

	signals:
		void dataChanged();
		void undoStateSignal();

	public slots:
		void triangleSelectedSlot(Triangle*);

	protected slots:
		void triangleScaledAction();
		void triangleRotatedAction();
		void triangleHFlippedAction();
		void triangleVFlippedAction();
		void moveTriangleLeftAction();
		void moveTriangleRightAction();
		void moveTriangleUpAction();
		void moveTriangleDownAction();
		void rotateTextEditedSlot();
		void scaleTextEditedSlot();
		void toggleMarkAction(bool);
		void selectedItemsChangedAction();

	protected:
		void closeEvent(QCloseEvent*);

	private:
		GenomeVector* genome;
		FigureEditor* editor;
		Triangle* selectedTriangle;
		double rotate_dx;
		double scale_dx;

		QButtonGroup* m_transformPosGroup;
		QButtonGroup* m_selectionTypeGroup;

		enum TransformPosition { Origin, Center, NodeO, NodeX, NodeY, Mark };
};


#endif

