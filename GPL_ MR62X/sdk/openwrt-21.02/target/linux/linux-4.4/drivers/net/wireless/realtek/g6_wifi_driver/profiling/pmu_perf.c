
#include <linux/smp.h>
#include <linux/kernel.h>
#include <linux/workqueue.h>
#include <linux/string.h>

#include "./pmu_perf.h"

u64 perf_count[4] = {0};
u64 cp3_count[4][6][CPU_CNT_MAX] = {0};
u64 cp3_count_min[4][6][CPU_CNT_MAX] = {0};
u64 cp3_count_max[4][6][CPU_CNT_MAX] = {0};
u64 cp3_cnt_temp[4][6][CPU_CNT_MAX] = {0};
char funcs[4][CPU_CNT_MAX][FUNC_LEN] = {0};
int func_in[4][CPU_CNT_MAX] = {0};
int func_exit[4][CPU_CNT_MAX] = {0};
const char *cnt_name[4][CPU_CNT_MAX] = {0};
int profile_stopped[4] = {1, 1, 1, 1};
/* cpu cycles, inst cnt, load cnt, STORE cnt, L1T cache, STALL_BACKEND */
u32 event_type[6] = {17, 8, 6, 7, 20, 36};
int lock_mode = 0;

struct work_struct ptool_wq[4] = {0};
int wq_action[4] = {0};		//0: set event / reset / start, 1:stop
int pmcr_el0_start = 7;
int pmcr_el0_stop = 0;

u32 group_idx = 0;
u32 cnt_start = CPU_TX_ALL;
u32 cnt_end = CPU_TEST8;

const struct cnt_group cnt_groups[] = 
{
	CPU_GROUP_SPEC(0,	CPU_TX_ALL,		CPU_TEST8,		"CPU_TX_ALL-CPU_TEST8"),
	CPU_GROUP_SPEC(1,	CPU_RX_ALL,		CPU_TEST18,		"CPU_RX_ALL-CPU_TEST18"),
	CPU_GROUP_SPEC(2,	CPU_TX1,		CPU_TX9,		"CPU_TX1-CPU_TX9 (Core, test2)"),
	CPU_GROUP_SPEC(3,	CPU_TX11,		CPU_TX18,		"CPU_TX11-CPU_TX18 (PHL)"),
	CPU_GROUP_SPEC(4,	CPU_RX1,		CPU_RX8,		"CPU_RX1-CPU_RX8"),
	CPU_GROUP_SPEC(5,	CPU_RX11,		CPU_RX18,		"CPU_RX11-CPU_RX18"),
	CPU_GROUP_SPEC(6,	CPU_LOCK1,		CPU_LOCK8,		"CPU_LOCK1-CPU_LOCK8"),
	CPU_GROUP_SPEC(7,	CPU_CACHE1,		CPU_CACHE8,		"CPU_CACHE1-CPU_CACHE8"),
	CPU_GROUP_SPEC(8,	CPU_CACHE11,	CPU_CACHE18,	"CPU_CACHE11-CPU_CACHE18"),
	CPU_GROUP_SPEC(9,	CPU_PHY1,		CPU_PHY8,		"CPU_PHY1-CPU_PHY8"),
	CPU_GROUP_SPEC(10,	CPU_PHY11,		CPU_PHY18,		"CPU_PHY11-CPU_PHY18"),
	CPU_GROUP_SPEC(11,	CPU_PHY21,		CPU_PHY28,		"CPU_PHY21-CPU_PHY28"),
	CPU_GROUP_SPEC(12,	CPU_TX21,		CPU_TX28,		"CPU_TX21-CPU_TX28"),
	CPU_GROUP_SPEC(13,	CPU_PHLRX01,	CPU_PHLRX08,	"CPU_PHLRX01"),
	CPU_GROUP_SPEC(14,	CPU_PHLRX11,	CPU_PHLRX18,	"CPU_PHLRX11"),
};

#define CPU_EVENT_SPEC(_event, _name) { \
	.event        	= (_event), \
	.name         	= (_name), \
}

