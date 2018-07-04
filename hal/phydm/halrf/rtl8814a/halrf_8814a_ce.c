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
#include "../phydm_precomp.h"



/*---------------------------Define Local Constant---------------------------*/
// 2010/04/25 MH Define the max tx power tracking tx agc power.
#define		ODM_TXPWRTRACK_MAX_IDX_8814A		6

/*---------------------------Define Local Constant---------------------------*/

//3============================================================
//3 Tx Power Tracking
//3============================================================


// Add CheckRFGainOffset By YuChen to make sure that RF gain offset will not over upperbound 4'b1010

u1Byte
CheckRFGainOffset(
	PDM_ODM_T			pDM_Odm,
	PWRTRACK_METHOD 	Method,
	u1Byte				RFPath
	)
{
	s1Byte	UpperBound = 10, LowerBound = -5; // 4'b1010 = 10
	s1Byte	Final_RF_Index = 0;
	BOOLEAN	bPositive = FALSE;
	u4Byte	bitMask = 0;
	u1Byte	Final_OFDM_Swing_Index = 0, TxScalingUpperBound = 28, TxScalingLowerBound = 4;// upper bound +2dB, lower bound -10dB
	PODM_RF_CAL_T  pRFCalibrateInfo = &(pDM_Odm->RFCalibrateInfo);
	if(Method == MIX_MODE)	//normal Tx power tracking
	{
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,("is 8814 MP chip\n"));
		bitMask = BIT19;
		pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath] = pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath] + pRFCalibrateInfo->KfreeOffset[RFPath];


		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("=========================== [Path-%d] TXBB Offset============================\n", RFPath));
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("Absolute_OFDMSwingIdx[RFPath](%d) = Absolute_OFDMSwingIdx[RFPath](%d) + KfreeOffset[RFPath](%d), RFPath=%d\n", pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath], pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath], pRFCalibrateInfo->KfreeOffset[RFPath], RFPath));

		if (pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath] >= 0)				/* check if RF_Index is positive or not*/
			bPositive = TRUE;
		else
			bPositive = FALSE;

		ODM_SetRFReg(pDM_Odm, RFPath, rRF_TxGainOffset, bitMask, bPositive);
		
		bitMask = BIT18|BIT17|BIT16|BIT15;
		Final_RF_Index = pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath]  / 2;	/*TxBB 1 step equal 1dB, BB swing 1step equal 0.5dB*/

	}
	
	if(Final_RF_Index > UpperBound)		//Upper bound = 10dB, if more htan upper bound, then move to bb swing max = +2dB
	{
		ODM_SetRFReg(pDM_Odm, RFPath, rRF_TxGainOffset, bitMask, UpperBound);	//set RF Reg0x55 per path
			
		Final_OFDM_Swing_Index = pRFCalibrateInfo->DefaultOfdmIndex + (pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath] - (UpperBound << 1));
		
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("Final_OFDM_Swing_Index(%d) = DefaultOfdmIndex(%d) + (Absolute_OFDMSwingIdx[RFPath](%d) - (UpperBound(%d) << 1)), RFPath=%d\n", Final_OFDM_Swing_Index, pRFCalibrateInfo->DefaultOfdmIndex, pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath], UpperBound, RFPath));
		
		if (Final_OFDM_Swing_Index > TxScalingUpperBound) {	/* bb swing upper bound = +2dB */
			Final_OFDM_Swing_Index = TxScalingUpperBound;
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("Final_OFDM_Swing_Index(%d) > TxScalingUpperBound(%d)   Final_OFDM_Swing_Index = TxScalingUpperBound\n", Final_OFDM_Swing_Index, TxScalingUpperBound));
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("===========================================================================\n"));
		}

		return Final_OFDM_Swing_Index;
	}
	else if(Final_RF_Index < LowerBound)	// lower bound = -5dB
	{
		ODM_SetRFReg(pDM_Odm, RFPath, rRF_TxGainOffset, bitMask, (-1)*(LowerBound));	//set RF Reg0x55 per path

		Final_OFDM_Swing_Index = pRFCalibrateInfo->DefaultOfdmIndex - ((LowerBound<<1) - pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath]);

		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("Final_OFDM_Swing_Index(%d) = DefaultOfdmIndex(%d) - ((LowerBound(%d)<<1) - Absolute_OFDMSwingIdx[RFPath](%d)), RFPath=%d\n", Final_OFDM_Swing_Index, pRFCalibrateInfo->DefaultOfdmIndex, LowerBound, pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath], RFPath));

		if (Final_OFDM_Swing_Index < TxScalingLowerBound) {	/* BB swing lower bound = -10dB */
			Final_OFDM_Swing_Index = TxScalingLowerBound;
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("Final_OFDM_Swing_Index(%d) > TxScalingLowerBound(%d)   Final_OFDM_Swing_Index = TxScalingLowerBound\n", Final_OFDM_Swing_Index, TxScalingLowerBound));
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("===========================================================================\n"));
		}
		return Final_OFDM_Swing_Index;
	}
	else		// normal case
	{

		if(bPositive == TRUE)
			ODM_SetRFReg(pDM_Odm, RFPath, rRF_TxGainOffset, bitMask, Final_RF_Index);	//set RF Reg0x55 per path
		else
			ODM_SetRFReg(pDM_Odm, RFPath, rRF_TxGainOffset, bitMask, (-1)*Final_RF_Index);	//set RF Reg0x55 per path

		Final_OFDM_Swing_Index = pRFCalibrateInfo->DefaultOfdmIndex + (pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath])%2;
		
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("Final_OFDM_Swing_Index(%d) = DefaultOfdmIndex(%d) + (Absolute_OFDMSwingIdx[RFPath])//2(%d), RFPath=%d\n", Final_OFDM_Swing_Index, pRFCalibrateInfo->DefaultOfdmIndex, (pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath])%2, RFPath));
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("===========================================================================\n"));

		return Final_OFDM_Swing_Index;	
	}
	
	return FALSE;
}


