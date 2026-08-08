/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1994 - 1999, 2000, 01, 06 Ralf Baechle
 * Copyright (C) 1995, 1996 Paul M. Antoine
 * Copyright (C) 1998 Ulf Carlsson
 * Copyright (C) 1999 Silicon Graphics, Inc.
 * Kevin D. Kissell, kevink@mips.com and Carsten Langgaard, carstenl@mips.com
 * Copyright (C) 2000, 01 MIPS Technologies, Inc.
 * Copyright (C) 2002, 2003, 2004, 2005, 2007  Maciej W. Rozycki
 *
 * Modified for RLX processors
 * Copyright (C) 2006-2012 Tony Wu (tonywu@realtek.com)
 */
#include <linux/compiler.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/smp.h>
#include <linux/spinlock.h>
#include <linux/kallsyms.h>
#include <linux/bootmem.h>
#include <linux/interrupt.h>
#include <linux/ptrace.h>
#include <linux/kgdb.h>
#include <linux/kdebug.h>

#include <asm/branch.h>
#include <asm/break.h>
#include <asm/cpu.h>
#include <asm/rlxregs.h>
#include <asm/module.h>
#include <asm/pgtable.h>
#include <asm/ptrace.h>
#include <asm/sections.h>
#include <asm/system.h>
#include <asm/tlbdebug.h>
#include <asm/traps.h>
#include <asm/uaccess.h>
#include <asm/watch.h>
#include <asm/mmu_context.h>
#include <asm/types.h>
#include <asm/stacktrace.h>
#include <asm/irq.h>
#include <asm/uasm.h>

extern asmlinkage void rlx_irq_dispatch(void);
extern asmlinkage void handle_tlbm(void);
extern asmlinkage void handle_tlbl(void);
extern asmlinkage void handle_tlbs(void);
extern asmlinkage void handle_adel(void);
extern asmlinkage void handle_ades(void);
#ifdef CONFIG_CPU_HAS_BUS_ERROR
extern asmlinkage void handle_ibe(void);
extern asmlinkage void handle_dbe(void);
#endif
extern asmlinkage void handle_sys(void);
extern asmlinkage void handle_bp(void);
extern asmlinkage void handle_ri(void);
extern asmlinkage void handle_cpu(void);
extern asmlinkage void handle_ov(void);
#ifdef CONFIG_CPU_HAS_WMPU
extern asmlinkage void handle_watch(void);
#endif
extern asmlinkage void handle_reserved(void);

#ifdef CONFIG_CPU_HAS_BUS_ERROR
void (*board_be_init)(void);
int (*board_be_handler)(struct pt_regs *regs, int is_fixup);
#endif


static void show_raw_backtrace(unsigned long reg29)
{
	unsigned long *sp = (unsigned long *)(reg29 & ~3);
	unsigned long addr;

	printk("Call Trace:");
#ifdef CONFIG_KALLSYMS
	printk("\n");
#endif
	while (!kstack_end(sp)) {
		unsigned long __user *p =
			(unsigned long __user *)(unsigned long)sp++;
		if (__get_user(addr, p)) {
			printk(" (Bad stack address)");
			break;
		}
		if (__kernel_text_address(addr))
			print_ip_sym(addr);
	}
	printk("\n");
}

#ifdef CONFIG_KALLSYMS
int raw_show_trace;
static int __init set_raw_show_trace(char *str)
{
	raw_show_trace = 1;
	return 1;
}
__setup("raw_show_trace", set_raw_show_trace);
#endif

static void show_backtrace(struct task_struct *task, const struct pt_regs *regs)
{
	unsigned long sp = regs->regs[29];
	unsigned long ra = regs->regs[31];
	unsigned long pc = regs->cp0_epc;

	if (raw_show_trace || !__kernel_text_address(pc)) {
		show_raw_backtrace(sp);
		return;
	}
	printk("Call Trace:\n");
	do {
		print_ip_sym(pc);
		pc = unwind_stack(task, &sp, pc, &ra);
	} while (pc);
	printk("\n");
}

