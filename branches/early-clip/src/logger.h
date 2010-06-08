/***************************************************************************
 *   Copyright (C) 2007, 2010 by David Bitseff                             *
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
#ifndef INCLUDE_LOGGER_H
#define INCLUDE_LOGGER_H

#include<QMutex>

#include "flam3util.h"

namespace Util
{
class Logger
{
	private:
		static Logger* m_self;
		int m_level;
		QTextStream* m_stream;
		QMutex m_mutex;

	protected:
		Logger(QTextStream&);

	public:
		~Logger();
		static Logger* getInstance();
		static int levelFor(char*);
		void logMessage(const QString&);
		void setLevel(int);
		int level();

		void info(QString&);
		void warn(QString&);
		void error(QString&);
		void critical(QString&);
		void fine(QString&);
		void finer(QString&);
		void finest(QString&);

		void info( const QString&);
		void warn( const QString&);
		void error( const QString&);
		void critical( const QString&);
		void fine( const QString&);
		void finer( const QString&);
		void finest( const QString&);

		void info(const char*);
		void warn(const char*);
		void error(const char*);
		void critical(const char*);
		void fine(const char*);
		void finer(const char*);
		void finest(const char*);


		Logger& operator<<(QString&);
		Logger& operator<<(const QString&);
		Logger& operator<<(const char*);
		Logger& operator<<(int);
		Logger& operator<<(double);

		enum LogLevel {  CRITICAL, ERROR, NONE, WARN, INFO,
							FINE, FINER, FINEST } ;

};
}

#define toNum(n) QString::number(n)

#define logCrit(s) Util::Logger::getInstance()->critical(s)
#define logError(s) Util::Logger::getInstance()->error(s)
#define logWarn(s) Util::Logger::getInstance()->warn(s)

#ifdef LOGGING

#define logInfo(s) Util::Logger::getInstance()->info(s)
#define logFine(s) Util::Logger::getInstance()->fine(s)
#define logFiner(s) Util::Logger::getInstance()->finer(s)
#define logFinest(s) Util::Logger::getInstance()->finest(s)

#else

#define logInfo(s)
#define logFine(s)
#define logFiner(s)
#define logFinest(s)

#endif

#endif
