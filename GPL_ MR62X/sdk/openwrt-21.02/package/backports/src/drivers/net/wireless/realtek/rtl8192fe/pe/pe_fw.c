#include "HalPrecomp.h" 

#include <pe_fw.h>


//Fetch from DT
static struct pe_addr_base pe_addr;

struct pe_addr_base* get_pe_addr_base(void)
{
	return &pe_addr;
} 

void download_pe_fw(void)
{
	struct pe_addr_base* pe_addr = get_pe_addr_base();
	
	//panic_printk("[PE_DOWNLOAD] start=======>\r\n");
	//panic_printk("Total Memory %d bits Used by PE\r\n", pe_addr->pe_fw_mem_size);
	//panic_printk("atu_virt_addr = %x, fw_virt_addr = %x\r\n cpu_virt_addr = %x, mem_virt_addr = %x\r\n", 
	//	pe_addr->pe_atu_virt_addr, pe_addr->pe_fw_virt_addr, pe_addr->pe_cpu_virt_addr, pe_addr->pe_mem_virt_addr);

	//Change PE boot addr, atu setting
	*(volatile unsigned int*)(pe_addr->pe_atu_virt_addr + 0x4) = 0x00001FC0;
	*(volatile unsigned int*)(pe_addr->pe_atu_virt_addr + 0x8) = 0x0000FFF0;
	*(volatile unsigned int*)(pe_addr->pe_atu_virt_addr + 0xC) = ((pe_addr->pe_fw_phys_addr & 0x0FFFFFFF) >> 16);
	*(volatile unsigned int*)(pe_addr->pe_atu_virt_addr + 0x10) = 0x02000000;
	*(volatile unsigned int*)(pe_addr->pe_atu_virt_addr + 0x14) = 0x00000000;
	*(volatile unsigned int*)(pe_addr->pe_atu_virt_addr + 0x18) = 0x00000000;
	*(volatile unsigned int*)(pe_addr->pe_atu_virt_addr + 0x1C) = 0x00000000;
	*(volatile unsigned int*)(pe_addr->pe_atu_virt_addr + 0x0) = 0x80000000;
	*(volatile unsigned int*)(pe_addr->pe_atu_virt_addr + 0x0) = 0x00000001;

	//Set PE IMEM 64KB/DMEM 32KB
	*(volatile unsigned int*)(pe_addr->pe_mem_virt_addr) = 0x12;
	//Set PE IMEM 32KB/DMEM 64KB
	//*(volatile unsigned int*)(pe_mem_virt_addr) = 0x14;
	//panic_printk("Set PE IMEM/DMEM value = %x \r\n", *(volatile unsigned int*)(pe_addr->pe_mem_virt_addr));
	
	//Hold CPU
	*(volatile unsigned int*)(pe_addr->pe_cpu_virt_addr + 0xC) |= 0x00100000;
	//panic_printk("Hold CPU, value = %x \r\n", *(volatile unsigned int*)(pe_addr->pe_cpu_virt_addr + 0xC));

	//MEMCPY
	HAL_memset(pe_addr->pe_fw_virt_addr, 0x0 , pe_addr->pe_fw_mem_size);
	HAL_memcpy(pe_addr->pe_fw_virt_addr ,data_wfo_rtl8192cd_start,(data_wfo_rtl8192cd_end - data_wfo_rtl8192cd_start));

	
	//__cpuc_flush_dcache_area((unsigned long)(pe_fw_virt_addr), 0x900000);
	
	//panic_printk("data_wfo_rtl8192cd_start = %x\r\n", data_wfo_rtl8192cd_start);
	//panic_printk("data_wfo_rtl8192cd_end = %x\r\n", data_wfo_rtl8192cd_end);
	//panic_printk("PE FW size = %d\r\n", (data_wfo_rtl8192cd_end - data_wfo_rtl8192cd_start));
	
	//Reset CPU
	*(volatile unsigned int*)(pe_addr->pe_cpu_virt_addr + 0xC) &= ~0x00100000;
	//panic_printk("Reset CPU, value = %x \r\n", *(volatile unsigned int*)(pe_addr->pe_cpu_virt_addr + 0xC));
	panic_printk("download offload firmware\r\n");

}