/*
 * This routine abuses get_user()/put_user() to reference pointers
 * with at least a bit of error checking ...
 */
static void show_stacktrace(struct task_struct *task,
	const struct pt_regs *regs)
{
	const int field = 2 * sizeof(unsigned long);
	long stackdata;
	int i;
	unsigned long __user *sp = (unsigned long __user *)regs->regs[29];

	printk("Stack :");
	i = 0;
	while ((unsigned long) sp & (PAGE_SIZE - 1)) {
		if (i && ((i % (64 / field)) == 0))
			printk("\n       ");
		if (i > 39) {
			printk(" ...");
			break;
		}

		if (__get_user(stackdata, sp++)) {
			printk(" (Bad stack address)");
			break;
		}

		printk(" %0*lx", field, stackdata);
		i++;
	}
	printk("\n");
	show_backtrace(task, regs);
}

void show_stack(struct task_struct *task, unsigned long *sp)
{
	struct pt_regs regs;
	if (sp) {
		regs.regs[29] = (unsigned long)sp;
		regs.regs[31] = 0;
		regs.cp0_epc = 0;
	} else {
		if (task && task != current) {
			regs.regs[29] = task->thread.reg29;
			regs.regs[31] = 0;
			regs.cp0_epc = task->thread.reg31;
		} else {
			prepare_frametrace(&regs);
		}
	}
	show_stacktrace(task, &regs);
}

/*
 * The architecture-independent dump_stack generator
 */
void dump_stack(void)
{
	struct pt_regs regs;

	prepare_frametrace(&regs);
	show_backtrace(current, &regs);
}

EXPORT_SYMBOL(dump_stack);

static void show_code(unsigned int __user *pc)
{
	long i;
	unsigned short __user *pc16 = NULL;

	printk("\nCode:");

	if ((unsigned long)pc & 1)
		pc16 = (unsigned short __user *)((unsigned long)pc & ~1);
	for(i = -3 ; i < 6 ; i++) {
		unsigned int insn;
		if (pc16 ? __get_user(insn, pc16 + i) : __get_user(insn, pc + i)) {
			printk(" (Bad address in epc)\n");
			break;
		}
		printk("%c%0*x%c", (i?' ':'<'), pc16 ? 4 : 8, insn, (i?' ':'>'));
	}
}

