#include <linux/module.h> 
#include <linux/kernel.h> 
#include <linux/init.h> 
#include <linux/proc_fs.h> 
#include <linux/seq_file.h>
#include <linux/cpu.h>
#include <linux/cpuset.h>
#include <linux/percpu.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <bspchip.h>

#include "./m1004kperf.h" //eric-cpu

#define HWPERF_INDEX_MAX 9

struct hwperf_stat_s {	
	u64 accCycle[2];
	u32 prevCount[2];
	u32 executedNum;
	int started;

#if 1 //eric-cpu
	u32 max_cycle[2];
	u32 min_cycle[2];
	u32 min_cycle_P[2];
	u8	is_min_zero[2];
#endif
};
typedef struct hwperf_stat_s hwperf_stat_t;

struct cnt_group cnt_groups[] = 
{
	CPU_GROUP_SPEC(0, CPU_TX_ALL, 	CPU_TEST8, 	"CPU_TX_ALL-CPU_TEST8"),
	CPU_GROUP_SPEC(1, CPU_RX_ALL, 	CPU_TEST18, "CPU_RX_ALL-CPU_TEST18"),
	CPU_GROUP_SPEC(2, CPU_TX1, 		CPU_TX9,	"CPU_TX1-CPU_TX9 (Core, test2)"),
	CPU_GROUP_SPEC(3, CPU_TX11, 	CPU_TX18, 	"CPU_TX11-CPU_TX18 (PHL)"),
	CPU_GROUP_SPEC(4, CPU_RX1, 		CPU_RX8,	"CPU_RX1-CPU_RX8"),
	CPU_GROUP_SPEC(5, CPU_RX11, 	CPU_RX18, 	"CPU_RX11-CPU_RX18"),
	CPU_GROUP_SPEC(6, CPU_LOCK1, 	CPU_LOCK8, 	"CPU_LOCK1-CPU_LOCK8"),
	CPU_GROUP_SPEC(7, CPU_CACHE1, 	CPU_CACHE8, "CPU_CACHE1-CPU_CACHE8"),
	CPU_GROUP_SPEC(8, CPU_CACHE11,	CPU_CACHE18,"CPU_CACHE11-CPU_CACHE18"),
	CPU_GROUP_SPEC(9, CPU_PHY1, 	CPU_PHY8,   "CPU_PHY1-CPU_PHY8"),
	CPU_GROUP_SPEC(10,CPU_PHY11, 	CPU_PHY18,  "CPU_PHY11-CPU_PHY18"),
	CPU_GROUP_SPEC(11,CPU_PHY21, 	CPU_PHY28,  "CPU_PHY21-CPU_PHY28"),
	CPU_GROUP_SPEC(12,CPU_TX21, 	CPU_TX28,	"CPU_TX21-CPU_TX28"),
	CPU_GROUP_SPEC(13,CPU_PHLRX01,	CPU_PHLRX08,	"CPU_PHLRX01"),
	CPU_GROUP_SPEC(14,CPU_PHLRX11,	CPU_PHLRX18,	"CPU_PHLRX11"),

};

u32 group_idx 	= 0;
u32 cnt_start 	= CPU_TX_ALL;
u32 cnt_end 	= CPU_TEST8;
u8  vpeid 		= VPEID_INVALID;
u8  tcid 		= TCID_INVALID;

DEFINE_PER_CPU(hwperf_stat_t[HWPERF_INDEX_MAX], hwperf_stats);
static u32 perfctrl0, perfctrl1;
static u32 perfevent, perfmask;

void hwperf_clear_all(void) {
	unsigned int cpu;
	hwperf_stat_t *ptr;
	for_each_possible_cpu(cpu) {
		ptr = per_cpu_ptr(hwperf_stats, cpu);
		memset(ptr, 0, sizeof(hwperf_stat_t[HWPERF_INDEX_MAX]));
		ptr->started = 0;
	}
}