VOID
ODM_TxPwrTrackSetPwr8814A(
	IN	PVOID		pDM_VOID,
	PWRTRACK_METHOD 	Method,
	u1Byte 				RFPath,
	u1Byte 				ChannelMappedIndex
	)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
		PADAPTER		Adapter = pDM_Odm->Adapter;
		PHAL_DATA_TYPE	pHalData = GET_HAL_DATA(Adapter);
		PODM_RF_CAL_T	pRFCalibrateInfo = &(pDM_Odm->RFCalibrateInfo);
		u1Byte			Final_OFDM_Swing_Index = 0; 

		if (Method == MIX_MODE)			
		{
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("pRFCalibrateInfo->DefaultOfdmIndex=%d, pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath]=%d, RF_Path = %d\n",
				pRFCalibrateInfo->DefaultOfdmIndex, pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath], RFPath));
		
			Final_OFDM_Swing_Index = CheckRFGainOffset(pDM_Odm, MIX_MODE, RFPath);
		}
		else if(Method == TSSI_MODE)
		{
			ODM_SetRFReg(pDM_Odm, RFPath, rRF_TxGainOffset, BIT18|BIT17|BIT16|BIT15, 0);
		}
		else if(Method == BBSWING)		// use for mp driver clean power tracking status
		{
			pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath] = pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath] + pRFCalibrateInfo->KfreeOffset[RFPath];

			Final_OFDM_Swing_Index = pRFCalibrateInfo->DefaultOfdmIndex + (pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath]);

			ODM_SetRFReg(pDM_Odm, RFPath, rRF_TxGainOffset, BIT18|BIT17|BIT16|BIT15, 0);
		}

		if((Method == MIX_MODE) || (Method == BBSWING))
		{
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("=========================== [Path-%d] BBSWING Offset============================\n", RFPath));

			switch(RFPath)
			{
				case ODM_RF_PATH_A:
					
					ODM_SetBBReg(pDM_Odm, rA_TxScale_Jaguar, 0xFFE00000, TxScalingTable_Jaguar[Final_OFDM_Swing_Index]);	//set BBswing

					ODM_RT_TRACE(pDM_Odm,ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
						("******Path_A Compensate with BBSwing , Final_OFDM_Swing_Index = %d \n", Final_OFDM_Swing_Index));
					break;

				case ODM_RF_PATH_B:
						
					ODM_SetBBReg(pDM_Odm, rB_TxScale_Jaguar, 0xFFE00000, TxScalingTable_Jaguar[Final_OFDM_Swing_Index]);	//set BBswing

					ODM_RT_TRACE(pDM_Odm,ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
						("******Path_B Compensate with BBSwing , Final_OFDM_Swing_Index = %d \n", Final_OFDM_Swing_Index));
					break;

				case ODM_RF_PATH_C:
						
					ODM_SetBBReg(pDM_Odm, rC_TxScale_Jaguar2, 0xFFE00000, TxScalingTable_Jaguar[Final_OFDM_Swing_Index]);	//set BBswing

					ODM_RT_TRACE(pDM_Odm,ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
						("******Path_C Compensate with BBSwing , Final_OFDM_Swing_Index = %d \n", Final_OFDM_Swing_Index));
			            	break;

				case ODM_RF_PATH_D:
						
					ODM_SetBBReg(pDM_Odm, rD_TxScale_Jaguar2, 0xFFE00000, TxScalingTable_Jaguar[Final_OFDM_Swing_Index]);	//set BBswing

					ODM_RT_TRACE(pDM_Odm,ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
						("******Path_D Compensate with BBSwing , Final_OFDM_Swing_Index = %d \n", Final_OFDM_Swing_Index));
					break;

				default:
					ODM_RT_TRACE(pDM_Odm,ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
						("Wrong Path name!!!! \n"));

				break;
			}
			
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("===========================================================================\n"));
		}
		return;
}	// ODM_TxPwrTrackSetPwr8814A


