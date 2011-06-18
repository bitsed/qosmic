/***************************************************************************
 *   Copyright (C) 2007 - 2011 by David Bitseff                            *
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
#include <QStandardItemModel>

#include "sheeploopwidget.h"
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
	QWidget(parent), genomes(gv), running(false)
{
	setupUi(this);

	connect(m_runToolButton, SIGNAL(clicked()), this, SLOT(runSheepButtonAction()));
	connect(m_saveToolButton, SIGNAL(clicked()), this, SIGNAL(saveSheepLoop()));
	connect(m_beginBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(beginBoxIndexChanged(QString)));
	connect(m_temporalSamplesEditor, SIGNAL(valueUpdated()), this, SLOT(temporalSamplesUpdated()));
	connect(m_animateButton, SIGNAL(clicked(bool)), this, SLOT(xformAnimateButtonClicked(bool)));

	QStandardItemModel* model = new QStandardItemModel(0, 4);
	m_motionElementsView->setModel(model);
	m_motionElementsView->setItemDelegate(new MotionViewItemDelegate(m_motionElementsView));

	connect(m_addToolButton, SIGNAL(clicked()), this, SLOT(addNewMotionElement()));
	connect(m_delToolButton, SIGNAL(clicked()), this, SLOT(delCurrentMotionElement()));
	connect(m_xformIdxBox, SIGNAL(currentIndexChanged(int)), this, SLOT(xformIdxBoxIndexChanged(int)));
	connect(m_animateModeBox, SIGNAL(currentIndexChanged(int)), this, SLOT(modeBoxIndexChanged(int)));
	connect(m_temporalFilterComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(temporalFilterTypeIndexChanged(int)));
	connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabWidgetIndexChanged(int)));
	connect(model, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(motionItemChanged(QStandardItem*)));
}

void SheepLoopWidget::xformIdxBoxIndexChanged(int idx)
{
	if (idx >= genomes->selectedGenome()->num_xforms)
	{
		logWarn(QString("SheepLoopWidget::xformIdxBoxIndexChanged : no xform %1 in genome %2").arg(idx).arg(genomes->selectedIndex()));
		return;
	}
	QStandardItemModel* model = qobject_cast<QStandardItemModel*>(m_motionElementsView->model());
	disconnect(model, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(motionItemChanged(QStandardItem*)));
	flam3_xform* xform = genomes->selectedGenome()->xform + idx;
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
	flam3_xform* xform = genomes->selectedGenome()->xform + xf_idx;
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
	flam3_xform* xform = genomes->selectedGenome()->xform + idx;
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
		flam3_xform* xform = genomes->selectedGenome()->xform + xf_idx;
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

void SheepLoopWidget::genomeSelectedSlot(int)
{
	genomesModifiedSlot();
}

void SheepLoopWidget::genomesModifiedSlot()
{
	int n_genomes = genomes->size();
	QString b_text = m_beginBox->currentText();
	QString e_text = m_endBox->currentText();

	m_beginBox->clear();
	for (int n = 1 ; n <= n_genomes ; n++)
		m_beginBox->addItem(QString::number(n));
	int text_idx = m_beginBox->findText(b_text);
	if (text_idx != -1)
		m_beginBox->setCurrentIndex(text_idx);

	m_endBox->clear();
	for (int n = 1 ; n <= n_genomes ; n++)
		m_endBox->addItem(QString::number(n));
	text_idx = m_endBox->findText(e_text);
	if (text_idx != -1)
		m_endBox->setCurrentIndex(text_idx);

	m_temporalFilterGroupBox->setEnabled(m_temporalSamplesEditor->value() != 1);

	m_xformIdxBox->blockSignals(true);
	int idx = m_xformIdxBox->currentIndex();
	m_xformIdxBox->clear();
	int n_xforms = genomes->selectedGenome()->num_xforms;
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

void SheepLoopWidget::beginBoxIndexChanged(QString st)
{
	int n_genomes = genomes->size();
	int idx = st.toInt();
	QString cur_text = m_endBox->currentText();

	m_endBox->clear();
	for (int n = idx ; n <= n_genomes ; n++)
		m_endBox->addItem(QString::number(n));
	int text_idx = m_endBox->findText(cur_text);
	if (text_idx != -1)
		m_endBox->setCurrentIndex(text_idx);
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
	genomesModifiedSlot();
	running = false;
	m_runToolButton->setIcon(QIcon(":/icons/silk/bullet_go.xpm"));
}

int SheepLoopWidget::beginIdx() const
{
	return m_beginBox->currentText().toInt();
}

int SheepLoopWidget::endIdx() const
{
	return m_endBox->currentText().toInt();
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
	m_framesBox->setEnabled(enabled);
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
	(genomes->selectedGenome()->xform + idx)->animate = flag;
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
				g->interpolation = flam3_interpolation_smooth;
			else
				g->interpolation = flam3_interpolation_linear;
			g->interpolation_type = interp_type;
			logInfo(QString("SheepLoopWidget::createSheepLoop : %1 using smooth interp %2").arg(n)
					.arg(g->interpolation == flam3_interpolation_smooth));
			g->palette_interpolation = palette_interp;
			g->palette_mode = palette_mode;
		}
		sheep = Util::create_genome_interpolation(genomes->data() + begin_idx, num_genomes, &dncp, stagger);
	}

	flam3_genome* current = genomes->selectedGenome();
	for (int i = 0 ; i < dncp ; i++)
	{
		// adjust the quality settings to match the current genome
		flam3_genome* genome = sheep + i;
		genome->ntemporal_samples =         temporal_samples;
		genome->temporal_filter_type =      temp_filter;
		genome->temporal_filter_width =     temp_filter_width;
		genome->temporal_filter_exp =       temp_filter_exp;
		genome->sample_density =            current->sample_density;
		genome->spatial_filter_radius =     current->spatial_filter_radius;
		genome->spatial_oversample =        current->spatial_oversample;
		genome->nbatches =                  current->nbatches;
		genome->estimator =                 current->estimator;
		genome->estimator_curve =           current->estimator_curve;
		genome->estimator_minimum =         current->estimator_minimum;
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
