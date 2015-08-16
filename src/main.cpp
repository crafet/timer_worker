
// Author crafet
// Date 2015-08-15 09:37

#include "timer_worker.h"

#define MAX_RELOADER_NAME 100
typedef struct reloadContext {
	char reloader_name[MAX_RELOADER_NAME];

	timespec reload_timestamp;
	//other args
	
}reload_context_t;

void reload_process(void* arg)
{
	log_info("start reload");
	reload_context_t* c = (reload_context_t*)arg;
	// process c
	log_info("end reload");
}

int main()
{
	log_info("%s %d \n", "test", 3);

	reload_context_t t;
	memset(t.reloader_name, 0, MAX_RELOADER_NAME_SIZE);
	snprintf(t.reloader_name, MAX_RELOADER_NAME_SIZE, "%s", "myreloader");
	t.reload_timestamp = seconds_from_now(60);

	namespace TW = crafet::timer_worker;
	//crafet::timer_worker::TimerWorker* tw = new crafet::timer_worker::TimerWorker();
	TW::TimerWorker* tw = new TW::TimerWorker();


	// define my process as a loop task
	tw->schedule_repeated(microsecond_from_now(), 60, reload, &t);
	if (!tw->start()) {
		log_error("failed to start reload task");		
	}


	return 0;
}

