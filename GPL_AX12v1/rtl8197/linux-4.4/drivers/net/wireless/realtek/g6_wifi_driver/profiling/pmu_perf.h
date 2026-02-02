#ifndef PMU_PERF_H_
#define PMU_PERF_H_

#include "./cpu_perf.h"

#define FUNC_LEN	64

int profile_config_event(u32 group, u32 *events, u8 size);
int profile_start(int event_start);
int profile_stop(void);
int profile_dump(void);
void profile(const char *func, u64 idx, const char *idx_name);
void profile_end(int idx);
void dump_cpu_event_name(void);

struct cnt_event
{
	u32 event;
	u8  name[32];
};

#define hwperf_start(index) \
	do { \
		/* mib check here */ \
		asm __volatile__("	MSR pmcr_el0,xzr \n":::); \
		{ \
			profile(__FUNCTION__, index, #index); \
			asm __volatile__(	"	MOV w0,#0x1 \n" \
			"	MSR pmcr_el0,x0 \n":::"x0"); \
		} \
	} while(0);

#define hwperf_stop(index) \
	do { \
		asm __volatile__("	MSR pmcr_el0,xzr \n":::); \
		profile_end(index); \
		asm __volatile__(	"	MOV w0,#0x1 \n" \
		"	MSR pmcr_el0,x0 \n":::"x0"); \
	} while(0);

#endif
