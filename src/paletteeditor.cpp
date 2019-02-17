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
#include <QPainter>
#include <QMessageBox>
#include <QFileDialog>
#include <QSettings>
#include <QLinearGradient>
#include <QButtonGroup>

#include "paletteeditor.h"
#include "flam3util.h"
#include "logger.h"

PaletteEditor::PaletteEditor ( QWidget* parent )
: QWidget(parent), checkers(15)
{
	setupUi ( this );
	loadPalette(0);

	m_palettesView->setModel(&m_flamPalettes);
	m_browseView->setModel(&m_browsePalettes);
	hasUGR = false;
	QSettings settings;
	settings.beginGroup("paletteeditor");
	m_tabWidget->setCurrentIndex(settings.value("tabwidgetindex", 0).toInt());
	m_lastBrowseDir = settings.value("lastdirectory", QDir::homePath()).toString();
	m_gradientEnds->setArrowDirection(Qt::DownArrow);
	m_gradientEnds->setMenuEnabled(false);

	m_gradientSpreadGroup = new QButtonGroup(this);
	m_gradientSpreadGroup->addButton(m_padSpreadButton, QGradient::PadSpread);
	m_gradientSpreadGroup->addButton(m_repeatSpreadButton, QGradient::RepeatSpread);
	m_gradientSpreadGroup->addButton(m_reflectSpreadButton, QGradient::ReflectSpread);
	m_padSpreadButton->setChecked(true);

	// restore the gradient from settings
	int nstops( settings.beginReadArray("gradient") );
	if (nstops > 1)
	{
		GradientStops stops;
		for (int n = 0 ; n < nstops ; n++)
		{
			settings.setArrayIndex(n);
			double pos( settings.value("pos").toDouble() );
			QColor color( settings.value("color").value<QColor>()) ;
			stops.append(GradientStop( pos, color ));
		}
		settings.endArray();
		p_stops = stops;
		m_gradientStops->setStops(stops);
	}
	stopsChangedAction();

	connect(m_palettesView, SIGNAL(pressed(const QModelIndex&)), this, SLOT(paletteIndexChangedAction(const QModelIndex&)));
	connect(m_browseView, SIGNAL(pressed(const QModelIndex&)), this, SLOT(selectGradientAction(const QModelIndex&)));
	connect(m_rotateSlider, SIGNAL(valueChanged(int)), this, SLOT(paletteRotatedAction(int)));
	connect(m_rotateSlider, SIGNAL(sliderReleased()), this, SIGNAL(undoStateSignal()));
	connect(m_openButton, SIGNAL(clicked(bool)), this, SLOT(openGradientAction(bool)));
	connect(m_gradientStops, SIGNAL(stopsChanged()), this, SLOT(stopsChangedAction()));
	connect(m_gradientStops, SIGNAL(stopsDropped()), this, SIGNAL(undoStateSignal()));
	connect(m_gradientEnds, SIGNAL(stopsChanged()), this, SLOT(stopsChangedAction()));
	connect(m_gradientEnds, SIGNAL(stopsDropped()), this, SIGNAL(undoStateSignal()));
	connect(m_gradientSpreadGroup, SIGNAL(buttonClicked(int)), this, SLOT(stopsChangedAction()));
	connect(m_gradientSpreadGroup, SIGNAL(buttonClicked(int)), this, SIGNAL(undoStateSignal()));
	connect(m_resetGradientButton, SIGNAL(clicked(bool)), this, SLOT(resetGradientAction()));
	connect(m_resetGradientButton, SIGNAL(clicked(bool)), this, SIGNAL(undoStateSignal()));
	connect(m_saveGradientButton, SIGNAL(clicked(bool)), this, SLOT(saveGradientAction()));
	connect(m_browseLineEdit, SIGNAL(returnPressed()), this, SLOT(browsePathChangedAction()));
	connect(m_randomGradientButton, SIGNAL(clicked(bool)), this, SLOT(createRandomGradientAction()));
	connect(m_randomGradientButton, SIGNAL(clicked(bool)), this, SIGNAL(undoStateSignal()));
}

void PaletteEditor::closeEvent(QCloseEvent* event)
{
	logInfo("PaletteEditor::closeEvent : saving settings");
	QSettings settings;
	settings.beginGroup("paletteeditor");
	settings.setValue("tabwidgetindex", m_tabWidget->currentIndex());
	settings.setValue("lastdirectory", m_lastBrowseDir);

	// save the gradient to the settings
	GradientStops stops( m_gradientStops->getStops() );
	settings.beginWriteArray("gradient");
	for (int n = 0 ; n < stops.size() ; n++)
	{
		settings.setArrayIndex(n);
		settings.setValue("pos", stops[n].first);
		settings.setValue("color", stops[n].second);
	}
	settings.endArray();
	event->accept();
}

void PaletteEditor::showEvent(QShowEvent* /*event*/)
{
	setPaletteView();
	buildPaletteSelector();
}

void PaletteEditor::resetGradientAction()
{
	m_gradientStops->setStops(p_stops);
}

