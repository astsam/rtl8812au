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
#define _RTL8814A_RF6052_C_

//#include <drv_types.h>
#include <rtl8814a_hal.h>


/*-----------------------------------------------------------------------------
 * Function:    PHY_RF6052SetBandwidth()
 *
 * Overview:    This function is called by SetBWModeCallback8190Pci() only
 *
 * Input:       PADAPTER				Adapter
 *			WIRELESS_BANDWIDTH_E	Bandwidth	//20M or 40M
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Note:		For RF type 0222D
 *---------------------------------------------------------------------------*/
VOID
PHY_RF6052SetBandwidth8814A(
	IN	PADAPTER				Adapter,
	IN	CHANNEL_WIDTH		Bandwidth)	//20M or 40M
{
	switch(Bandwidth)
	{
		case CHANNEL_WIDTH_20:
			/*DBG_871X("PHY_RF6052SetBandwidth8814A(), set 20MHz\n");*/
			PHY_SetRFReg(Adapter, RF_PATH_A, RF_CHNLBW_Jaguar, BIT11|BIT10, 3);
			PHY_SetRFReg(Adapter, RF_PATH_B, RF_CHNLBW_Jaguar, BIT11|BIT10, 3);
			PHY_SetRFReg(Adapter, RF_PATH_C, RF_CHNLBW_Jaguar, BIT11|BIT10, 3);
			PHY_SetRFReg(Adapter, RF_PATH_D, RF_CHNLBW_Jaguar, BIT11|BIT10, 3);
		break;
			
		case CHANNEL_WIDTH_40:
			/*DBG_871X("PHY_RF6052SetBandwidth8814A(), set 40MHz\n");*/
			PHY_SetRFReg(Adapter, RF_PATH_A, RF_CHNLBW_Jaguar, BIT11|BIT10, 1);	
			PHY_SetRFReg(Adapter, RF_PATH_B, RF_CHNLBW_Jaguar, BIT11|BIT10, 1);	
			PHY_SetRFReg(Adapter, RF_PATH_C, RF_CHNLBW_Jaguar, BIT11|BIT10, 1);	
			PHY_SetRFReg(Adapter, RF_PATH_D, RF_CHNLBW_Jaguar, BIT11|BIT10, 1);			
		break;
		
		case CHANNEL_WIDTH_80:
			/*DBG_871X("PHY_RF6052SetBandwidth8814A(), set 80MHz\n");*/
			PHY_SetRFReg(Adapter, RF_PATH_A, RF_CHNLBW_Jaguar, BIT11|BIT10, 0);	
			PHY_SetRFReg(Adapter, RF_PATH_B, RF_CHNLBW_Jaguar, BIT11|BIT10, 0);	
			PHY_SetRFReg(Adapter, RF_PATH_C, RF_CHNLBW_Jaguar, BIT11|BIT10, 0);	
			PHY_SetRFReg(Adapter, RF_PATH_D, RF_CHNLBW_Jaguar, BIT11|BIT10, 0);
		break;
			
		default:
			DBG_871X("PHY_RF6052SetBandwidth8814A(): unknown Bandwidth: %#X\n",Bandwidth );
			break;			
	}
}

