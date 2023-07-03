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
#ifndef __DRV_TYPES_LINUX_H__
#define __DRV_TYPES_LINUX_H__

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 15, 0))
/* Porting from linux kernel v5.15 48eab831ae8b9f7002a533fa4235eed63ea1f1a3 3f6cffb8604b537e3d7ea040d7f4368689638eaf*/
static inline void eth_hw_addr_set(struct net_device *dev, const u8 *addr)
{
    memcpy(dev->dev_addr, addr, ETH_ALEN);
}
#endif

#endif