void PaletteEditor::saveGradientAction()
{
	QString saveFile( QFileDialog::getSaveFileName(this,
		tr("Save gradient to a cpt file"),
		QFileInfo(m_lastBrowseDir).absoluteFilePath(),
		tr("CPT Gradient Files (*.cpt)")) );

	if (saveFile.isEmpty())
		return;

	QFileInfo file( saveFile );
	QString openDir( file.absoluteFilePath() );
	logInfo(QString("PaletteEditor::saveGradientAction : saving gradient to %1").arg(saveFile));
	if (openDir != m_lastBrowseDir)
		m_lastBrowseDir = openDir;

	QFile data(file.absoluteFilePath());
	if (!data.open(QFile::ReadWrite))
	{
		QMessageBox msgBox;
		msgBox.setText(tr("Error: Couldn't open file %1").arg(file.filePath()));
		msgBox.setIcon(QMessageBox::Warning);
		msgBox.exec();
		return;
	}

	QTextStream os(&data);
	os << "# COLOR_MODEL = RGB" << endl
		<< scientific << qSetRealNumberPrecision(6) << qSetFieldWidth(4);
	p_stops = m_gradientStops->getStops();
	qStableSort(p_stops.begin(), p_stops.end(), GradientStop::lessThanGradientStopComparison);
	GradientStops::const_iterator i = p_stops.constBegin();
	os << left << (*i).first
		<< right
		<< (*i).second.red()
		<< (*i).second.green()
		<< (*i).second.blue() << ' ';
	++i;
	for ( ; i != p_stops.constEnd(); ++i)
	{
		os << left << (*i).first
		<< right
		<< (*i).second.red()
		<< (*i).second.green()
		<< (*i).second.blue() << endl;

		if (i + 1 != p_stops.constEnd())
			os << left << (*i).first
			<< right
			<< (*i).second.red()
			<< (*i).second.green()
			<< (*i).second.blue() << ' ';
	}
	os << "B   0   0   0" << endl
		<< "F 255 255 255" << endl
		<< "N 255   0   0" << endl;

	data.close();
	if (data.error() != QFile::NoError)
	{
		QMessageBox msgBox;
		msgBox.setText(tr("Error: Couldn't write to file %1").arg(file.filePath()));
		msgBox.setIcon(QMessageBox::Warning);
		msgBox.exec();
	}
}

