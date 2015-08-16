
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


		inline TimerWorker::task_id_t TimerWorker::schedule_task(const Task&task)
		{
			pthread_mutex_lock(&mutex);

			// upper_bound return the position where the task's next_run_time
			// is little more than the new task.
			std::list<Task>::iterator iter = upper_bound(task_list.begin(),
					task_list.end(), task, task_less);

			// if task would be put to the begin position,
			// then should wake thread to process this task
			// if task is not put to the begin position
			// then insert this task to its position 
			bool should_wake_up = (it == task_list.begin());

			task_id_t task_id = ++next_id;
			task_list.insert(it, task)->task_id = task_id;

			if (should_wake_up) {
				pthread_cond_signal(&cond);
			}

			pthread_mutex_unlock(&mutex);

			return task_id;
		}


		TimerWorker::task_id_t TimerWorker::schedule(timespec run_time, task_func_t task_func, void* arg)
		{
			Task task;
			task.next_run_time = run_time;
			task.interval = 0;
			task.task_func_routine = task_func;
			task.arg = arg;
			return schedule_task(task);
		}

		TimerWorker::schedule_repeated(timespec run_time, long interval, task_func_t task_func, void* task_arg)
		{
			Task task;
			task.next_run_time = run_time;
			task.interval = interval;
			task.task_func_routine = task_func;
			task.arg = arg;
			return schedule_task(task);
		}


		void TimerWorker::unschedule(task_id_t task_id)
		{
			pthread_mutex_lock(&mutex);
			if (running_task_id == task_id) {
				running_task_id = 0;
			} else {
				for (std::list<Task>::iterator it = task_list.begin; it != task_list.end(); ++it) {
					if (it->task_id == task_id) {
						task_list.erase(it);
						break;
					}
				}
			}
			pthread_mutex_unlock(&mutex);
		}

		void TimerWorker::run()
		{
			log_info("thread run");

			bool lock_hold = false;

			while(!stop) {
				if (!lock_hold) {
					pthread_mutex_lock(&mutex);
					lock_hold = true;
				} // end of if(!lock_hold)

				if (task_list.empty()) {
					pthread_cond_wait(&cond, &mutex);
					continue;
				}

				timespec current_time;
				clock_gettime(CLOCK_REALTIME, &current_time);
				
				//get the first task
				const Task& task = task_list.front();
				
				// current time > task's next_run_time
				// need to run
				if (!timespec_less(current_time, task.next_run_time)) {
					Task to_run = task_list.front();
					task_list.pop_front();
					running_task_id = to_run.task_id;
					//release the lock asap
					pthread_mutex_unlock(&mutex);
					lock_hold = false;
					
					// run user-define processing callback funtion
					// with user-define args
					to_run.task_func_routine(to_run.arg);

					// check whether it is a loop task
					if (to_run.interval > 0) {
						to_run.next_run_time = microsecond_from_now(to_run.interval);
						pthread_mutex_lock(&mutex);
						lock_hold = true;
						
						// if unscheduled task
						if ( running_task_id != 0) {
							running_task_id = 0;
							std::list<Task>::iterator it = upper_bound(task_list.begin,
									task_list.end(), to_run, task_less);	
							task_list.insert(it, to_run);
						} else {
							// if task has been unscheduled
							// do not need to insert it back
						}
					}

					continue;
				} else {
					// wait its task next run time
					// sleep util next run time
					pthread_cond_timewait(&cond, &mutex, task.next_run_time);
					continue;
				}
			}// end of while

			// exit from while
			if (lock_hold) {
				pthread_mutex_unlock(&mutex);
			}
		}


		void TimerWorker::stop_and_join()
		{
			if (pthread_self() == thread) {
				stop = true;
			} else {
				
				pthread_mutex_lock(&mutex);
				stop = true;

				// wake up the timer thread in case it is sleeping
				// while wake up, the timer thread will step into run()
				// and then check stop = true
				pthread_cond_signal(&cond);
				pthread_mutex_unlock(&mutex);

				if (started) {
					if (pthread_join(thread, NULL) != 0) {
					   log_error("failed to join the timer thread");
						
					}
					}	
				}
			}
		}
	} // end of namespace timer_worker
} // end of namespace crafet
