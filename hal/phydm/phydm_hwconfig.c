/******************************************************************************
 *
 * Copyright(c) 2007 - 2017  Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * The full GNU General Public License is included in this distribution in the
 * file called LICENSE.
 *
 * Contact Information:
 * wlanfae <wlanfae@realtek.com>
 * Realtek Corporation, No. 2, Innovation Road II, Hsinchu Science Park,
 * Hsinchu 300, Taiwan.
 *
 * Larry Finger <Larry.Finger@lwfinger.net>
 *
 *****************************************************************************/

/* ************************************************************
 * include files
 * ************************************************************ */

#include "mp_precomp.h"
#include "phydm_precomp.h"

#define READ_AND_CONFIG_MP(ic, txt) (odm_read_and_config_mp_##ic##txt(dm))

#define READ_AND_CONFIG     READ_AND_CONFIG_MP

#define GET_VERSION_MP(ic, txt)		(odm_get_version_mp_##ic##txt())
#define GET_VERSION_TC(ic, txt)		(odm_get_version_tc_##ic##txt())

#define GET_VERSION(ic, txt) GET_VERSION_MP(ic, txt)

enum hal_status
odm_config_rf_with_header_file(
	struct dm_struct		*dm,
	enum odm_rf_config_type		config_type,
	u8			e_rf_path
)
{
#if (DM_ODM_SUPPORT_TYPE & ODM_WIN)
	void		*adapter = dm->adapter;
	PMGNT_INFO		mgnt_info = &((PADAPTER)adapter)->MgntInfo;
#endif
	enum hal_status	result = HAL_STATUS_SUCCESS;

	PHYDM_DBG(dm, ODM_COMP_INIT,
		"===>odm_config_rf_with_header_file (%s)\n", (dm->is_mp_chip) ? "MPChip" : "TestChip");
	PHYDM_DBG(dm, ODM_COMP_INIT,
		"support_platform: 0x%X, support_interface: 0x%X, board_type: 0x%X\n",
		dm->support_platform, dm->support_interface, dm->board_type);

	/* 1 AP doesn't use PHYDM power tracking table in these ICs */
#if (DM_ODM_SUPPORT_TYPE != ODM_AP)
#if (RTL8812A_SUPPORT == 1)
	if (dm->support_ic_type == ODM_RTL8812) {
		if (config_type == CONFIG_RF_RADIO) {
			if (e_rf_path == RF_PATH_A)
				READ_AND_CONFIG_MP(8812a, _radioa);
			else if (e_rf_path == RF_PATH_B)
				READ_AND_CONFIG_MP(8812a, _radiob);
		} else if (config_type == CONFIG_RF_TXPWR_LMT) {
#if (DM_ODM_SUPPORT_TYPE & ODM_WIN) && (DEV_BUS_TYPE == RT_PCI_INTERFACE)
			HAL_DATA_TYPE	*hal_data = GET_HAL_DATA(((PADAPTER)adapter));
			if ((hal_data->EEPROMSVID == 0x17AA && hal_data->EEPROMSMID == 0xA811) ||
			    (hal_data->EEPROMSVID == 0x10EC && hal_data->EEPROMSMID == 0xA812) ||
			    (hal_data->EEPROMSVID == 0x10EC && hal_data->EEPROMSMID == 0x8812))
				READ_AND_CONFIG_MP(8812a, _txpwr_lmt_hm812a03);
			else
#endif
				READ_AND_CONFIG_MP(8812a, _txpwr_lmt);
		}
	}
#endif
#if (RTL8821A_SUPPORT == 1)
	if (dm->support_ic_type == ODM_RTL8821) {
		if (config_type == CONFIG_RF_RADIO) {
			if (e_rf_path == RF_PATH_A)
				READ_AND_CONFIG_MP(8821a, _radioa);
		} else if (config_type == CONFIG_RF_TXPWR_LMT) {
			if (dm->support_interface == ODM_ITRF_USB) {
				if (dm->ext_pa_5g || dm->ext_lna_5g)
					READ_AND_CONFIG_MP(8821a, _txpwr_lmt_8811a_u_fem);
				else
					READ_AND_CONFIG_MP(8821a, _txpwr_lmt_8811a_u_ipa);
			} else {
#if (DM_ODM_SUPPORT_TYPE & ODM_WIN)
				if (mgnt_info->CustomerID == RT_CID_8821AE_ASUS_MB)
					READ_AND_CONFIG_MP(8821a, _txpwr_lmt_8821a_sar_8mm);
				else if (mgnt_info->CustomerID == RT_CID_ASUS_NB)
					READ_AND_CONFIG_MP(8821a, _txpwr_lmt_8821a_sar_5mm);
				else
#endif
					READ_AND_CONFIG_MP(8821a, _txpwr_lmt_8821a);
			}
		}
	}
#endif
#endif/* (DM_ODM_SUPPORT_TYPE !=  ODM_AP) */

	/* 1 All platforms support */
#if (RTL8814A_SUPPORT == 1)
	if (dm->support_ic_type == ODM_RTL8814A) {
		if (config_type == CONFIG_RF_RADIO) {
			if (e_rf_path == RF_PATH_A)
				READ_AND_CONFIG_MP(8814a, _radioa);
			else if (e_rf_path == RF_PATH_B)
				READ_AND_CONFIG_MP(8814a, _radiob);
			else if (e_rf_path == RF_PATH_C)
				READ_AND_CONFIG_MP(8814a, _radioc);
			else if (e_rf_path == RF_PATH_D)
				READ_AND_CONFIG_MP(8814a, _radiod);
		} else if (config_type == CONFIG_RF_TXPWR_LMT) {
			READ_AND_CONFIG_MP(8814a,_txpwr_lmt);
		}
	}
#endif
	if (config_type == CONFIG_RF_RADIO) {
		if (dm->fw_offload_ability & PHYDM_PHY_PARAM_OFFLOAD) {
			result = phydm_set_reg_by_fw(dm,
							PHYDM_HALMAC_CMD_END,
							0,
							0,
							0,
							(enum rf_path)0,
							0);
			PHYDM_DBG(dm, ODM_COMP_INIT,
				"rf param offload end!result = %d", result);
		}
	}

	return result;
}

