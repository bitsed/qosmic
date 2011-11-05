/***************************************************************************
 *   Copyright (C) 2007, 2008, 2009, 2011 by David Bitseff                 *
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
#include <QStandardItemModel>
#include <QListView>

#include "sheeploopwidget.h"
#include "mainpreviewwidget.h"
#include "logger.h"

//------------------------------------------------------------------------------

MotionViewVarItemEditor::MotionViewVarItemEditor(const MotionViewItemDelegate* d, QWidget* p)
: QComboBox(p), m_delegate(d)
{
	QStringList items;
	items << "animate" << "color" << "color_speed" << "density" << "opacity";
	foreach (QString s, Util::variation_names())
		items << QString("var: ") + s;
	foreach (QString s, Util::get_variable_names())
		items << QString("par: ") + s;
	items << "xform: a" << "xform: b" << "xform: c" << "xform: d" << "xform: e" << "xform: f"
		  << "post: a" << "post: b" << "post: c" << "post: d" << "post: e" << "post: f";
	addItems(items);
	connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(comboBoxCurrentIndexChanged(int)));
}

void MotionViewVarItemEditor::comboBoxCurrentIndexChanged(int /*idx*/)
{
	QTableView* view = m_delegate->view();
	m_delegate->setModelData(this, view->model(), view->currentIndex());
}

//------------------------------------------------------------------------------

MotionViewFunctionItemEditor::MotionViewFunctionItemEditor(const MotionViewItemDelegate* d, QWidget* p)
: QComboBox(p), m_delegate(d)
{
	addItems(QStringList() << "sin" << "hill" << "triangle");
	connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(comboBoxCurrentIndexChanged(int)));
}

void MotionViewFunctionItemEditor::comboBoxCurrentIndexChanged(int /*idx*/)
{
	QTableView* view = m_delegate->view();
	m_delegate->setModelData(this, view->model(), view->currentIndex());
}

//------------------------------------------------------------------------------

MotionViewIntItemEditor::MotionViewIntItemEditor(const MotionViewItemDelegate* d, QWidget* p)
: IntValueEditor(p), m_delegate(d)
{
	setMinimum(1);
	connect(this, SIGNAL(valueUpdated()), this, SLOT(spinnerValueChanged()));
}

void MotionViewIntItemEditor::spinnerValueChanged()
{
	QTableView* view = m_delegate->view();
	m_delegate->setModelData(this, view->model(), view->currentIndex());
}

//------------------------------------------------------------------------------

MotionViewDoubleItemEditor::MotionViewDoubleItemEditor(const MotionViewItemDelegate* d, QWidget* p)
: DoubleValueEditor(p), m_delegate(d)
{
	setDecimals(3);
	setMinimum(0.001);
	setSingleStep(0.1);
	connect(this, SIGNAL(valueUpdated()), this, SLOT(spinnerValueChanged()));
}

void MotionViewDoubleItemEditor::spinnerValueChanged()
{
	QTableView* view = m_delegate->view();
	m_delegate->setModelData(this, view->model(), view->currentIndex());
}

//------------------------------------------------------------------------------

MotionViewItemDelegate::MotionViewItemDelegate(QTableView* parent)
: QStyledItemDelegate(parent), m_tableview(parent)
{
}

QTableView* MotionViewItemDelegate::view() const
{
	return m_tableview;
}

QWidget* MotionViewItemDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	if (index.column() == 0)
		return new MotionViewVarItemEditor(this, parent);

	else if (index.column() == 1)
		return new MotionViewDoubleItemEditor(this, parent);

	else if (index.column() == 2)
		return new MotionViewFunctionItemEditor(this, parent);

	else if (index.column() == 3)
		return new MotionViewIntItemEditor(this, parent);

	else
		logWarn("MotionViewItemDelegate::createEditor : no editor created");

	return QStyledItemDelegate::createEditor(parent, option, index);
}

void MotionViewItemDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
	if ((index.column() == 0 || index.column() == 2) && editor->inherits("QComboBox"))
	{
		QComboBox* box = qobject_cast<QComboBox*>(editor);
		box->setCurrentIndex(box->findText(index.data().toString(), Qt::MatchEndsWith | Qt::MatchCaseSensitive));
	}
	else if (index.column() == 1 && editor->inherits("QAbstractSpinBox"))
	{
		DoubleValueEditor* e = qobject_cast<DoubleValueEditor*>(editor);
		e->setValue(qMax(0.001, index.data().toDouble()));
	}
	else if (index.column() == 3 && editor->inherits("QAbstractSpinBox"))
	{
		IntValueEditor* e = qobject_cast<IntValueEditor*>(editor);
		e->setValue(qMax(1, index.data().toInt()));
	}
	else
		logWarn(QString("MotionViewItemDelegate::setEditorData : unknown editor %1")
				.arg(editor->objectName()));
}

void MotionViewItemDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index ) const
{
	logFine(QString("MotionViewItemDelegate::setModelData : %1,%2").arg(index.row()).arg(index.column()));
	QStandardItemModel* m = qobject_cast<QStandardItemModel*>(model);
	if (index.column() == 0 || index.column() == 2)
	{
		QComboBox* box = qobject_cast<QComboBox*>(editor);
		m->itemFromIndex(index)->setText(box->currentText());
	}
	else if (index.column() == 1)
	{
		DoubleValueEditor* e = qobject_cast<DoubleValueEditor*>(editor);
		m->itemFromIndex(index)->setText(QString::number(e->value()));
	}
	else if (index.column() == 3)
	{
		IntValueEditor* e = qobject_cast<IntValueEditor*>(editor);
		m->itemFromIndex(index)->setText(QString::number(e->value()));
	}
}

//------------------------------------------------------------------------------

SheepLoopWidget::SheepLoopWidget(GenomeVector* gv, QWidget* parent) :
	QWidget(parent), QosmicWidget(this, objectName()), genomes(gv), running(false)
{
	setupUi(this);

	connect(m_runToolButton, SIGNAL(clicked()), this, SLOT(runSheepButtonAction()));
	connect(m_saveToolButton, SIGNAL(clicked()), this, SIGNAL(saveSheepLoop()));
	connect(m_beginBox, SIGNAL(currentIndexChanged(int)), this, SLOT(beginBoxIndexChanged(int)));
	connect(m_endBox, SIGNAL(currentIndexChanged(int)), this, SLOT(endBoxIndexChanged(int)));
	connect(m_temporalSamplesEditor, SIGNAL(valueUpdated()), this, SLOT(temporalSamplesUpdated()));
	connect(m_animateButton, SIGNAL(clicked(bool)), this, SLOT(xformAnimateButtonClicked(bool)));

	QStandardItemModel* model = new QStandardItemModel(0, 4);
	m_motionElementsView->setModel(model);
	m_motionElementsView->setItemDelegate(new MotionViewItemDelegate(m_motionElementsView));

	m_beginBox->setModel(gv);
	m_endBox->setModel(gv);
	m_genomeIdxBox->setModel(gv);

	qobject_cast<QListView*>(m_beginBox->view())->setSpacing(2);
	qobject_cast<QListView*>(m_endBox->view())->setSpacing(2);
	qobject_cast<QListView*>(m_genomeIdxBox->view())->setSpacing(2);

	connect(m_addToolButton, SIGNAL(clicked()), this, SLOT(addNewMotionElement()));
	connect(m_delToolButton, SIGNAL(clicked()), this, SLOT(delCurrentMotionElement()));
	connect(m_xformIdxBox, SIGNAL(currentIndexChanged(int)), this, SLOT(xformIdxBoxIndexChanged(int)));
	connect(m_genomeIdxBox, SIGNAL(currentIndexChanged(int)), this, SLOT(genomeSelectedSlot(int)));
	connect(m_animateModeBox, SIGNAL(currentIndexChanged(int)), this, SLOT(modeBoxIndexChanged(int)));
	connect(m_temporalFilterComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(temporalFilterTypeIndexChanged(int)));
	connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabWidgetIndexChanged(int)));
	connect(model, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(motionItemChanged(QStandardItem*)));
}