static void __show_regs(const struct pt_regs *regs)
{
	const int field = 2 * sizeof(unsigned long);
	unsigned int cause = regs->cp0_cause;
	int i;

	printk("Cpu %d\n", smp_processor_id());

	/*
	 * Saved main processor registers
	 */
	for (i = 0; i < 32; ) {
		if ((i % 4) == 0)
			printk("$%2d   :", i);
		if (i == 0)
			printk(" %0*lx", field, 0UL);
		else if (i == 26 || i == 27)
			printk(" %*s", field, "");
		else
			printk(" %0*lx", field, regs->regs[i]);

		i++;
		if ((i % 4) == 0)
			printk("\n");
	}

	printk("Hi    : %0*lx\n", field, regs->hi);
	printk("Lo    : %0*lx\n", field, regs->lo);

	/*
	 * Saved cp0 registers
	 */
	printk("epc   : %0*lx %pS\n", field, regs->cp0_epc,
	       (void *) regs->cp0_epc);
	printk("    %s\n", print_tainted());
	printk("ra    : %0*lx %pS\n", field, regs->regs[31],
	       (void *) regs->regs[31]);

	printk("Status: %08x    ", (uint32_t) regs->cp0_status);

	if (regs->cp0_status & ST0_KUO)
		printk("KUo ");
	if (regs->cp0_status & ST0_IEO)
		printk("IEo ");
	if (regs->cp0_status & ST0_KUP)
		printk("KUp ");
	if (regs->cp0_status & ST0_IEP)
		printk("IEp ");
	if (regs->cp0_status & ST0_KUC)
		printk("KUc ");
	if (regs->cp0_status & ST0_IEC)
		printk("IEc ");
	printk("\n");

	printk("Cause : %08x\n", cause);

	cause = (cause & CAUSEF_EXCCODE) >> CAUSEB_EXCCODE;
	if (1 <= cause && cause <= 5)
		printk("BadVA : %0*lx\n", field, regs->cp0_badvaddr);

	printk("PrId  : %08x\n", read_c0_prid());

#ifdef CONFIG_CPU_HAS_RADIAX
	printk("CBS0  : %08x\n", (uint32_t)regs->radiax[0]);
	printk("CBS1  : %08x\n", (uint32_t)regs->radiax[1]);
	printk("CBS2  : %08x\n", (uint32_t)regs->radiax[2]);
	printk("CBE0  : %08x\n", (uint32_t)regs->radiax[3]);
	printk("CBE1  : %08x\n", (uint32_t)regs->radiax[4]);
	printk("CBE2  : %08x\n", (uint32_t)regs->radiax[5]);
	printk("LPS0  : %08x\n", (uint32_t)regs->radiax[6]);
	printk("LPE0  : %08x\n", (uint32_t)regs->radiax[7]);
	printk("LPC0  : %08x\n", (uint32_t)regs->radiax[8]);
	printk("MMD   : %08x\n", (uint32_t)regs->radiax[9]);
	printk("M0LL  : %08x\n", (uint32_t)regs->radiax[10]);
	printk("M0LH  : %08x\n", (uint32_t)regs->radiax[11]);
	printk("M0HL  : %08x\n", (uint32_t)regs->radiax[12]);
	printk("M0HH  : %08x\n", (uint32_t)regs->radiax[13]);
	printk("M1LL  : %08x\n", (uint32_t)regs->radiax[14]);
	printk("M1LH  : %08x\n", (uint32_t)regs->radiax[15]);
	printk("M1HL  : %08x\n", (uint32_t)regs->radiax[16]);
	printk("M1HH  : %08x\n", (uint32_t)regs->radiax[17]);
	printk("M2LL  : %08x\n", (uint32_t)regs->radiax[18]);
	printk("M2LH  : %08x\n", (uint32_t)regs->radiax[19]);
	printk("M2HL  : %08x\n", (uint32_t)regs->radiax[20]);
	printk("M2HH  : %08x\n", (uint32_t)regs->radiax[21]);
	printk("M3LL  : %08x\n", (uint32_t)regs->radiax[22]);
	printk("M3LH  : %08x\n", (uint32_t)regs->radiax[23]);
	printk("M3HL  : %08x\n", (uint32_t)regs->radiax[24]);
	printk("N3HH  : %08x\n", (uint32_t)regs->radiax[25]);
#endif
}

/*
 * FIXME: really the generic show_regs should take a const pointer argument.
 */
void show_regs(struct pt_regs *regs)
{
	__show_regs((struct pt_regs *)regs);
}

void show_registers(const struct pt_regs *regs)
{
	const int field = 2 * sizeof(unsigned long);

	__show_regs(regs);
	print_modules();
	printk("Process %s (pid: %d, threadinfo=%p, task=%p, tls=%0*lx)\n",
	       current->comm, current->pid, current_thread_info(), current,
	      field, current_thread_info()->tp_value);
#ifdef CONFIG_CPU_HAS_TLS
	{
		unsigned long tls;

		tls = read_lxc0_userlocal();
		if (tls != current_thread_info()->tp_value)
			printk("*HwTLS: %0*lx\n", field, tls);
	}
#endif

	show_stacktrace(current, regs);
	show_code((unsigned int __user *) regs->cp0_epc);
	printk("\n");
}

static DEFINE_SPINLOCK(die_lock);