enum hal_status
odm_config_rf_with_tx_pwr_track_header_file(
	struct dm_struct		*dm
)
{
	PHYDM_DBG(dm, ODM_COMP_INIT,
		"===>odm_config_rf_with_tx_pwr_track_header_file (%s)\n", (dm->is_mp_chip) ? "MPChip" : "TestChip");
	PHYDM_DBG(dm, ODM_COMP_INIT,
		"support_platform: 0x%X, support_interface: 0x%X, board_type: 0x%X\n",
		dm->support_platform, dm->support_interface, dm->board_type);


	/* 1 AP doesn't use PHYDM power tracking table in these ICs */
#if (DM_ODM_SUPPORT_TYPE != ODM_AP)
#if RTL8821A_SUPPORT
	if (dm->support_ic_type == ODM_RTL8821) {
		if (dm->support_interface == ODM_ITRF_PCIE)
			READ_AND_CONFIG_MP(8821a, _txpowertrack_pcie);
		else if (dm->support_interface == ODM_ITRF_USB)
			READ_AND_CONFIG_MP(8821a, _txpowertrack_usb);
		else if (dm->support_interface == ODM_ITRF_SDIO)
			READ_AND_CONFIG_MP(8821a, _txpowertrack_sdio);
	}
#endif
#if RTL8812A_SUPPORT
	if (dm->support_ic_type == ODM_RTL8812) {
		if (dm->support_interface == ODM_ITRF_PCIE)
			READ_AND_CONFIG_MP(8812a, _txpowertrack_pcie);
		else if (dm->support_interface == ODM_ITRF_USB) {
			if (dm->rfe_type == 3 && dm->is_mp_chip)
				READ_AND_CONFIG_MP(8812a, _txpowertrack_rfe3);
			else
				READ_AND_CONFIG_MP(8812a, _txpowertrack_usb);
		}

	}
#endif
#endif/* (DM_ODM_SUPPORT_TYPE !=  ODM_AP) */


/* 1 All platforms support */
#if RTL8814A_SUPPORT
	if (dm->support_ic_type == ODM_RTL8814A) {
		if (dm->rfe_type == 0)
			READ_AND_CONFIG_MP(8814a, _txpowertrack_type0);
		else if (dm->rfe_type == 2)
			READ_AND_CONFIG_MP(8814a, _txpowertrack_type2);
		else if (dm->rfe_type == 5)
			READ_AND_CONFIG_MP(8814a, _txpowertrack_type5);
		/*else if (p_dm->rfe_type == 7)
			READ_AND_CONFIG_MP(8814a, _txpowertrack_type7);
		else if (p_dm->rfe_type == 8)
			READ_AND_CONFIG_MP(8814a, _txpowertrack_type8);*/
		else
			READ_AND_CONFIG_MP(8814a, _txpowertrack);

//		READ_AND_CONFIG_MP(8814a, _txpowertssi);
	}
#endif

	return HAL_STATUS_SUCCESS;
}