static void inline hwperf_ctrl_update(u32 event, u32 mask) {
	unsigned int cpu;
	hwperf_stat_t *ptr;
	perfctrl0 = ((mask)&0x1F) | ((event&0x00ff) << 5);
	perfctrl1 = ((mask)&0x1F) | ((event&0xff00) >> 3);

#if 1 //eric-vpeid
	if(vpeid != VPEID_INVALID){
		perfctrl0 |= ((CNT_ALL_TC_BY_VPEID&0x3) << 20)|((vpeid&0xf)<<16);
		perfctrl1 |= ((CNT_ALL_TC_BY_VPEID&0x3) << 20)|((vpeid&0xf)<<16);
	}
	else if(tcid != TCID_INVALID){
		perfctrl0 |= ((CNT_ALL_TC_BY_TCID&0x3) << 20)|((tcid&0x1f)<<22);
		perfctrl1 |= ((CNT_ALL_TC_BY_TCID&0x3) << 20)|((tcid&0x1f)<<22);
	}

	printk("[%s] 0x%x 0x%x \n", __FUNCTION__, perfctrl1, perfctrl0);
#endif
	
	for_each_possible_cpu(cpu) {
		ptr = per_cpu_ptr(hwperf_stats, cpu);
		ptr->started = 0;
	}
}

/*
static inline void ___hwperf_enable(void) {
	asm (
		"	.set push			\n"
		"	.set noreorder		\n"
		"	mtc0	$0, $25, 0	\n"
		"	mtc0	$0, $25, 2	\n"
		"	mtc0	$0, $25, 1	\n"
		"	mtc0	$0, $25, 3	\n"
		"	mtc0	%0, $25, 0	\n"		
		"	mtc0	%1, $25, 2	\n"		
		"	.set pop			\n"
		:
		: "r" (perfctrl0), 
		  "r" (perfctrl1)
	);
}
*/

static inline void ___hwperf_disable(void) {
	asm (
		"	.set push			\n"
		"	.set noreorder		\n"
		"	mtc0	$0, $25, 0	\n"
		"	mtc0	$0, $25, 2	\n"
		"	mtc0	$0, $25, 1	\n"
		"	mtc0	$0, $25, 3	\n"		
		"	.set pop			\n"		
	);
}



void hwperf_start(int index) {
	unsigned long flags;
	hwperf_stat_t *ptr;

	if(index>=cnt_start && index <= cnt_end)
		index = index - cnt_start;
	else 
		return;
	
	local_irq_save(flags);
	ptr = this_cpu_ptr(hwperf_stats);
	if (unlikely(ptr->started==0)) {
		ptr->started = 1;
		//write_c0_perfctrl0(0);
		//write_c0_perfctrl1(0);
		write_c0_perfctrl0(perfctrl0);
		write_c0_perfctrl1(perfctrl1);
	}
	//wmb();
	ptr[index].prevCount[0] = read_c0_perfcntr0();
	ptr[index].prevCount[1] = read_c0_perfcntr1();
	//wmb();
	
	local_irq_restore(flags);
}

void hwperf_stop(int index) 
{
	u32 tmp0, tmp1;
	unsigned long flags;
	hwperf_stat_t *ptr;

	if(index>=cnt_start && index <= cnt_end)
		index = index - cnt_start;
	else 
		return;
	
	local_irq_save(flags);
	asm (
		"	.set push			\n"
		"	.set noreorder		\n"
		//"	mtc0	$0, $25, 0	\n"
		//"	mtc0	$0, $25, 2	\n"
		"	mfc0	%0, $25, 1	\n"
		"	mfc0	%1, $25, 3	\n"			
		"	.set pop			\n"	
		: "=r" (tmp0), "=r" (tmp1)
	);
		
	ptr = this_cpu_ptr(hwperf_stats);
	
	tmp0 = tmp0 - ptr[index].prevCount[0];
	tmp1 = tmp1 - ptr[index].prevCount[1];
	
	ptr[index].accCycle[0] += tmp0;
	ptr[index].accCycle[1] += tmp1;
	ptr[index].executedNum++;	

#if 1 //eric-ctxt
	if(tmp0 > ptr[index].max_cycle[0])
		ptr[index].max_cycle[0] = tmp0;
	if(tmp0==0)
		ptr[index].is_min_zero[0] = 1;
	if(tmp0 < ptr[index].min_cycle[0] || (ptr[index].min_cycle[0]==0)){
		ptr[index].min_cycle[0] = tmp0;
		ptr[index].min_cycle_P[0] = tmp1;
	}

	if(tmp1 > ptr[index].max_cycle[1])
		ptr[index].max_cycle[1] = tmp1;
	if(tmp1==0)
		ptr[index].is_min_zero[1] = 1;
	if(tmp1 < ptr[index].min_cycle[1] || (ptr[index].min_cycle[1]==0)){
		ptr[index].min_cycle[1] = tmp1;
		ptr[index].min_cycle_P[1] = tmp0;
	}
#endif

	local_irq_restore(flags);
}

#if 1 //eric-cpu

