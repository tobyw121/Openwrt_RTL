/******************************************************************************
 *
 * Copyright(c) 2007 - 2019 Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 *****************************************************************************/
#define _GSPI_OPS_LINUX_C_

#include <drv_types.h>

int spi_send_msg(_adapter *adapter, struct spi_transfer xfers[], u32 IoAction)
{
	struct dvobj_priv *psddev;
	struct spi_device *spi;
	struct spi_message msg;
	int ret = 1;

	if (adapter == NULL) {
		RTW_ERR("%s: padapter is NULL!\n", __func__);
		return 1;
	}

	psddev = adapter_to_dvobj(adapter);
	spi = dvobj_to_gspi(psddev)->func;

	spi_message_init(&msg);
	spi_message_add_tail(&xfers[0], &msg);
	spi_message_add_tail(&xfers[1], &msg);
	spi_message_add_tail(&xfers[2], &msg);
	ret = spi_sync(spi, &msg);
	if (ret)
		RTW_INFO("%s: FAIL!\n", __func__);

	return ret;
}

int addr_convert(u32 addr)
{
	u32 domain_id = 0 ;
	u32 temp_addr = addr & 0xffff0000;

	if (temp_addr == 0) {
		domain_id = WLAN_IOREG_DOMAIN;
		return domain_id;
	}

	switch (temp_addr) {
	case SPI_LOCAL_OFFSET:
		domain_id = SPI_LOCAL_DOMAIN;
		break;
	case WLAN_IOREG_OFFSET:
		domain_id = WLAN_IOREG_DOMAIN;
		break;
	case FW_FIFO_OFFSET:
		domain_id = FW_FIFO_DOMAIN;
		break;
	case TX_HIQ_OFFSET:
		domain_id = TX_HIQ_DOMAIN;
		break;
	case TX_MIQ_OFFSET:
		domain_id = TX_MIQ_DOMAIN;
		break;
	case TX_LOQ_OFFSET:
		domain_id = TX_LOQ_DOMAIN;
		break;
	case RX_RXOFF_OFFSET:
		domain_id = RX_RXFIFO_DOMAIN;
		break;
	default:
		break;
	}
	/* sys_mib.Spi_Transation_record.domain_id =domain_id; */
	return domain_id;
}

static u32 buf_endian_reverse(u32 src)
{
	return ((src & 0x000000ff) << 24) | ((src & 0x0000ff00) << 8) |
		((src & 0x00ff0000) >> 8) | ((src & 0xff000000) >> 24);
}

void spi_get_status_info(_adapter *adapter, unsigned char *status)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(dvobj);

	pHalData->SdioTxFIFOFreePage[PUBLIC_QUEUE_IDX] = GET_STATUS_PUB_PAGE_NUM(status);
	pHalData->SdioTxFIFOFreePage[HI_QUEUE_IDX] = GET_STATUS_HI_PAGE_NUM(status);
	pHalData->SdioTxFIFOFreePage[MID_QUEUE_IDX] = GET_STATUS_MID_PAGE_NUM(status);
	pHalData->SdioTxFIFOFreePage[LOW_QUEUE_IDX] = GET_STATUS_LOW_PAGE_NUM(status);

	/* RTW_INFO("%s: Free page for HIQ(%#x),MIDQ(%#x),LOWQ(%#x),PUBQ(%#x)\n", */
	/*		__FUNCTION__, */
	/*		pHalData->SdioTxFIFOFreePage[HI_QUEUE_IDX], */
	/*		pHalData->SdioTxFIFOFreePage[MID_QUEUE_IDX], */
	/*		pHalData->SdioTxFIFOFreePage[LOW_QUEUE_IDX], */
	/*		pHalData->SdioTxFIFOFreePage[PUBLIC_QUEUE_IDX]); */
}