const struct cnt_event cnt_events[] =
{
	CPU_EVENT_SPEC(0,		"SW_INCR"),
	CPU_EVENT_SPEC(1,		"L1I_CACHE_REFILL"),
	CPU_EVENT_SPEC(2,		"L1I_TLB_REFILL"),
	CPU_EVENT_SPEC(3,		"L1D_CACHE_REFILL"),
	CPU_EVENT_SPEC(4,		"L1D_CACHE"),
	CPU_EVENT_SPEC(5,		"L1D_TLB_REFILL"),
	CPU_EVENT_SPEC(6,		"LD_RETIRED"),
	CPU_EVENT_SPEC(7,		"ST_RETIRED"),
	CPU_EVENT_SPEC(8,		"INST_RETIRED"),
	CPU_EVENT_SPEC(9,		"EXC_TAKEN"),
	CPU_EVENT_SPEC(10,		"EXC_RETURN"),
	CPU_EVENT_SPEC(11,		"CID_WRITE_RETIRED"),
	CPU_EVENT_SPEC(12,		"PC_WRITE_RETIRED"),
	CPU_EVENT_SPEC(13,		"BR_IMMED_RETIRED"),
	CPU_EVENT_SPEC(14,		"BR_RETURN_RETIRED"),
	CPU_EVENT_SPEC(15,		"UNALIGNED_LDST_RETIRED"),
	CPU_EVENT_SPEC(16,		"BR_MIS_PRED"),
	CPU_EVENT_SPEC(17,		"CPU_CYCLES"),
	CPU_EVENT_SPEC(18,		"BR_PRED"),
	CPU_EVENT_SPEC(19,		"MEM_ACCESS"),
	CPU_EVENT_SPEC(20,		"L1I_CACHE"),
	CPU_EVENT_SPEC(21,		"L1D_CACHE_WB"),
	CPU_EVENT_SPEC(22,		"L2D_CACHE"),
	CPU_EVENT_SPEC(23,		"L2D_CACHE_REFILL"),
	CPU_EVENT_SPEC(24,		"L2D_CACHE_WB"),
	CPU_EVENT_SPEC(25,		"BUS_ACCESS"),
	CPU_EVENT_SPEC(26,		"MEMORY_ERROR"),
	CPU_EVENT_SPEC(27,		"INST_SPEC"),
	CPU_EVENT_SPEC(28,		"TTBR_WRITE_RETIRED"),
	CPU_EVENT_SPEC(29,		"BUS_CYCLES"),
	CPU_EVENT_SPEC(30,		"CHAIN"),
	CPU_EVENT_SPEC(32,		"L2D_CACHE_ALLOCATE"),
	CPU_EVENT_SPEC(33,		"BR_RETIRED"),
	CPU_EVENT_SPEC(34,		"BR_MIS_PRED_RETIRED"),
	CPU_EVENT_SPEC(35,		"STALL_FRONTEND"),
	CPU_EVENT_SPEC(36,		"STALL_BACKEND"),
	CPU_EVENT_SPEC(37,		"L1D_TLB"),
	CPU_EVENT_SPEC(38,		"L1I_TLB"),
	CPU_EVENT_SPEC(41,		"L3D_CACHE_ALLOCATE"),
	CPU_EVENT_SPEC(42,		"L3D_CACHE_REFILL"),
	CPU_EVENT_SPEC(43,		"L3D_CACHE"),
	CPU_EVENT_SPEC(45,		"L2D_TLB_REFILL"),
	CPU_EVENT_SPEC(47,		"L2D_TLB"),
	CPU_EVENT_SPEC(52,		"DTLB_WALK"),
	CPU_EVENT_SPEC(53,		"ITLB_WALK"),
	CPU_EVENT_SPEC(54,		"LL_CACHE_RD"),
	CPU_EVENT_SPEC(55,		"LL_CACHE_MISS_RD"),
	CPU_EVENT_SPEC(56,		"REMOTE_ACCESS_RD"),
	CPU_EVENT_SPEC(64,		"L1D_CACHE_RD"),
	CPU_EVENT_SPEC(65,		"L1D_CACHE_WR"),
	CPU_EVENT_SPEC(66,		"L1D_CACHE_REFILL_RD"),
	CPU_EVENT_SPEC(67,		"L1D_CACHE_REFILL_WR"),
	CPU_EVENT_SPEC(68,		"L1D_CACHE_REFILL_INNER"),
	CPU_EVENT_SPEC(69,		"L1D_CACHE_REFILL_OUTER"),
	CPU_EVENT_SPEC(80,		"L2D_CACHE_RD"),
	CPU_EVENT_SPEC(81,		"L2D_CACHE_WR"),
	CPU_EVENT_SPEC(82,		"L2D_CACHE_REFILL_RD"),
	CPU_EVENT_SPEC(83,		"L2D_CACHE_REFILL_WR"),
	CPU_EVENT_SPEC(96,		"BUS_ACCESS_RD"),
	CPU_EVENT_SPEC(97,		"BUS_ACCESS_WR"),
	CPU_EVENT_SPEC(102,		"MEM_ACCESS_RD"),
	CPU_EVENT_SPEC(103,		"MEM_ACCESS_WR"),
	CPU_EVENT_SPEC(112,		"LD_SPEC"),
	CPU_EVENT_SPEC(113,		"ST_SPEC"),
	CPU_EVENT_SPEC(114,		"LDST_SPEC"),
	CPU_EVENT_SPEC(115,		"DP_SPEC"),
	CPU_EVENT_SPEC(116,		"ASE_SPEC"),
	CPU_EVENT_SPEC(117,		"VFP_SPEC"),
	CPU_EVENT_SPEC(118,		"PC_WRITE_SPEC"),
	CPU_EVENT_SPEC(119,		"CRYPTO_SPEC"),
	CPU_EVENT_SPEC(120,		"BR_IMMED_SPEC"),
	CPU_EVENT_SPEC(121,		"BR_RETURN_SPEC"),
	CPU_EVENT_SPEC(122,		"BR_INDIRECT_SPEC"),
	CPU_EVENT_SPEC(134,		"EXC_IRQ"),
	CPU_EVENT_SPEC(135,		"EXC_FIQ"),
	CPU_EVENT_SPEC(160,		"L3D_CACHE_RD"),
	CPU_EVENT_SPEC(162,		"L3D_CACHE_REFILL_RD"),
	CPU_EVENT_SPEC(192,		"L3D_CACHE_REFILL_PREFETCH"),
	CPU_EVENT_SPEC(193,		"L2D_CACHE_REFILL_PREFETCH"),
	CPU_EVENT_SPEC(194,		"L1D_CACHE_REFILL_PREFETCH"),
	CPU_EVENT_SPEC(195,		"L2D_WS_MODE"),
	CPU_EVENT_SPEC(196,		"L1D_WS_MODE_ENTRY"),
	CPU_EVENT_SPEC(197,		"L1D_WS_MODE"),
	CPU_EVENT_SPEC(198,		"PREDECODE_ERROR"),
	CPU_EVENT_SPEC(199,		"L3D_WS_MODE"),
	CPU_EVENT_SPEC(201,		"BR_COND_PRED"),
	CPU_EVENT_SPEC(202,		"BR_INDIRECT_MIS_PRED"),
	CPU_EVENT_SPEC(203,		"BR_INDIRECT_ADDR_MIS_PRED"),
	CPU_EVENT_SPEC(204,		"BR_COND_MIS_PRED"),
	CPU_EVENT_SPEC(205,		"BR_INDIRECT_ADDR_PRED"),
	CPU_EVENT_SPEC(206,		"BR_RETURN_ADDR_PRED"),
	CPU_EVENT_SPEC(207,		"BR_RETURN_ADDR_MIS_PRED"),
	CPU_EVENT_SPEC(208,		"L2D_LLWALK_TLB"),
	CPU_EVENT_SPEC(209,		"L2D_LLWALK_TLB_REFILL"),
	CPU_EVENT_SPEC(210,		"L2D_L2WALK_TLB"),
	CPU_EVENT_SPEC(211,		"L2D_L2WALK_TLB_REFILL"),
	CPU_EVENT_SPEC(212,		"L2D_S2_TLB"),
	CPU_EVENT_SPEC(213,		"L2D_S2_TLB_REFILL"),
	CPU_EVENT_SPEC(214,		"L2D_CACHE_STASH_DROPPED"),
	CPU_EVENT_SPEC(225,		"STALL_FRONTEND_CACHE"),
	CPU_EVENT_SPEC(226,		"STALL_FRONTEND_TLB"),
	CPU_EVENT_SPEC(227,		"STALL_FRONTEND_PDERR"),
	CPU_EVENT_SPEC(228,		"STALL_BACKEND_ILOCK"),
	CPU_EVENT_SPEC(229,		"STALL_BACKEND_ILOCK_AGU"),
	CPU_EVENT_SPEC(230,		"STALL_BACKEND_ILOCK_FPU"),
	CPU_EVENT_SPEC(231,		"STALL_BACKEND_LD"),
	CPU_EVENT_SPEC(232,		"STALL_BACKEND_ST"),
	CPU_EVENT_SPEC(233,		"STALL_BACKEND_LD_CACHE"),
	CPU_EVENT_SPEC(234,		"STALL_BACKEND_LD_TLB"),
	CPU_EVENT_SPEC(235,		"STALL_BACKEND_ST_STB"),
	CPU_EVENT_SPEC(236,		"STALL_BACKEND_ST_TLB"),
};

