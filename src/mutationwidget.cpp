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
#include <QSettings>
#include <QListView>
#include <QPainter>
#include <QDrag>
#include <QMimeData>

#include "mutationwidget.h"
#include "viewerpresetsmodel.h"
#include "logger.h"

MutationWidget::MutationWidget(GenomeVector* gen,  RenderThread* t, QWidget* parent)
: QWidget(parent), genome_offset(0), labels_size(82,68), mutation_speed(0.1),
  mutateA_start(0), mutateB_start(0), genome(gen), rthread(t)
{
	setupUi(this);

	labels << label_a1 << label_a2 << label_a3  // labels are defined in the .ui file
		   << label_a4 << label_a5 << label_a6
		   << label_a7 << label_a8
		   << label_b1 << label_b2 << label_b3
		   << label_b4 << label_b5 << label_b6
		   << label_b7 << label_b8
		   << label_c11 << label_c12 << label_c13
		   << label_c21 << label_c22 << label_c23
		   << label_c31 << label_c32 << label_c33
		   << label_c41 << label_c42 << label_c43
		   << label_c51 << label_c52 << label_c53
		   << label_c61 << label_c62 << label_c63
		   << label_c71 << label_c72 << label_c73
		   << label_c81 << label_c82 << label_c83;

	label_a1->setToolTip(tr("genome A"));
	label_b1->setToolTip(tr("genome B"));

	ViewerPresetsModel* presets = ViewerPresetsModel::getInstance();

	QSettings s;
	s.beginGroup("mutationwidget");
	labels_size = s.value("labelsize", labels_size).toSize();
	mutation_speed = s.value("speed", mutation_speed).toReal();
	quality_preset = s.value("preset", presets->presetNames().first()).toString();
	s.endGroup();

	QList<QColor> colors;
	colors << Qt::red << QColor(255, 128, 0) << Qt::yellow << Qt::green
		   << Qt::blue << Qt::magenta << Qt::gray;
	for ( int n = 0 ;  n < 40 ; n++)
	{
		flam3_genome* g = new flam3_genome();
		mutations << g;
		labels[n]->setGenome(g);
		labels[n]->setMinimumSize(labels_size);
		labels[n]->setMaximumSize(labels_size);
		labels[n]->setFrameStyle(QFrame::NoFrame | QFrame::Plain);

		if (n > 0 && n < 16)
		{
			if (n < 8)
			{
				MutationPreviewWidget* p = labels[n % 8];
				p->setFrameEdge(MutationPreviewWidget::LEFT);
				p->setFrameColor(colors[(n + 6) % 7]);
			}
			else if (n > 8)
			{
				MutationPreviewWidget* p = labels[n % 16];
				p->setFrameEdge(MutationPreviewWidget::RIGHT);
				p->setFrameColor(colors[(n + 5) % 7]);
			}
		}
		RenderRequest* r = new RenderRequest(genome->data(), labels_size, tr("mutation %1").arg(n + 1), RenderRequest::Queued);
		requests << r;
		connect(labels[n], SIGNAL(mutationASelected(MutationPreviewWidget*)), this, SLOT(mutationASelectedAction(MutationPreviewWidget*)));
		connect(labels[n], SIGNAL(mutationBSelected(MutationPreviewWidget*)), this, SLOT(mutationBSelectedAction(MutationPreviewWidget*)));
		connect(labels[n], SIGNAL(genomeSelected(MutationPreviewWidget*)), this, SLOT(genomeSelectedAction(MutationPreviewWidget*)));
	}

	aComboBox->setModel(genome);
	bComboBox->setModel(genome);
	qobject_cast<QListView*>(aComboBox->view())->setSpacing(2);
	qobject_cast<QListView*>(bComboBox->view())->setSpacing(2);

	connect(aComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectorAIndexChangedSlot(int)));
	connect(bComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectorBIndexChangedSlot(int)));
	connect(labels[0], SIGNAL(genomeDropped(int)), this, SLOT(selectorAIndexChangedSlot(int)));
	connect(labels[8], SIGNAL(genomeDropped(int)), this, SLOT(selectorBIndexChangedSlot(int)));
	connect(aUpButton, SIGNAL(clicked()), this, SLOT(rotateAMutationsUp()));
	connect(aDownButton, SIGNAL(clicked()), this, SLOT(rotateAMutationsDown()));
	connect(bUpButton, SIGNAL(clicked()), this, SLOT(rotateBMutationsUp()));
	connect(bDownButton, SIGNAL(clicked()), this, SLOT(rotateBMutationsDown()));
	connect(rthread, SIGNAL(flameRendered(RenderEvent*)), this, SLOT(flameRenderedAction(RenderEvent*)));
	connect(configButton, SIGNAL(clicked()), this, SLOT(showConfigDialog()));
	connect(regenButton, SIGNAL(clicked()), this, SLOT(mutate()));
}

