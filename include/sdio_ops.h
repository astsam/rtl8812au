/******************************************************************************
 *
 * Copyright(c) 2007 - 2017 Realtek Corporation.
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
#ifndef __SDIO_OPS_H__
#define __SDIO_OPS_H__

/* Follow mac team suggestion, default I/O fail return value is 0xFF */
#define SDIO_ERR_VAL8	0xFF
#define SDIO_ERR_VAL16	0xFFFF
#define SDIO_ERR_VAL32	0xFFFFFFFF

#ifdef PLATFORM_LINUX
#include <sdio_ops_linux.h>
#endif

extern void sdio_set_intf_ops(_adapter *padapter, struct _io_ops *pops);
void dump_sdio_card_info(void *sel, struct dvobj_priv *dvobj);

u32 sdio_init(struct dvobj_priv *dvobj);
void sdio_deinit(struct dvobj_priv *dvobj);
int sdio_alloc_irq(struct dvobj_priv *dvobj);
void sdio_free_irq(struct dvobj_priv *dvobj);

#if 0
extern void sdio_func1cmd52_read(struct intf_hdl *pintfhdl, u32 addr, u32 cnt, u8 *rmem);
extern void sdio_func1cmd52_write(struct intf_hdl *pintfhdl, u32 addr, u32 cnt, u8 *wmem);
#endif
extern u8 SdioLocalCmd52Read1Byte(PADAPTER padapter, u32 addr);
extern void SdioLocalCmd52Write1Byte(PADAPTER padapter, u32 addr, u8 v);
extern s32 _sdio_local_read(PADAPTER padapter, u32 addr, u32 cnt, u8 *pbuf);
extern s32 sdio_local_read(PADAPTER padapter, u32 addr, u32 cnt, u8 *pbuf);
extern s32 _sdio_local_write(PADAPTER padapter, u32 addr, u32 cnt, u8 *pbuf);
extern s32 sdio_local_write(PADAPTER padapter, u32 addr, u32 cnt, u8 *pbuf);

u32 _sdio_read32(PADAPTER padapter, u32 addr);
s32 _sdio_write32(PADAPTER padapter, u32 addr, u32 val);

extern void sd_int_hdl(PADAPTER padapter);
extern u8 CheckIPSStatus(PADAPTER padapter);

#ifdef CONFIG_RTL8821A
extern void InitInterrupt8821AS(PADAPTER padapter);
extern void EnableInterrupt8821AS(PADAPTER padapter);
extern void DisableInterrupt8821AS(PADAPTER padapter);
extern u8 HalQueryTxBufferStatus8821AS(PADAPTER padapter);
extern u8 HalQueryTxOQTBufferStatus8821ASdio(PADAPTER padapter);
#if defined(CONFIG_WOWLAN) || defined(CONFIG_AP_WOWLAN)
void ClearInterrupt8821AS(PADAPTER padapter);
#endif /* defined(CONFIG_WOWLAN) || defined(CONFIG_AP_WOWLAN) */
#endif /* CONFIG_RTL8821A */

/**
 * rtw_sdio_get_block_size() - Get block size of SDIO transfer
 * @d		struct dvobj_priv*
 *
 * The unit of return value is byte.
 */
static inline u32 rtw_sdio_get_block_size(struct dvobj_priv *d)
{
	return d->intf_data.block_transfer_len;
}

/**
 * rtw_sdio_cmd53_align_size() - Align size to one CMD53 could complete
 * @d		struct dvobj_priv*
 * @len		length to align
 *
 * Adjust len to align block size, and the new size could be transfered by one
 * CMD53.
 * If len < block size, it would keep original value, otherwise the value
 * would be rounded up by block size.
 *
 * Return adjusted length.
 */
static inline size_t rtw_sdio_cmd53_align_size(struct dvobj_priv *d, size_t len)
{
	u32 blk_sz;

	blk_sz = rtw_sdio_get_block_size(d);
	if (len <= blk_sz)
		return len;

	return _RND(len, blk_sz);
}

#endif /* !__SDIO_OPS_H__ */