void __noreturn die(const char * str, struct pt_regs * regs)
{
	static int die_counter;
	int sig = SIGSEGV;

	oops_enter();

	if (notify_die(DIE_OOPS, str, regs, 0, current->thread.trap_no, SIGSEGV) == NOTIFY_STOP)
		sig = 0;

	console_verbose();
	spin_lock_irq(&die_lock);
	bust_spinlocks(1);

	printk("%s[#%d]:\n", str, ++die_counter);
	show_registers(regs);
	add_taint(TAINT_DIE);
	spin_unlock_irq(&die_lock);

	oops_exit();

	if (in_interrupt())
		panic("Fatal exception in interrupt");

	if (panic_on_oops) {
		printk(KERN_EMERG "Fatal exception: panic in 5 seconds\n");
		ssleep(5);
		panic("Fatal exception");
	}

	do_exit(sig);
}

#ifdef CONFIG_CPU_HAS_BUS_ERROR
extern struct exception_table_entry __start___dbe_table[];
extern struct exception_table_entry __stop___dbe_table[];

__asm__(
"	.section	__dbe_table, \"a\"\n"
"	.previous			\n");

/* Given an address, look for it in the exception tables. */
static const struct exception_table_entry *search_dbe_tables(unsigned long addr)
{
	const struct exception_table_entry *e;

	e = search_extable(__start___dbe_table, __stop___dbe_table - 1, addr);
	if (!e)
		e = search_module_dbetables(addr);
	return e;
}

asmlinkage void do_be(struct pt_regs *regs)
{
	const int field = 2 * sizeof(unsigned long);
	const struct exception_table_entry *fixup = NULL;
	int data = regs->cp0_cause & 4;
	int action = MIPS_BE_FATAL;

	/* XXX For now.  Fixme, this searches the wrong table ...  */
	if (data && !user_mode(regs))
		fixup = search_dbe_tables(exception_epc(regs));

	if (fixup)
		action = MIPS_BE_FIXUP;

	if (board_be_handler)
		action = board_be_handler(regs, fixup != NULL);

	switch (action) {
	case MIPS_BE_DISCARD:
		return;
	case MIPS_BE_FIXUP:
		if (fixup) {
			regs->cp0_epc = fixup->nextinsn;
			return;
		}
		break;
	default:
		break;
	}

	/*
	 * Assume it would be too dangerous to continue ...
	 */
	printk(KERN_ALERT "%s bus error, epc == %0*lx, ra == %0*lx\n",
	       data ? "Data" : "Instruction",
	       field, regs->cp0_epc, field, regs->regs[31]);
	if (notify_die(DIE_OOPS, "bus error", regs, SIGBUS, 0, 0)
	    == NOTIFY_STOP)
		return;

	die_if_kernel("Oops", regs);
	force_sig(SIGBUS, current);
}
#endif

/*
 * ll/sc, rdhwr, sync emulation
 */

#define OPCODE 0xfc000000
#define BASE   0x03e00000
#define RT     0x001f0000
#define OFFSET 0x0000ffff
#define LL     0xc0000000
#define SC     0xe0000000
#define SPEC0  0x00000000
#define SPEC3  0x7c000000
#define RD     0x0000f800
#define FUNC   0x0000003f
#define SYNC   0x0000000f
#define RDHWR  0x0000003b

/*
 * The ll_bit is cleared by r*_switch.S
 */

#ifndef CONFIG_CPU_HAS_LLSC
unsigned int ll_bit;
struct task_struct *ll_task;

static inline int simulate_ll(struct pt_regs *regs, unsigned int opcode)
{
	unsigned long value, __user *vaddr;
	long offset;

	/*
	 * analyse the ll instruction that just caused a ri exception
	 * and put the referenced address to addr.
	 */

	/* sign extend offset */
	offset = opcode & OFFSET;
	offset <<= 16;
	offset >>= 16;

	vaddr = (unsigned long __user *)
	        ((unsigned long)(regs->regs[(opcode & BASE) >> 21]) + offset);

	if ((unsigned long)vaddr & 3)
		return SIGBUS;
	if (get_user(value, vaddr))
		return SIGSEGV;

	preempt_disable();

	if (ll_task == NULL || ll_task == current) {
		ll_bit = 1;
	} else {
		ll_bit = 0;
	}
	ll_task = current;

	preempt_enable();

	regs->regs[(opcode & RT) >> 16] = value;

	return 0;
}

