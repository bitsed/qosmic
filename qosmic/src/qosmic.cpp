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

#include <QApplication>
#include <QTranslator>
#include <QFileInfo>

#include "qosmic.h"
#include "logger.h"
#include "mainwindow.h"

using namespace Util;

int main(int argc, char* argv[])
{
	Q_INIT_RESOURCE(qosmic);
	QCoreApplication::setOrganizationName("qosmic");
	QCoreApplication::setApplicationName("qosmic");

	QApplication app(argc, argv);
	app.setWindowIcon(QIcon(":icons/qosmic.xpm"));

	// Initialize the logger
	Logger::getInstance()->setLevel(Logger::levelFor(getenv("log")));
	logInfo(QString("main() : Qosmic (version %1)").arg(QOSMIC_VERSION));

	// Load translations if necessary
	QTranslator translator;
	{
		QString locale = QLocale::system().name();
		logInfo("main() : system locale is %s", locale.toLatin1().constData());
		QString qmDir(QOSMIC_TRANSDIR);
		QString qmFile(QString("qosmic_") + locale);

		if (translator.load(qmFile, qmDir))
		{
			logInfo(QString("main() : installing translations for %1").arg(locale));
			app.installTranslator(&translator);
		}
		else
		{
			logInfo(QString("main() : no translations found for locale %1").arg(locale));
			logInfo("main() : using default locale");
		}
	}

	if (!QFileInfo(getenv("flam3_palettes")).exists())
	{
		// look for flam3 palettes
		QStringList palettes;
		palettes << QString(QOSMIC_FLAM3DIR).append("/flam3-palettes.xml")
				 << "./flam3-palettes.xml"
				 << "./flam3/flam3-palettes.xml" ;

		bool no_palette(true);
		foreach (QString palette, palettes)
			if (QFileInfo(palette).exists())
			{
				putenv(QString("flam3_palettes=%1").arg(palette).toLatin1().data());
				no_palette = false;
				break;
			}

		if (no_palette)
		{
			cerr << QString(QCoreApplication::translate("CoreApp", "Error: "
														"No palettes file found at:\n"
														"%1\n%2\n%3\n%4\n\n"
														"The flam3 palettes xml file could not\n"
														"be found.  You can set this path using\n"
														"the flam3_palettes environment variable.\n\n"
														"For example:\n"
														"flam3_palettes=/some/path-to/flam3-palettes.xml qosmic"))
					.arg(palettes[0], palettes[1], palettes[2], getenv("flam3_palettes")) << endl;
			return 1;
		}
	}

	if (argc > 1 &&
			QString(argv[1]).contains(QRegExp("--?(?:help|h|ver).*")))
	{
		cout << QString(QCoreApplication::translate("CoreApp", "Qosmic %1\n"
			"Usage: qosmic [flam3 file]\n\n"
			"environment variables:\n"
			"log=%2\n"
			"flam3_verbose=%3\n"
			"flam3_nthreads=%4\n"
			"flam3_palettes=%5"))
			.arg(QOSMIC_VERSION)
			.arg(Logger::getInstance()->level())
			.arg(QString(getenv("flam3_verbose")).toInt())
			.arg(QString(getenv("flam3_nthreads")).toInt() > 0 ?
				QString(getenv("flam3_nthreads")).toInt() : flam3_count_nthreads())
			.arg(getenv("flam3_palettes"))
			<< endl;
		return 0;
	}

	MainWindow* mw = new MainWindow();
	QString fname(QOSMIC_AUTOSAVE);
	if (argc > 1)
	{
		fname = QString(argv[1]);
		if (!mw->loadFile(fname))
		{
			cerr << QString(QCoreApplication::translate("CoreApp",
				"Couldn't load file %1")).arg(fname) << endl;
			return 1;
		}
	}
	else if (QFileInfo(fname).exists())
	{
		mw->loadFile(fname);
		mw->setCurrentFile("");
	}
	else
		mw->setFlameXML();

	mw->show();
	logInfo("main() : qosmic started");
	return app.exec();
}