void MutationWidget::showConfigDialog()
{
	MutationConfigDialog d(this);
	d.setPreviewSize(labels_size);
	d.setSpeed(mutation_speed);
	d.setPreset(quality_preset);
	d.move(QCursor::pos());
	d.exec();
	QSize s = d.previewSize();
	qreal speed = d.speed();
	QString preset = d.preset();
	bool render = false;
	if (speed != mutation_speed)
	{
		mutation_speed = speed;
		QSettings().setValue("mutationwidget/speed", mutation_speed);
	}
	if (s != labels_size)
	{
		labels_size = s;
		QSettings().setValue("mutationwidget/labelsize", labels_size);
		foreach (MutationPreviewWidget* p, labels)
		{	p->setMinimumSize(labels_size);
			p->setMaximumSize(labels_size);
		}
		render = true;
	}
	if (preset != quality_preset)
	{
		quality_preset = preset;
		QSettings().setValue("mutationwidget/preset", quality_preset);
		ViewerPresetsModel* presets = ViewerPresetsModel::getInstance();
		foreach (MutationPreviewWidget* p, labels)
			presets->applyPreset(quality_preset, p->genome());
		render = true;
	}
	if (render)
	{
		cancelRequests();
		foreach (RenderRequest* r, requests)
		{
			r->setSize(labels_size);
			rthread->render(r);
		}
	}
}

void MutationWidget::rotateAMutationsUp()
{
	mutateA_start = (mutateA_start + 1) % 7;
	logInfo("MutationWidget::rotateAMutationsUp : mutateA_start %d", mutateA_start);
	flam3_genome tmp_genome = flam3_genome();
	QList<MutationPreviewWidget*>::iterator i = labels.begin() + 1;
	QList<MutationPreviewWidget*>::iterator e = labels.begin() + 8;
	flam3_copy(&tmp_genome, (*i)->genome());
	QPixmap tmp_pixmap(*((*i)->pixmap()));
	QString tmp_tip((*i)->toolTip());
	QColor tmp_color((*i)->frameColor());
	while (++i != e)
	{
		QList<MutationPreviewWidget*>::iterator p = i - 1;
		flam3_copy((*p)->genome(), (*i)->genome());
		(*p)->setPixmap(*((*i)->pixmap()));
		(*p)->setToolTip((*i)->toolTip());
		(*p)->setFrameColor((*i)->frameColor());
		(*p)->update();
	}
	e--;
	flam3_copy((*e)->genome(), &tmp_genome);
	(*e)->setPixmap(tmp_pixmap);
	(*e)->setToolTip(tmp_tip);
	(*e)->setFrameColor(tmp_color);
	(*e)->update();
	cancelRequests();
	cross();
}

void MutationWidget::rotateAMutationsDown()
{
	mutateA_start = (mutateA_start + 6) % 7;
	logInfo("MutationWidget::rotateAMutationsUp : mutateA_start %d", mutateA_start);
	flam3_genome tmp_genome = flam3_genome();
	QList<MutationPreviewWidget*>::iterator i = labels.begin() + 7;
	QList<MutationPreviewWidget*>::iterator e = labels.begin() + 0;
	flam3_copy(&tmp_genome, (*i)->genome());
	QPixmap tmp_pixmap(*((*i)->pixmap()));
	QString tmp_tip((*i)->toolTip());
	QColor tmp_color((*i)->frameColor());
	while (--i != e)
	{
		QList<MutationPreviewWidget*>::iterator p = i + 1;
		flam3_copy((*p)->genome(), (*i)->genome());
		(*p)->setPixmap(*((*i)->pixmap()));
		(*p)->setToolTip((*i)->toolTip());
		(*p)->setFrameColor((*i)->frameColor());
		(*p)->update();
	}
	e++;
	flam3_copy((*e)->genome(), &tmp_genome);
	(*e)->setPixmap(tmp_pixmap);
	(*e)->setToolTip(tmp_tip);
	(*e)->setFrameColor(tmp_color);
	(*e)->update();
	cancelRequests();
	cross();
}

