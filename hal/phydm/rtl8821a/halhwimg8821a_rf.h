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
#ifndef __INC_MP_RF_HW_IMG_8821A_H
#define __INC_MP_RF_HW_IMG_8821A_H


/******************************************************************************
*                           RadioA.TXT
******************************************************************************/

void
odm_read_and_config_mp_8821a_radioa(/* TC: Test Chip, MP: MP Chip*/
	struct PHY_DM_STRUCT  *p_dm_odm
);
u32 odm_get_version_mp_8821a_radioa(void);

/******************************************************************************
*                           RadioB.TXT
******************************************************************************/

void
odm_read_and_config_mp_8821a_radiob(/* TC: Test Chip, MP: MP Chip*/
	struct PHY_DM_STRUCT  *p_dm_odm
);
u32 odm_get_version_mp_8821a_radiob(void);

/******************************************************************************
*                           TxPowerTrack_PCIE.TXT
******************************************************************************/

void
odm_read_and_config_mp_8821a_txpowertrack_pcie(/* TC: Test Chip, MP: MP Chip*/
	struct PHY_DM_STRUCT  *p_dm_odm
);
u32 odm_get_version_mp_8821a_txpowertrack_pcie(void);

/******************************************************************************
*                           TxPowerTrack_SDIO.TXT
******************************************************************************/

void
odm_read_and_config_mp_8821a_txpowertrack_sdio(/* TC: Test Chip, MP: MP Chip*/
	struct PHY_DM_STRUCT  *p_dm_odm
);
u32 odm_get_version_mp_8821a_txpower_track_sdio(void);

/******************************************************************************
*                           TxPowerTrack_USB.TXT
******************************************************************************/

void
odm_read_and_config_mp_8821a_txpowertrack_usb(/* TC: Test Chip, MP: MP Chip*/
	struct PHY_DM_STRUCT  *p_dm_odm
);
u32 odm_get_version_mp_8821a_txpower_track_usb(void);

/******************************************************************************
*                           TXPWR_LMT_8811AU_FEM.TXT
******************************************************************************/

void
odm_read_and_config_mp_8821a_txpwr_lmt_8811a_u_fem(/* TC: Test Chip, MP: MP Chip*/
	struct PHY_DM_STRUCT  *p_dm_odm
);
u32 odm_get_version_mp_8821a_txpwr_lmt_8811a_u_fem(void);

/******************************************************************************
*                           TXPWR_LMT_8811AU_IPA.TXT
******************************************************************************/

void
odm_read_and_config_mp_8821a_txpwr_lmt_8811a_u_ipa(/* TC: Test Chip, MP: MP Chip*/
	struct PHY_DM_STRUCT  *p_dm_odm
);
u32 odm_get_version_mp_8821a_txpwr_lmt_8811a_u_ipa(void);

/******************************************************************************
*                           TXPWR_LMT_8821A.TXT
******************************************************************************/

void
odm_read_and_config_mp_8821a_txpwr_lmt_8821a(/* TC: Test Chip, MP: MP Chip*/
	struct PHY_DM_STRUCT  *p_dm_odm
);
u32 odm_get_version_mp_8821a_txpwr_lmt_8821a(void);

/******************************************************************************
*                           TXPWR_LMT_8821A_E202SA.TXT
******************************************************************************/

void
odm_read_and_config_mp_8821a_txpwr_lmt_8821A_e202sa(/* TC: Test Chip, MP: MP Chip*/
	struct PHY_DM_STRUCT  *p_dm_odm
);
u32 odm_get_version_mp_8821a_txpwr_lmt_8821a_e202sa(void);

/******************************************************************************
*                           TXPWR_LMT_8821A_SAR_13dBm.TXT
******************************************************************************/

void
odm_read_and_config_mp_8821a_txpwr_lmt_8821a_sar_13dBm(/* TC: Test Chip, MP: MP Chip*/
	struct PHY_DM_STRUCT  *p_dm_odm
);
u32 odm_get_version_mp_8821a_txpwr_lmt_8821a_sar_13dBm(void);

/******************************************************************************
*                           TXPWR_LMT_8821A_SAR_5mm.TXT
******************************************************************************/

void
odm_read_and_config_mp_8821A_txpwr_lmt_8821a_sar_5mm(/* TC: Test Chip, MP: MP Chip*/
	struct PHY_DM_STRUCT  *p_dm_odm
);
u32 odm_get_version_mp_8821a_txpwr_lmt_8821a_sar_5mm(void);

/******************************************************************************
*                           TXPWR_LMT_8821A_SAR_8mm.TXT
******************************************************************************/

void
odm_read_and_config_mp_8821a_txpwr_lmt_8821a_sar_8mm(/* TC: Test Chip, MP: MP Chip*/
	struct PHY_DM_STRUCT  *p_dm_odm
);
u32 odm_get_version_mp_8821a_txpwr_lmt_8821a_sar_8mm(void);

#endif
#endif /* end of HWIMG_SUPPORT*/