void SheepLoopWidget::xformIdxBoxIndexChanged(int idx)
{
	int genome_idx = qMax(0, m_genomeIdxBox->currentIndex());
	flam3_genome* genome = genomes->data() + genome_idx;
	if (!genome || idx >= genome->num_xforms || genome_idx < 0 || genome_idx >= genomes->size())
	{
		logWarn("SheepLoopWidget::xformIdxBoxIndexChanged : no xform %d in genome %d", idx, genome_idx);
		return;
	}
	QStandardItemModel* model = qobject_cast<QStandardItemModel*>(m_motionElementsView->model());
	disconnect(model, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(motionItemChanged(QStandardItem*)));
	flam3_xform* xform = genome->xform + idx;
	m_animateButton->setChecked(xform->animate);
	model->clear();
	model->setHorizontalHeaderLabels(QStringList()
	<< QString("element")
	<< QString("value")
	<< QString("function")
	<< QString("frequency"));
	connect(model, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(motionItemChanged(QStandardItem*)));
	for (int n = 0 ; n < xform->num_motion ; n++)
	{
		int m_idx = n;
		flam3_xform* motion = xform->motion + m_idx;
		int var_idx = -1;
		for (int i = 0 ; i < flam3_nvariations ; i++)
			if (motion->var[i] != 0.0)
			{
				var_idx = i;
				break;
			}
		if (var_idx != -1)
		{
			model->setItem(n, 0, new QStandardItem(QString("var: ") + Util::variation_names().at(var_idx)));
			model->setItem(n, 1, new QStandardItem(QString::number(motion->var[var_idx])));
		}
		else
		{
			bool found_val = false;
			foreach(QString s, Util::get_variable_names())
			{
				double val = Util::get_xform_variable(motion, s);
				if (val != 0.0)
				{
					model->setItem(n, 0, new QStandardItem(QString("par: ") + s));
					model->setItem(n, 1, new QStandardItem(QString::number(val)));
					found_val = true;
					break;
				}
			}
			if (!found_val)
			{
				if (motion->animate != 0.0)
				{
					model->setItem(n, 0, new QStandardItem(QString("animate")));
					model->setItem(n, 1, new QStandardItem(QString::number(motion->animate)));
				}
				else if (motion->color != 0.0)
				{
					model->setItem(n, 0, new QStandardItem(QString("color")));
					model->setItem(n, 1, new QStandardItem(QString::number(motion->color)));
				}
				else if (motion->color_speed != 0.0)
				{
					model->setItem(n, 0, new QStandardItem(QString("color_speed")));
					model->setItem(n, 1, new QStandardItem(QString::number(motion->color_speed)));
				}
				else if (motion->density != 0.0)
				{
					model->setItem(n, 0, new QStandardItem(QString("density")));
					model->setItem(n, 1, new QStandardItem(QString::number(motion->density)));
				}
				else if (motion->opacity != 0.0)
				{
					model->setItem(n, 0, new QStandardItem(QString("opacity")));
					model->setItem(n, 1, new QStandardItem(QString::number(motion->opacity)));
				}
				else if (motion->c[0][0] != 0.0)
				{
					model->setItem(n, 0, new QStandardItem(QString("xform: a")));
					model->setItem(n, 1, new QStandardItem(QString::number(motion->c[0][0])));
				}
				else if (motion->c[1][0] != 0.0)
				{
					model->setItem(n, 0, new QStandardItem(QString("xform: b")));
					model->setItem(n, 1, new QStandardItem(QString::number(motion->c[1][0])));
				}
				else if (motion->c[2][0] != 0.0)
				{
					model->setItem(n, 0, new QStandardItem(QString("xform: c")));
					model->setItem(n, 1, new QStandardItem(QString::number(motion->c[2][0])));
				}
				else if (motion->c[0][1] != 0.0)
				{
					model->setItem(n, 0, new QStandardItem(QString("xform: d")));
					model->setItem(n, 1, new QStandardItem(QString::number(motion->c[0][1])));
				}
				else if (motion->c[1][1] != 0.0)
				{
					model->setItem(n, 0, new QStandardItem(QString("xform: e")));
					model->setItem(n, 1, new QStandardItem(QString::number(motion->c[1][1])));
				}
				else if (motion->c[2][1] != 0.0)
				{
					model->setItem(n, 0, new QStandardItem(QString("xform: f")));
					model->setItem(n, 1, new QStandardItem(QString::number(motion->c[2][1])));
				}
				else if (motion->post[0][0] != 0.0)
				{
					model->setItem(n, 0, new QStandardItem(QString("post: a")));
					model->setItem(n, 1, new QStandardItem(QString::number(motion->post[0][0])));
				}
				else if (motion->post[1][0] != 0.0)
				{
					model->setItem(n, 0, new QStandardItem(QString("post: b")));
					model->setItem(n, 1, new QStandardItem(QString::number(motion->post[1][0])));
				}
				else if (motion->post[2][0] != 0.0)
				{
					model->setItem(n, 0, new QStandardItem(QString("post: c")));
					model->setItem(n, 1, new QStandardItem(QString::number(motion->post[2][0])));
				}
				else if (motion->post[0][1] != 0.0)
				{
					model->setItem(n, 0, new QStandardItem(QString("post: d")));
					model->setItem(n, 1, new QStandardItem(QString::number(motion->post[0][1])));
				}
				else if (motion->post[1][1] != 0.0)
				{
					model->setItem(n, 0, new QStandardItem(QString("post: e")));
					model->setItem(n, 1, new QStandardItem(QString::number(motion->post[1][1])));
				}
				else if (motion->post[2][1] != 0.0)
				{
					model->setItem(n, 0, new QStandardItem(QString("post: f")));
					model->setItem(n, 1, new QStandardItem(QString::number(motion->post[2][1])));
				}
			}
		}
		if (motion->motion_func == MOTION_SIN)
			model->setItem(n, 2, new QStandardItem("sin"));
		else if (motion->motion_func == MOTION_HILL)
			model->setItem(n, 2, new QStandardItem("hill"));
		else
			model->setItem(n, 2, new QStandardItem("triangle"));
		model->setItem(n, 3, new QStandardItem(QString::number(motion->motion_freq)));
	}
	m_motionElementsView->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
	m_motionElementsView->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
}