void PaletteEditor::stopsChangedAction()
{
	static const int GradientBufferLastIdx = GradientBufferSize - 1;
	static const qreal dx  = 1.0 / GradientBufferSize;
	QSize s( m_gradientLabel->maximumSize() );
	QRect r( QPoint(0, 0), QSize(s.width(), (s.height() / 2.0 ) ) );
	QImage palette_image(s, QImage::Format_RGB32);
	QPainter painter(&palette_image);
	GradientStops stops(m_gradientStops->getStops());
	qStableSort(stops.begin(), stops.end(), GradientStop::lessThanGradientStopComparison);

	// now apply the ends and update the palette
	GradientStops ends( m_gradientEnds->getStops() );
	QGradient::Spread spread((QGradient::Spread)m_gradientSpreadGroup->checkedId());

	GradientStop n_stop(stops.first());
	QRgb ccolor = n_stop.second.rgba();
	for (int n = 0, fpos = n_stop.first * GradientBufferSize  ; n < fpos ; n++)
		m_gradient[qMin(n, GradientBufferLastIdx)] = ccolor;

	int last_stop_idx = stops.size() - 1;
	for (int begin_idx = 0; begin_idx < last_stop_idx ; begin_idx++)
	{
		GradientStop a = stops.at(begin_idx);
		GradientStop b = stops.at(begin_idx + 1);
		QColor ac = a.second;
		QColor bc = b.second;
		qreal d = ( b.first - a.first );
		qreal rdx, gdx, bdx, adx;
		if (b.colorspace == 0)
		{
			rdx = ( (bc.red()   - ac.red() )   / d ) * dx;
			gdx = ( (bc.green() - ac.green() ) / d ) * dx;
			bdx = ( (bc.blue()  - ac.blue() )  / d ) * dx;
			adx = ( (bc.alpha() - ac.alpha() ) / d ) * dx;
		}
		else
		{
			rdx = ( (bc.hue()        - ac.hue() )        / d ) * dx;
			gdx = ( (bc.saturation() - ac.saturation() ) / d ) * dx;
			bdx = ( (bc.value()      - ac.value() )      / d ) * dx;
			adx = ( (bc.alpha()      - ac.alpha() )      / d ) * dx;

			if (b.colorspace == 1)
			{
				if (rdx == 0.0)
					rdx = 180.0 / d * dx;
				else if (rdx < 0)
					rdx *= -1;
			}
			else
			{
				if (rdx == 0.0)
					rdx = -180.0 / d * dx;
				else if (rdx > 0)
					rdx *= -1;
			}
		}
		int n  = a.first * GradientBufferSize ;
		int nb = (int)(b.first * GradientBufferSize );
		for (int i = 0 ; n < nb ; i++, n++)
		{
			if (b.colorspace == 0)
			{
				m_gradient[n] = qRgba(
						qBound(0, (int)( ac.red()   + rdx * i + 0.5 ), 255),
						qBound(0, (int)( ac.green() + gdx * i + 0.5 ), 255),
						qBound(0, (int)( ac.blue()  + bdx * i + 0.5 ), 255),
						qBound(0, (int)( ac.alpha() + adx * i + 0.5 ), 255));
			}
			else
			{
				int h = (int)( ac.hue() + rdx * i + 0.5 );
				if (h < 0)
					h += 360;
				m_gradient[n] = QColor::fromHsv(h % 360,
						qBound(0, (int)( ac.saturation() + gdx * i + 0.5 ), 255),
						qBound(0, (int)( ac.value()  + bdx * i + 0.5 ), 255),
						qBound(0, (int)( ac.alpha() + adx * i + 0.5 ), 255)).rgba();
			}
		}
	}

	n_stop = stops.last();
	ccolor = n_stop.second.rgba();
	for (int n = n_stop.first * GradientBufferSize ; n < GradientBufferSize ; n++)
		m_gradient[n] = ccolor;

	qreal start(ends.at(0).first);
	qreal end(ends.at(1).first);
	int begin_idx = start * 256 ;
	int end_idx   = end   * 256 ;
	int ibuf_size = end_idx - begin_idx;
	flam3_palette_entry* ibuf = new flam3_palette_entry[ibuf_size]();

	// a very acute filter
	qreal c2 = 0.01;
	qreal c3 = 1.0;
	qreal c4 = 0.01;
	qreal norm = c2 + c3 + c4;
	qreal k = 0.0;
	qreal skip( (GradientBufferSize / 256.0) / qMax(qreal(1.0/GradientBufferSize), end - start) );
	for (int n = 0 ; n < ibuf_size ; n++, k += skip)
	{
		int j = k;
		QRgb a2( m_gradient[qBound(0, j + 0, GradientBufferLastIdx)] );
		QRgb a3( m_gradient[qMin(j + 1, GradientBufferLastIdx)] );
		QRgb a4( m_gradient[qMin(j + 2, GradientBufferLastIdx)] );

		ibuf[n].color[0] = (qRed(a2)*c2   + qRed(a3)*c3   + qRed(a4)*c4 )   / (norm * 255.);
		ibuf[n].color[1] = (qGreen(a2)*c2 + qGreen(a3)*c3 + qGreen(a4)*c4 ) / (norm * 255.);
		ibuf[n].color[2] = (qBlue(a2)*c2  + qBlue(a3)*c3  + qBlue(a4)*c4 )  / (norm * 255.);
		ibuf[n].color[3] = (qAlpha(a2)*c2 + qAlpha(a3)*c3 + qAlpha(a4)*c4 ) / (norm * 255.);
	}

	// update the gradient editor label
	painter.fillRect(QRect(QPoint(0,0), s), checkers);
	if (ibuf_size == 256)
	{
		for (int n = 0, h = s.height() ; n < 256 ; n++)
		{
			painter.setPen(QColor::fromRgbF(ibuf[n].color[0], ibuf[n].color[1], ibuf[n].color[2], ibuf[n].color[3]));
			painter.drawLine(n, 0, n, h);
		}
	}
	else
	{
		for (int n = 0, h = s.height(), j = 0 ; n < 256 ; n++, j += 4)
		{
			QRgb a2( m_gradient[qBound(0, j + 0, GradientBufferLastIdx)] );
			QRgb a3( m_gradient[qMin(j + 1, GradientBufferLastIdx)] );
			QRgb a4( m_gradient[qMin(j + 2, GradientBufferLastIdx)] );
			QRgb r((qRed(a2)*c2   + qRed(a3)*c3   + qRed(a4)*c4 )   / norm );
			QRgb g((qGreen(a2)*c2 + qGreen(a3)*c3 + qGreen(a4)*c4 ) / norm );
			QRgb b((qBlue(a2)*c2  + qBlue(a3)*c3  + qBlue(a4)*c4 )  / norm );
			QRgb a((qAlpha(a2)*c2 + qAlpha(a3)*c3 + qAlpha(a4)*c4 ) / norm );
			QColor c(r, g, b, a);
			painter.setPen(c);
			painter.drawLine(n, 0, n, h);
		}
	}
	m_gradientLabel->setPixmap(QPixmap::fromImage(palette_image));

	// Rescale the gradient colors into the palette with a simple filter
	if (spread == QGradient::PadSpread)
	{
		QRgb fc(m_gradient[0]);
		flam3_palette_entry e = { 0., { qRed(fc)/255., qGreen(fc)/255., qBlue(fc)/255., qAlpha(fc)/255. }};
		for (int n = 0 ; n < begin_idx ; n++)
			p[n] = e;

		for (int n = begin_idx, j = 0 ; n < end_idx ; n++, j++)
			p[n] = ibuf[j];

		fc = m_gradient[GradientBufferLastIdx];
		e = (flam3_palette_entry){ 1., { qRed(fc)/255., qGreen(fc)/ 255., qBlue(fc)/255., qAlpha(fc)/255. }};
		for (int n = end_idx ; n < 256 ; n++)
			p[n] = e;
	}
	else if (spread == QGradient::RepeatSpread)
	{
		for (int n = begin_idx, j = 0 ; n < 256 ; n++, j++)
			p[n] = ibuf[j % ibuf_size];
		for (int n = begin_idx - 1, j = ibuf_size * 4096 - 1 ; n >= 0 ; n--, j--)
			p[n] = ibuf[j % ibuf_size];
	}
	else if (spread == QGradient::ReflectSpread)
	{
		for (int n = begin_idx, j = 0, h = 4096*ibuf_size -1 ; n < begin_idx + ibuf_size ; n++, j++, h--)
		{
			for (int k = n, q = n + ibuf_size ; k < 256 ; k += 2*ibuf_size, q += 2*ibuf_size )
			{
				p[k] = ibuf[j % ibuf_size];
				if (q < 256)
					p[q] = ibuf[h % ibuf_size];
			}
		}
		for (int n = begin_idx - 1, j = ibuf_size * 4096 - 1, h = 0 ; n >= begin_idx - ibuf_size ; n--, j--, h++)
		{
			for (int k = n, q = n - ibuf_size ; k >= 0 ; k -= 2*ibuf_size, q -= 2*ibuf_size )
			{
				p[k] = ibuf[h % ibuf_size];
				if (q >= 0)
					p[q] = ibuf[j % ibuf_size];
			}
		}
	}
	delete[] ibuf;

	setPaletteView();
	emit paletteChanged();
}