void MutationWidget::rotateBMutationsUp()
{
	mutateB_start = (mutateB_start + 1) % 7;
	flam3_genome tmp_genome = flam3_genome();
	QList<MutationPreviewWidget*>::iterator i = labels.begin() + 9;
	QList<MutationPreviewWidget*>::iterator e = labels.begin() + 16;
	flam3_copy(&tmp_genome, (*i)->genome());
	QPixmap tmp_pixmap(*((*i)->pixmap()));
	QString tmp_tip((*i)->toolTip());
	QColor tmp_color((*i)->frameColor());
	while (++i != e)
	{
		QList<MutationPreviewWidget*>::iterator p = i - 1;
		flam3_copy((*p)->genome(), (*i)->genome());
		(*p)->setPixmap(*((*i)->pixmap()));
		(*p)->setToolTip((*i)->toolTip());
		(*p)->setFrameColor((*i)->frameColor());
		(*p)->update();
	}
	e--;
	flam3_copy((*e)->genome(), &tmp_genome);
	(*e)->setPixmap(tmp_pixmap);
	(*e)->setToolTip(tmp_tip);
	(*e)->setFrameColor(tmp_color);
	(*e)->update();
	cancelRequests();
	cross();
}

void MutationWidget::rotateBMutationsDown()
{
	mutateB_start = (mutateB_start + 6) % 7;
	flam3_genome tmp_genome = flam3_genome();
	QList<MutationPreviewWidget*>::iterator i = labels.begin() + 15;
	QList<MutationPreviewWidget*>::iterator e = labels.begin() + 8;
	flam3_copy(&tmp_genome, (*i)->genome());
	QPixmap tmp_pixmap(*((*i)->pixmap()));
	QString tmp_tip((*i)->toolTip());
	QColor tmp_color((*i)->frameColor());
	while (--i != e)
	{
		QList<MutationPreviewWidget*>::iterator p = i + 1;
		flam3_copy((*p)->genome(), (*i)->genome());
		(*p)->setPixmap(*((*i)->pixmap()));
		(*p)->setToolTip((*i)->toolTip());
		(*p)->setFrameColor((*i)->frameColor());
		(*p)->update();
	}
	e++;
	flam3_copy((*e)->genome(), &tmp_genome);
	(*e)->setPixmap(tmp_pixmap);
	(*e)->setToolTip(tmp_tip);
	(*e)->setFrameColor(tmp_color);
	(*e)->update();
	cancelRequests();
	cross();
}


void MutationWidget::selectorAIndexChangedSlot(int idx)
{
	if (!isVisible())
		return;

	int idx_a = qMax(0, idx);
	if (idx_a != aComboBox->currentIndex())
	{
		aComboBox->blockSignals(true);
		aComboBox->setCurrentIndex(idx_a);
		aComboBox->blockSignals(false);
	}

	flam3_genome* genome_a = genome->data() + idx_a;
	if (genome_a->num_xforms > 0)
	{
		logFine(QString("MutationWidget::selectorAIndexChangedSlot : genome_a has %1 xforms").arg(genome_a->num_xforms));
		flam3_copy(mutations[0], genome_a);
		ViewerPresetsModel::getInstance()->applyPreset(quality_preset, mutations[0]);
		labels[0]->setGenome(mutations[0]);
		requests[0]->setGenome(mutations.at(0));
	}

	cancelRequests();
	mutateAB('a');
	cross();
}

void MutationWidget::selectorBIndexChangedSlot(int idx)
{
	if (!isVisible())
		return;

	int idx_b = qMax(0, idx);
	if (idx_b != bComboBox->currentIndex())
	{
		bComboBox->blockSignals(true);
		bComboBox->setCurrentIndex(idx_b);
		bComboBox->blockSignals(false);
	}

	flam3_genome* genome_b = genome->data() + idx_b;
	if (genome_b->num_xforms > 0)
	{
		logFine(QString("MutationWidget::selectorBIndexChangedSlot : genome_b has %1 xforms").arg(genome_b->num_xforms));
		flam3_copy(mutations[8], genome_b);
		ViewerPresetsModel::getInstance()->applyPreset(quality_preset, mutations[8]);
		labels[8]->setGenome(mutations[8]);
		requests[8]->setGenome(mutations.at(8));
	}

	cancelRequests();
	mutateAB('b');
	cross();
}


