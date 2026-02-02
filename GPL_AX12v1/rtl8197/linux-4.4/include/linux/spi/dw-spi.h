#ifndef __DW_SPI_H__
#define __DW_SPI_H__

/*
 * Each SPI slave device to work with dw_api controller should
 * has such a structure claiming its working mode (PIO/DMA etc),
 * which can be save in the "controller_data" member of the
 * struct spi_device
 */
struct dw_spi_chip {
	u8 poll_mode;	/* 0 for contoller polling mode */
	u8 type;	/* SPI/SSP/Micrwire */
	u8 enable_dma;
	void (*cs_control)(u32 command);
};

#ifdef CONFIG_SPI_DW_MID_DMA
#include <linux/intel_mid_dma.h>
#include <linux/pci.h>
#define DMA_SLAVE_TYPE intel_mid_dma_slave
#endif

#if defined(CONFIG_SPI_DW_DMA) || defined(CONFIG_SERIAL_8250_DMA)
#include <linux/dw_dmac.h>
#define DMA_SLAVE_TYPE dw_dma_slave
#endif

#if defined(CONFIG_SPI_DW_MID_DMA) || defined(CONFIG_SPI_DW_DMA) || defined(CONFIG_SERIAL_8250_DMA)
struct mid_dma {
	struct DMA_SLAVE_TYPE	dmas_tx;
	struct DMA_SLAVE_TYPE	dmas_rx;
};
#endif

#endif