void PaletteEditor::createRandomGradientAction()
{
	int nstops = m_randomGradientSpinBox->value();
	while (nstops < 2)
		nstops = flam3_random01() * 128;
	GradientStops stops;
	for (int n = 0 ; n < nstops ; n++)
	{
		qreal idx = flam3_random01();
		int r = flam3_random01() * 255;
		int g = flam3_random01() * 255;
		int b = flam3_random01() * 255;
		GradientStop s(idx, QColor(r, g, b));
		stops << s;
	}
	m_gradientStops->setStops(stops);
}

void PaletteEditor::loadPalette(int palette_idx)
{
	flam3_get_palette(palette_idx, p, 0.0);
}

void PaletteEditor::setPaletteView()
{
	QSize s = m_paletteLabel->maximumSize();
	QImage palette(s.width(), s.height(), QImage::Format_RGB32);
	QPainter painter(&palette);
	painter.fillRect(palette.rect(), checkers);
	int rvalue = m_rotateSlider->value() ;
	int n = 0;
	for (int i = (255 - rvalue) % 255 ; n < 256 ; i = (i + 1) % 256, n++)
	{
		painter.setPen(QColor::fromRgbF(p[i].color[0], p[i].color[1], p[i].color[2], p[i].color[3]));
		painter.drawLine(n, 0, n, s.height() - 1);
	}
	m_paletteLabel->setPixmap(QPixmap::fromImage(palette));
}


void PaletteEditor::buildPaletteSelector()
{
	// only do this once, but it takes a while, so only when asked
	static bool built = false;
	if (built) return;
	built = true;
	logInfo("PaletteEditor::buildPaletteSelector : generating palettes");
	QSize s = m_palettesView->iconSize();
	for (int n = 0 ; n < PaletteCount ; n++)
	{
		flam3_palette p;
		flam3_get_palette(n, p, 0.0);
		QImage palette(s.width(), s.height(), QImage::Format_RGB32);
		QPainter painter(&palette);
		for (int i = 0 ; i < 256 ; i++)
		{
			painter.setPen(QColor::fromRgbF(p[i].color[0], p[i].color[1], p[i].color[2]));
			painter.drawLine(i, 0, i, s.height());
		}
		m_flamPalettes.addGradient(QPixmap::fromImage(palette));
	}
	if (!m_lastBrowseDir.isEmpty())
	{
		// restore p_stops for the initial call to resetGradientAction
		GradientStops tmp(p_stops);
		openGradientAction(true);
		p_stops = tmp;
	}

}


void PaletteEditor::paletteIndexChangedAction(const QModelIndex& idx)
{
	if (!idx.isValid()) return;
	loadPalette(idx.row());
	setPaletteView();
	emit paletteChanged();
	emit undoStateSignal();
}

void PaletteEditor::paletteRotatedAction(int /*idx*/)
{
	setPaletteView();
	emit paletteChanged();
}

// i'd hoped to avoid this type of call
void PaletteEditor::getPalette(flam3_palette in)
{
	int rvalue = m_rotateSlider->value() ;
	int n = 0;
	for (int i = (255-rvalue) % 255 ; n < 256 ; i = (i + 1) % 256, n++)
	{
		in[n].color[0] = p[i].color[0];
		in[n].color[1] = p[i].color[1];
		in[n].color[2] = p[i].color[2];
		in[n].color[3] = p[i].color[3];
	}
}

// and it's evil twin!
void PaletteEditor::setPalette(flam3_palette in)
{
	for (int n = 0 ; n < 256 ; n++)
	{
		p[n].color[0] = in[n].color[0];
		p[n].color[1] = in[n].color[1];
		p[n].color[2] = in[n].color[2];
		p[n].color[3] = in[n].color[3];
	}
	m_rotateSlider->blockSignals(true);
	m_rotateSlider->setSliderPosition(0);
	m_rotateSlider->blockSignals(false);
	setPaletteView();
}

