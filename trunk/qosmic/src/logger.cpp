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
#include <QThread>
#include <QMutexLocker>
#include <cstdarg>

#include "logger.h"

QTextStream cout(stdout);
QTextStream cerr(stdout);

namespace Util
{

Logger* Logger::m_self = 0;// initialize pointer

Logger::Logger()
{
	m_stream = &cout;
	m_level = INFO;
}

Logger::Logger(QTextStream& out)
{
	m_stream = &out;
	m_level = INFO;
}

Logger* Logger::getInstance()
{
	if (m_self == 0)
		m_self = new Logger(cout);
	return m_self;
}

void Logger::info(QString& msg)
{
  QMutexLocker locker(&m_mutex);
  if(m_level >= INFO)
	*m_stream << QString("info [%1]: ").
	  arg((long) QThread::currentThreadId()) << msg << endl;
}

void Logger::warn(QString& msg)
{
  QMutexLocker locker(&m_mutex);
  if(m_level >= WARN)
	*m_stream << QString("warn [%1]: ").
	  arg((long) QThread::currentThreadId()) << msg << endl;
}

void Logger::error(QString& msg)
{
  QMutexLocker locker(&m_mutex);
  if(m_level >= ERROR)
	*m_stream << QString("error [%1]: ").
	  arg((long) QThread::currentThreadId()) << msg << endl;
}

void Logger::critical(QString& msg)
{
  QMutexLocker locker(&m_mutex);
  if(m_level >= CRITICAL)
	*m_stream << QString("critical [%1]: ").
	  arg((long) QThread::currentThreadId()) << msg << endl;
}

void Logger::fine(QString& msg)
{
  QMutexLocker locker(&m_mutex);
  if(m_level >= FINE)
	*m_stream << QString("fine [%1]: ").
	  arg((long) QThread::currentThreadId()) << msg << endl;
}

void Logger::finer(QString& msg)
{
  QMutexLocker locker(&m_mutex);
  if(m_level >= FINER)
	*m_stream << QString("finer [%1]: ").
	  arg((long) QThread::currentThreadId()) << msg << endl;
}

void Logger::finest(QString& msg)
{
  QMutexLocker locker(&m_mutex);
  if(m_level >= FINEST)
	*m_stream << QString("finest [%1]: ").
	  arg((long) QThread::currentThreadId()) << msg << endl;
}

void Logger::info(const QString& msg)
{
  QMutexLocker locker(&m_mutex);
  if(m_level >= INFO)
	*m_stream << QString("info [%1]: ").
	  arg((long) QThread::currentThreadId()) << msg << endl;
}

void Logger::warn(const QString& msg)
{
  QMutexLocker locker(&m_mutex);
  if(m_level >= WARN)
	*m_stream << QString("warn [%1]: ").
	  arg((long) QThread::currentThreadId()) << msg << endl;
}

void Logger::error(const QString& msg)
{
  QMutexLocker locker(&m_mutex);
  if(m_level >= ERROR)
	*m_stream << QString("error [%1]: ").
	  arg((long) QThread::currentThreadId()) << msg << endl;
}

void Logger::critical(const QString& msg)
{
  QMutexLocker locker(&m_mutex);
  if(m_level >= CRITICAL)
	*m_stream << QString("critical [%1]: ").
	  arg((long) QThread::currentThreadId()) << msg << endl;
}

void Logger::fine(const QString& msg)
{
  QMutexLocker locker(&m_mutex);
  if(m_level >= FINE)
	*m_stream << QString("fine [%1]: ").
	  arg((long) QThread::currentThreadId()) << msg << endl;
}

void Logger::finer(const QString& msg)
{
  QMutexLocker locker(&m_mutex);
  if(m_level >= FINER)
	*m_stream << QString("finer [%1]: ").
	  arg((long) QThread::currentThreadId()) << msg << endl;
}

void Logger::finest(const QString& msg)
{
  QMutexLocker locker(&m_mutex);
  if(m_level >= FINEST)
	*m_stream << QString("finest [%1]: ").
	  arg((long) QThread::currentThreadId()) << msg << endl;
}


void Logger::info(const char* msg, ...)
{
	QMutexLocker locker(&m_mutex);
	if(m_level >= INFO)
	{
		va_list ap;
		va_start(ap, msg);
		*m_stream << QString("info [%1]: ").arg((long)QThread::currentThreadId())
				  << QString().vsprintf(msg, ap)
				  << endl;
		va_end(ap);
	}
}

void Logger::warn(const char* msg, ...)
{
	QMutexLocker locker(&m_mutex);
	if(m_level >= WARN)
	{
		va_list ap;
		va_start(ap, msg);
		*m_stream << QString("warn [%1]: ").arg((long)QThread::currentThreadId())
				  << QString().vsprintf(msg, ap)
				  << endl;
		va_end(ap);
	}
}

void Logger::error(const char* msg, ...)
{
	QMutexLocker locker(&m_mutex);
	if(m_level >= ERROR)
	{
		va_list ap;
		va_start(ap, msg);
		*m_stream << QString("error [%1]: ").arg((long)QThread::currentThreadId())
				  << QString().vsprintf(msg, ap)
				  << endl;
		va_end(ap);
	}
}

void Logger::critical(const char* msg, ...)
{
	QMutexLocker locker(&m_mutex);
	if(m_level >= CRITICAL)
	{
		va_list ap;
		va_start(ap, msg);
		*m_stream << QString("critical [%1]: ").arg((long)QThread::currentThreadId())
				  << QString().vsprintf(msg, ap)
				  << endl;
		va_end(ap);
	}
}

void Logger::fine(const char* msg, ...)
{
	QMutexLocker locker(&m_mutex);
	if(m_level >= FINE)
	{
		va_list ap;
		va_start(ap, msg);
		*m_stream << QString("fine [%1]: ").arg((long)QThread::currentThreadId())
				  << QString().vsprintf(msg, ap)
				  << endl;
		va_end(ap);
	}
}

void Logger::finer(const char* msg, ...)
{
	QMutexLocker locker(&m_mutex);
	if(m_level >= FINER)
	{
		va_list ap;
		va_start(ap, msg);
		*m_stream << QString("finer [%1]: ").arg((long)QThread::currentThreadId())
				  << QString().vsprintf(msg, ap)
				  << endl;
		va_end(ap);
	}
}

void Logger::finest(const char* msg, ...)
{
	QMutexLocker locker(&m_mutex);
	if(m_level >= FINEST)
	{
		va_list ap;
		va_start(ap, msg);
		*m_stream << QString("finest [%1]: ").arg((long)QThread::currentThreadId())
				  << QString().vsprintf(msg, ap)
				  << endl;
		va_end(ap);
	}
}


void Logger::setLevel(int level)
{
	m_level = level;
}

int Logger::level()
{
	return m_level;
}

Logger::~Logger()
{
}

int Logger::levelFor(char* c)
{
	QString s = QString(c).toUpper();
	if (s.isEmpty() || s == "NONE")
		return Logger::NONE;
	else if (s == "INFO")
		return Logger::INFO;
	else if (s == "WARN")
		return Logger::WARN;
	else if (s == "ERROR")
		return Logger::ERROR;
	else if (s == "CRITICAL")
		return Logger::CRITICAL;
	else if (s == "FINE")
		return Logger::FINE;
	else if (s == "FINER")
		return Logger::FINER;
	else if (s == "FINEST")
		return Logger::FINEST;
	else if (!s.isEmpty())
		return qBound((int)NONE, s.toInt(), (int)FINEST);
	return 0;
}

Logger & Logger::operator <<( QString & msg)
{
	logMessage(QString(msg));
	return *m_self;
}

Logger & Logger::operator <<( const QString & msg)
{
	logMessage(msg);
	return *m_self;
}

Logger & Logger::operator <<( const char * msg)
{
	logMessage(QString(msg));
	return *m_self;
}

Logger & Logger::operator <<( int msg)
{
	logMessage(QString::number(msg));
	return *m_self;
}

Logger & Logger::operator <<( double msg)
{
	logMessage(QString::number(msg));
	return *m_self;
}

void Logger::logMessage(const QString & msg)
{
	switch (m_level)
	{
		case ERROR:
			error(msg);
			break;
		case CRITICAL:
			critical(msg);
			break;
		case NONE:
			break;
		case WARN:
			warn(msg);
			break;
		case INFO:
			info(msg);
			break;
		case FINE:
			fine(msg);
			break;
		case FINER:
			finer(msg);
			break;
		case FINEST:
			finest(msg);
	}
}

}
