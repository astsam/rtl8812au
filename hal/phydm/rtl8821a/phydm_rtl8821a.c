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

/* ************************************************************
 * include files
 * ************************************************************ */

#include "mp_precomp.h"

#include "../phydm_precomp.h"

#if (RTL8821A_SUPPORT == 1)

void odm_dynamic_try_state_agg_8821a(
	struct PHY_DM_STRUCT		*p_dm_odm
	)
{
	if ((p_dm_odm->support_ic_type & ODM_RTL8821) && (p_dm_odm->support_interface == ODM_ITRF_USB)) {
		if(p_dm_odm->rssi_min > 25)
			odm_write_1byte(p_dm_odm, 0x4CF, 0x02);
		else if(p_dm_odm->rssi_min < 20)
			odm_write_1byte(p_dm_odm, 0x4CF, 0x00);
	}
}

void odm_dynamic_packet_detection_th_8821a(
	struct PHY_DM_STRUCT	*p_dm_odm
	)
{
	if (p_dm_odm->support_ic_type & ODM_RTL8821) {
		if (p_dm_odm->rssi_min <= 25) {
			odm_set_bb_reg(p_dm_odm, rPwed_TH_Jaguar, bMaskDWord, 0x2aaaf1a8);
			odm_set_bb_reg(p_dm_odm, rBWIndication_Jaguar, BIT26, 1);
		} else if (p_dm_odm->rssi_min >= 30) {
			odm_set_bb_reg(p_dm_odm, rPwed_TH_Jaguar, bMaskDWord, 0x2aaaeec8);
			odm_set_bb_reg(p_dm_odm, rBWIndication_Jaguar, BIT26, 0);
		}
	}
}

void odm_hw_setting_8821a(
	struct PHY_DM_STRUCT	*p_dm_odm
	)
{
	odm_dynamic_try_state_agg_8821a(p_dm_odm);
	odm_dynamic_packet_detection_th_8821a(p_dm_odm);
}

#endif //#if (RTL8821A_SUPPORT == 1)

