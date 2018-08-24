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

#ifndef	__ODM_RTL8812A_H__
#define __ODM_RTL8812A_H__

#ifdef DYN_ANT_WEIGHTING_SUPPORT
void phydm_dynamic_ant_weighting_8812a(void	*dm_void);
#endif

#if (defined(CONFIG_PATH_DIVERSITY))

void
odm_path_diversity_init_8812a(struct dm_struct	*dm);

void
odm_path_diversity_8812a(struct dm_struct	*dm);

void
odm_set_tx_path_by_tx_info_8812a(
	struct dm_struct		*dm,
	u8			*desc,
	u8			mac_id
);
#endif

void
phydm_hwsetting_8812a(
	struct dm_struct		*dm
);

#endif