// select a gradient from the browse list
void PaletteEditor::selectGradientAction(const QModelIndex& idx)
{
	if (idx.isValid())
	{
		if (hasUGR)
		{
			logFine("PaletteEditor::selectGradientAction : UGR %d", idx.row());
			setPalette(ugrList[idx.row()].pa);
			emit paletteChanged();
			emit undoStateSignal();
		}
		else
		{
			QFileInfo file = m_browseFileList.at(idx.row());
			logFine("PaletteEditor::selectGradientAction : selecting %d", idx.row());
			if ((file.suffix() == "ggr" && loadGIMPGradient(file, p))
			||  (file.suffix() == "cpt" && loadCPTGradient(file, p)))
			{
				m_gradientEnds->resetStops();
				m_gradientStops->setStops(p_stops);
				emit undoStateSignal();
			}
			else
				QMessageBox::warning(this, tr("Invalid path"),
					tr("The directory path %1 couldn't be opened.")
					.arg(file.absoluteFilePath()), QMessageBox::NoButton);
		}
	}
}

// open a and parse a directory containing gimp gradient files
void PaletteEditor::openGradientAction(bool noprompt)
{
	logFine(QString("PaletteEditor::openGradientAction : %1").arg(noprompt));
	QString openDir;
	if (noprompt)
	{
		openDir = m_lastBrowseDir;
		m_lastBrowseDir.clear();
	}
	else
	{
		openDir = QFileDialog::getOpenFileName(this,
			tr("Select a gradient file to load"),
			QFileInfo(m_lastBrowseDir).absoluteFilePath(),
			tr("Gradient Files (*.cpt *.ggr *.ugr)"));
	}

	if (openDir.isEmpty())
		return;

	if (openDir != m_lastBrowseDir)
		m_lastBrowseDir = openDir;

	if (openDir.indexOf(QRegExp(".*\\.ugr$")) >= 0)
	{
		logFine(QString("PaletteEditor::openGradientAction : ugr %1").arg(openDir));
		QFileInfo ugrFile(openDir);
		if (ugrFile.exists())
		{
			loadUGRGradients(ugrFile);
			QSize s = m_browseView->iconSize();
			QImage palette(s.width(), s.height(), QImage::Format_RGB32);
			QPainter painter(&palette);
			m_browsePalettes.clear();
			foreach (flam3_palette_t t, ugrList)
			{
				for (int n = 0 ; n < 256 ; n++)
				{
					painter.setPen(QColor::fromRgbF(t.pa[n].color[0], t.pa[n].color[1], t.pa[n].color[2]));
					painter.drawLine(n, 0, n, s.height());
				}
				m_browsePalettes.addGradient(QPixmap::fromImage(palette));
			}
			hasUGR=true;
			m_browseLineEdit->setText(m_lastBrowseDir);
		}
	}
	else
	{
		logFine("PaletteEditor::openGradientAction : load cpt,ggr");
		QFileInfo file(openDir);
		QFileInfo path(file);
		if (!path.isDir())
			path = QFileInfo(path.absolutePath());

		QDir dir(path.absoluteFilePath());
		QStringList filters;
		filters << "*.ggr" << "*.cpt";
		m_browseFileList = dir.entryInfoList(filters, QDir::Files, QDir::Name);
		m_browsePalettes.clear();
		m_browseLineEdit->setText(path.absoluteFilePath());
		// build the palette list
		foreach (QFileInfo i, m_browseFileList)
		{
			flam3_palette pa;
			if ((i.suffix() == "ggr" && loadGIMPGradient(i, pa))
			||  (i.suffix() == "cpt" && loadCPTGradient(i, pa)))
			{
				QSize s = m_browseView->iconSize();
				QImage palette(s.width(), s.height(), QImage::Format_RGB32);
				QPainter painter(&palette);
				painter.fillRect(0, 0, s.width(), s.height(), checkers);
				for (int n = 0 ; n < 256 ; n++)
				{
					painter.setPen(QColor::fromRgbF(pa[n].color[0], pa[n].color[1], pa[n].color[2], pa[n].color[3]));
					painter.drawLine(n, 0, n, s.height());
				}
				m_browsePalettes.addGradient(QPixmap::fromImage(palette));
				logFinest(QString("PaletteEditor::openGradientAction : adding %1")
					.arg(i.absoluteFilePath()));
			}
			else
			{
				logWarn(QString("PaletteEditor::openGradientAction : skipping %1")
					.arg(i.absoluteFilePath()));
				m_browseFileList.removeAll(i);
			}
		}
		ugrList.clear();
		hasUGR=false;
		if (!noprompt)
		{
			// now load the gradient selected in the file dialog.
			if (m_browseFileList.contains(file))
			{
				logFine(QString("PaletteEditor::openGradientAction : loading %1")
					.arg(file.fileName()));
				int row = m_browseFileList.indexOf(file);
				selectGradientAction(m_browsePalettes.index(row));
			}
			else
				logWarn(QString("PaletteEditor::openGradientAction : %1 not in list")
					.arg(file.fileName()));
		}
	}
}