static inline int simulate_sc(struct pt_regs *regs, unsigned int opcode)
{
	unsigned long __user *vaddr;
	unsigned long reg;
	long offset;

	/*
	 * analyse the sc instruction that just caused a ri exception
	 * and put the referenced address to addr.
	 */

	/* sign extend offset */
	offset = opcode & OFFSET;
	offset <<= 16;
	offset >>= 16;

	vaddr = (unsigned long __user *)
	        ((unsigned long)(regs->regs[(opcode & BASE) >> 21]) + offset);
	reg = (opcode & RT) >> 16;

	if ((unsigned long)vaddr & 3)
		return SIGBUS;

	preempt_disable();

	if (ll_bit == 0 || ll_task != current) {
		regs->regs[reg] = 0;
		preempt_enable();
		return 0;
	}

	preempt_enable();

	if (put_user(regs->regs[reg], vaddr))
		return SIGSEGV;

	regs->regs[reg] = 1;

	return 0;
}

/*
 * ll uses the opcode of lwc0 and sc uses the opcode of swc0.  That is both
 * opcodes are supposed to result in coprocessor unusable exceptions if
 * executed on ll/sc-less processors.  That's the theory.  In practice a
 * few processors such as NEC's VR4100 throw reserved instruction exceptions
 * instead, so we're doing the emulation thing in both exception handlers.
 */
static int simulate_llsc(struct pt_regs *regs, unsigned int opcode)
{
	if ((opcode & OPCODE) == LL)
		return simulate_ll(regs, opcode);
	if ((opcode & OPCODE) == SC)
		return simulate_sc(regs, opcode);

	return -1;			/* Must be something else ... */
}
#endif

#ifndef CONFIG_CPU_HAS_SYNC
static int simulate_sync(struct pt_regs *regs, unsigned int opcode)
{
	if ((opcode & OPCODE) == SPEC0 && (opcode & FUNC) == SYNC)
		return 0;

	return -1;			/* Must be something else ... */
}
#endif

asmlinkage void do_ov(struct pt_regs *regs)
{
	siginfo_t info;

	die_if_kernel("Integer overflow", regs);

	info.si_code = FPE_INTOVF;
	info.si_signo = SIGFPE;
	info.si_errno = 0;
	info.si_addr = (void __user *) regs->cp0_epc;
	force_sig_info(SIGFPE, &info, current);
}

static void do_trap_or_bp(struct pt_regs *regs, unsigned int code,
	const char *str)
{
	siginfo_t info;
	char b[40];

	if (notify_die(DIE_TRAP, str, regs, code, 0, 0) == NOTIFY_STOP)
		return;

	/*
	 * A short test says that IRIX 5.3 sends SIGTRAP for all trap
	 * insns, even for trap and break codes that indicate arithmetic
	 * failures.  Weird ...
	 * But should we continue the brokenness???  --macro
	 */
	switch (code) {
	case BRK_OVERFLOW:
	case BRK_DIVZERO:
		scnprintf(b, sizeof(b), "%s instruction in kernel code", str);
		die_if_kernel(b, regs);
		if (code == BRK_DIVZERO)
			info.si_code = FPE_INTDIV;
		else
			info.si_code = FPE_INTOVF;
		info.si_signo = SIGFPE;
		info.si_errno = 0;
		info.si_addr = (void __user *) regs->cp0_epc;
		force_sig_info(SIGFPE, &info, current);
		break;
	case BRK_BUG:
		die_if_kernel("Kernel bug detected", regs);
		force_sig(SIGTRAP, current);
		break;
	default:
		scnprintf(b, sizeof(b), "%s instruction in kernel code", str);
		die_if_kernel(b, regs);
		force_sig(SIGTRAP, current);
	}
}