void MutationWidget::genomeSelectedAction(MutationPreviewWidget* ptr)
{
	int idx = mutations.indexOf(ptr->genome(),0);
	logFine(QString("MutationWidget::genomeSelectedAction : genome %1 selected").arg(idx));
	if (idx >= 0)
		emit genomeSelected(mutations[idx]);
}

void MutationWidget::mutationASelectedAction(MutationPreviewWidget* ptr)
{
	if (ptr)
	{
		int idx = mutations.indexOf(ptr->genome(), 0);
		logFine(QString("MutationWidget::mutationASelectedAction : mutating %1").arg(idx));
		if (idx == -1)
			reset();
		else
		{
			if (idx > 0)
			{
				mutations.swap(idx, 0);
				labels[0]->setGenome(ptr->genome());
			}
			cancelRequests();
			mutateAB('a');
			cross();
		}
	}
	else
		reset();
}

void MutationWidget::mutationBSelectedAction(MutationPreviewWidget* ptr)
{
	if (ptr)
	{
		int idx = mutations.indexOf(ptr->genome(), 0);
		logFine(QString("MutationWidget::mutationBSelectedAction : mutating %1").arg(idx));
		if (idx == -1)
			reset();
		else
		{
			if (idx >= 0)
			{
				mutations.swap(idx, 8);
				labels[8]->setGenome(ptr->genome());
			}
			cancelRequests();
			mutateAB('b');
			cross();
		}
	}
	else
		reset();
}

void MutationWidget::flameRenderedAction(RenderEvent* e)
{
	RenderRequest* req = e->request();
	if (!(req->type() == RenderRequest::Queued
			   && requests.contains(req)) )
		return;

	int idx = requests.indexOf(req, 0);
	logFine(QString("MutationWidget::flameRenderedAction : setting genome %1,g=0x%2,req=0x%3")
			.arg(idx).arg((long)req->genome(),0,16).arg((long)req,0,16));
	labels[idx]->setGenome(req->genome());
	labels[idx]->setPixmap(QPixmap::fromImage(req->image()));
	e->accept();
}

void MutationWidget::showEvent(QShowEvent* e)
{
	if (!e->spontaneous())
	{
		logFine(QString("MutationWidget::showEvent : resetting"));
		reset();
	}
}

#define genome_ptr (genome->selectedGenome())

void MutationWidget::cancelRequests()
{
	rthread->running_mutex.lock();
	foreach (RenderRequest* req, requests)
	{
		if (!req->finished())
			rthread->cancel(req);
	}
	rthread->running_mutex.unlock();
}

void MutationWidget::mutateAB(char ab='a')
{
	flam3_genome tmp_genome = flam3_genome();
	int variations[flam3_nvariations] = { flam3_variation_random };
	int nvars = 1;
	int symmetry = 0;
	double speed = mutation_speed;
	int mutate_mode = mutateA_start;
	int mutate_idx  = 0;
	int start_idx = 0;
	int end_idx = 8;
	if (ab == 'b')
	{
		start_idx = 8;
		end_idx = 16;
		mutate_mode = mutateB_start;
	}
	for ( int n = start_idx ; n < end_idx ; n++ )
	{
		flam3_genome* g = mutations.at(n);
		if ( (n % 8) == 0 )
		{
			logFine(QString("MutationWidget::mutateAB : rendering genome %1").arg(n));
			mutate_idx  = n;
		}
		else
		{
			logFine(QString("MutationWidget::mutateAB : mutating %1 -> %2 mode %3").arg(mutate_idx).arg(n).arg(mutate_mode));
			flam3_copy(&tmp_genome, mutations.at(mutate_idx));
			// flam3_mutate() calls add_to_action() which needs this size char[]
			char modstr[flam3_max_action_length] = "";
			flam3_mutate(&tmp_genome, (mutate_mode++ % 7), variations, nvars, symmetry, speed, Util::get_isaac_randctx(), modstr);
			if (tmp_genome.num_xforms > 0)
			{
				flam3_copy(g, &tmp_genome);
				// apply any symmetry set by the mutation
				flam3_add_symmetry(g, tmp_genome.symmetry);
				g->symmetry = 1;
				labels[n]->setToolTip(QString(modstr));
				logFine(QString("MutationWidget::mutateAB : modstr '%1'").arg(QString(modstr)));
			}
			else
				logWarn("MutationWidget::mutateAB : zero xforms in mutation");
		}
		RenderRequest* req = requests.at(n);
		req->setGenome(g);
		rthread->render(req);
	}
	clear_cp(&tmp_genome, flam3_defaults_on);
}

