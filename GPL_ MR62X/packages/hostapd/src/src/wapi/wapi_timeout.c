#include "wapi_common.h"


struct timeout_entry *entry_list = NULL;

void rtl_run_timeout_entry(void)
{
	struct timeout_entry *p = NULL;
	struct timespec timenow;

	while (entry_list != NULL) {
		p = entry_list;

		memset(&timenow, 0, sizeof(struct timespec));   
		clock_gettime(CLOCK_MONOTONIC, &timenow);

		if (!(p->time.tv_sec < timenow.tv_sec
			|| (p->time.tv_sec == timenow.tv_sec
			&& p->time.tv_nsec <= timenow.tv_nsec)))
			break;	   /* no, it's not time yet */

		entry_list = p->next;
		(*p->func)(p->arg);
	}
}