bool PaletteEditor::loadUGRGradients(QFileInfo& file)
{
	logInfo(QString("PaletteEditor::loadUGRGradients : opening %1").arg(file.fileName()));
	QFile data(file.absoluteFilePath());
	if (!data.open(QFile::ReadOnly))
		return false;
	QTextStream is(&data);

	const double part = 256.0; // palette entries
	const double div  = part / 400; // entries / range
	QRegExp rx("index=(\\d+)\\s+color=(\\d+)");
	ugrList.clear();

	while (!is.atEnd())
	{
		flam3_palette pa;
		QString buf = is.readLine();
		// look for the first index
		while (! (rx.indexIn(buf) >= 0 || is.atEnd()))
			buf = is.readLine();
		if (is.atEnd()) break; // and break after the last palette in the file

		int pidx = 0;
		while (( rx.indexIn(buf) >= 0 ) && !is.atEnd())
		{
			int idx   = rx.cap(1).toInt();
			int color = rx.cap(2).toInt();
			int j = (int)((double)idx*div);
			int b = (color >> 16) & 0xff;
			int g = (color >> 8) & 0xff;
			int r = color & 0xff;
			while (pidx <= qMin(j, 255))
			{
				pa[pidx].color[0] = (double)r/255.;
				pa[pidx].color[1] = (double)g/255.;
				pa[pidx].color[2] = (double)b/255.;
				pa[pidx].color[3] = 1.;
				pidx++;
			}

			logFinest(QString("PaletteEditor::openUGRGradients : j=%1: %2,%3,%4")
				.arg(j).arg(pa[j].color[0]).arg(pa[j].color[1]).arg(pa[j].color[2]));

			buf = is.readLine();
		}
		ugrList.append(flam3_palette_t(pa));

	}

	return true;
}