void MutationWidget::cross()
{
	flam3_genome tmp_genome = flam3_genome();
	for ( int n = 16, k = 0 ; n < 40 ; n += 3, k++ )
	{
		for ( int j = 0 ; j < 3 ; j++)
		{
			flam3_genome* g = mutations.at(n + j);
			flam3_genome* a = mutations.at(n - 16 - 2*k);
			flam3_genome* b = mutations.at(n - 8  - 2*k);
			char modstr[flam3_max_action_length] = "";
			logFine(QString("MutationWidget::cross : crossing %1 and %2 -> %3 mode %4").arg(n - 16 - 2*k).arg(n - 8 - 2*k).arg(n + j).arg(j));
			flam3_cross(a, b, &tmp_genome, j, Util::get_isaac_randctx(), modstr);
			if (tmp_genome.num_xforms > 0)
			{
				flam3_copy(g, &tmp_genome);
				// apply any symmetry set by the mutation
				flam3_add_symmetry(g, tmp_genome.symmetry);
				g->symmetry = 1;
				labels[n + j]->setToolTip(QString(modstr));
				logFine(QString("MutationWidget::cross : modstr '%1'").arg(QString(modstr)));
			}
			else
				logWarn("MutationWidget::cross : zero xforms in cross");

			RenderRequest* req = requests.at(n + j);
			req->setGenome(g);
			rthread->render(req);
		}
	}
	clear_cp(&tmp_genome, flam3_defaults_on);
}

void MutationWidget::mutate()
{
	cancelRequests();
	mutateAB('a');
	mutateAB('b');
	cross();
}

void MutationWidget::reset()
{
	if (!isVisible())
		return;

	logFine("MutationWidget::reset : ");
	int idx_a = qMax(0, aComboBox->currentIndex());
	int idx_b = qMax(0, bComboBox->currentIndex());

	flam3_genome* genome_a = genome->data() + idx_a;
	flam3_genome* genome_b = genome->data() + idx_b;

	bool init_a = (mutations[0]->num_xforms < 1) && (genome_a->num_xforms > 0);
	bool init_b = (mutations[8]->num_xforms < 1) && (genome_b->num_xforms > 0);
	if (init_a)
	{
		logFine(QString("MutationWidget::reset : genome_a has %1 xforms").arg(genome_a->num_xforms));
		flam3_copy(mutations[0], genome_a);
		ViewerPresetsModel::getInstance()->applyPreset(quality_preset, mutations[0]);
		labels[0]->setGenome(mutations[0]);
		requests[0]->setGenome(mutations.at(0));
	}

	if (init_b)
	{
		logFine(QString("MutationWidget::reset : genome_b has %1 xforms").arg(genome_b->num_xforms));
		flam3_copy(mutations[8], genome_b);
		ViewerPresetsModel::getInstance()->applyPreset(quality_preset, mutations[8]);
		labels[8]->setGenome(mutations[8]);
		requests[8]->setGenome(mutations.at(8));
	}

	if (init_a || init_b)
		mutate();
}

//------------------------------------------------------------------------------

MutationPreviewWidget::MutationPreviewWidget(QWidget* parent)
	: QLabel(parent), frame_edge(NONE)
{
}

void MutationPreviewWidget::mousePressEvent(QMouseEvent* e)
{
	if (e->button() == Qt::LeftButton)
		 dragStartPosition = e->pos();
	emit previewClicked(this, e);
}

