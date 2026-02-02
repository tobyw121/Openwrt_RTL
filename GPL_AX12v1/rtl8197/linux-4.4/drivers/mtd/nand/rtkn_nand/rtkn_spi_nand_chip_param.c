#include "rtkn_spi_nand_chip_param.h"
#include <nand_spi/nand_spi_ctrl.h>
#include <nand_spi/nand_spi.h>

#define SECTION_RO

extern struct rtknflash *rtkn;
static int read_dummy_cycle = 1;

#define SPI_NAND_BUS_CLK		(200000000)

SECTION_RO cmd_info_t nand_spi_cmd_info = {
    .w_cmd = NAND_SPI_PROGRAM_LOAD,
    .w_addr_io = SIO_WIDTH,
    .w_data_io = SIO_WIDTH,
    .r_cmd = NAND_SPI_NORMAL_READ,
    .r_addr_io =        SIO_WIDTH,
    .r_data_io = SIO_WIDTH,
    ._wait_spi_nand_ready = nand_spi_wait_spi_nand_ready,
};



/* io function */
static void rtkn_quad_enable(unsigned int chipid)
{
	switch(chipid){
	case GD_RDID_GD5F1GQ4U:
	case MXIC_RDID_MX35LF1GE4AB:
	//case ETRON_RDID_EM73C044SNB:
		nand_spi_default_quad_enable();
		break;
	case WINBOND_RDID_W25N01GV:
		nand_spi_winbond_quad_enable();
		break;
#if 0
	case TOSHIBA_RDID_TC58CVG0S3HxAIx:
		nand_spi_toshiba_quad_enable();
		break;
#endif
	default:
		break;
	}
	return;
}

static void rtkn_set_read_dummy_cycle(unsigned int chipid, enum IO_READ_TYPE type)
{
	if(type != FAST_READ_QIO)
		read_dummy_cycle = 1;
	else{
		switch(chipid){
			case GD_RDID_GD5F1GQ4U:
				read_dummy_cycle = 1;
				break;
			default:
				read_dummy_cycle = 2;
		}
	}
	return;
}

int rtkn_get_read_dummy_cycle(void)
{
	return read_dummy_cycle;
}

static void rtkn_set_ioCfg(unsigned int chipid,enum IO_READ_TYPE read_io,enum IO_WRITE_TYPE write_io)
{
	switch(read_io){
		case READ_NORMAL:
			nand_spi_cmd_info.r_cmd = NAND_SPI_NORMAL_READ;
			nand_spi_cmd_info.r_addr_io = SIO_WIDTH;
			nand_spi_cmd_info.r_data_io = SIO_WIDTH;
			rtkn_set_read_dummy_cycle(chipid,read_io);
			break;
		case FAST_READ_DIO:
			nand_spi_cmd_info.r_cmd = NAND_SPI_FAST_READ_DIO;
			nand_spi_cmd_info.r_addr_io = DIO_WIDTH;
			nand_spi_cmd_info.r_data_io = DIO_WIDTH;
			rtkn_set_read_dummy_cycle(chipid,read_io);
			break;
		case FAST_READ_QIO:
			nand_spi_cmd_info.r_cmd = NAND_SPI_FAST_READ_QIO;
			nand_spi_cmd_info.r_addr_io = QIO_WIDTH;
			nand_spi_cmd_info.r_data_io = QIO_WIDTH;
			rtkn_quad_enable(chipid);
			rtkn_set_read_dummy_cycle(chipid,read_io);
			break;
		case FAST_READ_CACHE2:
			nand_spi_cmd_info.r_cmd = NAND_SPI_FAST_READ_X2;
			nand_spi_cmd_info.r_addr_io = SIO_WIDTH;
			nand_spi_cmd_info.r_data_io = DIO_WIDTH;
			rtkn_set_read_dummy_cycle(chipid,read_io);
			break;
		case FAST_READ_CACHE4:
			nand_spi_cmd_info.r_cmd = NAND_SPI_FAST_READ_X4;
			nand_spi_cmd_info.r_addr_io = SIO_WIDTH;
			nand_spi_cmd_info.r_data_io = QIO_WIDTH;
			rtkn_quad_enable(chipid);
			rtkn_set_read_dummy_cycle(chipid,read_io);
			break;
		default:
			printk("should not happen\n");
	}

	switch(write_io){
		case WRITE_NORMAL:
			nand_spi_cmd_info.w_cmd = NAND_SPI_PROGRAM_LOAD;
			nand_spi_cmd_info.w_addr_io = SIO_WIDTH;
			nand_spi_cmd_info.w_data_io = SIO_WIDTH;
			break;
		case WRITE_PROGRAMME4:
			nand_spi_cmd_info.w_cmd = NAND_SPI_PROGRAM_LOAD_X4;
			nand_spi_cmd_info.w_addr_io = SIO_WIDTH;
			nand_spi_cmd_info.w_data_io = QIO_WIDTH;
			rtkn_quad_enable(chipid);
			break;
		default:
			printk("should not happen\n");
	}

	return;
}
static void rtkn_get_ioCfg(bool dio,bool qio,bool cache_2, bool cache_4,bool programme_4,enum IO_READ_TYPE *read_io,enum IO_WRITE_TYPE *write_io)
{
	*read_io = READ_NORMAL;
	*write_io = WRITE_NORMAL;

	/* dio */
	#ifdef CONFIG_NAND_SPI_USE_DIO	
	if(dio == TRUE){
		*read_io = FAST_READ_DIO;
	}
	#endif

	/* qio */
	#ifdef CONFIG_NAND_SPI_USE_QIO
	if(qio == TRUE){
		*read_io = FAST_READ_QIO;
	}
	#endif

	/* cache *2 */
	#ifdef CONFIG_NAND_SPI_USE_DATA_IOx2
		if(cache_2 == TRUE)
			*read_io = FAST_READ_CACHE2;
	#endif

	/* cache *4 */
	#ifdef CONFIG_NAND_SPI_USE_DATA_IOx4
		if(cache_4 == TRUE)
			*read_io = FAST_READ_CACHE4;
	#endif

	/* programme * 4*/
	#ifdef CONFIG_NAND_SPI_PROGRAM_USE_DATA_IOx4
		if(programme_4 == TRUE)
			*write_io = WRITE_PROGRAMME4;
	#endif

	return;
}