bool PaletteEditor::loadGIMPGradient(QFileInfo& file, flam3_palette pa)
{
	logFinest(QString("PaletteEditor::loadGIMPGradient : %1").arg(file.fileName()));
	QFile data(file.absoluteFilePath());
	if (!data.open(QFile::ReadOnly))
		return false;
	QTextStream is(&data);

	int cpoints;
	QString buf = is.readLine();
	if (buf != "GIMP Gradient")
		return false;

	is >> buf;
	if (buf == "Name:")
		buf = is.readLine(); // buf now contains name of gradient
	is >> cpoints;
	p_stops.clear();
	double leftc,midc,rightc;
	double leftr,leftg,leftb,lefta;
	double rightr,rightg,rightb,righta;
	int blending, coloring, buckets;
	qreal rightc_idx_last(-1.0);
	QColor rightc_color_last;

	for (int n = 0 ; n < cpoints ; n++)
	{
		is >> leftc >> midc >> rightc
			>> leftr >> leftg >> leftb >> lefta
			>> rightr >> rightg >> rightb >> righta
			>> blending >> coloring ;

		if (blending != 0)
			logWarn(QString("PaletteEditor::loadGIMPGradient : %1 has unsupported "
			"blending mode %2 on line %3").arg(file.fileName()).arg(blending).arg(n+1));
		if (coloring < 0 || coloring > 2)
		{
			logWarn(QString("PaletteEditor::loadGIMPGradient : %1 has unsupported "
			"coloring mode %2 on line %3").arg(file.fileName()).arg(coloring).arg(n+1));
			coloring = 0;
		}

		qreal leftc_idx( leftc );
		QColor leftc_color( QColor::fromRgbF(leftr, leftg, leftb, lefta) );
		if (coloring > 0)
		{
			// convert to hsv color space
			leftr = leftc_color.hueF();
			leftg = leftc_color.saturationF();
			leftb = leftc_color.valueF();

			QColor c(QColor::fromRgbF(rightr, rightg, rightb, righta));
			rightr = c.hueF();
			rightg = c.saturationF();
			rightb = c.valueF();

		}

		if (leftc_idx == rightc_idx_last)
		{
			if (leftc_color == rightc_color_last)
				p_stops.pop_back();
			else
			{
				logWarn(QString("PaletteEditor::loadGIMPGradient : %1 has unmatched "
				"left/right color entries on lines %2 - %3").arg(file.fileName()).arg(n).arg(n+1));
				leftc_idx += 0.00001;
			}
		}
		else if (rightc_idx_last != -1)
		{
			logError(QString("PaletteEditor::loadGIMPGradient : %1 has unmatched "
			"left/right index entries on lines %2 - %3").arg(file.fileName()).arg(n).arg(n+1));
			leftc_idx += 0.00001;
		}
		p_stops << GradientStop(leftc_idx, leftc_color, coloring);
		int pstart = (int)(leftc * 256.);
		int pmid   = (int)(midc  * 256.);
		int pend   = (int)(rightc * 256.);
		buckets = pmid - pstart;

		double rdx = (rightr - leftr)/2.;
		double gdx = (rightg - leftg)/2.;
		double bdx = (rightb - leftb)/2.;
		double adx = (righta - lefta)/2.;

		// rotate a full 360 degrees in hsv colorspace
		if (coloring == 1 && rdx == 0.)
			rdx = 0.5;
		else if (coloring == 2 && rdx == 0.)
			rdx = -0.5;

		double rinc = rdx / buckets;
		double ginc = gdx / buckets;
		double binc = bdx / buckets;
		double ainc = adx / buckets;

		for (int j = pstart ; j < pmid ; j++)
		{
			if (coloring == 0)
			{
				pa[j].color[0] = qBound(0., leftr, 1.0);
				pa[j].color[1] = qBound(0., leftg, 1.0);
				pa[j].color[2] = qBound(0., leftb, 1.0);
				pa[j].color[3] = qBound(0., lefta, 1.0);
			}
			else
			{
				if (leftr >= 1.0)
					leftr -= 1.0;
				else if (leftr < 0)
					leftr += 1.0;
				QColor hsv(QColor::fromHsvF(
					qBound(0., leftr, 1.0),
					qBound(0., leftg, 1.0),
					qBound(0., leftb, 1.0),
					qBound(0., lefta, 1.0)));
				pa[j].color[0] = hsv.redF();
				pa[j].color[1] = hsv.greenF();
				pa[j].color[2] = hsv.blueF();
				pa[j].color[3] = hsv.alphaF();
			}
			leftr += rinc;
			leftg += ginc;
			leftb += binc;
			lefta += ainc;
		}
		qreal midc_idx( midc );
		QColor midc_color;
		if (coloring == 0)
			midc_color = QColor::fromRgbF(
			qBound(qreal(0.0), leftc_color.redF() + rdx, qreal(1.0)),
			qBound(qreal(0.0), leftc_color.greenF() + gdx, qreal(1.0)),
			qBound(qreal(0.0), leftc_color.blueF() + bdx, qreal(1.0)),
			qBound(qreal(0.0), leftc_color.alphaF() + adx, qreal(1.0)) );
		else
		{
			qreal h = leftc_color.hueF() + rdx;
			if (h >= 1.0)
				h -= 1.0;
			else if (h < 0)
				h += 1.0;
			midc_color = QColor::fromHsvF(
			qBound(qreal(0.0), h , qreal(1.0)),
			qBound(qreal(0.0), leftc_color.saturationF() + gdx, qreal(1.0)),
			qBound(qreal(0.0), leftc_color.valueF() + bdx, qreal(1.0)),
			qBound(qreal(0.0), leftc_color.alphaF() + adx, qreal(1.0)) );
		}

		if (midc_idx == leftc_idx)
		{
			logWarn(QString("PaletteEditor::loadGIMPGradient : %1 has right "
			"adjusting midc entry on line %2").arg(file.fileName()).arg(n+1));
			midc_idx += 0.00001;
		}
		p_stops << GradientStop(midc_idx, midc_color, coloring);
		buckets = pend - pmid;
		rinc = rdx / buckets;
		ginc = gdx / buckets;
		binc = bdx / buckets;
		ainc = adx / buckets;
		for (int j = pmid ; j < pend ; j++)
		{
			if (coloring == 0)
			{
				pa[j].color[0] = qBound(0., leftr, 1.0);
				pa[j].color[1] = qBound(0., leftg, 1.0);
				pa[j].color[2] = qBound(0., leftb, 1.0);
				pa[j].color[3] = qBound(0., lefta, 1.0);
			}
			else
			{
				if (leftr >= 1.0)
					leftr -= 1.0;
				else if (leftr < 0)
					leftr += 1.0;

				QColor hsv(QColor::fromHsvF(
					qBound(0., leftr, 1.0),
					qBound(0., leftg, 1.0),
					qBound(0., leftb, 1.0),
					qBound(0., lefta, 1.0)));
				pa[j].color[0] = hsv.redF();
				pa[j].color[1] = hsv.greenF();
				pa[j].color[2] = hsv.blueF();
				pa[j].color[3] = hsv.alphaF();
			}
			leftr += rinc;
			leftg += ginc;
			leftb += binc;
			lefta += ainc;
		}
		rightc_idx_last = rightc;

		if (coloring == 0)
			rightc_color_last = QColor::fromRgbF(rightr, rightg, rightb, righta);
		else
			rightc_color_last = QColor::fromHsvF(rightr, rightg, rightb, righta);

		if (rightc_idx_last == midc_idx)
		{
			logWarn(QString("PaletteEditor::loadGIMPGradient : %1 has left "
			"adjusting midc entry on line %2").arg(file.fileName()).arg(n+1));
			midc_idx -= 0.000001;
			p_stops.pop_back();
			GradientStop astop(midc_idx, midc_color, coloring);
			p_stops<<astop;
		}
		p_stops << GradientStop(rightc_idx_last, rightc_color_last, coloring);
	}
	return true;
}