#ifndef CONFIG_PLATFORM_ARM_RK3066
int spi_read_write_reg(_adapter *adapter, int  write_flag, u32 addr, char *buf, int len, u32 eddien)
{
	int  fun = 1, domain_id = 0x0; /* LOCAL */
	unsigned int cmd = 0 ;
	int byte_en = 0 ;/* ,i = 0 ; */
	int ret = 0;
	unsigned char status[8] = {0};
	unsigned int data_tmp = 0;
	/* u32 force_bigendian = !eddien; */
	u32 force_bigendian = eddien;

	if (len != 1 && len != 2 && len != 4)
		return -1;

	domain_id = addr_convert(addr);

	addr &= 0x7fff;
	len &= 0xff;
	if (write_flag) { /* write register */
		int remainder = addr % 4;
		u32 val32 = *(u32 *)buf;
		switch (len) {
		case 1:
			byte_en = (0x1 << remainder);
			data_tmp = (val32 & 0xff) << (remainder * 8);
			break;
		case 2:
			byte_en = (0x3 << remainder);
			data_tmp = (val32 & 0xffff) << (remainder * 8);
			break;
		case 4:
			byte_en = 0xf;
			data_tmp = val32 & 0xffffffff;
			break;
		default:
			byte_en = 0xf;
			data_tmp = val32 & 0xffffffff;
			break;
		}
	} else { /* read register */
		switch (len) {
		case 1:
			byte_en = 0x1;
			break;
		case 2:
			byte_en = 0x3;
			break;
		case 4:
			byte_en = 0xf;
			break;
		default:
			byte_en = 0xf;
			break;
		}
	}

	/* addr = 0xF0 4byte: 0x2800f00f */
	REG_LEN_FORMAT(&cmd, byte_en);
	REG_ADDR_FORMAT(&cmd, (addr & 0xfffffffc));
	REG_DOMAIN_ID_FORMAT(&cmd, domain_id);
	REG_FUN_FORMAT(&cmd, fun);
	REG_RW_FORMAT(&cmd, write_flag);

	/* RTW_INFO("spi_read_write_reg cmd1: %x, data_tmp is %x\n",cmd, data_tmp); */

	if (force_bigendian)
		cmd = buf_endian_reverse(cmd);

	/* io is one by one, so we do not need fwps_lock here */
	/* rtw_spin_lock(&padapter->halpriv.fwps_lock); */
	/* padapter->io_fifo_processing = _TRUE; */
	if (!write_flag && (domain_id != RX_RXFIFO_DOMAIN)) {
		u32 read_data = 0;
		struct spi_transfer xfers[3];
		_rtw_memset(xfers, 0x00, 3 * sizeof(struct spi_transfer));
		_rtw_memset(buf, 0x0, len);

		xfers[0].tx_buf = &cmd;
		xfers[0].len = 4;

		xfers[1].rx_buf = status;
		xfers[1].len = 8;

		xfers[2].rx_buf = &read_data;
		xfers[2].len = 4;

		/* RTW_INFO("spi_read_write_reg: read_data is %x\n", read_data); */
		ret = spi_send_msg(adapter, xfers, 0);
		if (ret) {
			RTW_ERR("%s: FAIL!(%d) addr=0x%05x\n", __func__, ret, addr);
			read_data = 0;
			_rtw_memset(status, 0, 8);
		}

		/* RTW_INFO("spi_read_write_reg: read_data is %x\n", read_data); */
		read_data = ReadLE4Byte(read_data);
		/* add for 8810 */
#ifdef CONFIG_BIG_ENDIAN
		if (!force_bigendian)
			read_data = buf_endian_reverse(read_data);
#else
		if (force_bigendian)
			read_data = buf_endian_reverse(read_data);
#endif
		*(u32 *)buf = read_data;
		/* RTW_INFO("spi_read_write_reg: read: buf is %x %x %x %x\n", buf[0], buf[1], buf[2], buf[3]); */
	} else if (write_flag) {
		struct spi_transfer xfers[3];
		_rtw_memset(xfers, 0x00, 3 * sizeof(struct spi_transfer));

		xfers[0].tx_buf = &cmd;
		xfers[0].len = 4;

		xfers[1].tx_buf = &data_tmp;
		xfers[1].len = 4;

		xfers[2].rx_buf = status;
		xfers[2].len = 8;

		/* RTW_INFO("spi_read_write_reg data_tmp 111: %x\n",data_tmp); */
#ifdef CONFIG_BIG_ENDIAN
		if (!force_bigendian)
			data_tmp = buf_endian_reverse(data_tmp);
#else
		if (force_bigendian)
			data_tmp = buf_endian_reverse(data_tmp);
#endif
		ret = spi_send_msg(adapter, xfers, 0);
		if (ret) {
			RTW_ERR("%s: FAIL!(%d) addr=0x%05x\n", __func__, ret, addr);
			_rtw_memset(status, 0, 8);
		}
	}

	/* padapter->io_fifo_processing = _FALSE; */

	spi_get_status_info(adapter, (unsigned char *)status);

	return ret;
}