VOID
GetDeltaSwingTable_8814A(
	IN	PVOID		pDM_VOID,
	OUT pu1Byte 			*TemperatureUP_A,
	OUT pu1Byte 			*TemperatureDOWN_A,
	OUT pu1Byte 			*TemperatureUP_B,
	OUT pu1Byte 			*TemperatureDOWN_B	
	)
{
	PDM_ODM_T	pDM_Odm = (PDM_ODM_T)pDM_VOID;
    PADAPTER        Adapter   		 = pDM_Odm->Adapter;
	PODM_RF_CAL_T  	pRFCalibrateInfo = &(pDM_Odm->RFCalibrateInfo);
	HAL_DATA_TYPE  	*pHalData  		 = GET_HAL_DATA(Adapter);
	u1Byte		TxRate			= 0xFF;
	u1Byte         	channel   		 = pHalData->CurrentChannel;

	
	if (pDM_Odm->mp_mode == TRUE) {
	#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN | ODM_CE))
		#if (DM_ODM_SUPPORT_TYPE & ODM_WIN)
			#if (MP_DRIVER == 1)
					PMPT_CONTEXT pMptCtx = &(Adapter->MptCtx);
					
					TxRate = MptToMgntRate(pMptCtx->MptRateIndex);
			#endif
		#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
				PMPT_CONTEXT pMptCtx = &(Adapter->mppriv.MptCtx);
				
				TxRate = MptToMgntRate(pMptCtx->MptRateIndex);
		#endif	
	#endif
	} else {
		u2Byte	rate	 = *(pDM_Odm->pForcedDataRate);
		
		if (!rate) { /*auto rate*/
			if (pDM_Odm->TxRate != 0xFF) {
			#if (DM_ODM_SUPPORT_TYPE & ODM_WIN)
						TxRate = Adapter->HalFunc.GetHwRateFromMRateHandler(pDM_Odm->TxRate);
			#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
						TxRate = HwRateToMRate(pDM_Odm->TxRate);
			#endif
			}
		} else { /*force rate*/
			TxRate = (u1Byte)rate;
		}
	}
		
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("Power Tracking TxRate=0x%X\n", TxRate));

	if (1 <= channel && channel <= 14) {
		if (IS_CCK_RATE(TxRate)) {
			*TemperatureUP_A   = pRFCalibrateInfo->DeltaSwingTableIdx_2GCCKA_P;
			*TemperatureDOWN_A = pRFCalibrateInfo->DeltaSwingTableIdx_2GCCKA_N;
			*TemperatureUP_B   = pRFCalibrateInfo->DeltaSwingTableIdx_2GCCKB_P;
			*TemperatureDOWN_B = pRFCalibrateInfo->DeltaSwingTableIdx_2GCCKB_N;		
		} else {
			*TemperatureUP_A   = pRFCalibrateInfo->DeltaSwingTableIdx_2GA_P;
			*TemperatureDOWN_A = pRFCalibrateInfo->DeltaSwingTableIdx_2GA_N;
			*TemperatureUP_B   = pRFCalibrateInfo->DeltaSwingTableIdx_2GB_P;
			*TemperatureDOWN_B = pRFCalibrateInfo->DeltaSwingTableIdx_2GB_N;			
		}
	} else if (36 <= channel && channel <= 64) {
		*TemperatureUP_A   = pRFCalibrateInfo->DeltaSwingTableIdx_5GA_P[0];
		*TemperatureDOWN_A = pRFCalibrateInfo->DeltaSwingTableIdx_5GA_N[0];
		*TemperatureUP_B   = pRFCalibrateInfo->DeltaSwingTableIdx_5GB_P[0];
		*TemperatureDOWN_B = pRFCalibrateInfo->DeltaSwingTableIdx_5GB_N[0];
	} else if (100 <= channel && channel <= 144) {
		*TemperatureUP_A   = pRFCalibrateInfo->DeltaSwingTableIdx_5GA_P[1];
		*TemperatureDOWN_A = pRFCalibrateInfo->DeltaSwingTableIdx_5GA_N[1];
		*TemperatureUP_B   = pRFCalibrateInfo->DeltaSwingTableIdx_5GB_P[1];
		*TemperatureDOWN_B = pRFCalibrateInfo->DeltaSwingTableIdx_5GB_N[1];
	} else if (149 <= channel && channel <= 173) {
		*TemperatureUP_A   = pRFCalibrateInfo->DeltaSwingTableIdx_5GA_P[2]; 
		*TemperatureDOWN_A = pRFCalibrateInfo->DeltaSwingTableIdx_5GA_N[2]; 
		*TemperatureUP_B   = pRFCalibrateInfo->DeltaSwingTableIdx_5GB_P[2]; 
		*TemperatureDOWN_B = pRFCalibrateInfo->DeltaSwingTableIdx_5GB_N[2]; 
	} else {
		*TemperatureUP_A   = (pu1Byte)DeltaSwingTableIdx_2GA_P_8188E;
		*TemperatureDOWN_A = (pu1Byte)DeltaSwingTableIdx_2GA_N_8188E;	
		*TemperatureUP_B   = (pu1Byte)DeltaSwingTableIdx_2GA_P_8188E;
		*TemperatureDOWN_B = (pu1Byte)DeltaSwingTableIdx_2GA_N_8188E;		
	}


	
	return;
}


