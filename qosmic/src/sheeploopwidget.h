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

#ifndef SHEEPLOOPWIDGET_H
#define SHEEPLOOPWIDGET_H

#include <QStandardItem>
#include <QStyledItemDelegate>

#include "ui_sheeploopwidget.h"
#include "genomevector.h"
#include "qosmicwidget.h"


/**
 * An item delegate that provides the widgets that edit the xform motion
 * element parameters.
 */
class MotionViewItemDelegate : public QStyledItemDelegate
{
	Q_OBJECT

	QTableView* m_tableview;


	public:
		MotionViewItemDelegate(QTableView* =0);
		QWidget* createEditor(QWidget*, const QStyleOptionViewItem&, const QModelIndex&) const;
		void setEditorData(QWidget*, const QModelIndex&) const;
		void setModelData(QWidget*, QAbstractItemModel*, const QModelIndex&) const;
		QTableView* view() const;
};


/**
 * A set of widgets that allow proper mouse event handling for widgets returned
 * by the MotionItemDelegate.
 */

class MotionViewVarItemEditor : public QComboBox
{
	Q_OBJECT

	const MotionViewItemDelegate* m_delegate;

	public:
		MotionViewVarItemEditor(const MotionViewItemDelegate*, QWidget* =0);

	private slots:
		void comboBoxCurrentIndexChanged(int);

};

class MotionViewFunctionItemEditor : public QComboBox
{
	Q_OBJECT

	const MotionViewItemDelegate* m_delegate;

	public:
		MotionViewFunctionItemEditor(const MotionViewItemDelegate*, QWidget* =0);

	private slots:
		void comboBoxCurrentIndexChanged(int);
};

class MotionViewDoubleItemEditor : public DoubleValueEditor
{
	Q_OBJECT

	const MotionViewItemDelegate* m_delegate;

	public:
		MotionViewDoubleItemEditor(const MotionViewItemDelegate*, QWidget* =0);

	private slots:
		void spinnerValueChanged();
};

class MotionViewIntItemEditor : public IntValueEditor
{
	Q_OBJECT

	const MotionViewItemDelegate* m_delegate;

	public:
		MotionViewIntItemEditor(const MotionViewItemDelegate*, QWidget* =0);

	private slots:
		void spinnerValueChanged();
};


class SheepLoopWidget : public QWidget, public QosmicWidget, private Ui::SheepLoopWidget
{
	Q_OBJECT

	GenomeVector* genomes;
	bool running; // signal the runSheepLoop routine to start/stop sequence

	public:
		enum AnimationMode { Sequence, Interpolate };

		explicit SheepLoopWidget(GenomeVector*, QWidget* parent=0);
		int beginIdx() const;
		int endIdx() const;
		int frames() const;
		int interpolation() const;
		int interpolationType() const;
		int temporalSamples() const;
		int temporalFilterType() const;
		double temporalFilterWidth() const;
		double temporalFilterExp() const;
		double stagger() const;
		int loops() const;
		int paletteInterpolation() const;
		int paletteMode() const;
		AnimationMode animationMode() const;
		flam3_genome* createSheepLoop(int&);

	public slots:
		void genomeSelectedSlot(int);
		void genomesModifiedSlot();
		void runSheepButtonAction();
		void reset();

	signals:
		void runSheepLoop(bool);
		void saveSheepLoop();

	protected:
		void changeEvent(QEvent*);
		void showEvent(QShowEvent*);
		void hideEvent(QHideEvent*);

	private slots:
		void tabWidgetIndexChanged(int);
		void beginBoxIndexChanged(int);
		void endBoxIndexChanged(int);
		void temporalSamplesUpdated();
		void addNewMotionElement();
		void delCurrentMotionElement();
		void motionItemChanged(QStandardItem*);
		void xformIdxBoxIndexChanged(int);
		void modeBoxIndexChanged(int);
		void temporalFilterTypeIndexChanged(int);
		void xformAnimateButtonClicked(bool);
};

#endif // SHEEPLOOPWIDGET_H