asmlinkage void do_bp(struct pt_regs *regs)
{
	unsigned int opcode, bcode;

	if (__get_user(opcode, (unsigned int __user *) exception_epc(regs)))
		goto out_sigsegv;

	/*
	 * There is the ancient bug in the MIPS assemblers that the break
	 * code starts left to bit 16 instead to bit 6 in the opcode.
	 * Gas is bug-compatible, but not always, grrr...
	 * We handle both cases with a simple heuristics.  --macro
	 */
	bcode = ((opcode >> 6) & ((1 << 20) - 1));
	if (bcode >= (1 << 10))
		bcode >>= 10;

	do_trap_or_bp(regs, bcode, "Break");
	return;

out_sigsegv:
	force_sig(SIGSEGV, current);
}

asmlinkage void do_tr(struct pt_regs *regs)
{
	unsigned int opcode, tcode = 0;

	if (__get_user(opcode, (unsigned int __user *) exception_epc(regs)))
		goto out_sigsegv;

	/* Immediate versions don't provide a code.  */
	if (!(opcode & OPCODE))
		tcode = ((opcode >> 6) & ((1 << 10) - 1));

	do_trap_or_bp(regs, tcode, "Trap");
	return;

out_sigsegv:
	force_sig(SIGSEGV, current);
}

asmlinkage void do_ri(struct pt_regs *regs)
{
	unsigned int __user *epc = (unsigned int __user *)exception_epc(regs);
	unsigned long old_epc = regs->cp0_epc;
	unsigned int opcode = 0;
	int status = -1;

	if (notify_die(DIE_RI, "RI Fault", regs, SIGSEGV, 0, 0)
	    == NOTIFY_STOP)
		return;

	die_if_kernel("Reserved instruction in kernel code", regs);

	if (unlikely(compute_return_epc(regs) < 0))
		return;

	if (unlikely(get_user(opcode, epc) < 0))
		status = SIGSEGV;

#ifndef CONFIG_CPU_HAS_LLSC
	if (!cpu_has_llsc && status < 0)
		status = simulate_llsc(regs, opcode);
#endif

#ifndef CONFIG_CPU_HAS_SYNC
	if (status < 0)
		status = simulate_sync(regs, opcode);
#endif

	if (status < 0)
		status = SIGILL;

	if (unlikely(status > 0)) {
		regs->cp0_epc = old_epc;		/* Undo skip-over.  */
		force_sig(status, current);
	}
}

asmlinkage void do_cpu(struct pt_regs *regs)
{
	unsigned int __user *epc;
	unsigned long old_epc;
	unsigned int opcode;
	unsigned int cpid;
	int status;
	unsigned long __maybe_unused flags;

	die_if_kernel("do_cpu invoked from kernel context!", regs);

	cpid = (regs->cp0_cause >> CAUSEB_CE) & 3;

	switch (cpid) {
	case 0:
		epc = (unsigned int __user *)exception_epc(regs);
		old_epc = regs->cp0_epc;
		opcode = 0;
		status = -1;

		if (unlikely(compute_return_epc(regs) < 0))
			return;

		if (unlikely(get_user(opcode, epc) < 0))
			status = SIGSEGV;

#ifndef CONFIG_CPU_HAS_LLSC
		if (!cpu_has_llsc && status < 0)
			status = simulate_llsc(regs, opcode);
#endif

		if (status < 0)
			status = SIGILL;

		if (unlikely(status > 0)) {
			regs->cp0_epc = old_epc;	/* Undo skip-over.  */
			force_sig(status, current);
		}

		return;

	case 1:
	case 2:
	case 3:
		break;
	}

	force_sig(SIGILL, current);
}

/*
 * Called with interrupts disabled.
 */
#ifdef CONFIG_CPU_HAS_WMPU
asmlinkage void do_watch(struct pt_regs *regs)
{
	u32 cause;

	/*
	 * Clear WP (bit 22) bit of cause register so we don't loop
	 * forever.
	 */
	cause = read_c0_cause();
	cause &= ~(1 << 22);
	write_c0_cause(cause);

	/*
	 * If the current thread has the watch registers loaded, save
	 * their values and send SIGTRAP.  Otherwise another thread
	 * left the registers set, clear them and continue.
	 */
	if (test_tsk_thread_flag(current, TIF_LOAD_WATCH)) {
		mips_read_watch_registers();
		local_irq_enable();
		force_sig(SIGTRAP, current);
	} else {
		mips_clear_watch_registers();
		local_irq_enable();
	}
}
#endif