void SheepLoopWidget::motionItemChanged(QStandardItem* item)
{
	int xf_idx = m_xformIdxBox->currentIndex();
	int genome_idx = m_genomeIdxBox->currentIndex();
	flam3_genome* genome = genomes->data() + genome_idx;
	flam3_xform* xform = genome->xform + xf_idx;
	int row = item->row();
	int col = item->column();
	logFine(QString("SheepLoopWidget::motionItemChanged %1,%2").arg(row).arg(col));
	flam3_xform* motion = xform->motion + row;
	QStandardItemModel* model = qobject_cast<QStandardItemModel*>(m_motionElementsView->model());
	if (col == 0)
	{
		QStandardItem* i = model->item(row, 1);
		if (i)
		{
			flam3_xform tmp = *motion;
			memset(motion, 0, sizeof(flam3_xform));
			QString name = item->text();
			double value = i->data(Qt::DisplayRole).toDouble();
			if (name.startsWith("var: "))
			{
				name = name.remove(0,5);
				logFine(QString("SheepLoopWidget::motionItemChanged setting var '%1'").arg(name));
				motion->var[Util::flam3_variations().value(name)] = value;
			}
			else if (name.startsWith("par: "))
			{
				name = name.remove(0,5);
				Util::set_xform_variable(motion, name, value);
			}
			else if (name == "animate")
				motion->animate = value;
			else if (name == "color")
				motion->color = value;
			else if (name == "color_speed")
				motion->color_speed = value;
			else if (name == "density")
				motion->density = value;
			else if (name == "opacity")
				motion->opacity = value;
			else if (name == "xform: a")
				motion->c[0][0] = value;
			else if (name == "xform: b")
				motion->c[1][0] = value;
			else if (name == "xform: c")
				motion->c[2][0] = value;
			else if (name == "xform: d")
				motion->c[0][1] = value;
			else if (name == "xform: e")
				motion->c[1][1] = value;
			else if (name == "xform: f")
				motion->c[2][1] = value;
			else if (name == "post: a")
				motion->post[0][0] = value;
			else if (name == "post: b")
				motion->post[1][0] = value;
			else if (name == "post: c")
				motion->post[2][0] = value;
			else if (name == "post: d")
				motion->post[0][1] = value;
			else if (name == "post: e")
				motion->post[1][1] = value;
			else if (name == "post: f")
				motion->post[2][1] = value;
			else
				logWarn(QString("SheepLoopWidget::motionItemChanged : unknown element %1").arg(name));
			motion->motion_func = tmp.motion_func;
			motion->motion_freq = tmp.motion_freq;
		}
	}
	else if (col == 1)
	{
		QStandardItem* i = model->item(row, 0);
		if (i)
		{
			flam3_xform tmp = *motion;
			memset(motion, 0, sizeof(flam3_xform));
			QString name = i->text();
			double value = item->data(Qt::DisplayRole).toDouble();
			if (name.startsWith("var: "))
			{
				name = name.remove(0,5);
				logFine(QString("SheepLoopWidget::motionItemChanged setting var '%1'").arg(name));
				motion->var[Util::flam3_variations().value(name)] = value;
			}
			else if (name.startsWith("par: "))
			{
				name = name.remove(0,5);
				Util::set_xform_variable(motion, name, value);
			}
			else if (name == "animate")
				motion->animate = value;
			else if (name == "color")
				motion->color = value;
			else if (name == "color_speed")
				motion->color_speed = value;
			else if (name == "density")
				motion->density = value;
			else if (name == "opacity")
				motion->opacity = value;
			else if (name == "xform: a")
				motion->c[0][0] = value;
			else if (name == "xform: b")
				motion->c[1][0] = value;
			else if (name == "xform: c")
				motion->c[2][0] = value;
			else if (name == "xform: d")
				motion->c[0][1] = value;
			else if (name == "xform: e")
				motion->c[1][1] = value;
			else if (name == "xform: f")
				motion->c[2][1] = value;
			else if (name == "post: a")
				motion->post[0][0] = value;
			else if (name == "post: b")
				motion->post[1][0] = value;
			else if (name == "post: c")
				motion->post[2][0] = value;
			else if (name == "post: d")
				motion->post[0][1] = value;
			else if (name == "post: e")
				motion->post[1][1] = value;
			else if (name == "post: f")
				motion->post[2][1] = value;
			else
				logWarn(QString("SheepLoopWidget::motionItemChanged : unknown element %1").arg(name));
			motion->motion_func = tmp.motion_func;
			motion->motion_freq = tmp.motion_freq;
		}
	}
	else if (col == 2)
	{
		QString m_func = item->text();
		if (m_func == "sin")
			motion->motion_func = MOTION_SIN;
		else if (m_func == "hill")
			motion->motion_func = MOTION_HILL;
		else
			motion->motion_func = MOTION_TRIANGLE;

	}
	else
		motion->motion_freq = item->data(Qt::DisplayRole).toInt();
}

