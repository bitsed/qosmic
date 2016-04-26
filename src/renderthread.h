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
#ifndef RENDERTHREAD_H
#define RENDERTHREAD_H

#include <QImage>
#include <QTime>
#include <QStatusBar>
#include <QThread>
#include <QMutex>
#include <QQueue>

#include "flam3util.h"

/**
  * Clients submit a RenderRequest to the RenderThread which calls
  * flam3_render().  A RenderResponse is emitted from the RenderThread once the
  * requested work is completed.  A RenderRequest's Type determines how the work
  * is scheduled in relation to other currently running requests.
  */
class RenderRequest
{
	public:
		enum Type { Preview, Image, File, Queued } ;

	private:
		flam3_genome* m_genome;
		flam3_genome m_genome_template;
		double m_time;
		int m_ngenomes;
		Type m_type;
		QSize m_size;
		QString m_name;
		QImage m_image;
		bool m_finished;
		QMutex m_img_mutex;

	public:
		RenderRequest(flam3_genome* g=0, QSize s=QSize(),
						QString n=QString(), Type t=Queued);

		void setImagePresets(flam3_genome);
		flam3_genome* imagePresets();
		void setGenome(flam3_genome*);
		flam3_genome* genome() const;
		void setTime(double);
		double time() const;
		void setNumGenomes(int);
		int numGenomes() const;
		void setType(const Type&);
		Type type() const;
		void setSize(const QSize&);
		QSize size() const;
		void setName(const QString&);
		QString name() const;
		void setImage(QImage&);
		QImage& image();
		void setFinished(bool);
		bool finished() const;
};
typedef QList<RenderRequest*> RenderRequestList;

/**
  * The RenderThread emits a RenderEvent once the work associated with a
  * RenderRequest has completed.
  */
class RenderEvent
{
	RenderRequest* m_request;
	bool m_accepted;

	public:
		RenderEvent();
		void accept(bool=true);
		bool accepted() const;
		void setRequest(RenderRequest*);
		RenderRequest* request() const;
};


/**
 * Since the progress callback to flam3_render() uses C linkage, this helper
 * class allows a status message to be updated asynchronously.  Implement this
 * to have the current status given to your class.
 */
class RenderStatus : public QObject
{
	Q_OBJECT

	public:
		enum Flag { Busy, Killed, Idle }  ;
		Flag State;
		RenderRequest::Type Type;
		QString Name;
		QTime Runtime;
		QTime EstRemain;
		double Percent;

		RenderStatus();
		RenderStatus(const RenderStatus&);
		RenderStatus& operator=(const RenderStatus&);
		QString getMessage();
		QString getEstRemain();
		QString getRuntime();
		void createMessage();

	private:
		QString message;
		QString estRemainString;
		QString runtimeString;
};

// The RenderThread is the StatusProvider
class StatusProvider
{
	public:
		virtual RenderStatus& getStatus() =0;
		virtual ~StatusProvider() =0 ;
};

// The thread that moves StatusProvider info to the StatusWatchers
class StatusObserver : public QThread
{
	Q_OBJECT

	public:
		StatusObserver(StatusProvider*);
		void start();
		void run();

		bool running;
		StatusProvider* provider;
		RenderStatus status;

	signals:
		void statusUpdated(RenderStatus*);
};


/**
 * This is the thread that schedules calls to the flam3_render() function.
 * RenderThread serializes all calls to flam3_render(). Clients are notified
 * when their jobs are finished.  Clients submit a RenderRequest to this class,
 * and they catch RenderResponse signals when a job is complete.
 */
class RenderThread : public QThread, public StatusProvider
{
	Q_OBJECT

	public:
		enum ImageFormat { RGB32, ARGB32_OPAQUE, ARGB32_TRANS } ;

	private:
		static bool _stop_current_job;
		static double _est_remain;
		static double _percent_finished;
		static stat_struct _stats;
		static QTime ptimer;
		static int _progress_callback(void*, double, int, double);
		static RenderThread* singleInstance;

		flam3_frame flame;
		RenderRequest* preview_request;
		RenderRequest* image_request;
		QList<RenderEvent*> event_list;
		QQueue<RenderRequest*> request_queue;
		QMutex rqueue_mutex;
		RenderRequest* current_request;
		RenderStatus status;

		QString msg;
		bool rendering;
		bool render_loop_flag;
		bool kill_all_jobs;
		QImage img_buf;
		StatusObserver* so;
		void init_status_cb();
		int channels;
		int alpha_trans;
		int millis;
		ImageFormat img_format;
		QString rtype;

		RenderThread();

	public:
		QMutex running_mutex;
		bool running; // flag to kill thread
		static RenderThread* getInstance();
		~RenderThread();
		virtual void run();
		RenderStatus& getStatus();
		double finished();
		bool isRendering();
		void setFormat(ImageFormat);
		ImageFormat format() const;
		bool earlyClip() const;
		void setEarlyClip(bool);
		RenderRequest* current() const;
		void start();
		void render(RenderRequest*);
		void cancel(RenderRequest*);

	public slots:
		void stopRendering();
		void stop();
		void killAll();

	signals:
		void flameRenderingKilled();
		void flameRendered(RenderEvent*);
		void statusUpdated(RenderStatus*);
};



#endif
