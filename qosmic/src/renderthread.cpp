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
#include <QFileInfo>

#include "renderthread.h"
#include "flam3util.h"
#include "logger.h"


// global 'static' vars used in callback. they're prefixed with an underscore.
bool   RenderThread::_stop_current_job = false;
double RenderThread::_est_remain;
double RenderThread::_percent_finished;
stat_struct RenderThread::_stats;

// singleton instance
RenderThread* RenderThread::singleInstance = 0;


QTime RenderThread::ptimer;
/**
 * this callback is needed to control the rendering function.  it also
 * helps calculate the estimated time remaining.
*/
int RenderThread::_progress_callback(
		void* /*parameter*/, double /*vari*/, int /*varn*/, double est)
{
	if (est != 0.0)
	{
		double elapsed = ptimer.elapsed() ;
		_est_remain = est * 1000.0;
		_percent_finished = elapsed / (_est_remain + elapsed) * 100.0;
	}

	if (_stop_current_job)
		return 1;

	return 0;
}

void RenderThread::init_status_cb()
{
	_est_remain = 0.0;
	_percent_finished = 0.0;
}

RenderThread::RenderThread() :
	rendering(false),
	kill_all_jobs(false),
	millis(0),
	running(true)
{
	// stuff to control the flam3_render function
	render_loop_flag  = false;
	setFormat(RGB32);

	flam3_init_frame(&flame);
	flame.progress = &_progress_callback;
	flame.bits = 64;
	flame.ngenomes = 1; // only render one genome
	flame.time = 0.0;
	flame.bytes_per_channel = 1;
	flame.pixel_aspect_ratio = 1.0;
	flame.sub_batch_size = 10000;
	flame.nthreads = QString(getenv("flam3_nthreads")).toInt();
	flame.verbose  = QString(getenv("flam3_verbose")).toInt();
	flame.earlyclip = 0;

	if (flame.nthreads < 1)
		flame.nthreads = flam3_count_nthreads();

	logInfo(QString("RenderThread::RenderThread : using %1 rendering thread(s)").arg(flame.nthreads));

	preview_request = 0;
	image_request = 0;

	so = new StatusObserver(this);
	so->start();
	connect(so, SIGNAL(statusUpdated(RenderStatus*)),
			this, SIGNAL(statusUpdated(RenderStatus*)),Qt::DirectConnection);
}

RenderThread* RenderThread::getInstance()
{
	if (singleInstance == 0)
		singleInstance = new RenderThread();
	return singleInstance;
}

RenderThread::~RenderThread()
{
	running = false;
	so->running = false;
	delete so;
}