// a silly cpt gradient file parser n' stuffer.  this thing
// is not very robust.  cpt files with problems/typos will break it!
bool PaletteEditor::loadCPTGradient(QFileInfo& file, flam3_palette pa)
{
	logFinest(QString("PaletteEditor::loadCPTGradient : parsing %1").arg(file.fileName()));
	QFile data(file.absoluteFilePath());
	if (!data.open(QFile::ReadOnly))
		return false;
	QTextStream is(&data);

	QString buf = is.readLine();
	qint64 pos = is.pos();
	// skip comments
	while (buf.startsWith("#"))
	{
		pos = is.pos();
		buf = is.readLine();
	}
	is.seek(pos);

	double z0;
	double z1 = -1.0;
	double z0r, z0g, z0b, z1r, z1g, z1b;
	double offset = 0.;

	// first figure out the gradient range, and map
	// that into the palette's range (256)
	qint64 start_pos = pos;
	is >> z0 >> z0r >> z0g >> z0b >> z1 >> z1r >> z1g >> z1b;
	double idx_start = z0; // start position is always first
	if (idx_start != 0.0)
	{
		// make all positions >= 0
		offset = idx_start * -1.0;
		idx_start = 0.0;
	}
	QRegExp rx("^\\s*[BFN]\\s+\\d+");
	while (!is.atEnd())
	{
		pos = is.pos();
		buf = is.readLine();

		if (buf.isEmpty())
			continue;
		else if ( rx.indexIn(buf) != -1 )
			break;
		else
		{
			is.seek(pos);
			is >> z0 >> z0r >> z0g >> z0b >> z1 >> z1r >> z1g >> z1b;
			is.readLine();
		}
	}
	// end position is in the last entry
	double idx_end = z1 + offset;
	logFinest("PaletteEditor::loadCPTGradient : idx range %d %d", idx_start, idx_end);
	// now go back and try again
	is.seek(start_pos);
	z1 = idx_start;

	const double part = 256.0; // palette entries
	const double div  = part / (idx_end - idx_start); // entries / range
	qreal z1_idx_last(-1.0);
	QColor z1_color_last;
	p_stops.clear();
	while ((z1 < (idx_end - offset)) && is.pos() > 0)
	{
		is >> z0 >> z0r >> z0g >> z0b >> z1 >> z1r >> z1g >> z1b;
		is.readLine();
		// move all indexes up, and use reals for rgb
		z0 += offset;  z1 += offset;
		int pstart = qRound(z0 * div);
		int pend   = qRound(z1 * div);

		qreal z0_idx( (qreal)z0 / idx_end );
		QColor z0_color(QColor::fromRgb(z0r, z0g, z0b));
		if (z0_idx == z1_idx_last)
		{
			if (z0_color == z1_color_last)
				p_stops.pop_back();
			else
			{
				logWarn("PaletteEditor::loadCPTGradient : %s has right adjusting "
				"z0 entry at pos %d", file.fileName().toLatin1().constData(), is.pos());
				z0_idx += 0.0001;
			}
		}
		p_stops << GradientStop(z0_idx,  z0_color);
		z0r /= part; z0g /= part ; z0b /= part ;
		double pdx = pend - pstart ;
		double rdx = (z1r / part - z0r) / pdx ;
		double gdx = (z1g / part - z0g) / pdx;
		double bdx = (z1b / part - z0b) / pdx;
		logFinest(QString("PaletteEditor::loadCPTGradient : z0 %1 \t %2 %3 %4")
			.arg(z0).arg(z0r).arg(z0g).arg(z0b));
		logFinest(QString("PaletteEditor::loadCPTGradient : z1 %1 \t %2 %3 %4")
			.arg(z1).arg(z1r / part).arg(z1g / part).arg(z1b / part));

		int j = pstart;
		for ( ; j < pend && j <= 255; j++)
		{
			pa[j].color[0] = qBound(0., z0r, 1.) ;
			pa[j].color[1] = qBound(0., z0g, 1.) ;
			pa[j].color[2] = qBound(0., z0b, 1.) ;
			pa[j].color[3] = 1. ;
			logFinest(QString("PaletteEditor::loadCPTGradient : p[%1] \t %2 %3 %4")
				.arg(j).arg(p[j].color[0]).arg(p[j].color[1]).arg(p[j].color[2]));
			z0r += rdx ;
			z0g += gdx;
			z0b += bdx;
		}
		z1_idx_last = (qreal)z1 / idx_end;
		z1_color_last = QColor::fromRgb(z1r, z1g, z1b);
		p_stops << GradientStop(z1_idx_last,  z1_color_last);
		if (j >= 255) break;
	}
	return true;
}

QString PaletteEditor::lastBrowseDir() const
{
	return m_lastBrowseDir;
}

void PaletteEditor::setLastBrowseDir(const QString& value)
{
	m_lastBrowseDir = value;
}

void PaletteEditor::browsePathChangedAction()
{
	QString text( m_browseLineEdit->text() );
	if (text != m_lastBrowseDir)
	{
		QFileInfo file(text);
		if (file.exists())
		{
			m_lastBrowseDir = file.absoluteFilePath();
			openGradientAction(true);
		}
		else
			QMessageBox::information(this, tr("Invalid path"),
				tr("The gradient path %1 doesn't exist.")
				.arg(file.absoluteFilePath()), QMessageBox::NoButton);
	}
}