VOID
GetDeltaSwingTable_8814A_PathCD(
	IN	PVOID		pDM_VOID,
	OUT pu1Byte 			*TemperatureUP_C,
	OUT pu1Byte 			*TemperatureDOWN_C,
	OUT pu1Byte 			*TemperatureUP_D,
	OUT pu1Byte 			*TemperatureDOWN_D	
	)
{
	PDM_ODM_T	pDM_Odm = (PDM_ODM_T)pDM_VOID;
    PADAPTER        Adapter   		 = pDM_Odm->Adapter;
	PODM_RF_CAL_T  	pRFCalibrateInfo = &(pDM_Odm->RFCalibrateInfo);
	HAL_DATA_TYPE  	*pHalData  		 = GET_HAL_DATA(Adapter);
	u1Byte		TxRate			= 0xFF;
	u1Byte         	channel   		 = pHalData->CurrentChannel;

	
	if (pDM_Odm->mp_mode == TRUE) {
	#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN | ODM_CE))
		#if (DM_ODM_SUPPORT_TYPE & ODM_WIN)
			#if (MP_DRIVER == 1)
					PMPT_CONTEXT pMptCtx = &(Adapter->MptCtx);
					
					TxRate = MptToMgntRate(pMptCtx->MptRateIndex);
			#endif
		#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
				PMPT_CONTEXT pMptCtx = &(Adapter->mppriv.MptCtx);
				
				TxRate = MptToMgntRate(pMptCtx->MptRateIndex);
		#endif	
	#endif
	} else {
		u2Byte	rate	 = *(pDM_Odm->pForcedDataRate);
		
		if (!rate) { /*auto rate*/
			if (pDM_Odm->TxRate != 0xFF) {
			#if (DM_ODM_SUPPORT_TYPE & ODM_WIN)
						TxRate = Adapter->HalFunc.GetHwRateFromMRateHandler(pDM_Odm->TxRate);
			#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
						TxRate = HwRateToMRate(pDM_Odm->TxRate);
			#endif
			}
		} else { /*force rate*/
			TxRate = (u1Byte)rate;
		}
	}
		
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("Power Tracking TxRate=0x%X\n", TxRate));

	if ( 1 <= channel && channel <= 14) {
		if (IS_CCK_RATE(TxRate)) {
			*TemperatureUP_C  = pRFCalibrateInfo->DeltaSwingTableIdx_2GCCKC_P;
			*TemperatureDOWN_C = pRFCalibrateInfo->DeltaSwingTableIdx_2GCCKC_N;
			 *TemperatureUP_D   = pRFCalibrateInfo->DeltaSwingTableIdx_2GCCKD_P;
			*TemperatureDOWN_D = pRFCalibrateInfo->DeltaSwingTableIdx_2GCCKD_N;		
		} else {
			*TemperatureUP_C   = pRFCalibrateInfo->DeltaSwingTableIdx_2GC_P;
			*TemperatureDOWN_C = pRFCalibrateInfo->DeltaSwingTableIdx_2GC_N;
			*TemperatureUP_D   = pRFCalibrateInfo->DeltaSwingTableIdx_2GD_P;
			*TemperatureDOWN_D = pRFCalibrateInfo->DeltaSwingTableIdx_2GD_N;			
		}
	} else if (36 <= channel && channel <= 64) {
		*TemperatureUP_C   = pRFCalibrateInfo->DeltaSwingTableIdx_5GC_P[0];
		*TemperatureDOWN_C = pRFCalibrateInfo->DeltaSwingTableIdx_5GC_N[0];
		*TemperatureUP_D   = pRFCalibrateInfo->DeltaSwingTableIdx_5GD_P[0];
		*TemperatureDOWN_D = pRFCalibrateInfo->DeltaSwingTableIdx_5GD_N[0];
	} else if (100 <= channel && channel <= 144) {
		*TemperatureUP_C   = pRFCalibrateInfo->DeltaSwingTableIdx_5GC_P[1];
		*TemperatureDOWN_C = pRFCalibrateInfo->DeltaSwingTableIdx_5GC_N[1];
		*TemperatureUP_D   = pRFCalibrateInfo->DeltaSwingTableIdx_5GD_P[1];
		*TemperatureDOWN_D = pRFCalibrateInfo->DeltaSwingTableIdx_5GD_N[1];
	} else if (149 <= channel && channel <= 173) {
		*TemperatureUP_C   = pRFCalibrateInfo->DeltaSwingTableIdx_5GC_P[2]; 
		*TemperatureDOWN_C = pRFCalibrateInfo->DeltaSwingTableIdx_5GC_N[2]; 
		*TemperatureUP_D   = pRFCalibrateInfo->DeltaSwingTableIdx_5GD_P[2]; 
		*TemperatureDOWN_D = pRFCalibrateInfo->DeltaSwingTableIdx_5GD_N[2]; 
	} else {
		*TemperatureUP_C   = (pu1Byte)DeltaSwingTableIdx_2GA_P_8188E;
		*TemperatureDOWN_C = (pu1Byte)DeltaSwingTableIdx_2GA_N_8188E;	
		*TemperatureUP_D   = (pu1Byte)DeltaSwingTableIdx_2GA_P_8188E;
		*TemperatureDOWN_D = (pu1Byte)DeltaSwingTableIdx_2GA_N_8188E;		
	}
	
	return;
}