void SheepLoopWidget::addNewMotionElement()
{
	logFine("SheepLoopWidget::addNewMotionElement : ");
	int idx = m_xformIdxBox->currentIndex();
	int genome_idx = m_genomeIdxBox->currentIndex();
	flam3_genome* genome = genomes->data() + genome_idx;
	flam3_xform* xform = genome->xform + idx;
	flam3_add_motion_element(xform);
	QStandardItemModel* model = qobject_cast<QStandardItemModel*>(m_motionElementsView->model());
	int row = model->rowCount();
	model->setItem(row, 0, new QStandardItem("var: linear"));
	model->setItem(row, 1, new QStandardItem("1"));
	model->setItem(row, 2, new QStandardItem("sin"));
	model->setItem(row, 3, new QStandardItem("1"));
}

void SheepLoopWidget::delCurrentMotionElement()
{
	logFine("SheepLoopWidget::delCurrentMotionElement : ");
	QModelIndex idx = m_motionElementsView->currentIndex();
	if (idx.isValid())
	{
		int xf_idx = m_xformIdxBox->currentIndex();
		int genome_idx = m_genomeIdxBox->currentIndex();
		flam3_genome* genome = genomes->data() + genome_idx;
		flam3_xform* xform = genome->xform + xf_idx;
		int row = idx.row();
		flam3_xform* motion = xform->motion;
		int last_motion = xform->num_motion - 1;
		for (int n = row ; n < last_motion ; n++)
			*(motion + n) = *(motion + n + 1);
		xform->num_motion -= 1;
		xform->motion = (struct xform*)realloc(motion, xform->num_motion * sizeof(struct xform));
		xformIdxBoxIndexChanged(xf_idx);
		QStandardItemModel* model = qobject_cast<QStandardItemModel*>(m_motionElementsView->model());
		m_motionElementsView->setCurrentIndex(model->index(model->rowCount() - 1, 0));
	}
}