void MutationPreviewWidget::mouseMoveEvent(QMouseEvent* e)
{
	if (!(e->buttons() & Qt::LeftButton))
		return;
	if ((e->pos() - dragStartPosition).manhattanLength()
			< QApplication::startDragDistance())
		return;

	QDrag* drag = new QDrag(this);
	QMimeData* mimeData = new QMimeData;
	mimeData->setData("application/x-mutationpreviewwidget", QByteArray());
	drag->setMimeData(mimeData);
	drag->setPixmap(*pixmap());
	drag->exec(Qt::CopyAction | Qt::MoveAction);
}

void MutationPreviewWidget::mouseReleaseEvent(QMouseEvent* e)
{
	if (e->button() == Qt::LeftButton)
	{
		if ((e->pos() - dragStartPosition).manhattanLength()
			< QApplication::startDragDistance())
				emit mutationASelected(this);
	}
	else if (e->button() == Qt::RightButton)
		emit mutationBSelected(this);
	else
		emit genomeSelected(this);
}

flam3_genome* MutationPreviewWidget::genome()
{
	return g;
}

void MutationPreviewWidget::setGenome(flam3_genome* gen)
{
	g = gen;
}

void MutationPreviewWidget::dragEnterEvent(QDragEnterEvent* event)
{
	if (event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist"))
		event->acceptProposedAction();
	else if ((event->source() != this)
			 && event->mimeData()->hasFormat("application/x-mutationpreviewwidget"))
		event->acceptProposedAction();
}

void MutationPreviewWidget::dropEvent(QDropEvent* event)
{
	if (event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist"))
	{
		int genome_idx = event->mimeData()->data("application/x-qabstractitemmodeldatalist").toInt();
		emit genomeDropped(genome_idx);
		event->acceptProposedAction();
	}
	else if ((event->source() != this)
			 && event->mimeData()->hasFormat("application/x-mutationpreviewwidget"))
	{
		if (objectName() == "label_a1")
			emit mutationASelected(qobject_cast<MutationPreviewWidget*>(event->source()));
		else
			emit mutationBSelected(qobject_cast<MutationPreviewWidget*>(event->source()));
		event->acceptProposedAction();
	}
}

void MutationPreviewWidget::setFrameColor(const QColor& c)
{
	frame_color = c;
}

QColor MutationPreviewWidget::frameColor() const
{
	return frame_color;
}

void MutationPreviewWidget::setFrameEdge(MutationPreviewWidget::Edge e)
{
	frame_edge = e;
}

MutationPreviewWidget::Edge MutationPreviewWidget::frameEdge() const
{
	return frame_edge;
}

void MutationPreviewWidget::paintEvent(QPaintEvent* /*event*/)
{
	QPainter p(this);
	const QPixmap* pix = pixmap();
	if (pix)
	{
		p.drawPixmap(0, 0, *pix);

		if (frame_edge == LEFT)
		{
			int fh = frameGeometry().height() - 1;
			p.setPen(frame_color);
			p.drawLine(0, 0, 0, fh);
			p.drawLine(1, 0, 1, fh);
		}
		else if (frame_edge == RIGHT)
		{
			int rhs = frameGeometry().width() - 1;
			int fh  = frameGeometry().height() - 1;
			p.setPen(frame_color);
			p.drawLine(rhs - 1, 0, rhs - 1, fh);
			p.drawLine(rhs, 0, rhs, fh);
		}
	}
}

//-------------------------------------------------------------------------------------------

MutationConfigDialog::MutationConfigDialog(QWidget* parent) : QDialog(parent)
{
	setupUi(this);

	qualityComboBox->setModel(ViewerPresetsModel::getInstance());
}

void MutationConfigDialog::setPreviewSize(const QSize& size)
{
	sizewLineEdit->updateValue(size.width());
	sizehLineEdit->updateValue(size.height());
}

QSize MutationConfigDialog::previewSize() const
{
	return QSize(sizewLineEdit->value(), sizehLineEdit->value());
}

void MutationConfigDialog::setSpeed(const qreal value)
{
	speedEditor->setValue(value);
}

qreal MutationConfigDialog::speed() const
{
	return speedEditor->value();
}

QString MutationConfigDialog::preset() const
{
	return qualityComboBox->currentText();
}

void MutationConfigDialog::setPreset(const QString& s)
{
	qualityComboBox->setCurrentIndex(qualityComboBox->findText(s));
}

