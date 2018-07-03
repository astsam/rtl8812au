/******************************************************************************
*
* Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved.
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
* You should have received a copy of the GNU General Public License along with
* this program; if not, write to the Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
*
*
******************************************************************************/

/*Image2HeaderVersion: 2.18*/
#if (RTL8821A_SUPPORT == 1)
#ifndef __INC_MP_BB_HW_IMG_8821A_H
#define __INC_MP_BB_HW_IMG_8821A_H


/******************************************************************************
*                           AGC_TAB.TXT
******************************************************************************/

void
odm_read_and_config_mp_8821a_agc_tab(/* TC: Test Chip, MP: MP Chip*/
	struct PHY_DM_STRUCT  *p_dm_odm
);
u32 odm_get_version_mp_8821a_agc_tab(void);

/******************************************************************************
*                           PHY_REG.TXT
******************************************************************************/

void
odm_read_and_config_mp_8821a_phy_reg(/* TC: Test Chip, MP: MP Chip*/
	struct PHY_DM_STRUCT  *p_dm_odm
);
u32 odm_get_version_mp_8821a_phy_reg(void);

/******************************************************************************
*                           PHY_REG_PG.TXT
******************************************************************************/

void
odm_read_and_config_mp_8821a_phy_reg_pg(/* TC: Test Chip, MP: MP Chip*/
	struct PHY_DM_STRUCT  *p_dm_odm
);
u32 odm_get_version_mp_8821a_phy_reg_pg(void);

/******************************************************************************
*                           PHY_REG_PG_E202SA.TXT
******************************************************************************/

void
odm_read_and_config_mp_8821a_phy_reg_pg_e202sa(/* TC: Test Chip, MP: MP Chip*/
	struct PHY_DM_STRUCT  *p_dm_odm
);
u32 odm_get_version_mp_8821a_phy_reg_pg_e202sa(void);

#endif
#endif /* end of HWIMG_SUPPORT*/