void SheepLoopWidget::changeEvent(QEvent *e)
{
	QWidget::changeEvent(e);
	switch (e->type()) {
	case QEvent::LanguageChange:
		retranslateUi(this);
		break;
	default:
		break;
	}
}

void SheepLoopWidget::genomeSelectedSlot(int idx)
{
	m_genomeIdxBox->blockSignals(true);
	m_genomeIdxBox->setCurrentIndex(idx);
	m_genomeIdxBox->blockSignals(false);
	genomesModifiedSlot();
}

void SheepLoopWidget::genomesModifiedSlot()
{
	if (!isVisible())
		return;

	if (m_beginBox->currentIndex() > m_endBox->currentIndex())
	{
		m_endBox->blockSignals(true);
		m_endBox->setCurrentIndex(m_beginBox->currentIndex());
		m_endBox->blockSignals(false);
	}

	m_temporalFilterGroupBox->setEnabled(m_temporalSamplesEditor->value() != 1);
	int g_idx = m_genomeIdxBox->currentIndex();
	if (genomes->size() > 0 && 0 <= g_idx && g_idx < genomes->size())
	{
		m_xformIdxBox->blockSignals(true);
		int idx = m_xformIdxBox->currentIndex();
		m_xformIdxBox->clear();
		int n_xforms = (genomes->data () + g_idx)->num_xforms;
		QStringList idxs;
		for (int i = 1 ; i <= n_xforms ; i++)
			idxs << QString::number(i);
		m_xformIdxBox->addItems(idxs);
		if (idx >= 0 && idx < m_xformIdxBox->count())
		{
			m_xformIdxBox->setCurrentIndex(idx);
			xformIdxBoxIndexChanged(idx);
		}
		else
			xformIdxBoxIndexChanged(0);
		m_xformIdxBox->blockSignals(false);
	}
}

void SheepLoopWidget::endBoxIndexChanged(int /*idx*/)
{
	if (m_beginBox->currentIndex() > m_endBox->currentIndex())
	{
		m_beginBox->blockSignals(true);
		m_beginBox->setCurrentIndex(m_endBox->currentIndex());
		m_beginBox->blockSignals(false);
	}
}

void SheepLoopWidget::beginBoxIndexChanged(int /*idx*/)
{
	if (m_beginBox->currentIndex() > m_endBox->currentIndex())
	{
		m_endBox->blockSignals(true);
		m_endBox->setCurrentIndex(m_beginBox->currentIndex());
		m_endBox->blockSignals(false);
	}
}

