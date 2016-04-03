/***************************************************************************
 *   Copyright (C) 2007-2016 by David Bitseff                              *
 *   bitsed@gmail.com                                                      *
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
#ifndef TRIANGLEDENSITYWIDGET_H
#define TRIANGLEDENSITYWIDGET_H

#include <QSlider>
#include <QLabel>
#include <QMouseEvent>

#include "ui_triangledensitywidget.h"
#include "qosmicwidget.h"
#include "flam3util.h"
#include "xfedit.h"


// this is an interface the TriangleDensityWidget implements to use the sliders.
class SliderGroup
{
	public:
		SliderGroup() {}
		virtual ~SliderGroup() {}
		virtual void sliderGroupChanged(int idx)=0;
};

class GroupedSlider : public QSlider
{
	Q_OBJECT

	int m_idx;
	SliderGroup* m_group;
	QPointF m_pos;

	public:
		GroupedSlider(Qt::Orientation o, SliderGroup* p, int idx)
			: QSlider(o), m_idx(idx), m_group(p)
		{
		}
		void sliderChange(SliderChange change)
		{
			if (change & QAbstractSlider::SliderValueChange)
				m_group->sliderGroupChanged(m_idx);
			QSlider::sliderChange(change);
		}
		void mousePressEvent(QMouseEvent* event)
		{
			m_pos = event->globalPos();
			m_group->sliderGroupChanged(m_idx);
			QSlider::mousePressEvent(event);
		}
		void mouseReleaseEvent(QMouseEvent* event)
		{
			if (m_pos != event->globalPos())
				emit undoStateSignal();
			QSlider::mouseReleaseEvent(event);
		}

	signals:
		void undoStateSignal();
};



class TriangleDensityWidget
	: public QWidget, public QosmicWidget, public SliderGroup,
	private Ui::TriangleDensityWidget
{
	Q_OBJECT

	Triangle* selectedTriangle;
	FigureEditor* triangleScene;
	GenomeVector* genome;
	QList<GroupedSlider*> sliders;
	QList<QLabel*> slider_names;
	int grouped_slider_idx;

	flam3_genome  other_genome;
	flam3_genome  merged_genome;
	QList<GroupedSlider*> merged_sliders;
	QList<QLabel*> merged_slider_names;
	int merged_grouped_slider_idx;
	bool is_merged;

	public:
		TriangleDensityWidget(GenomeVector*, FigureEditor*, QWidget* =0);
		~TriangleDensityWidget();
		void sliderGroupChanged(int);
		flam3_genome* getMergedGenome();
		bool hasMergedGenome();

	signals:
		void dataChanged();
		void genomeMerged();
		void undoStateSignal();

	public slots:
		void triangleSelectedSlot(Triangle*);
		void distToolButtonSlot(QAbstractButton*);
		void distToolButtonSlot_2(QAbstractButton*);
		void reset();

	protected slots:
		void groupedSliderChangedSlot(int);
		void groupedSliderPressedSlot();
		void groupedMergedSliderChangedSlot(int);
		void mergeComboBoxChangedAction(int);
		void mergeWithOtherGenomeAction();

	private:
		void resetSliders(double);
		double getNorm();
};


#endif