void RenderThread::run()
{
	logInfo("RenderThread::run : starting thread");
	while (running)
	{
		running_mutex.lock();
		RenderRequest* job;
		if (preview_request != 0)
		{
			job = preview_request;
			preview_request = 0;
		}
		else if (image_request != 0)
		{
			job = image_request;
			image_request = 0;
		}
		else
		{
			rqueue_mutex.lock();
			if (request_queue.isEmpty())
			{
				// sleep only after checking for requests
				current_request = 0;
				rqueue_mutex.unlock();
				running_mutex.unlock();
				usleep(10000);
				continue;
			}
			else
			{
				job = request_queue.dequeue();
				logFine("RenderThread::run : dequeueing request %#x", (long)job);
				rqueue_mutex.unlock();
			}
		}
		render_loop_flag = true;
		current_request = job;

		// make sure there is something to calculate
		bool no_pos_xf = true;
		for (flam3_xform* xf = job->genome()->xform ;
			 xf < job->genome()->xform + job->genome()->num_xforms ; xf++)
			if (xf->density > 0.0)
			{
				no_pos_xf = false;
				break;
			}
		if (no_pos_xf)
		{
			logWarn(QString("RenderThread::run : no xform in request 0x%1").arg((long)job,0,16));
			running_mutex.unlock();
			continue;
		}

		logFiner(QString("RenderThread::run : rendering request 0x%1").arg((long)job,0,16));
		rtype = job->name();
		flame.time = job->time();
		flame.ngenomes = job->numGenomes();
		flam3_genome* genomes = new flam3_genome[flame.ngenomes]();
		flam3_genome* job_genome = job->genome();
		for (int n = 0 ; n < flame.ngenomes ; n++)
			flam3_copy(genomes + n, job_genome + n);
		flame.genomes = genomes;
		QSize imgSize(job->size());
		if (!imgSize.isEmpty())
		{
			for (int n = 0 ; n < flame.ngenomes ; n++)
			{
				flam3_genome* genome = genomes + n;
				// scale images, previews, etc. if necessary
				int width  = genome->width;
				genome->width  = imgSize.width();
				genome->height = imgSize.height();

				// "rescale" the image scale to maintain the camera
				// for smaller/larger image size
				genome->pixels_per_unit /= ((double)width) / genome->width;
			}
		}

		// Load image quality settings for Image, Preview, and File types
		switch (job->type())
		{
			case RenderRequest::File:
				rtype = QFileInfo(job->name()).fileName();

			case RenderRequest::Image:
			case RenderRequest::Preview:
			case RenderRequest::Queued:
			{
				const flam3_genome* g = job->imagePresets();
				if (g->nbatches > 0) // valid quality settings for nbatches > 0
					for (int n = 0 ; n < flame.ngenomes ; n++)
					{
						flam3_genome* genome = genomes + n;
						genome->sample_density =            g->sample_density;
						genome->spatial_filter_radius =     g->spatial_filter_radius;
						genome->spatial_oversample =        g->spatial_oversample;
						genome->nbatches =                  g->nbatches;
						genome->ntemporal_samples =         g->ntemporal_samples;
						genome->estimator =                 g->estimator;
						genome->estimator_curve =           g->estimator_curve;
						genome->estimator_minimum =         g->estimator_minimum;
					}
			}

			default:
				;
		}

		// add symmetry xforms before rendering
		for (int n = 0 ; n < flame.ngenomes ; n++)
		{
			flam3_genome* genome = genomes + n;
			if (genome->symmetry != 1)
				flam3_add_symmetry(genome, genome->symmetry);
		}

		int msize = channels * genomes->width * genomes->height;
		unsigned char* out = new unsigned char[msize];
		unsigned char* head = out;
		logFine("RenderThread::run : allocated %d bytes, rendering...", msize);
		init_status_cb();
		rendering = true;
		ptimer.start();
		int rv = flam3_render(&flame, out, 0, channels, alpha_trans, &_stats);
		millis = ptimer.elapsed();
		rendering = false;
		render_loop_flag = false;

		if (_stop_current_job) // if stopRendering() is called
		{
			logFine(QString("RenderThread::run : %1 rendering stopped").arg(rtype));
			delete[] head;
			for (int n = 0 ; n < flame.ngenomes ; n++)
				clear_cp(genomes + n, flam3_defaults_off);
			delete[] genomes;
			if (kill_all_jobs)
			{
				preview_request = 0;
				image_request = 0;
				rqueue_mutex.lock();
				request_queue.clear();
				rqueue_mutex.unlock();
				kill_all_jobs = false;
				emit flameRenderingKilled();
			}
			else
				if (job->type() == RenderRequest::Queued)
				{
					logFine("RenderThread::run : re-adding queued request");
					rqueue_mutex.lock();
					request_queue.prepend(job);
					rqueue_mutex.unlock();
				}

			_stop_current_job = false;
			running_mutex.unlock();
			continue;
		}

		QSize buf_size(genomes->width, genomes->height);
		if (img_format == RGB32)
		{
			if (buf_size != img_buf.size())
				img_buf = QImage(buf_size, QImage::Format_RGB32);
			if (rv == 0)
			{
				for (int h = 0 ; h < genomes->height ; h++)
					for (int w = 0 ; w < genomes->width ; w++, out += channels)
						img_buf.setPixel(QPoint(w, h), qRgb(out[0], out[1], out[2]));
			}
			else
				img_buf.fill(0);
		}
		else
		{
			if (buf_size != img_buf.size())
				img_buf = QImage(buf_size, QImage::Format_ARGB32);
			if (rv == 0)
			{
				for (int h = 0 ; h < genomes->height ; h++)
					for (int w = 0 ; w < genomes->width ; w++, out += channels)
						img_buf.setPixel(QPoint(w, h), qRgba(out[0], out[1], out[2], out[3]));
			}
			else
				img_buf.fill(0);
		}
		delete[] head;
		for (int n = 0 ; n < flame.ngenomes ; n++)
			clear_cp(genomes + n, flam3_defaults_off);
		delete[] genomes;

		if (job->type() == RenderRequest::File)
			img_buf.save(job->name(), "png", 100);

		job->setImage(img_buf);
		job->setFinished(true);

		// look for a free event
		RenderEvent* event = 0;
		foreach (RenderEvent* e, event_list)
			if (e->accepted())
			{
				e->accept(false);
				event = e;
				break;
			}

		if (!event)
		{
			logFinest(QString("RenderThread::run : adding event"));
			event = new RenderEvent();
			event->accept(false);
			event_list.append(event);
		}
		logFiner(QString("RenderThread::run : event list size %1")
				.arg(event_list.size()));

		event->setRequest(job);
		emit flameRendered(event);
		logFiner(QString("RenderThread::run : finished"));
		running_mutex.unlock();
	}

	logInfo("RenderThread::run : thread exiting");
}