static void rtkn_set_io(unsigned int chipid,bool dio,bool qio,bool cahce_2,bool cache_4,bool programme_4)
{
	enum IO_READ_TYPE read_io;
	enum IO_WRITE_TYPE write_io;
	
	rtkn_get_ioCfg(dio,qio,cahce_2,cache_4,programme_4,&read_io,&write_io);
	rtkn_set_ioCfg(chipid,read_io,write_io);
}
/* io function */

static void rtkn_set_clkdiv(unsigned int maxclk)
{
	/* set clkdiv */
	int clkdiv;

	clkdiv = SPI_NAND_BUS_CLK % maxclk == 0 ? SPI_NAND_BUS_CLK/maxclk: (SPI_NAND_BUS_CLK/maxclk +1);
	clkdiv = clkdiv % 2 == 0 ? clkdiv: (clkdiv/2+1)*2;
	clkdiv = clkdiv/2 -1;

	/*tmp code */
	clkdiv =  1;
	//prom_printf("clkdiv=%d\n",clkdiv);
	REG32(SNFCFR) = (REG32(SNFCFR) & ~(7<<4)) | (clkdiv<<4);

#if 0

	/* pipe lat */
	if(clkdiv == 0){
		if(lat > 1)
			lat = 1;
	}else{
		if(lat > 3)
			lat = 3;
	}
	REG32(SNFCFR) = (REG32(SNFCFR) & ~(0x3<<8)) | (lat <<8);
#endif
	return;
}


/***********************die on ecc **************************/
int rtkn_default_get_dieon_ecc_status(void)
{
	int res;
	res = nand_spi_get_spi_nand_ecc_status();

	if(res == 0x2){	// no data refresh
		return -1;
	}else
		return 0;
}

static void rtkn_set_dieon_ecc(int (*get_dieon_ecc_status)(void),void	(*get_dieon_ecc_info)(void))
{
	rtkn->chip_param.get_dieon_ecc_status = get_dieon_ecc_status;
	rtkn->chip_param.get_dieon_ecc_info = get_dieon_ecc_info;
	nand_spi_enable_on_die_ecc();
	return;
}

/* toshiba 8bit die on ecc */
void rtkn_toshiba8_get_dieon_ecc_info(void)
{
	unsigned int res;
	
	 /* ECC_E */
    res = nand_spi_get_feature_register(0xB0);
    printk("Get Feature: address 0x%x, value=%x\n",0xB0,res);
    if(res & (1<<4)){
		printk("die on ecc enable\n");
    }

    /* ECCS1/ECCS0 */
    res = nand_spi_get_feature_register(0xC0);
    printk("Get Feature: address 0x%x, value=%x\n",0xC0,res);
    printk("last page: ECCS1=%d,ECCS0=%d\n",((res &(1<<5))>>5),((res & (1<<4)) >>4));

    res  = nand_spi_get_feature_register(0x10);
    printk("Get Feature: address 0x%x, value=%x\n",0x10,res);

    res  = nand_spi_get_feature_register(0x20);
    printk("Get Feature: address 0x%x, value=%x\n",0x20,res);

    res  = nand_spi_get_feature_register(0x40);
    printk("Get Feature: address 0x%x, value=%x\n",0x40,res);

    res  = nand_spi_get_feature_register(0x50);
    printk("Get Feature: address 0x%x, value=%x\n",0x50,res);

	return;
}
/***********************die on ecc **************************/