void wq_func(struct work_struct *pwq)
{
	int cpu_id = smp_processor_id();
	//printk("[WQ] cpu_id=%d wq_act=%d\n",cpu_id,wq_action[cpu_id]);
	switch (wq_action[cpu_id])
	{
	case 0: //set event reset and start
		{
			u32 pmcntenset_el0 = 0;
			u32 pmselr_el0 = 0;
			u32 pmxevtyper_el0 = 0;
			int i;

			asm __volatile__(
					"	MRS %0, pmcntenset_el0		\n"
					:"=r"(pmcntenset_el0)::
			);

			pmcntenset_el0 |= 0x8000003f;

			asm __volatile__(
					"	MSR pmcntenset_el0,%0	\n"
					::"r"(pmcntenset_el0):
			);

			for (i = 0; i < 6; i++)
			{
				pmselr_el0 = i;
				asm __volatile__(
						"	MSR pmselr_el0,%0	\n"
						::"r"(pmselr_el0):
				);

				pmxevtyper_el0 = event_type[i];
				asm __volatile__(
						"	MSR pmxevtyper_el0,%0	\n"
						::"r"(pmxevtyper_el0):
				);
			}

			memset(&funcs[cpu_id][0][0], 0, CPU_CNT_MAX * FUNC_LEN);
			memset(&func_in[cpu_id][0], 0, sizeof(int) * CPU_CNT_MAX);
			memset(&func_exit[cpu_id][0], 0, sizeof(int) * CPU_CNT_MAX);
			memset(&cp3_count[cpu_id][0][0], 0, sizeof(u64) * CPU_CNT_MAX * 6);
			memset(&cp3_count_max[cpu_id][0][0], 0, sizeof(u64) * CPU_CNT_MAX * 6);
			memset(&cp3_count_min[cpu_id][0][0], 0xff, sizeof(u64) * CPU_CNT_MAX * 6);
			memset(&cp3_cnt_temp[cpu_id][0][0], 0, sizeof(u64) * CPU_CNT_MAX * 6);

			profile_stopped[cpu_id] = 0;
			asm __volatile__(
					"	MSR pmcr_el0,%0	\n"
					::"r"(pmcr_el0_start):
			);

		}
		break;

	case 1: //stop
		asm __volatile__(
				"	MSR pmcr_el0,%0	\n"
				::"r"(pmcr_el0_stop):
		);
		profile_stopped[cpu_id] = 1;
		break;
	}
}