// see Table 6.46 and 6.47 of MD00343-2B-24K-SUM-03.11.pdf for all of the event and counter0/1 meaning
#define PERF_EVENT_CYCLE				0x0100	// Counter 0: Cycles; Counter 1: Instructions completed
#define PERF_EVENT_ICACHE_MISS			0x0909	// Counter 0: Instruction Cache accesses; Counter 1: Instruction cache misses
#define PERF_EVENT_ICACHE_MISS_CYCLE	0x2525	// Counter 0: I$ Miss stall cycles; Counter 1: D$ miss stall cycles
#define PERF_EVENT_DCACHE_MISS			0x010B	// Counter 0: Data cache misses; Counter 1: Instructions completed

#define CPU_EVENT_SPEC(_event, _name) {              \
	.event        	= (_event),                   \
	.name         	= (_name),                     \
}

struct cnt_event_display cnt0_events[] = 
{
	CPU_EVENT_SPEC(CNT0_CYCLE, 				"CNT0_CYCLE"),
	CPU_EVENT_SPEC(CNT0_INS_COMPLETE, 		"CNT0_INS_COMPLETE"),
	CPU_EVENT_SPEC(CNT0_INS_CACHE_ACC, 		"CNT0_INS_CACHE_ACC"),
	CPU_EVENT_SPEC(CNT0_DATA_CACHE_ACC, 	"CNT0_DATA_CACHE_ACC"),
	CPU_EVENT_SPEC(CNT0_DATA_CACHE_MISS, 	"CNT0_DATA_CACHE_MISS"),
	CPU_EVENT_SPEC(CNT0_STALL, 				"CNT0_STALL"),
	CPU_EVENT_SPEC(CNT0_EXCEPTION, 			"CNT0_EXCEPTION"),
	CPU_EVENT_SPEC(CNT0_IFU_STALL,			"CNT0_IFU_STALL"),
	CPU_EVENT_SPEC(CNT0_I_MISS_STALL,		"CNT0_I_MISS_STALL"),
};

struct cnt_event_display cnt1_events[] = 
{
	CPU_EVENT_SPEC(CNT1_CYCLE, 				"CNT1_CYCLE"),
	CPU_EVENT_SPEC(CNT1_INS_COMPLETE, 		"CNT1_INS_COMPLETE"),
	CPU_EVENT_SPEC(CNT1_INS_CACHE_MISS, 	"CNT1_INS_CACHE_MISS"),
	CPU_EVENT_SPEC(CNT1_DATA_CACHE_MISS, 	"CNT1_DATA_CACHE_MISS"),
	CPU_EVENT_SPEC(CNT1_REPLAY,				"CNT1_REPLAY"),
	CPU_EVENT_SPEC(CNT1_S_THREAD,			"CNT1_S_THREAD"),
	CPU_EVENT_SPEC(CNT1_PREFETCH,			"CNT1_PREFETCH"),
	CPU_EVENT_SPEC(CNT1_ALU_STALL,			"CNT1_ALU_STALL"),
	CPU_EVENT_SPEC(CNT1_D_MISS_STALL,		"CNT1_D_MISS_STALL"),	
	CPU_EVENT_SPEC(CNT1_D_MISS_STALL2,		"CNT1_D_MISS_STALL2"),
};

#define CPU_CNT_EVENT0 CNT0_DATA_CACHE_ACC
#define CPU_CNT_EVENT1 CNT1_DATA_CACHE_MISS

u32 perf_event = (CPU_CNT_EVENT1<<8)|CPU_CNT_EVENT0;


void hwperf_init(void)
{
	perfevent = perf_event; //0x100;
	perfmask  = 0x6;
	//perfmask  = 0xf;

	printk("[%s] perfevent=0x%x perfmask=0x%x \n", __FUNCTION__, perfevent, perfmask);
	
	hwperf_clear_all();
	hwperf_ctrl_update(perfevent, perfmask);
}

void hwperf_reinit(u32 event, u32 group, u8 _vpeid, u8 _tcid)
{
	perf_event = event;
	perfevent = perf_event; 
	perfmask  = 0x6;
	//perfmask  = 0xf;

	if(group >= ARRAY_SIZE(cnt_groups))
		group = 0;

	vpeid = _vpeid;
	tcid = _tcid;

	printk("[%s] perfevent=0x%x perfmask=0x%x vpeid=0x%x txid=0x%x\n", 
		__FUNCTION__, perfevent, perfmask, vpeid, tcid);
	printk("[%s] group=%d (%s)idx=%d~%d \n", __FUNCTION__, group, 
		cnt_groups[group].name, cnt_groups[group].start, cnt_groups[group].end);

	group_idx = group;
	cnt_start = cnt_groups[group].start;
	cnt_end = cnt_groups[group].end;
	
	hwperf_clear_all();
	hwperf_ctrl_update(perfevent, perfmask);
}