void SheepLoopWidget::runSheepButtonAction()
{
	running = !running;
	if (running)
		m_runToolButton->setIcon(QIcon(":/icons/silk/stop.xpm"));
	else
		m_runToolButton->setIcon(QIcon(":/icons/silk/bullet_go.xpm"));
	emit runSheepLoop(running);
}

void SheepLoopWidget::temporalSamplesUpdated()
{
	m_temporalFilterGroupBox->setEnabled(m_temporalSamplesEditor->value() != 1);
}

void SheepLoopWidget::reset()
{
	m_beginBox->setCurrentIndex(qMax(0, m_beginBox->currentIndex()));
	m_endBox->setCurrentIndex(qMax(0, m_endBox->currentIndex()));
	m_genomeIdxBox->setCurrentIndex(qMax(0, m_genomeIdxBox->currentIndex()));
	genomesModifiedSlot();
	running = false;
	m_runToolButton->setIcon(QIcon(":/icons/silk/bullet_go.xpm"));
}

int SheepLoopWidget::beginIdx() const
{
	return m_beginBox->currentIndex() + 1;
}

int SheepLoopWidget::endIdx() const
{
	return m_endBox->currentIndex() + 1;
}

int SheepLoopWidget::frames() const
{
	return m_framesBox->value();
}

int SheepLoopWidget::interpolation() const
{
	return m_interpolationBox->currentIndex();
}

int SheepLoopWidget::interpolationType() const
{
	return m_interpolationTypeBox->currentIndex();
}

int SheepLoopWidget::temporalSamples() const
{
	return m_temporalSamplesEditor->value();
}

int SheepLoopWidget::temporalFilterType() const
{
	return m_temporalFilterComboBox->currentIndex();
}

double SheepLoopWidget::temporalFilterWidth() const
{
	return m_temporalFilterWidthEditor->value();
}

double SheepLoopWidget::temporalFilterExp() const
{
	return m_temporalFilterExpEditor->value();
}

double SheepLoopWidget::stagger() const
{
	return m_staggerBox->value();
}

int SheepLoopWidget::loops() const
{
	return m_loopsBox->value();
}

void SheepLoopWidget::modeBoxIndexChanged(int idx)
{
	bool enabled = (idx == 0);
	m_loopsBox->setEnabled(enabled);
	label_loops->setEnabled(enabled);
	m_framesBox->setEnabled(enabled);
	label_frames->setEnabled(enabled);
}

int SheepLoopWidget::paletteInterpolation() const
{
	return m_paletteInterpBox->currentIndex();
}

int SheepLoopWidget::paletteMode() const
{
	return m_paletteInterpTypeBox->currentIndex();
}

SheepLoopWidget::AnimationMode SheepLoopWidget::animationMode() const
{
	return m_animateModeBox->currentIndex() == 0 ? Sequence : Interpolate;
}

void SheepLoopWidget::tabWidgetIndexChanged(int idx)
{
	if (idx == 2)
		m_motionElementsView->resizeRowsToContents();
}

void SheepLoopWidget::temporalFilterTypeIndexChanged(int idx)
{
	m_temporalFilterExpEditor->setEnabled(idx == 2);
}

void SheepLoopWidget::xformAnimateButtonClicked(bool flag)
{
	int idx = m_xformIdxBox->currentIndex();
	int genome_idx = m_genomeIdxBox->currentIndex();
	flam3_genome* genome = genomes->data() + genome_idx;
	(genome->xform + idx)->animate = flag;
}