asmlinkage void do_reserved(struct pt_regs *regs)
{
	/*
	 * Game over - no way to handle this if it ever occurs.  Most probably
	 * caused by a new unknown cpu type or after another deadly
	 * hard/software error.
	 */
	show_regs(regs);
	panic("Caught reserved exception %ld - should not happen.",
	      (regs->cp0_cause & 0x7f) >> 2);
}

unsigned long exception_handlers[32];

void __init *set_except_vector(int n, void *addr)
{
	unsigned long handler = (unsigned long) addr;
	unsigned long old_handler = exception_handlers[n];

	exception_handlers[n] = handler;
	return (void *)old_handler;
}

extern void cpu_cache_init(void);
extern void tlb_init(void);
extern void flush_tlb_handlers(void);

void __cpuinit per_cpu_trap_init(void)
{
	unsigned int cpu = smp_processor_id();
	unsigned int status_set = ST0_CU0;

	/*
	 * Disable coprocessors and select 32-bit or 64-bit addressing
	 * and the 16/32 or 32/32 FPR register model.  Reset the BEV
	 * flag that some firmware may have left set and the TS bit (for
	 * IP27).  Set XX for ISA IV code to work.
	 */
	change_c0_status(ST0_CU|ST0_BEV, status_set);

	if (!cpu_data[cpu].asid_cache)
		cpu_data[cpu].asid_cache = ASID_FIRST_VERSION;
	TLBMISS_HANDLER_SETUP();

	atomic_inc(&init_mm.mm_count);
	current->active_mm = &init_mm;
	BUG_ON(current->mm);
	enter_lazy_tlb(&init_mm, current);

	cpu_cache_init();
	tlb_init();
}

#define RLX_TRAP_VEC_BASE	0x80000080
#define RLX_TRAP_VEC_SIZE	0x80

void __init trap_init(void)
{
	extern char rlx_trap_dispatch;
	unsigned long i;

#if defined(CONFIG_KGDB)
	if (kgdb_early_setup)
		return;	/* Already done */
#endif

	per_cpu_trap_init();

	/*
	 * Setup default vectors
	 */
	for (i = 0; i <= 31; i++)
		set_except_vector(i, handle_reserved);

	/*
	 * The Data Bus Errors / Instruction Bus Errors are signaled
	 * by external hardware.  Therefore these two exceptions
	 * may have board specific handlers.
	 */
#ifdef CONFIG_CPU_HAS_BUS_ERROR
	if (board_be_init)
		board_be_init();
#endif

	set_except_vector(0, rlx_irq_dispatch);
	set_except_vector(1, handle_tlbm);
	set_except_vector(2, handle_tlbl);
	set_except_vector(3, handle_tlbs);

	set_except_vector(4, handle_adel);
	set_except_vector(5, handle_ades);

#ifdef CONFIG_CPU_HAS_BUS_ERROR
	set_except_vector(6, handle_ibe);
	set_except_vector(7, handle_dbe);
#endif

	set_except_vector(8, handle_sys);
	set_except_vector(9, handle_bp);
	set_except_vector(10, handle_ri);
	set_except_vector(11, handle_cpu);
	set_except_vector(12, handle_ov);

	/*
	 * Only some CPUs have the watch exceptions.
	 */
#ifdef CONFIG_CPU_HAS_WMPU
	set_except_vector(23, handle_watch);
#endif

	memcpy((void *)(RLX_TRAP_VEC_BASE), &rlx_trap_dispatch, RLX_TRAP_VEC_SIZE);

	local_flush_icache_range(RLX_TRAP_VEC_BASE, RLX_TRAP_VEC_BASE + RLX_TRAP_VEC_SIZE);
	flush_tlb_handlers();

	sort_extable(__start___dbe_table, __stop___dbe_table);
}