#else


int spi_read_write_reg(_adapter *adapter, int  write_flag, u32 addr, char *buf, int len, u32 eddien)
{
	int  fun = 1, domain_id = 0x0; /* LOCAL */
	unsigned int cmd = 0 ;
	int byte_en = 0 ;/* ,i = 0 ; */
	int ret = 0;
	unsigned char status[12] = {0};
	unsigned int data_tmp = 0;
	/* u32 force_bigendian = !eddien; */
	u32 force_bigendian = eddien;
	struct dvobj_priv *psddev;
	struct spi_device *spi;
	PGSPI_DATA spi_data;

	psddev = adapter_to_dvobj(adapter);
	spi_data = dvobj_to_gspi(psddev);
	spi = spi_data->func;

	if (len != 1 && len != 2 && len != 4)
		return -1;

	domain_id = addr_convert(addr);

	addr &= 0x7fff;
	len &= 0xff;
	if (write_flag) { /* write register */
		int remainder = addr % 4;
		u32 val32 = *(u32 *)buf;
		switch (len) {
		case 1:
			byte_en = (0x1 << remainder);
			data_tmp = (val32 & 0xff) << (remainder * 8);
			break;
		case 2:
			byte_en = (0x3 << remainder);
			data_tmp = (val32 & 0xffff) << (remainder * 8);
			break;
		case 4:
			byte_en = 0xf;
			data_tmp = val32 & 0xffffffff;
			break;
		default:
			byte_en = 0xf;
			data_tmp = val32 & 0xffffffff;
			break;
		}
	} else { /* read register */
		switch (len) {
		case 1:
			byte_en = 0x1;
			break;
		case 2:
			byte_en = 0x3;
			break;
		case 4:
			byte_en = 0xf;
			break;
		default:
			byte_en = 0xf;
			break;
		}
	}

	/* addr = 0xF0 4byte: 0x2800f00f */
	REG_LEN_FORMAT(&cmd, byte_en);
	REG_ADDR_FORMAT(&cmd, (addr & 0xfffffffc));
	REG_DOMAIN_ID_FORMAT(&cmd, domain_id);
	REG_FUN_FORMAT(&cmd, fun);
	REG_RW_FORMAT(&cmd, write_flag);

	/* RTW_INFO("spi_read_write_reg cmd1: %x, data_tmp is %x\n",cmd, data_tmp); */

	if (force_bigendian)
		cmd = buf_endian_reverse(cmd);

	/* io is one by one, so we do not need fwps_lock here */
	/* rtw_spin_lock(&padapter->halpriv.fwps_lock); */
	/* padapter->io_fifo_processing = _TRUE; */
	if (!write_flag && (domain_id != RX_RXFIFO_DOMAIN)) {
		u32 read_data = 0;
		struct spi_transfer xfers[3];
		_rtw_memset(xfers, 0x00, 3 * sizeof(struct spi_transfer));
		_rtw_memset(buf, 0x0, len);

		xfers[0].tx_buf = &cmd;
		xfers[0].len = 4;

		xfers[1].rx_buf = status;
		xfers[1].len = 8;

		xfers[2].rx_buf = &read_data;
		xfers[2].len = 4;
		/* spi->bits_per_word=8; */
		spi_write_then_read(spi, &cmd, 4, status, 12);
		_rtw_memcpy(&read_data, &status[8], 4);

		/* RTW_INFO("spi_read_write_reg: read_data is %x\n", read_data); */
		/*	ret = spi_send_msg(adapter, xfers, 0); */
		if (ret) {
			RTW_ERR("%s: FAIL!(%d) addr=0x%05x\n", __func__, ret, addr);
			read_data = 0;
			_rtw_memset(status, 0, 8);
		}

		/* RTW_INFO("spi_read_write_reg: read_data is %x\n", read_data); */
		read_data = ReadLE4Byte(read_data);
		/* add for 8810 */
#ifdef CONFIG_BIG_ENDIAN
		if (!force_bigendian)
			read_data = buf_endian_reverse(read_data);
#else
		if (force_bigendian)
			read_data = buf_endian_reverse(read_data);
#endif
		*(u32 *)buf = read_data;
		/* RTW_INFO("spi_read_write_reg: read: buf is %x %x %x %x\n", buf[0], buf[1], buf[2], buf[3]); */
	} else if (write_flag) {
		unsigned char tmp_buf[8];
		struct spi_transfer xfers[3];
		_rtw_memset(xfers, 0x00, 3 * sizeof(struct spi_transfer));

		xfers[0].tx_buf = &cmd;
		xfers[0].len = 4;

		xfers[1].tx_buf = &data_tmp;
		xfers[1].len = 4;

		xfers[2].rx_buf = status;
		xfers[2].len = 8;

		/* RTW_INFO("spi_read_write_reg data_tmp 111: %x\n",data_tmp); */
#ifdef CONFIG_BIG_ENDIAN
		if (!force_bigendian)
			data_tmp = buf_endian_reverse(data_tmp);
#else
		if (force_bigendian)
			data_tmp = buf_endian_reverse(data_tmp);
#endif

		_rtw_memcpy(tmp_buf, &cmd, 4);
		_rtw_memcpy(&tmp_buf[4], &data_tmp, 4);

		/* spi->bits_per_word=8; */
		spi_write_then_read(spi, tmp_buf, 8, status, 8);
		/* ret = spi_send_msg(adapter, xfers, 0); */
		if (ret) {
			RTW_ERR("%s: FAIL!(%d) addr=0x%05x\n", __func__, ret, addr);
			_rtw_memset(status, 0, 8);
		}
	}

	/* padapter->io_fifo_processing = _FALSE; */

	spi_get_status_info(adapter, (unsigned char *)status);

	return ret;
}