enum hal_status
odm_config_bb_with_header_file(
	struct dm_struct		*dm,
	enum odm_bb_config_type		config_type
)
{
#if (DM_ODM_SUPPORT_TYPE & ODM_WIN)
	void		*adapter = dm->adapter;
	PMGNT_INFO		mgnt_info = &((PADAPTER)adapter)->MgntInfo;
#endif
	enum hal_status	result = HAL_STATUS_SUCCESS;

	/* 1 AP doesn't use PHYDM initialization in these ICs */
#if (DM_ODM_SUPPORT_TYPE != ODM_AP)
#if (RTL8812A_SUPPORT == 1)
	if (dm->support_ic_type == ODM_RTL8812) {
		if (config_type == CONFIG_BB_PHY_REG)
			READ_AND_CONFIG_MP(8812a, _phy_reg);
		else if (config_type == CONFIG_BB_AGC_TAB)
			READ_AND_CONFIG_MP(8812a, _agc_tab);
		else if (config_type == CONFIG_BB_PHY_REG_PG) {
			if (dm->rfe_type == 3 && dm->is_mp_chip)
				READ_AND_CONFIG_MP(8812a, _phy_reg_pg_asus);
#if (DM_ODM_SUPPORT_TYPE & ODM_WIN)
			else if (mgnt_info->CustomerID == RT_CID_WNC_NEC && dm->is_mp_chip)
				READ_AND_CONFIG_MP(8812a, _phy_reg_pg_nec);
#if RT_PLATFORM == PLATFORM_MACOSX
			/*{1827}{1024} for BUFFALO power by rate table. Isaiah 2013-11-29*/
			else if (mgnt_info->CustomerID == RT_CID_DNI_BUFFALO)
				READ_AND_CONFIG_MP(8812a, _phy_reg_pg_dni);
			/* TP-Link T4UH, Isaiah 2015-03-16*/
			else if (mgnt_info->CustomerID == RT_CID_TPLINK_HPWR) {
				pr_debug("RT_CID_TPLINK_HPWR:: _PHY_REG_PG_TPLINK\n");
				READ_AND_CONFIG_MP(8812a, _phy_reg_pg_tplink);
			}
#endif
#endif
			else
				READ_AND_CONFIG_MP(8812a, _phy_reg_pg);
		} else if (config_type == CONFIG_BB_PHY_REG_MP)
			READ_AND_CONFIG_MP(8812a, _phy_reg_mp);
		else if (config_type == CONFIG_BB_AGC_TAB_DIFF) {
			dm->fw_offload_ability &= ~PHYDM_PHY_PARAM_OFFLOAD;
			/*AGC_TAB DIFF dont support FW offload*/
			if ((*dm->channel >= 36)  && (*dm->channel  <= 64))
				AGC_DIFF_CONFIG_MP(8812a, lb);
			else if (*dm->channel >= 100)
				AGC_DIFF_CONFIG_MP(8812a, hb);
		}
	}
#endif
#if (RTL8821A_SUPPORT == 1)
	if (dm->support_ic_type == ODM_RTL8821) {
		if (config_type == CONFIG_BB_PHY_REG)
			READ_AND_CONFIG_MP(8821a, _phy_reg);
		else if (config_type == CONFIG_BB_AGC_TAB)
			READ_AND_CONFIG_MP(8821a, _agc_tab);
		else if (config_type == CONFIG_BB_PHY_REG_PG) {
#if (DM_ODM_SUPPORT_TYPE & ODM_WIN)
#if (DEV_BUS_TYPE == RT_PCI_INTERFACE)
			HAL_DATA_TYPE	*hal_data = GET_HAL_DATA(((PADAPTER)adapter));

			if ((hal_data->EEPROMSVID == 0x1043 && hal_data->EEPROMSMID == 0x207F))
				READ_AND_CONFIG_MP(8821a, _phy_reg_pg_e202_sa);
			else
#endif
#if (RT_PLATFORM == PLATFORM_MACOSX)
				/*{1827}{1022} for BUFFALO power by rate table. Isaiah 2013-10-18*/
				if (mgnt_info->CustomerID == RT_CID_DNI_BUFFALO) {
					/*{1024} for BUFFALO power by rate table. (JP/US)*/
					if (mgnt_info->ChannelPlan == RT_CHANNEL_DOMAIN_US_2G_CANADA_5G)
						READ_AND_CONFIG_MP(8821a, _phy_reg_pg_dni_us);
					else
						READ_AND_CONFIG_MP(8821a, _phy_reg_pg_dni_jp);
				} else
#endif
#endif
					READ_AND_CONFIG_MP(8821a, _phy_reg_pg);
		}
	}
#endif

#endif/* (DM_ODM_SUPPORT_TYPE !=  ODM_AP) */


	/* 1 All platforms support */
#if (RTL8814A_SUPPORT == 1)
	if (dm->support_ic_type == ODM_RTL8814A) {
		if (config_type == CONFIG_BB_PHY_REG)
			READ_AND_CONFIG_MP(8814a, _phy_reg);
		else if (config_type == CONFIG_BB_AGC_TAB)
			READ_AND_CONFIG_MP(8814a, _agc_tab);
		else if (config_type == CONFIG_BB_PHY_REG_PG) {
			/*if (p_dm->rfe_type == 0)
				READ_AND_CONFIG_MP(8814a,_phy_reg_pg_type0);
			else if (dm->rfe_type == 2)
				READ_AND_CONFIG_MP(8814a,_phy_reg_pg_type2);				
			else if (dm->rfe_type == 3)
				READ_AND_CONFIG_MP(8814a,_phy_reg_pg_type3);		
			else if (dm->rfe_type == 4)
				READ_AND_CONFIG_MP(8814a,_phy_reg_pg_type4);		
			else if (dm->rfe_type == 5)
				READ_AND_CONFIG_MP(8814a,_phy_reg_pg_type5);		
			else if (dm->rfe_type == 7)
				READ_AND_CONFIG_MP(8814a,_phy_reg_pg_type7);
			else if (dm->rfe_type == 8)
				READ_AND_CONFIG_MP(8814a,_phy_reg_pg_type8);
			else*/
				READ_AND_CONFIG_MP(8814a,_phy_reg_pg);
		}
		else if (config_type == CONFIG_BB_PHY_REG_MP)
			READ_AND_CONFIG_MP(8814a, _phy_reg_mp);
	}
#endif

	if (config_type == CONFIG_BB_PHY_REG || config_type == CONFIG_BB_AGC_TAB)
		if (dm->fw_offload_ability & PHYDM_PHY_PARAM_OFFLOAD) {
			result = phydm_set_reg_by_fw(dm,
								PHYDM_HALMAC_CMD_END,
								0,
								0,
								0,
								(enum rf_path)0,
								0);
			PHYDM_DBG(dm, ODM_COMP_INIT,
				"phy param offload end!result = %d", result);
		}

	return result;
}

