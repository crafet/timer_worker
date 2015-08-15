#include <list>
#include <pthread.h>
#include <time.h>

static inline timespec microsecond_from(timespec start_time, long microseconds)
{
	start_time.tv_nsec += microseconds*1000L;
	if (start_time.tv_nsec >= 1000000000L) {
	
		start_time.tv_sec += start_time.tv_nsec / 1000000000L;
		start_time.tv_nsec = start_time.tv_nsec % 1000000000L;
	}

	return start_time;
}


static inline timespec microsecond_from_now(long microsecond)
{
	timespec time;
	// get current time
	clock_gettime(CLOCK_REALTIME, &time);
	return time_from(time, microsecond); 
}

static inline timespec seconds_from_now(long second)
{
	return microsecond_from_now(second*1000000L);
}


namespace crafet {
	namespace timer_worker {

		class TimerWorker {
			public:

			// user define task processing function
			typedef void* (*task_func_t) (void*);
			typedef uint64_t task_id_t;

			TimerWorker();
			~TimerWorker();


			// start the worker to work
			bool start();

			// stop the worker and wait its finish
			// by pthread_join
			void stop_and_join();


			task_id_t schedule(timespec run_time, task_func_t task_func_routine, void* task_arg);


			// schedule a task which would be run in a loop
			task_id_t schedule_repeated(timespec run_time, long interval, task_func_t task_func_routine, void* task_arg);

			void unschedule(task_id_t task_id);

			private:

			struct Task 
			{
				task_id_t task_id;
				timespec next_run_time;
				long interval;
				task_func_t task_func_routine;
				void *arg;
			};

			// thread entry routine
			static void* timer_worker_routine(void* arg);

			// task compare function
			static bool task_less(const Task& a, const Task& b);

			task_id_t schedule_task(const Task& task);
			
			// will be called at timer_worker_routine;
			void run();

			bool started;
			bool stop;

			std::list<Task> task_list;
			pthread_t thread;

			// used to wake up the core thread
			pthread_cond_t cond;
			
			// race condition for task
			pthread_mutex_t mutex;
			task_id_t next_id;
			task_id_t running_task_id;
		};

	} // end of namespace timer_worker
} // end of namespace crafet