static void rtkn_set_chip_size(unsigned int pagesize,unsigned int oobsize, unsigned int page_per_block, unsigned int blocknum)
{
	rtkn->chip_param.pagesize = pagesize;
	rtkn->chip_param.oobsize = oobsize;
	rtkn->chip_param.num_chunk_per_block = page_per_block;
	rtkn->chip_param.num_block = blocknum;
	return;
}

int rtkn_set_chip_param(unsigned int chipid)
{
	int i;
	unsigned int real_chipid;

	for(i = 0; i< sizeof(nand_chip_id)/sizeof(nand_chip_param_T);i++){		
		real_chipid = chipid;
		real_chipid = real_chipid >> ((4-nand_chip_id[i].id_len)*8);
		
		if(nand_chip_id[i].id == real_chipid){
			/* rtkn param */
			rtkn->chip_param.id = nand_chip_id[i].id;
			rtkn->chip_param.isLastPage = nand_chip_id[i].isLastPage;
			nand_spi_set_buffer_mode();
        	nand_spi_block_unprotect();

			/* die on ecc */
			if( nand_chip_id[i].enable_dieon_ecc == TRUE){
				rtkn_set_dieon_ecc(nand_chip_id[i].get_dieon_ecc_status,nand_chip_id[i].get_dieon_ecc_info);
				rtkn->chip_param.enable_dieon_ecc = TRUE;
			}else{
				nand_spi_disable_on_die_ecc();
				rtkn->chip_param.enable_dieon_ecc = FALSE;
			}
			rtkn->chip_param.romcode_read_dieon_ecc = nand_chip_id[i].romcode_read_dieon_ecc;
			rtkn->chip_param.romcode_write_dieon_ecc = nand_chip_id[i].romcode_write_dieon_ecc;
			
			rtkn_set_chip_size(nand_chip_id[i].pagesize,nand_chip_id[i].oobsize,nand_chip_id[i].num_chunk_per_block,nand_chip_id[i].num_block);
			rtkn_set_clkdiv(nand_chip_id[i].maxclk);
			rtkn_set_io(nand_chip_id[i].id,nand_chip_id[i].support_dio,nand_chip_id[i].support_qio,
							nand_chip_id[i].support_read_cache_2,nand_chip_id[i].support_read_cache_4,nand_chip_id[i].support_programme_cache_4);
			return 0;
		}
	}

	return -1;
}

#ifdef SPI_NAND_IO_TEST
static int rtkn_test_check(int index,enum IO_READ_TYPE read_type,enum IO_WRITE_TYPE write_type)
{
	if((read_type == FAST_READ_CACHE2 && nand_chip_id[index].support_read_cache_2 == false)
		|| (read_type == FAST_READ_CACHE4 && nand_chip_id[index].support_read_cache_4 == false)
		|| (read_type == FAST_READ_DIO && nand_chip_id[index].support_dio == false)
		|| (read_type == FAST_READ_QIO && nand_chip_id[index].support_qio == false)
		|| (write_type == WRITE_PROGRAMME4&& nand_chip_id[index].support_programme_cache_4== false)
		)
		return -1;

	if(write_type == WRITE_PROGRAMME4 && (read_type != FAST_READ_QIO && read_type != FAST_READ_CACHE4))
		return -1;

	return 0;
}

int rtkn_nand_iotest(enum IO_READ_TYPE read_type,enum IO_WRITE_TYPE write_type)
{
	int i;
	unsigned int real_chipid;
	unsigned int read_id;
	read_id = nand_spi_read_id();
		
	for(i = 0; i< sizeof(nand_chip_id)/sizeof(nand_chip_param_T);i++){		
		real_chipid = read_id ;
		real_chipid = real_chipid >> ((4-nand_chip_id[i].id_len)*8);
		if(nand_chip_id[i].id == real_chipid){
			break;
		}
	}

	if(i == sizeof(nand_chip_id)/sizeof(nand_chip_param_T)){
		printk("%s:%d,CHIPID=%x,cannot find chip\n",__func__,__LINE__,real_chipid);
		return -1;
	}

	if(rtkn_test_check(i,read_type,write_type) != 0){
		printk("/**********chipid = 0x%x, read_type=%d, write_type=%d not support*************/\n",real_chipid,read_type,write_type);
		return -1;
	}
	printk("/**********chipid = 0x%x, read_type=%d, write_type=%d start*************/\n",real_chipid,read_type,write_type);
	rtkn_set_ioCfg(real_chipid,read_type,write_type);
	return 0;

}
#endif