enum hal_status
odm_config_mac_with_header_file(
	struct dm_struct	*dm
)
{
	enum hal_status	result = HAL_STATUS_SUCCESS;
	PHYDM_DBG(dm, ODM_COMP_INIT,
		"===>odm_config_mac_with_header_file (%s)\n", (dm->is_mp_chip) ? "MPChip" : "TestChip");
	PHYDM_DBG(dm, ODM_COMP_INIT,
		"support_platform: 0x%X, support_interface: 0x%X, board_type: 0x%X\n",
		dm->support_platform, dm->support_interface, dm->board_type);

	/* 1 AP doesn't use PHYDM initialization in these ICs */
#if (DM_ODM_SUPPORT_TYPE != ODM_AP)
#if (RTL8812A_SUPPORT == 1)
	if (dm->support_ic_type == ODM_RTL8812)
		READ_AND_CONFIG_MP(8812a, _mac_reg);
#endif
#if (RTL8821A_SUPPORT == 1)
	if (dm->support_ic_type == ODM_RTL8821)
		READ_AND_CONFIG_MP(8821a, _mac_reg);
#endif

#endif/* (DM_ODM_SUPPORT_TYPE !=  ODM_AP) */

	/* 1 All platforms support */
#if (RTL8814A_SUPPORT == 1)
	if (dm->support_ic_type == ODM_RTL8814A)
		READ_AND_CONFIG_MP(8814a, _mac_reg);
#endif

	if (dm->fw_offload_ability & PHYDM_PHY_PARAM_OFFLOAD) {
		result = phydm_set_reg_by_fw(dm,
							PHYDM_HALMAC_CMD_END,
							0,
							0,
							0,
							(enum rf_path)0,
							0);
		PHYDM_DBG(dm, ODM_COMP_INIT,
			"mac param offload end!result = %d", result);
	}

	return result;
}