#endif

u8 spi_read8(_adapter *adapter, unsigned int addr, s32 *err)
{
	unsigned int ret = 0;
	int val32 = 0 , remainder = 0 ;
	s32 _err = 0;

	_err = spi_read_write_reg(adapter, 0, addr & 0xFFFFFFFC, (char *)&ret, 4, 0);
	remainder = addr % 4;
	val32 = ret;
	val32 = (val32 & (0xff << (remainder << 3))) >> (remainder << 3);

	if (err)
		*err = _err;

	return (u8)val32;

}

u16 spi_read16(_adapter *adapter, u32 addr, s32 *err)
{
	unsigned int ret = 0;
	int val32 = 0 , remainder = 0 ;
	s32 _err = 0;

	_err = spi_read_write_reg(adapter, 0, addr & 0xFFFFFFFC, (char *)&ret, 4, 0);
	remainder = addr % 4;
	val32 = ret;
	val32 = (val32 & (0xffff << (remainder << 3))) >> (remainder << 3);

	if (err)
		*err = _err;

	return (u16)val32;
}

u32 spi_read32(_adapter *adapter, u32 addr, s32 *err)
{
	unsigned int ret = 0;
	s32 _err = 0;

	_err = spi_read_write_reg(adapter, 0, addr & 0xFFFFFFFC, (char *)&ret, 4, 0);
	if (err)
		*err = _err;

	return  ret;
}

void spi_write8(_adapter *adapter, u32 addr, u8 buf, s32 *err)
{
	int ret = 0;

	ret = spi_read_write_reg(adapter, 1, addr, (char *)&buf, 1, 0);
	if (err)
		*err = ret;
}

void spi_write16(_adapter *adapter, u32 addr, u16 buf, s32 *err)
{
	int ret = 0;

	ret = spi_read_write_reg(adapter, 1, addr, (char *)&buf, 2, 0);
	if (err)
		*err = ret;
}

