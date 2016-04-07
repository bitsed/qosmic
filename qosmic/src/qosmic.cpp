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
		QLocale locale;
		logInfo(QString("main() : system locale is %1").arg(locale.name()));

		if (translator.load(locale, "qosmic", "_", ":/ts", ".qm"))
		{
			logInfo(QString("main() : installing translations for %1").arg(locale.name()));
			app.installTranslator(&translator);
		}
		else
			logInfo(QString("main() : no translations found for %1").arg(locale.name()));
	r

	if (argc > 1 &&
			QString(argv[1]).contains(QRegExp("--?(?:help|h|ver).*")))
	{
		cout << QString(QCoreApplication::translate("CoreApp", "Qosmic %1\n"
			"Usage: qosmic [flam3 file]\n\n"
			"environment variables:\n"
			"log=%2\n"
			"flam3_verbose=%3\n"
			"flam3_nthreads=%4\n"))
			.arg(QOSMIC_VERSION)
			.arg(Logger::getInstance()->level())
			.arg(QString(getenv("flam3_verbose")).toInt())
			.arg(QString(getenv("flam3_nthreads")).toInt() > 0 ?
				QString(getenv("flam3_nthreads")).toInt() : flam3_count_nthreads())
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