u32
odm_get_hw_img_version(
	struct dm_struct	*dm
)
{
	u32  version = 0;

	/* 1 AP doesn't use PHYDM initialization in these ICs */
#if (DM_ODM_SUPPORT_TYPE != ODM_AP)
#if (RTL8821A_SUPPORT == 1)
	if (dm->support_ic_type == ODM_RTL8821)
		version = GET_VERSION_MP(8821a, _mac_reg);
#endif
#if (RTL8812A_SUPPORT == 1)
	if (dm->support_ic_type == ODM_RTL8812)
		version = GET_VERSION_MP(8812a, _mac_reg);
#endif
#endif /* (DM_ODM_SUPPORT_TYPE != ODM_AP) */

	/*1 All platforms support*/
#if (RTL8814A_SUPPORT == 1)
	if (dm->support_ic_type == ODM_RTL8814A)
		version = GET_VERSION_MP(8814a, _mac_reg);
#endif

	return version;
}


u32
query_phydm_trx_capability(
	struct dm_struct					*dm
)
{
	u32 value32 = 0xFFFFFFFF;

	return value32;
}

u32
query_phydm_stbc_capability(
	struct dm_struct					*dm
)
{
	u32 value32 = 0xFFFFFFFF;

	return value32;
}

u32
query_phydm_ldpc_capability(
	struct dm_struct					*dm
)
{
	u32 value32 = 0xFFFFFFFF;

	return value32;
}

u32
query_phydm_txbf_parameters(
	struct dm_struct					*dm
)
{
	u32 value32 = 0xFFFFFFFF;

	return value32;
}

u32
query_phydm_txbf_capability(
	struct dm_struct					*dm
)
{
	u32 value32 = 0xFFFFFFFF;

	return value32;
}