void dump_cpu_event_name(void)
{
	int idx = 0;
	
	printk("===\n");	
	
	for(idx=0; idx<ARRAY_SIZE(cnt_groups); idx++)
		printk("group=%02d (%s)idx=%d~%d \n", idx, cnt_groups[idx].name, cnt_groups[idx].start, cnt_groups[idx].end);

	printk("===\n");

	for(idx=0; idx<ARRAY_SIZE(cnt1_events); idx++)
		printk("(%02d)=%s \n", cnt1_events[idx].event, cnt1_events[idx].name);

	printk("===\n");
	
	for(idx=0; idx<ARRAY_SIZE(cnt0_events); idx++)
		printk("(%02d)=%s \n", cnt0_events[idx].event, cnt0_events[idx].name);

	printk("===\n");

	display_cpu_event_name();
}

void display_cpu_event_name(void)
{
	u32 event0 = perf_event&0xFF;
	u32 event1 = (perf_event&0xFF00)>>8;

	int idx, tmp =0;

	printk("current group=%d (%s)idx=%d~%d \n", group_idx, cnt_groups[group_idx].name, cnt_start, cnt_end);

	idx = -1;
	for(tmp = 0; tmp<ARRAY_SIZE(cnt1_events); tmp++){
		if(event1 == cnt1_events[tmp].event){
			idx = tmp;
			break;
		}
	}
	if(idx >=0)
		printk("[1]=%s ,", cnt1_events[idx].name);
	else
		printk("[1]=%d UNKNOWN ,", cnt1_events[idx].event);


	idx = -1;
	for(tmp = 0; tmp<ARRAY_SIZE(cnt0_events); tmp++){
		if(event0 == cnt0_events[tmp].event){
			idx = tmp;
			break;
		}
	}
	if(idx >=0)
		printk("[0]=%s ", cnt0_events[idx].name);
	else
		printk("[0]=%d UNKNOWN ,", cnt0_events[idx].event);

	printk("\n");

}

void hwperf_dump(void)
{
	int i = 0;

	display_cpu_event_name();

	for (i=0; i<HWPERF_INDEX_MAX;i++) {
		u64 avrgCycle[2] = { 0, 0 };
		hwperf_stat_t *pstat;
		unsigned int cpu;
		int isprinted;
		
		isprinted = 0;
		
		for_each_possible_cpu(cpu) {
			pstat = &(per_cpu_ptr(hwperf_stats, cpu)[i]);
			
			if (pstat->executedNum) {
				avrgCycle[0] = div64_u64(pstat->accCycle[0],pstat->executedNum);
				avrgCycle[1] = div64_u64(pstat->accCycle[1],pstat->executedNum);			
			} else 
				continue;
			
			if(isprinted==0) {
				printk("    [idx=%2d]\n", (i+1));
				isprinted=1;
			} 
			//else 
				//printk("   ");

			printk("[cpu%d]exeNum=%10u\n", cpu, pstat->executedNum);
			//printk("[cpu%d]accCycle =[1]%15llu [0]%15llu \n", cpu, pstat->accCycle[1], pstat->accCycle[0]);
			printk("[cpu%d]avg      =[1]%15llu [0]%15llu \n", cpu, avrgCycle[1], avrgCycle[0]);
#if 1 //eric-ctxt
			//printk("[cpu%d]MAX      =[1]     %10u [0]     %10u \n", cpu, pstat->max_cycle[1], pstat->max_cycle[0]);
			printk("[cpu%d]min      =[1]     %10u [0]     %10u (%d %d)\n", 
				cpu, pstat->min_cycle[1], pstat->min_cycle[0], pstat->is_min_zero[1], pstat->is_min_zero[0]);
			//printk("[cpu%d]minP     =[1]     %10u [0]     %10u \n", 
				//cpu, pstat->min_cycle_P[1], pstat->min_cycle_P[0]);


			//printk("[cpu%d]ctxt     =[o]     %10u [a]     %10u \n", cpu, pstat->ctxt_all, (pstat->ctxt_all/pstat->executedNum));
			//printk("[cpu%d]lock     =[o]     %10u [a]     %10u \n", cpu, pstat->lock_all, (pstat->lock_all/pstat->executedNum));
#endif
		}
	}

}
#endif
//module_init(hwperf_init);