RenderStatus& RenderThread::getStatus()
{
	status.Name = rtype;

	if (rendering)
	{
		status.State = RenderStatus::Busy;
		status.Type = current_request->type();
		status.EstRemain = QTime::fromMSecsSinceStartOfDay((int)_est_remain);
		status.Percent = _percent_finished;
	}
	else if (kill_all_jobs || _stop_current_job)
	{
		status.State = RenderStatus::Killed;
	}
	else
	{
		status.State = RenderStatus::Idle;
		status.Runtime = QTime::fromMSecsSinceStartOfDay(millis);
	}
	return status;
}

bool RenderThread::isRendering()
{
	return render_loop_flag && current_request;
}

/**
 * stops the current job and clears all remaining requests.
 */
void RenderThread::killAll()
{
	if (render_loop_flag)
	{
		kill_all_jobs = true;
		_stop_current_job = true;
	}
}

/**
 * stop the current job and select the next request.
 */
void RenderThread::stopRendering()
{
	if (render_loop_flag)
		_stop_current_job = true;
}

void RenderThread::start()
{
	running = true;
	QThread::start();
}

void RenderThread::stop()
{
	running = false;
	preview_request = 0;
	image_request = 0;
	rqueue_mutex.lock();
	request_queue.clear();
	rqueue_mutex.unlock();
	stopRendering();
}

double RenderThread::finished()
{
	return _percent_finished;
}

void RenderThread::render(RenderRequest* req)
{
	logFiner(QString("RenderThread::render : req 0x%1").arg((long)req,0,16));
	if (req->type() == RenderRequest::Preview)
	{
		preview_request = req;
		// rendering a preview preempts everything except files and previews
		if (isRendering())
			switch (current_request->type())
			{
				case RenderRequest::Image:
				case RenderRequest::Queued:
					stopRendering();
				default:
					;
			}
	}
	else if (req->type() == RenderRequest::Image)
		image_request = req;

	else if (request_queue.contains(req))
		logWarn(QString("RenderThread::render : req 0x%1 already queued")
				.arg((long)req,0,16));
	else
	{
		logFine("RenderThread::render : queueing req %#x", (long)req);
		req->setFinished(false);
		rqueue_mutex.lock();
		request_queue.enqueue(req);
		rqueue_mutex.unlock();
	}
}

void RenderThread::cancel(RenderRequest* req)
{
	if (req->type() == RenderRequest::Queued)
	{
		rqueue_mutex.lock();
		int count = request_queue.removeAll(req);
		logFine("RenderThread::cancel : removing %d queued requests", count);
		rqueue_mutex.unlock();
	}
	else if (req->type() == RenderRequest::Preview)
		preview_request = 0;
	else if (req->type() == RenderRequest::Image)
		image_request = 0;
	else
		logWarn("RenderThread::cancel : unknown request type %d", (int)req->type());
}


RenderThread::ImageFormat RenderThread::format() const
{
	return img_format;
}

void RenderThread::setFormat(ImageFormat f)
{
	img_format = f;
	if (f == RGB32)
		channels = 3;
	else
	{
		channels = 4;
		alpha_trans = ( f == ARGB32_OPAQUE ? 0 : 1 );
	}
}

bool RenderThread::earlyClip() const
{
	return flame.earlyclip == 1;
}

void RenderThread::setEarlyClip(bool t)
{
	flame.earlyclip = ( t ? 1 : 0 );
}

// Observer used by renderthread which notifies StatusWatchers
StatusObserver::StatusObserver(StatusProvider* p)
	: running(true), provider(p)
{
}