static int
phy_RF6052_Config_ParaFile_8814A(
	IN	PADAPTER		Adapter
	)
{
	u32					u4RegValue=0;
	u8					eRFPath;
	int					rtStatus = _SUCCESS;
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);
	static char			sz8814RadioAFile[] = RTL8814A_PHY_RADIO_A;
	static char			sz8814RadioBFile[] = RTL8814A_PHY_RADIO_B;
	static char			sz8814RadioCFile[] = RTL8814A_PHY_RADIO_C;
	static char			sz8814RadioDFile[] = RTL8814A_PHY_RADIO_D;
	static char 		sz8814TxPwrTrack[] = RTL8814A_TXPWR_TRACK;
	char				*pszRadioAFile = NULL, *pszRadioBFile = NULL, *pszRadioCFile = NULL, *pszRadioDFile = NULL, *pszTxPwrTrack = NULL;


	pszRadioAFile = sz8814RadioAFile;
	pszRadioBFile = sz8814RadioBFile;
	pszRadioCFile = sz8814RadioCFile;
	pszRadioDFile = sz8814RadioDFile;
	pszTxPwrTrack = sz8814TxPwrTrack;

	//3//-----------------------------------------------------------------
	//3// <2> Initialize RF
	//3//-----------------------------------------------------------------
	//for(eRFPath = RF_PATH_A; eRFPath <pHalData->NumTotalRFPath; eRFPath++)
	for(eRFPath = 0; eRFPath <pHalData->NumTotalRFPath; eRFPath++)
	{
		/*----Initialize RF fom connfiguration file----*/
		switch(eRFPath)
		{
		case RF_PATH_A:
#ifdef CONFIG_LOAD_PHY_PARA_FROM_FILE
			if (PHY_ConfigRFWithParaFile(Adapter, pszRadioAFile, eRFPath) == _FAIL)
#endif //CONFIG_LOAD_PHY_PARA_FROM_FILE
			{
#ifdef CONFIG_EMBEDDED_FWIMG
				if(HAL_STATUS_FAILURE ==ODM_ConfigRFWithHeaderFile(&pHalData->odmpriv,CONFIG_RF_RADIO, (ODM_RF_RADIO_PATH_E)eRFPath))
					rtStatus = _FAIL;
#endif //CONFIG_EMBEDDED_FWIMG
			}
			break;
		case RF_PATH_B:
#ifdef CONFIG_LOAD_PHY_PARA_FROM_FILE
			if (PHY_ConfigRFWithParaFile(Adapter, pszRadioBFile, eRFPath) == _FAIL)
#endif //CONFIG_LOAD_PHY_PARA_FROM_FILE
			{
#ifdef CONFIG_EMBEDDED_FWIMG
				if(HAL_STATUS_FAILURE ==ODM_ConfigRFWithHeaderFile(&pHalData->odmpriv,CONFIG_RF_RADIO, (ODM_RF_RADIO_PATH_E)eRFPath))
					rtStatus = _FAIL;
#endif //CONFIG_EMBEDDED_FWIMG
			}
			break;
		case RF_PATH_C:
#ifdef CONFIG_LOAD_PHY_PARA_FROM_FILE
			if (PHY_ConfigRFWithParaFile(Adapter, pszRadioCFile, eRFPath) == _FAIL)
#endif //CONFIG_LOAD_PHY_PARA_FROM_FILE
			{
#ifdef CONFIG_EMBEDDED_FWIMG
				if(HAL_STATUS_FAILURE ==ODM_ConfigRFWithHeaderFile(&pHalData->odmpriv,CONFIG_RF_RADIO, (ODM_RF_RADIO_PATH_E)eRFPath))
					rtStatus = _FAIL;
#endif //CONFIG_EMBEDDED_FWIMG
			}
			break;
		case RF_PATH_D:
#ifdef CONFIG_LOAD_PHY_PARA_FROM_FILE
			if (PHY_ConfigRFWithParaFile(Adapter, pszRadioDFile, eRFPath) == _FAIL)
#endif //CONFIG_LOAD_PHY_PARA_FROM_FILE
			{
#ifdef CONFIG_EMBEDDED_FWIMG
				if(HAL_STATUS_FAILURE ==ODM_ConfigRFWithHeaderFile(&pHalData->odmpriv,CONFIG_RF_RADIO, (ODM_RF_RADIO_PATH_E)eRFPath))
					rtStatus = _FAIL;
#endif //CONFIG_EMBEDDED_FWIMG
			}
			break;
		default:
			break;
		}

		if(rtStatus != _SUCCESS){
			DBG_871X("%s():Radio[%d] Fail!!", __FUNCTION__, eRFPath);
			goto phy_RF6052_Config_ParaFile_Fail;
		}

	}
	
	u4RegValue = PHY_QueryRFReg(Adapter, RF_PATH_A, RF_RCK1_Jaguar, bRFRegOffsetMask);
	PHY_SetRFReg(Adapter, RF_PATH_B,  RF_RCK1_Jaguar, bRFRegOffsetMask, u4RegValue);
	PHY_SetRFReg(Adapter, RF_PATH_C,  RF_RCK1_Jaguar, bRFRegOffsetMask, u4RegValue);
	PHY_SetRFReg(Adapter, RF_PATH_D,  RF_RCK1_Jaguar, bRFRegOffsetMask, u4RegValue);
	
	//3 -----------------------------------------------------------------
	//3 Configuration of Tx Power Tracking 
	//3 -----------------------------------------------------------------

#ifdef CONFIG_LOAD_PHY_PARA_FROM_FILE
	if (PHY_ConfigRFWithTxPwrTrackParaFile(Adapter, pszTxPwrTrack) == _FAIL)
#endif //CONFIG_LOAD_PHY_PARA_FROM_FILE
	{
#ifdef CONFIG_EMBEDDED_FWIMG
		ODM_ConfigRFWithTxPwrTrackHeaderFile(&pHalData->odmpriv);
#endif
	}

	//RT_TRACE(COMP_INIT, DBG_LOUD, ("<---phy_RF6052_Config_ParaFile_8812()\n"));

phy_RF6052_Config_ParaFile_Fail:
	return rtStatus;
}


int
PHY_RF6052_Config_8814A(
	IN	PADAPTER		Adapter)
{
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);
	int					rtStatus = _SUCCESS;

	// Initialize general global value
	pHalData->NumTotalRFPath = 4;

	//
	// Config BB and RF
	//
	rtStatus = phy_RF6052_Config_ParaFile_8814A(Adapter);

	return rtStatus;

}


/* End of HalRf6052.c */