int profile_config_event(u32 group, u32 *events, u8 size)
{
	u8 i = 0;

	if (size > 6)
		return -1;

	group_idx = group;
	cnt_start = cnt_groups[group].start;
	cnt_end = cnt_groups[group].end;

	for (i = 0; i < size; i++) {
		event_type[i] = events[i];
	}

	return 0;
}

int profile_start(int event_start)
{
	int cpu_id = smp_processor_id();
	int i;

	if (event_start == -1)
		lock_mode = 1;
	else
		lock_mode = 0;

	//printk("this proc in CPU[%d]\n",smp_processor_id());

	for (i = 0; i < 4; i++) {
		if (ptool_wq[i].func == NULL) {
			INIT_WORK(&ptool_wq[i], wq_func);
		}

		wq_action[i] = 0;
		if (cpu_id != i)
			queue_work_on(i, system_highpri_wq, &ptool_wq[i]);
		else
			wq_func(NULL);
	}

	return 0;
}

int profile_stop(void)
{
	int i;
	int cpu_id = smp_processor_id();
	lock_mode = 0;

	for (i = 0; i < 4; i++) {
		if (ptool_wq[i].func == NULL) {
			INIT_WORK(&ptool_wq[i], wq_func);
		}

		wq_action[i] = 1;
		if (cpu_id != i)
			queue_work_on(i, system_highpri_wq, &ptool_wq[i]);
		else
			wq_func(NULL);
	}
	return 0;
}

void profile(const char *func, u64 idx, const char *idx_name)
{
	int j;
	int cpu_id = smp_processor_id();

	if (likely(profile_stopped[cpu_id] == 1))
		return;

	if (idx < cnt_start || idx > cnt_end)
		return;

	if (lock_mode == 1)
		local_irq_disable();

	if (func_in[cpu_id][idx]++ == 0) {
		memcpy(funcs[cpu_id][idx], (char *)func, FUNC_LEN-1);
		cnt_name[cpu_id][idx] = idx_name;
	}

	for (j = 0; j < 6; j++) {
		asm __volatile__(
			"	MSR pmselr_el0,%0	\n"
			::"r"(j):
		);

		asm __volatile__(
			"	MRS %0, pmxevcntr_el0		\n"
			:"=r"(cp3_cnt_temp[cpu_id][j][idx])::
		);

		//cp3_cnt_temp[j][idx]=pmxevcntr_el0;
	}
}

