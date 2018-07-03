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

#ifndef __HAL_PHY_RF_8821A_H__
#define __HAL_PHY_RF_8821A_H__

/*--------------------------Define Parameters-------------------------------*/
void halrf_rf_lna_setting_8821a(
		struct PHY_DM_STRUCT	*p_dm,
		enum phydm_lna_set type
);
void configure_txpower_track_8821a(
	struct _TXPWRTRACK_CFG	*p_config
	);

void odm_tx_pwr_track_set_pwr_8821a(
	void*		pDM_VOID,
	enum pwrtrack_method 	Method,
	u8 				RFPath,
	u8 				ChannelMappedIndex
	);

void phy_lc_calibrate_8821a(
	void*		pDM_VOID
);

void get_delta_swing_table_8821a(
	void*		pDM_VOID,
	u8* 			*TemperatureUP_A,
	u8* 			*TemperatureDOWN_A,
	u8* 			*TemperatureUP_B,
	u8* 			*TemperatureDOWN_B	
);
#endif	// #ifndef __HAL_PHY_RF_8821A_H__								

