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
#ifndef INCLUDE_LOGGER_H
#define INCLUDE_LOGGER_H

#include <QMutex>
#include <QTextStream>

// undef these tokens to avoid conflicts with other
// environments that might define them (win32).
#undef CRITICAL
#undef ERROR
#undef NONE
#undef WARN
#undef INFO
#undef FINE
#undef FINER
#undef FINEST


extern QTextStream cout;
extern QTextStream cerr;

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
		Logger();
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

		void info(const char*, ...);
		void warn(const char*, ...);
		void error(const char*, ...);
		void critical(const char*, ...);
		void fine(const char*, ...);
		void finer(const char*, ...);
		void finest(const char*, ...);


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

#define logCrit(...) Util::Logger::getInstance()->critical(__VA_ARGS__)
#define logError(...) Util::Logger::getInstance()->error(__VA_ARGS__)
#define logWarn(...) Util::Logger::getInstance()->warn(__VA_ARGS__)

#ifdef LOGGING

#define logInfo(...) Util::Logger::getInstance()->info(__VA_ARGS__)
#define logFine(...) Util::Logger::getInstance()->fine(__VA_ARGS__)
#define logFiner(...) Util::Logger::getInstance()->finer(__VA_ARGS__)
#define logFinest(...) Util::Logger::getInstance()->finest(__VA_ARGS__)

#else

#define logInfo(...)
#define logFine(...)
#define logFiner(...)
#define logFinest(...)

#endif

#endif