flam3_genome* SheepLoopWidget::createSheepLoop(int& ncp)
{
	static flam3_genome* sheep = 0;
	static int dncp = 0;

	// clear all previously used sheep
	if (sheep != NULL && dncp > 0)
	{
		for (int i = 0; i < dncp ; i++)
			clear_cp(sheep + i, flam3_defaults_on);
		free(sheep);
		dncp = 0;
	}
	int begin_idx = beginIdx(); // on (0, n]
	int end_idx = endIdx();
	int num_genomes = (end_idx - begin_idx) + 1;
	double stagger = this->stagger();
	int temporal_samples = temporalSamples();
	int interp =  interpolation();
	int interp_type = interpolationType();
	int palette_interp = paletteInterpolation();
	int palette_mode = paletteMode();
	int temp_filter = temporalFilterType();
	int temp_filter_width = temporalFilterWidth();
	double temp_filter_exp = temporalFilterExp();
	AnimationMode mode = animationMode();
	begin_idx--; // reindex to [0, num_genomes)

	if (mode == Sequence)
	{
		for (int n = begin_idx ; n < num_genomes ; n++)
		{
			flam3_genome* g = genomes->data() + n;
			g->palette_interpolation = palette_interp;
			g->palette_mode = palette_mode;
			// bug in libflam3:
			// the sheep sequence will crash+burn with smooth interpolation.
			g->interpolation = flam3_interpolation_linear;
			g->interpolation_type = interp_type;
		}
		int nframes = frames();
		int loops = this->loops();
		// grab a sheep-loop sequence
		sheep = Util::create_genome_sequence(genomes->data() + begin_idx, num_genomes, &dncp, nframes, loops, stagger);
	}
	else
	{
		for (int n = begin_idx ; n < num_genomes ; n++)
		{
			flam3_genome* g = genomes->data() + n;
			if ((interp == flam3_interpolation_smooth) && (n > begin_idx) && (n < num_genomes - 2))
			{
				logInfo("SheepLoopWidget::createSheepLoop : time %d using smooth interp", (int)g->time);
				g->interpolation = flam3_interpolation_smooth;
			}
			else
			{
				logInfo("SheepLoopWidget::createSheepLoop : time %d using linear interp", (int)g->time);
				g->interpolation = flam3_interpolation_linear;
			}
			g->interpolation_type = interp_type;
			g->palette_interpolation = palette_interp;
			g->palette_mode = palette_mode;
		}
		sheep = Util::create_genome_interpolation(genomes->data() + begin_idx, num_genomes, &dncp, stagger);
	}

	MainPreviewWidget* preview = dynamic_cast<MainPreviewWidget*>(getWidget("MainPreviewWidget"));
	flam3_genome current = flam3_genome();
	if (preview->isPresetSelected())
		current = preview->preset();
	else
		current = *(genomes->selectedGenome());

	for (int i = 0 ; i < dncp ; i++)
	{
		// adjust the quality settings to match the preview widget settings
		flam3_genome* genome = sheep + i;
		genome->ntemporal_samples =         temporal_samples;
		genome->temporal_filter_type =      temp_filter;
		genome->temporal_filter_width =     temp_filter_width;
		genome->temporal_filter_exp =       temp_filter_exp;
		genome->sample_density =            current.sample_density;
		genome->spatial_filter_radius =     current.spatial_filter_radius;
		genome->spatial_oversample =        current.spatial_oversample;
		genome->nbatches =                  current.nbatches;
		genome->estimator =                 current.estimator;
		genome->estimator_curve =           current.estimator_curve;
		genome->estimator_minimum =         current.estimator_minimum;
		genome->symmetry = 1;

		if ((interp == flam3_interpolation_smooth) && (i > 0) && (i < (dncp - 2)))
			genome->interpolation = flam3_interpolation_smooth;
		else
			genome->interpolation = flam3_interpolation_linear;
		genome->interpolation_type = interp_type;
	}

	ncp = dncp;
	return sheep;
}

void SheepLoopWidget::showEvent(QShowEvent* e)
{
	if (!e->spontaneous())
	{
		genomes->usingPreviews(true);
		genomesModifiedSlot();
	}
}

void SheepLoopWidget::hideEvent(QHideEvent* e)
{
	if (!e->spontaneous())
		genomes->usingPreviews(false);
}
