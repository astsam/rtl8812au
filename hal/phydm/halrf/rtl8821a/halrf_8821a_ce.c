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

#include "mp_precomp.h"
#include "../../phydm_precomp.h"



/*---------------------------Define Local Constant---------------------------*/
/* 2010/04/25 MH Define the max tx power tracking tx agc power. */
#define		ODM_TXPWRTRACK_MAX_IDX8821A		6

/*---------------------------Define Local Constant---------------------------*/


/* 3============================================================
 * 3 Tx Power Tracking
 * 3============================================================ */
void halrf_rf_lna_setting_8821a(
		struct PHY_DM_STRUCT	*p_dm,
		enum phydm_lna_set type
)
{
	/*phydm_disable_lna*/
	if (type == phydm_lna_disable) {
		odm_set_rf_reg(p_dm, RF_PATH_A, 0xef, 0x80000, 0x1);
		odm_set_rf_reg(p_dm, RF_PATH_A, 0x30, 0xfffff, 0x18000);	/*select Rx mode*/
		odm_set_rf_reg(p_dm, RF_PATH_A, 0x31, 0xfffff, 0x3f7ff);
		odm_set_rf_reg(p_dm, RF_PATH_A, 0x32, 0xfffff, 0xc22bf);	/*disable LNA*/
		odm_set_rf_reg(p_dm, RF_PATH_A, 0xef, 0x80000, 0x0);
		if (p_dm->rf_type > RF_1T1R) {
			odm_set_rf_reg(p_dm, RF_PATH_B, 0xef, 0x80000, 0x1);
			odm_set_rf_reg(p_dm, RF_PATH_B, 0x30, 0xfffff, 0x18000);	/*select Rx mode*/
			odm_set_rf_reg(p_dm, RF_PATH_B, 0x31, 0xfffff, 0x3f7ff);
			odm_set_rf_reg(p_dm, RF_PATH_B, 0x32, 0xfffff, 0xc22bf);	/*disable LNA*/
			odm_set_rf_reg(p_dm, RF_PATH_B, 0xef, 0x80000, 0x0);
		}
	} else if (type == phydm_lna_enable) {
		odm_set_rf_reg(p_dm, RF_PATH_A, 0xef, 0x80000, 0x1);
		odm_set_rf_reg(p_dm, RF_PATH_A, 0x30, 0xfffff, 0x18000);	/*select Rx mode*/
		odm_set_rf_reg(p_dm, RF_PATH_A, 0x31, 0xfffff, 0x3f7ff);
		odm_set_rf_reg(p_dm, RF_PATH_A, 0x32, 0xfffff, 0xc26bf);	/*disable LNA*/
		odm_set_rf_reg(p_dm, RF_PATH_A, 0xef, 0x80000, 0x0);
		if (p_dm->rf_type > RF_1T1R) {
			odm_set_rf_reg(p_dm, RF_PATH_B, 0xef, 0x80000, 0x1);
			odm_set_rf_reg(p_dm, RF_PATH_B, 0x30, 0xfffff, 0x18000);	/*select Rx mode*/
			odm_set_rf_reg(p_dm, RF_PATH_B, 0x31, 0xfffff, 0x3f7ff);
			odm_set_rf_reg(p_dm, RF_PATH_B, 0x32, 0xfffff, 0xc26bf);	/*disable LNA*/
			odm_set_rf_reg(p_dm, RF_PATH_B, 0xef, 0x80000, 0x0);
		}
	}
}

