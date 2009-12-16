/***************************************************************************
 *   Copyright (C) 2007 by David Bitseff                                   *
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

#include <QApplication>
#include <QTranslator>
#include <QFileInfo>
#include <QDir>

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
	app.setWindowIcon(QIcon(":icons/qosmicicon.xpm"));

	// Initialize the logger
	Logger::getInstance()->setLevel(Logger::levelFor(getenv("log")));
	logInfo(QString("main() : Qosmic (version %1)").arg(QOSMIC_VERSION));

	// Load translations if necessary
	QString locale = QLocale::system().name();
	QTranslator translator;
	if (locale.contains(QRegExp("^en|C|POSIX")))
		logInfo("main() : using default locale");
	else
	{
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

	// look for flam3 palettes
	QString palettes(QOSMIC_FLAM3DIR);
	if (palettes.size() != 0)
		palettes += "/flam3-palettes.xml";

	if (QFileInfo(palettes).exists())
		setenv("flam3_palettes", palettes.toAscii().data(), 0);
	else if (QFileInfo("./flam3-palettes.xml").exists())
		setenv("flam3_palettes","./flam3-palettes.xml", 0);
	else if (QFileInfo("./flam3/flam3-palettes.xml").exists())
		setenv("flam3_palettes","./flam3/flam3-palettes.xml", 0);
	else if (!QFileInfo(getenv("flam3_palettes")).exists())
	{
		cerr << QString(QCoreApplication::translate("CoreApp", "Error: "
				"No palettes file found in %1\n\n\n"
				"./flam3-palettes.xml\n"
				"./flam3/flam3-palettes.xml\n%2\n\n"
				"The flam3 palettes xml file could not\n"
				"be found.  You can set this path using\n"
				"the flam3_palettes environment variable.\n\n"
				"For example:\n"
				"flam3_palettes=/some/path-to/flam3-palettes.xml qosmic"))
				.arg(palettes).arg(getenv("flam3_palettes")) << endl;
		return 1;
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
				QString(getenv("flam3_nthreads")).toInt() : NTHREADS)
			.arg(getenv("flam3_palettes"))
			<< endl;
		return 0;
	}

	MainWindow* mw = new MainWindow();
	QString fname = QDir::home().absoluteFilePath(QOSMIC_TMP_FILE);
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