void ConfigureTxpowerTrack_8814A(
	PTXPWRTRACK_CFG	pConfig
	)
{
	pConfig->SwingTableSize_CCK = CCK_TABLE_SIZE;
	pConfig->SwingTableSize_OFDM = OFDM_TABLE_SIZE;
	pConfig->Threshold_IQK = 8;
	pConfig->AverageThermalNum = AVG_THERMAL_NUM_8814A;
	pConfig->RfPathCount = MAX_PATH_NUM_8814A;
	pConfig->ThermalRegAddr = RF_T_METER_88E;
		
	pConfig->ODM_TxPwrTrackSetPwr = ODM_TxPwrTrackSetPwr8814A;
	pConfig->DoIQK = DoIQK_8814A;
	pConfig->PHY_LCCalibrate = PHY_LCCalibrate_8814A;
	pConfig->GetDeltaSwingTable = GetDeltaSwingTable_8814A;
	pConfig->GetDeltaSwingTable8814only = GetDeltaSwingTable_8814A_PathCD;
}

VOID	
phy_LCCalibrate_8814A(
	IN PDM_ODM_T		pDM_Odm,
	IN	BOOLEAN		is2T
	)
{
	u4Byte	/*RF_Amode=0, RF_Bmode=0,*/ LC_Cal = 0, tmp = 0, cnt;
	
	//Check continuous TX and Packet TX
	u4Byte	reg0x914 = ODM_Read4Byte(pDM_Odm, rSingleTone_ContTx_Jaguar);;

	// Backup RF reg18.

	if((reg0x914 & 0x70000) == 0)
		ODM_Write1Byte(pDM_Odm, REG_TXPAUSE, 0xFF);			

	//3 3. Read RF reg18
	LC_Cal = ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_CHNLBW, bRFRegOffsetMask);

	//3 4. Set LC calibration begin bit15
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_CHNLBW, bRFRegOffsetMask, 0x1b126);

	ODM_delay_ms(100);		

	for (cnt = 0; cnt < 100; cnt++) {
		if (ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_CHNLBW, 0x8000) != 0x1)
			break;
		ODM_delay_ms(10);
	}
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("retry cnt = %d\n", cnt));

	ODM_SetRFReg( pDM_Odm, ODM_RF_PATH_A, RF_CHNLBW, bRFRegOffsetMask, 0x13126);
	ODM_SetRFReg( pDM_Odm, ODM_RF_PATH_A, RF_CHNLBW, bRFRegOffsetMask, 0x13124);
	//3 Restore original situation
	if((reg0x914 & 70000) == 0)
		ODM_Write1Byte(pDM_Odm, REG_TXPAUSE, 0x00);	

	// Recover channel number
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_CHNLBW, bRFRegOffsetMask, LC_Cal);

	DbgPrint("Call %s\n", __FUNCTION__);
}