void spi_write32(_adapter *adapter, u32 addr, u32 buf, s32 *err)
{
	int	ret = 0;

	ret = spi_read_write_reg(adapter, 1, addr, (char *)&buf, 4, 0);
	if (err)
		*err = ret;
}

unsigned int spi_write8_endian(_adapter *adapter, unsigned int addr, unsigned int buf, u32 big)
{
	int ret = 0;

	ret = spi_read_write_reg(adapter, 1, addr, (char *)&buf, 1, big);
	return ret;
}
#ifndef CONFIG_PLATFORM_ARM_RK3066
void spi_write_tx_fifo(_adapter *adapter, u8 *buf, int len, u32 fifo)
{
	int fun = 1; /* TX_HIQ_FIFO */
	unsigned int cmd = 0;
	unsigned char status[8];
	u8 more_data = 0;
	int ret = 0;

	struct spi_transfer xfers[3];
	_rtw_memset(xfers, 0x00, 3 * sizeof(struct spi_transfer));

	xfers[0].tx_buf = &cmd;
	xfers[0].len = 4;

	xfers[1].tx_buf = buf;
	xfers[1].len = len;/* len/4; */

	xfers[2].rx_buf = status;
	xfers[2].len = 8;


	FIFO_LEN_FORMAT(&cmd, len);               /* TX Agg len */
	FIFO_DOMAIN_ID_FORMAT(&cmd, fifo);
	FIFO_FUN_FORMAT(&cmd, fun);
	FIFO_RW_FORMAT(&cmd, (unsigned int)1);  /* write */

	_rtw_memset(status, 0x00, 8);

	ret = spi_send_msg(adapter, xfers, 1);
	if (ret) {
		RTW_INFO("%s: FAIL!(%d)\n", __func__, ret);
		_rtw_memset(status, 0, 8);
	}

	spi_get_status_info(adapter, status);

	more_data = GET_STATUS_HISR_LOW8BIT(status) & BIT(0);
	if (more_data) {
		/*	rtw_queue_delayed_work(adapter->recv_wq, &adapter->recv_work, 0, (void*)adapter); */
		struct dvobj_priv *dvobj = adapter->dvobj;
		PGSPI_DATA pgspi_data = dvobj_to_gspi(dvobj);

		if (pgspi_data->priv_wq)
			queue_delayed_work(pgspi_data->priv_wq, &pgspi_data->irq_work, 0);
	}


	return;
}

#else
void spi_write_tx_fifo(_adapter *adapter, u8 *buf, int len, u32 fifo)
{
	int fun = 1; /* TX_HIQ_FIFO */
	unsigned int cmd = 0;
	unsigned char status[8];
	unsigned char *tx_buf;
	u8 more_data = 0;
	int ret = 0;
	struct dvobj_priv *psddev;
	struct spi_device *spi;
	int error = 0;

	psddev = adapter_to_dvobj(adapter);
	spi = dvobj_to_gspi(psddev)->func;

	tx_buf = rtw_malloc(len + 4);
	if (!tx_buf)
		RTW_INFO("%s: malloc FAIL!\n", __func__);


	FIFO_LEN_FORMAT(&cmd, len);               /* TX Agg len */
	FIFO_DOMAIN_ID_FORMAT(&cmd, fifo);
	FIFO_FUN_FORMAT(&cmd, fun);
	FIFO_RW_FORMAT(&cmd, (unsigned int)1);  /* write */

	_rtw_memcpy(tx_buf, &cmd, 4);
	_rtw_memcpy(tx_buf + 4, buf, len);

	_rtw_memset(status, 0x00, 8);

	ret = spi_write_then_read(spi, tx_buf, len + 4, status, 8);

	rtw_mfree(tx_buf, len + 4);
	if (ret) {
		RTW_INFO("%s: FAIL!(%d)\n", __func__, ret);
		_rtw_memset(status, 0, 8);
	}

	spi_get_status_info(adapter, status);

	more_data = GET_STATUS_HISR_LOW8BIT(status) & BIT(0);
	/*	if(more_data) { */
	/*	rtw_queue_delayed_work(adapter->recv_wq, &adapter->recv_work, 0, (void*)adapter);
	*	struct dvobj_priv *dvobj= adapter->dvobj;
	*		PGSPI_DATA pgspi_data = dvobj_to_gspi(dvobj); */
	/*	if (pgspi_data->priv_wq) */
	/*		queue_delayed_work(pgspi_data->priv_wq, &pgspi_data->irq_work, 0);
	*	} */


	return;
}
#endif