void profile_end(int idx)
{
	int j;
	u64 cnt;
	int cpu_id = smp_processor_id();

	if (likely(profile_stopped[cpu_id] == 1))
		return;

	if (idx < cnt_start || idx > cnt_end)
		return;

	if (lock_mode == 1)
		local_irq_enable();

	// fetch counter
	for (j = 0; j < 6; j++) {
		asm __volatile__(
			"	MSR pmselr_el0,%0	\n"
			::"r"(j):
		);
		asm __volatile__(
			"	MRS %0, pmxevcntr_el0		\n"
			:"=r"(perf_count[cpu_id])::
		);

		if (likely(cp3_cnt_temp[cpu_id][j][idx] <= perf_count[cpu_id])) {
			cnt = perf_count[cpu_id] - cp3_cnt_temp[cpu_id][j][idx];
		} else {
			cnt = perf_count[cpu_id] + 0x100000000LL - cp3_cnt_temp[cpu_id][j][idx];
		}

		if (cnt < cp3_count_min[cpu_id][j][idx])
			cp3_count_min[cpu_id][j][idx] = cnt;

		if (cnt > cp3_count_max[cpu_id][j][idx])
			cp3_count_max[cpu_id][j][idx] = cnt;

		cp3_count[cpu_id][j][idx] += cnt;
	}

	++func_exit[cpu_id][idx];
}

int profile_dump(void)
{

	int i, j = 0, func_cnt[4] = {0}, c = 0, k = 0;


	asm __volatile__(
	"	MSR pmcr_el0,xzr \n"
	:::
	);

	printk("Disable local interrupt mode: %s\n\n", lock_mode ? "on" : "off");


	printk("[HASH_],FUNC_IN_COUNT,FUNC_EXIT_COUNT, FUNCTION\n");
	printk("%23s[EVT],%20s,%15s,%15s,%15s\n\n", "EVENT_NAME", "EVENT_TOTAL_CNT", "EVENT_CNT(AVG)", "EVENT_CNT(MIN)", "EVENT_CNT(MAX)");

	for (c = 0; c < 4; c++) {
		printk("CPU[%d]:\n", c);
		for (i = 0; i < CPU_CNT_MAX; i++) {
			if (func_in[c][i] != 0) {
				++func_cnt[c];
				printk("[%5d %s],%13d,%15d", i, cnt_name[c][i], func_in[c][i], func_exit[c][i]);
				printk(", %s\n", &funcs[c][i][0]);
				for (j = 0; j < 6; j++) {
					for (k = 0; k < ARRAY_SIZE(cnt_events); k++) {
						if (event_type[j] == cnt_events[k].event) {
							u64 avg = cp3_count[c][j][i];
							do_div(avg, func_in[c][i]);
							printk("%23s[%3d]", cnt_events[k].name, cnt_events[k].event);
							printk(",%20llu", cp3_count[c][j][i]);
							printk(",%15llu", avg);
							printk(",%15llu", cp3_count_min[c][j][i]);
							printk(",%15llu\n", cp3_count_max[c][j][i]);
						}
					}
				}
				printk("\n");
			}
		}

		printk("Total %d functions in CPU%d\n\n", func_cnt[c], c);
	}

	return 0;
}

void display_cpu_event_name(void)
{
	int idx, i = 0, j = 0;

	printk("current group=%d (%s)idx=%d~%d \n", group_idx, cnt_groups[group_idx].name, cnt_start, cnt_end);

	for (i = 5; i >= 0; i--) {
		idx = -1;
		for (j = 0; j < ARRAY_SIZE(cnt_events); j++) {
			if (event_type[i] == cnt_events[j].event) {
				idx = j;
				break;
			}
		}

		if (idx >= 0)
			printk("[%u] = %s ,", i, cnt_events[idx].name);
		else
			printk("[%u] = %u UNKNOWN ,", i, event_type[i]);
	}

	printk("\n");
}

void dump_cpu_event_name(void)
{
	int idx = 0;
	
	printk("===\n");	
	
	for (idx = 0; idx < ARRAY_SIZE(cnt_groups); idx++)
		printk("group = %02d (%s)idx = %d~%d \n", idx, cnt_groups[idx].name, cnt_groups[idx].start, cnt_groups[idx].end);

	printk("===\n");

	for (idx = 0; idx < ARRAY_SIZE(cnt_events); idx++)
		printk("(%02d) = %s \n", cnt_events[idx].event, cnt_events[idx].name);

	printk("===\n");

	display_cpu_event_name();
}