VOID	
phy_APCalibrate_8814A(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
	IN PDM_ODM_T		pDM_Odm,
#else
	IN	PADAPTER	pAdapter,
#endif
	IN	s1Byte 		delta,
	IN	BOOLEAN		is2T
	)
{
}


VOID
PHY_LCCalibrate_8814A(
	IN	PVOID		pDM_VOID
	)
{
	BOOLEAN 		bStartContTx = FALSE, bSingleTone = FALSE, bCarrierSuppression = FALSE;
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
	PADAPTER 		pAdapter = pDM_Odm->Adapter;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);	
	
#if (MP_DRIVER == 1)
#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)	
	PMPT_CONTEXT	pMptCtx = &(pAdapter->MptCtx);
	bStartContTx = pMptCtx->bStartContTx;
	bSingleTone = pMptCtx->bSingleTone;
	bCarrierSuppression = pMptCtx->bCarrierSuppression;
#else
	PMPT_CONTEXT	pMptCtx = &(pAdapter->mppriv.MptCtx);		
#endif	
#endif
#endif	

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("===> PHY_LCCalibrate_8814A\n"));

//#if (MP_DRIVER == 1)	
	phy_LCCalibrate_8814A(pDM_Odm, TRUE);
//#endif 

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("<=== PHY_LCCalibrate_8814A\n"));

}

VOID
PHY_APCalibrate_8814A(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
	IN PDM_ODM_T		pDM_Odm,
#else
	IN	PADAPTER	pAdapter,
#endif
	IN	s1Byte 		delta	
	)
{

}


VOID	                                                 
PHY_DPCalibrate_8814A(                                   
	IN 	PDM_ODM_T	pDM_Odm                             
	)
{
}


BOOLEAN 
phy_QueryRFPathSwitch_8814A(
	IN	PADAPTER	pAdapter
	)
{
	return TRUE;
}


BOOLEAN PHY_QueryRFPathSwitch_8814A(	
	IN	PADAPTER	pAdapter
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);

#if DISABLE_BB_RF
	return TRUE;
#endif

	return phy_QueryRFPathSwitch_8814A(pAdapter);
}


VOID phy_SetRFPathSwitch_8814A(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
	IN PDM_ODM_T		pDM_Odm,
#else
	IN	PADAPTER	pAdapter,
#endif
	IN	BOOLEAN		bMain,
	IN	BOOLEAN		is2T
	)
{
}
VOID PHY_SetRFPathSwitch_8814A(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
	IN PDM_ODM_T		pDM_Odm,
#else
	IN	PADAPTER	pAdapter,
#endif
	IN	BOOLEAN		bMain
	)
{
}





