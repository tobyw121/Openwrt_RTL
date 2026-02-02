#ifndef M1004KPERF_H_
#define M1004KPERF_H_

#include "./cpu_perf.h"

void hwperf_init(void);
void hwperf_reinit(u32 event, u32 group, u8 _vpeid, u8 _tcid);
void hwperf_start(int index);
void hwperf_stop(int index);
void hwperf_dump(void);
void hwperf_clear_all(void);	
void display_cpu_event_name(void);
void dump_cpu_event_name(void);

enum CNT_0_EVENT {
	CNT0_CYCLE = 0, 
	CNT0_INS_COMPLETE = 1,
	CNT0_INS_CACHE_ACC = 9,
	CNT0_DATA_CACHE_ACC = 10,
	CNT0_DATA_CACHE_MISS = 11,
	CNT0_STALL = 18,
	CNT0_EXCEPTION = 23, 
	CNT0_IFU_STALL = 25,
	CNT0_I_MISS_STALL = 37,
};

enum CNT_1_EVENT {
	CNT1_CYCLE = 0, 
	CNT1_INS_COMPLETE = 1,
	CNT1_INS_CACHE_MISS = 9,
	CNT1_DATA_CACHE_MISS = 11,
	CNT1_REPLAY = 18, 
	CNT1_S_THREAD = 23, 
	CNT1_PREFETCH = 24, 
	CNT1_ALU_STALL = 25,
	CNT1_D_MISS_STALL = 37,
	CNT1_D_MISS_STALL2 = 39,
};

enum MT_EN_TYPE {
	CNT_ALL_TC_ALL_VPE = 0,
	CNT_ALL_TC_BY_VPEID = 1,		
	CNT_ALL_TC_BY_TCID = 2,
	CNT_RSVD = 3,
};

#define VPEID_INVALID 0xFF
#define TCID_INVALID 0xFF

struct cnt_event_display 
{
	u32 event;
	u8  name[32];
};

#endif
