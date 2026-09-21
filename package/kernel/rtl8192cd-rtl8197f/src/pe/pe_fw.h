#ifndef _PE_FW_H_
#define _PE_FW_H_

struct pe_addr_base
{
	unsigned long pe_reserved_virt_addr;
	unsigned long pe_reserved_phys_addr;
	unsigned long pe_reserved_mem_size;
	unsigned long pe_fw_virt_addr;
	unsigned long pe_fw_virt_addr_lomem;
 	unsigned long pe_fw_phys_addr;
	unsigned long pe_fw_mem_size;
 	unsigned long pe_cpu_virt_addr; 
 	unsigned long pe_atu_virt_addr;
 	unsigned long pe_mem_virt_addr;
	unsigned long pe_sram_virt_addr;
 	unsigned long pe_sram_phys_addr;
	unsigned long pe_sram_mem_size;
};

struct pe_addr_base* get_pe_addr_base(void);
void download_pe_fw(void);
#endif