void StatusObserver::start()
{
	running = true;
	QThread::start();
}

void StatusObserver::run()
{
	logInfo("StatusObserver::run : running");
	while (running)
	{
		status = provider->getStatus() ;
		status.createMessage(); // only do this once per update
		emit statusUpdated(&status);
		usleep(250000);
	}
}

StatusProvider::~StatusProvider()
{
}

QString RenderStatus::getMessage()
{
	return message;
}


void RenderStatus::createMessage()
{
	if (State == Busy)
	{
		QString t_format;
		if (EstRemain.hour() > 1)
			t_format = tr("hh:mm:ss");
		else if (EstRemain.minute() > 1)
			t_format = tr("mm:ss");
		else
			t_format = tr("s.z");
		estRemainString = EstRemain.toString(t_format);
		message = tr("rendering... %L1% ( %2 remaining )")
				.arg(Percent, 0, 'f', 1, '0')
				.arg(estRemainString);
	}
	else if (State == Killed)
		message = tr("%1 rendering stopped").arg(Name);

	else
	{
		QString t_format;
		if (Runtime.hour() > 1)
			t_format = tr("hh:mm:ss.z");
		else if (Runtime.minute() > 1)
			t_format = tr("mm:ss");
		else
		{
			t_format = tr("s.z");
			runtimeString = Runtime.toString(t_format);
			message = tr("%1 rendered in %2 seconds").arg(Name).arg(runtimeString);
			return;
		}

		runtimeString = Runtime.toString(t_format);
		message = tr("%1 rendered in %2").arg(Name).arg(runtimeString);
	}
}

QString RenderStatus::getEstRemain()
{
	return estRemainString;
}

QString RenderStatus::getRuntime()
{
	return runtimeString;
}

RenderStatus::RenderStatus() : QObject()
{
}

RenderStatus::RenderStatus(const RenderStatus& c) : QObject()
{
	*this = c;
}

RenderStatus& RenderStatus::operator=(const RenderStatus& c)
{
	Name = c.Name;
	Type = c.Type;
	Runtime = c.Runtime;
	EstRemain = c.EstRemain;
	Percent = c.Percent;
	State = c.State;
	return *this;
}

// rendering events
RenderEvent::RenderEvent()
{
	m_request = 0;
}


RenderRequest* RenderEvent::request() const
{
	return m_request;
}


void RenderEvent::setRequest(RenderRequest* r)
{
	m_request = r;
}

void RenderEvent::accept(bool t)
{
	m_accepted = t;
}

bool RenderEvent::accepted() const
{
	return m_accepted;
}



// rendering requests
RenderRequest::RenderRequest(flam3_genome* g, QSize s, QString n, Type t)
: m_genome(g), m_genome_template(), m_time(0), m_ngenomes(1), m_type(t),
	m_size(s), m_name(n), m_finished(true)
{
}



flam3_genome* RenderRequest::genome() const
{
	return m_genome;
}


void RenderRequest::setGenome(flam3_genome* value)
{
	m_genome = value;
}

flam3_genome* RenderRequest::imagePresets()
{
	return &m_genome_template;
}

void RenderRequest::setImagePresets(flam3_genome value)
{
	m_genome_template = value;
}

QSize RenderRequest::size() const
{
	return m_size;
}


void RenderRequest::setSize(const QSize& value)
{
	m_size = value;
}


QImage& RenderRequest::image()
{
	QMutexLocker locker(&m_img_mutex);
	return m_image;
}

void RenderRequest::setImage(QImage& p)
{
	QMutexLocker locker(&m_img_mutex);
	m_image = p;
}

QString RenderRequest::name() const
{
	return m_name;
}


void RenderRequest::setName(const QString& value)
{
	m_name = value;
}


RenderRequest::Type RenderRequest::type() const
{
	return m_type;
}


void RenderRequest::setType(const Type& value)
{
	m_type = value;
}


bool RenderRequest::finished() const
{
	return m_finished;
}


void RenderRequest::setFinished(bool value)
{
	m_finished = value;
}

double RenderRequest::time() const
{
	return m_time;
}

void RenderRequest::setTime(double time)
{
	m_time = time;
}

int RenderRequest::numGenomes() const
{
	return m_ngenomes;
}

void RenderRequest::setNumGenomes(int n)
{
	m_ngenomes = n;
}

RenderRequest* RenderThread::current() const
{
	return current_request;
}


