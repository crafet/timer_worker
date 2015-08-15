
// Author crafet@gmail.com
// Date 2015-08-15 18:33

#include <algorithm>

#include "log.h"
#include "timer_worker.h"


namespace crafet {
	namespace timer_worker {

		bool timespace_less(const timespec& a, const timespec& b)
		{
			if (a.tv_sec != b.tv_sec) {
				return a.tv_sec < b.tv_sec;
			} else {
				return a.tv_nsec < b.tv_nsec;
			}
		}

		// compare task next run time, since we need insert the 
		// task into task list by the run time
		// we keep the task list sorted by task next time
		// next time is their priority
		bool TimerWorker::task_less(const Task& a, const Task& b)
		{
			return timespec_less(a.next_run_time, b.next_run_time);

		}

		// static function for class TimerWorker
		void* TimerWorker::timer_worker_routine(void* arg)
		{
			if (NULL == arg) {
				return NULL;
			}

			TimerWorker* worker = (TimerWorker*) arg;
			
			// thread processing function
			worker->run();
			return NULL;
		}

		TimerWorker::TimerWorker()
			: started(false), stop(false), next_id(0), running_task_id(0)
		{
			pthread_mutex_init(&mutex, NULL);
			pthread_cond_init(&cond, NULL);
		}

		TimerWorker::~TimerWorker()
		{

			// stop the worker
			if (started && !stop) {
				stop_and_join();
			}
			pthread_mutex_destroy(&mutex);
			pthread_cond_destroy(&cond);

		}

		bool Timerworker::start()
		{
			if (started) {
				return false;
			}

			int ret = pthread_create(&thread, NULL, &TimerWorker::timer_worker_routine, this);
			// succ
			if ( ret == 0) {
		
				started = true;
				return true;
			} else {

				// create thread failed
				return false;
			} // end of else
		}


	} // end of namespace timer_worker
} // end of namespace crafet