#ifndef CONFIG_PLATFORM_ARM_RK3066
int spi_read_rx_fifo(_adapter *adapter, unsigned char *buf, int len, struct spi_more_data *pmore_data)
{
	int fun = 1, domain_id = 0x1f; /* RX_FIFO */
	unsigned int cmd = 0;
	unsigned char status[8];
	int ret = 0;
	struct spi_transfer xfers[3];

	_rtw_memset(xfers, 0x00, 3 * sizeof(struct spi_transfer));

	xfers[0].tx_buf = &cmd;
	xfers[0].len = 4;

	xfers[1].rx_buf = buf;
	xfers[1].len = len;

	xfers[2].rx_buf = status;
	xfers[2].len = 8;

	FIFO_LEN_FORMAT(&cmd, len);               /* TX Agg len */
	FIFO_DOMAIN_ID_FORMAT(&cmd, domain_id);
	FIFO_FUN_FORMAT(&cmd, fun);
	FIFO_RW_FORMAT(&cmd, 0); /* read */

	_rtw_memset(status, 0x00, 8);
	_rtw_memset(buf, 0x0, len);

	ret = spi_send_msg(adapter, xfers, 1);
	if (ret) {
		RTW_ERR("%s: FAIL!(%d)\n", __func__, ret);
		_rtw_memset(status, 0x00, 8);
		_rtw_memset(buf, 0x0, len);
		return _FAIL;
	}

	spi_get_status_info(adapter, (unsigned char *)status);
	pmore_data->more_data = GET_STATUS_HISR_LOW8BIT(status) & BIT(0);
	pmore_data->len = GET_STATUS_RX_LENGTH(status);

	return _SUCCESS;
}

#else
int spi_read_rx_fifo(_adapter *adapter, unsigned char *buf, int len, struct spi_more_data *pmore_data)
{
	int fun = 1, domain_id = 0x1f; /* RX_FIFO */
	unsigned int cmd = 0;
	unsigned char status[8];
	unsigned char *rx_buf;
	int ret = 0;
	struct dvobj_priv *psddev;
	struct spi_device *spi;
	int error = 0;
	static unsigned int last_len = 0;
	static unsigned char common = 0;
	psddev = adapter_to_dvobj(adapter);
	spi = dvobj_to_gspi(psddev)->func;

	rx_buf = rtw_malloc(len + 8);
	if (!rx_buf) {
		RTW_ERR("%s: malloc FAIL!\n", __func__);
		return _FAIL;
	}

	FIFO_LEN_FORMAT(&cmd, len);               /* TX Agg len */
	FIFO_DOMAIN_ID_FORMAT(&cmd, domain_id);
	FIFO_FUN_FORMAT(&cmd, fun);
	FIFO_RW_FORMAT(&cmd, 0); /* read */

	_rtw_memset(status, 0x00, 8);
	_rtw_memset(rx_buf, 0x0, len + 8);

	ret = spi_write_then_read(spi, &cmd, 4, rx_buf, len + 8);
	_rtw_memcpy(buf, rx_buf, len);
	_rtw_memcpy(status, rx_buf + len, 8);
	rtw_mfree(rx_buf, len + 8);
	if (ret) {
		RTW_ERR("%s: FAIL!(%d)\n", __func__, ret);
		_rtw_memset(status, 0x00, 8);
		_rtw_memset(buf, 0x0, len);
		return _FAIL;
	}

	spi_get_status_info(adapter, (unsigned char *)status);
	pmore_data->more_data = GET_STATUS_HISR_LOW8BIT(status) & BIT(0);
	pmore_data->len = GET_STATUS_RX_LENGTH(status);

	return _SUCCESS;
}
#endif