void odm_tx_pwr_track_set_pwr8821a(
	void	*p_dm_void,
	enum pwrtrack_method	method,
	u8	rf_path,
	u8	channel_mapped_index
)
{
	struct PHY_DM_STRUCT	*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _ADAPTER		*adapter = p_dm_odm->adapter;
	HAL_DATA_TYPE	*p_hal_data = GET_HAL_DATA(adapter);

	u8			pwr_tracking_limit = 26; /* +1.0dB */
	u8			tx_rate = 0xFF;
	u8			final_ofdm_swing_index = 0;
	u8			final_cck_swing_index = 0;
	u8			i = 0;
	u32			final_bb_swing_idx[1];
	struct odm_rf_calibration_structure	*p_rf_calibrate_info = &(p_dm_odm->rf_calibrate_info);

	if (*(p_dm_odm->p_mp_mode) == true) {
#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN | ODM_CE))
#if (DM_ODM_SUPPORT_TYPE & ODM_WIN)
#if (MP_DRIVER == 1)
		PMPT_CONTEXT p_mpt_ctx = &(adapter->mpt_ctx);

		tx_rate = mpt_to_mgnt_rate(p_mpt_ctx->mpt_rate_index);
#endif
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
		PMPT_CONTEXT p_mpt_ctx = &(adapter->mppriv.mpt_ctx);

		tx_rate = mpt_to_mgnt_rate(p_mpt_ctx->mpt_rate_index);
#endif
#endif
	} else {
		u16	rate	 = *(p_dm_odm->p_forced_data_rate);

		if (!rate) { /*auto rate*/
			if (rate != 0xFF) {
#if (DM_ODM_SUPPORT_TYPE & ODM_WIN)
			tx_rate = adapter->HalFunc.GetHwRateFromMRateHandler(p_dm_odm->tx_rate);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
			tx_rate = hw_rate_to_m_rate(p_dm_odm->tx_rate);
#endif
			}
		} else /*force rate*/
			tx_rate = (u8)rate;
	}
		
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("Power Tracking TxRate=0x%X\n", tx_rate));
	ODM_RT_TRACE(p_dm_odm,ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,("===>ODM_TxPwrTrackSetPwr8821A\n"));

	if(tx_rate != 0xFF) {
		/* 2 CCK */
		if((tx_rate >= MGN_1M)&&(tx_rate <= MGN_11M))
			pwr_tracking_limit = 32; //+4dB
		//2 OFDM
		else if((tx_rate >= MGN_6M)&&(tx_rate <= MGN_48M))
			pwr_tracking_limit = 30; //+3dB
		else if(tx_rate == MGN_54M)
			pwr_tracking_limit = 28; //+2dB
		//2 HT
		else if((tx_rate >= MGN_MCS0)&&(tx_rate <= MGN_MCS2)) //QPSK/BPSK
			pwr_tracking_limit = 34; //+5dB
		else if((tx_rate >= MGN_MCS3)&&(tx_rate <= MGN_MCS4)) //16QAM
			pwr_tracking_limit = 30; //+3dB
		else if((tx_rate >= MGN_MCS5)&&(tx_rate <= MGN_MCS7)) //64QAM
			pwr_tracking_limit = 28; //+2dB
			
		//2 VHT
		else if((tx_rate >= MGN_VHT1SS_MCS0)&&(tx_rate <= MGN_VHT1SS_MCS2)) //QPSK/BPSK
			pwr_tracking_limit = 34; //+5dB
		else if((tx_rate >= MGN_VHT1SS_MCS3)&&(tx_rate <= MGN_VHT1SS_MCS4)) //16QAM
			pwr_tracking_limit = 30; //+3dB
		else if((tx_rate >= MGN_VHT1SS_MCS5)&&(tx_rate <= MGN_VHT1SS_MCS6)) //64QAM
			pwr_tracking_limit = 28; //+2dB
		else if(tx_rate == MGN_VHT1SS_MCS7) //64QAM
			pwr_tracking_limit = 26; //+1dB
		else if(tx_rate == MGN_VHT1SS_MCS8) //256QAM
			pwr_tracking_limit = 24; //+0dB
		else if(tx_rate == MGN_VHT1SS_MCS9) /* 256QAM */
			pwr_tracking_limit = 22; //-1dB

		else			
			pwr_tracking_limit = 24;
	}
	ODM_RT_TRACE(p_dm_odm,ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,("tx_rate=0x%x, pwr_tracking_limit=%d\n", tx_rate, pwr_tracking_limit));

	if (method == BBSWING) {
		if (rf_path == RF_PATH_A) {
			final_bb_swing_idx[RF_PATH_A] = (p_dm_odm->rf_calibrate_info.OFDM_index[RF_PATH_A] > pwr_tracking_limit) ? pwr_tracking_limit : p_dm_odm->rf_calibrate_info.OFDM_index[RF_PATH_A];
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("p_dm_odm->rf_calibrate_info.OFDM_index[RF_PATH_A]=%d, p_dm_odm->RealBbSwingIdx[RF_PATH_A]=%d\n",
				p_dm_odm->rf_calibrate_info.OFDM_index[RF_PATH_A], final_bb_swing_idx[RF_PATH_A]));
			odm_set_bb_reg(p_dm_odm, REG_B_TX_SCALE_JAGUAR, 0xFFE00000, tx_scaling_table_jaguar[final_bb_swing_idx[RF_PATH_A]]);
		}
	}

	else if (method == MIX_MODE) {
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("p_rf_calibrate_info->default_ofdm_index=%d, p_rf_calibrate_info->absolute_ofdm_swing_idx[rf_path]=%d, RF_Path = %d\n",
			p_rf_calibrate_info->default_ofdm_index, p_rf_calibrate_info->absolute_ofdm_swing_idx[rf_path], rf_path));

		final_cck_swing_index = p_rf_calibrate_info->default_cck_index + p_rf_calibrate_info->absolute_ofdm_swing_idx[rf_path];
		final_ofdm_swing_index = p_rf_calibrate_info->default_ofdm_index + p_rf_calibrate_info->absolute_ofdm_swing_idx[rf_path];

		if (rf_path == RF_PATH_A)  
		{
			if (final_ofdm_swing_index > pwr_tracking_limit)     /*BBSwing higher then Limit*/
			{
				p_rf_calibrate_info->remnant_cck_swing_idx = final_cck_swing_index - pwr_tracking_limit;
				p_rf_calibrate_info->remnant_ofdm_swing_idx[rf_path] = final_ofdm_swing_index - pwr_tracking_limit;            

				odm_set_bb_reg(p_dm_odm, REG_A_TX_SCALE_JAGUAR, 0xFFE00000, tx_scaling_table_jaguar[pwr_tracking_limit]);

				p_rf_calibrate_info->modify_tx_agc_flag_path_a = true;

				phy_set_tx_power_level_by_path(adapter, *p_dm_odm->p_channel, RF_PATH_A);

				ODM_RT_TRACE(p_dm_odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Path_A Over BBSwing Limit, PwrTrackingLimit = %d , Remnant TxAGC Value = %d\n", pwr_tracking_limit, p_rf_calibrate_info->remnant_ofdm_swing_idx[rf_path]));
			} else if (final_ofdm_swing_index <= 0) {
				p_rf_calibrate_info->remnant_cck_swing_idx = final_cck_swing_index;
				p_rf_calibrate_info->remnant_ofdm_swing_idx[rf_path] = final_ofdm_swing_index;

				odm_set_bb_reg(p_dm_odm, REG_A_TX_SCALE_JAGUAR, 0xFFE00000, tx_scaling_table_jaguar[0]);

				p_rf_calibrate_info->modify_tx_agc_flag_path_a = true;

				phy_set_tx_power_level_by_path(adapter, *p_dm_odm->p_channel, RF_PATH_A);

				ODM_RT_TRACE(p_dm_odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Path_A Lower then BBSwing lower bound  0, Remnant tx_agc value = %d\n", p_rf_calibrate_info->remnant_ofdm_swing_idx[rf_path]));
			} else {
				odm_set_bb_reg(p_dm_odm, REG_A_TX_SCALE_JAGUAR, 0xFFE00000, tx_scaling_table_jaguar[final_ofdm_swing_index]);

				ODM_RT_TRACE(p_dm_odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Path_A Compensate with BBSwing, final_ofdm_swing_index = %d\n", final_ofdm_swing_index));

				if (p_rf_calibrate_info->modify_tx_agc_flag_path_a) { /*If tx_agc has changed, reset tx_agc again*/
					p_rf_calibrate_info->remnant_cck_swing_idx = 0;
					p_rf_calibrate_info->remnant_ofdm_swing_idx[rf_path] = 0;

					phy_set_tx_power_level_by_path(adapter, *p_dm_odm->p_channel, RF_PATH_A);

					p_rf_calibrate_info->modify_tx_agc_flag_path_a = false;

					ODM_RT_TRACE(p_dm_odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Path_A p_dm_odm->Modify_TxAGC_Flag = false\n"));
				}
			}
		}
	}
	else
	{
		return;
	}
}	// odm_TxPwrTrackSetPwr88E

void get_delta_swing_table_8821a(
	void*		p_dm_void,
	u8* 			*TemperatureUP_A,
	u8* 			*TemperatureDOWN_A,
	u8* 			*TemperatureUP_B,
	u8* 			*TemperatureDOWN_B	
	)
{
	struct PHY_DM_STRUCT	*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
    	struct _ADAPTER        *adapter   		 = p_dm_odm->adapter;
	struct odm_rf_calibration_structure	*p_rf_calibrate_info = &(p_dm_odm->rf_calibrate_info);
	HAL_DATA_TYPE  	*pHalData  		 = GET_HAL_DATA(adapter);
	u8			TxRate			= 0xFF;
	u8         	channel   		 = pHalData->current_channel;

	if (*(p_dm_odm->p_mp_mode) == true) {
	#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN | ODM_CE))
		#if (DM_ODM_SUPPORT_TYPE & ODM_WIN)
			#if (MP_DRIVER == 1)
					PMPT_CONTEXT pMptCtx = &(adapter->MptCtx);
					
					TxRate = mpt_to_mgnt_rate(pMptCtx->MptRateIndex);
			#endif
		#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
				PMPT_CONTEXT pMptCtx = &(adapter->mppriv.mpt_ctx);
				
				TxRate = mpt_to_mgnt_rate(pMptCtx->mpt_rate_index);
		#endif	
	#endif
	} else {
		u16	rate	 = *(p_dm_odm->p_forced_data_rate);
		
		if (!rate) { /*auto rate*/
			if (rate != 0xFF) {
			#if (DM_ODM_SUPPORT_TYPE & ODM_WIN)
						TxRate = adapter->HalFunc.GetHwRateFromMRateHandler(pDM_Odm->TxRate);
			#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
						TxRate = hw_rate_to_m_rate(p_dm_odm->tx_rate);
			#endif
			}
		} else { /*force rate*/
			TxRate = (u8)rate;
		}
	}
		
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("Power Tracking TxRate=0x%X\n", TxRate));

		
	if ( 1 <= channel && channel <= 14) {
		if (IS_CCK_RATE(TxRate)) {
	        *TemperatureUP_A   = p_rf_calibrate_info->delta_swing_table_idx_2g_cck_a_p;
	        *TemperatureDOWN_A = p_rf_calibrate_info->delta_swing_table_idx_2g_cck_a_n;
	        *TemperatureUP_B   = p_rf_calibrate_info->delta_swing_table_idx_2g_cck_b_p;
	        *TemperatureDOWN_B = p_rf_calibrate_info->delta_swing_table_idx_2g_cck_b_n;		
		} else {
	        *TemperatureUP_A   = p_rf_calibrate_info->delta_swing_table_idx_2ga_p;
	        *TemperatureDOWN_A = p_rf_calibrate_info->delta_swing_table_idx_2ga_n;
	        *TemperatureUP_B   = p_rf_calibrate_info->delta_swing_table_idx_2gb_p;
	        *TemperatureDOWN_B = p_rf_calibrate_info->delta_swing_table_idx_2gb_n;			
		}
 	} else if ( 36 <= channel && channel <= 64) {
        *TemperatureUP_A   = p_rf_calibrate_info->delta_swing_table_idx_5ga_p[0];
        *TemperatureDOWN_A = p_rf_calibrate_info->delta_swing_table_idx_5ga_n[0];
        *TemperatureUP_B   = p_rf_calibrate_info->delta_swing_table_idx_5gb_p[0];
        *TemperatureDOWN_B = p_rf_calibrate_info->delta_swing_table_idx_5gb_n[0];
    } else if ( 100 <= channel && channel <= 140) {
        *TemperatureUP_A   = p_rf_calibrate_info->delta_swing_table_idx_5ga_p[1];
        *TemperatureDOWN_A = p_rf_calibrate_info->delta_swing_table_idx_5ga_n[1];
        *TemperatureUP_B   = p_rf_calibrate_info->delta_swing_table_idx_5gb_p[1];
        *TemperatureDOWN_B = p_rf_calibrate_info->delta_swing_table_idx_5gb_n[1];
    } else if ( 149 <= channel && channel <= 173) {
        *TemperatureUP_A   = p_rf_calibrate_info->delta_swing_table_idx_5ga_p[2]; 
        *TemperatureDOWN_A = p_rf_calibrate_info->delta_swing_table_idx_5ga_n[2]; 
        *TemperatureUP_B   = p_rf_calibrate_info->delta_swing_table_idx_5gb_p[2]; 
        *TemperatureDOWN_B = p_rf_calibrate_info->delta_swing_table_idx_5gb_n[2]; 
    } else {
	    *TemperatureUP_A   = (u8*)delta_swing_table_idx_2ga_p_8188e;
	    *TemperatureDOWN_A = (u8*)delta_swing_table_idx_2ga_n_8188e;	
	    *TemperatureUP_B   = (u8*)delta_swing_table_idx_2ga_p_8188e;
	    *TemperatureDOWN_B = (u8*)delta_swing_table_idx_2ga_n_8188e;		
    }
	
	return;
}

void configure_txpower_track_8821a(
	struct _TXPWRTRACK_CFG	*p_config
	)
{
	p_config->swing_table_size_cck = TXSCALE_TABLE_SIZE;
	p_config->swing_table_size_ofdm = TXSCALE_TABLE_SIZE;
	p_config->threshold_iqk = IQK_THRESHOLD;
	p_config->average_thermal_num = AVG_THERMAL_NUM_8812A;
	p_config->rf_path_count = MAX_PATH_NUM_8821A;
	p_config->thermal_reg_addr = RF_T_METER_8812A;
		
	p_config->odm_tx_pwr_track_set_pwr = odm_tx_pwr_track_set_pwr8821a;
	p_config->do_iqk = do_iqk_8821a;
	p_config->phy_lc_calibrate = phy_lc_calibrate_8821a;
	p_config->get_delta_swing_table = get_delta_swing_table_8821a;
}

#define		DP_BB_REG_NUM		7
#define		DP_RF_REG_NUM		1
#define		DP_RETRY_LIMIT		10
#define		DP_PATH_NUM		2
#define		DP_DPK_NUM		3
#define		DP_DPK_VALUE_NUM	2

void phy_lc_calibrate_8821a(
	void*		p_dm_void
	)
{
	struct PHY_DM_STRUCT	*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	u8		StartTime; 
	u8		ProgressingTime;

	StartTime = odm_get_current_time( p_dm_odm);
	phy_lc_calibrate_8812a(p_dm_odm);
	ProgressingTime = odm_get_progressing_time( p_dm_odm, StartTime);
	ODM_RT_TRACE(p_dm_odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD,  ("LCK ProgressingTime = %d\n", ProgressingTime));
}


