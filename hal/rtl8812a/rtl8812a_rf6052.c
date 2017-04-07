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
#define _RTL8812A_RF6052_C_

/* #include <drv_types.h> */
#include <rtl8812a_hal.h>


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
PHY_RF6052SetBandwidth8812(
	IN	PADAPTER				Adapter,
	IN	CHANNEL_WIDTH		Bandwidth)	/* 20M or 40M */
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);

	switch (Bandwidth) {
	case CHANNEL_WIDTH_20:
		/* RTW_INFO("PHY_RF6052SetBandwidth8812(), set 20MHz\n"); */
		PHY_SetRFReg(Adapter, RF_PATH_A, RF_CHNLBW_Jaguar, BIT11 | BIT10, 3);
		PHY_SetRFReg(Adapter, RF_PATH_B, RF_CHNLBW_Jaguar, BIT11 | BIT10, 3);
		break;

	case CHANNEL_WIDTH_40:
		/* RTW_INFO("PHY_RF6052SetBandwidth8812(), set 40MHz\n"); */
		PHY_SetRFReg(Adapter, RF_PATH_A, RF_CHNLBW_Jaguar, BIT11 | BIT10, 1);
		PHY_SetRFReg(Adapter, RF_PATH_B, RF_CHNLBW_Jaguar, BIT11 | BIT10, 1);
		break;

	case CHANNEL_WIDTH_80:
		/* RTW_INFO("PHY_RF6052SetBandwidth8812(), set 80MHz\n"); */
		PHY_SetRFReg(Adapter, RF_PATH_A, RF_CHNLBW_Jaguar, BIT11 | BIT10, 0);
		PHY_SetRFReg(Adapter, RF_PATH_B, RF_CHNLBW_Jaguar, BIT11 | BIT10, 0);
		break;

	default:
		RTW_INFO("PHY_RF6052SetBandwidth8812(): unknown Bandwidth: %#X\n", Bandwidth);
		break;
	}
}

static int
phy_RF6052_Config_ParaFile_8812(
	IN	PADAPTER		Adapter
)
{
	u8					eRFPath;
	int					rtStatus = _SUCCESS;
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);

	/* 3 */ /* ----------------------------------------------------------------- */
	/* 3 */ /* <2> Initialize RF */
	/* 3 */ /* ----------------------------------------------------------------- */
	/* for(eRFPath = RF_PATH_A; eRFPath <pHalData->NumTotalRFPath; eRFPath++) */
	for (eRFPath = 0; eRFPath < pHalData->NumTotalRFPath; eRFPath++) {
		/*----Initialize RF fom connfiguration file----*/
		switch (eRFPath) {
		case RF_PATH_A:
#ifdef CONFIG_LOAD_PHY_PARA_FROM_FILE
			if (PHY_ConfigRFWithParaFile(Adapter, PHY_FILE_RADIO_A, eRFPath) == _FAIL)
#endif
			{
#ifdef CONFIG_EMBEDDED_FWIMG
				if (HAL_STATUS_FAILURE == ODM_ConfigRFWithHeaderFile(&pHalData->odmpriv, CONFIG_RF_RADIO, (ODM_RF_RADIO_PATH_E)eRFPath))
					rtStatus = _FAIL;
#endif
			}
			break;
		case RF_PATH_B:
#ifdef CONFIG_LOAD_PHY_PARA_FROM_FILE
			if (PHY_ConfigRFWithParaFile(Adapter, PHY_FILE_RADIO_B, eRFPath) == _FAIL)
#endif
			{
#ifdef CONFIG_EMBEDDED_FWIMG
				if (HAL_STATUS_FAILURE == ODM_ConfigRFWithHeaderFile(&pHalData->odmpriv, CONFIG_RF_RADIO, (ODM_RF_RADIO_PATH_E)eRFPath))
					rtStatus = _FAIL;
#endif
			}
			break;
		default:
			break;
		}

		if (rtStatus != _SUCCESS) {
			RTW_INFO("%s():Radio[%d] Fail!!", __FUNCTION__, eRFPath);
			goto phy_RF6052_Config_ParaFile_Fail;
		}

	}

	/* 3 ----------------------------------------------------------------- */
	/* 3 Configuration of Tx Power Tracking */
	/* 3 ----------------------------------------------------------------- */

#ifdef CONFIG_LOAD_PHY_PARA_FROM_FILE
	if (PHY_ConfigRFWithTxPwrTrackParaFile(Adapter, PHY_FILE_TXPWR_TRACK) == _FAIL)
#endif
	{
#ifdef CONFIG_EMBEDDED_FWIMG
		ODM_ConfigRFWithTxPwrTrackHeaderFile(&pHalData->odmpriv);
#endif
	}


phy_RF6052_Config_ParaFile_Fail:
	return rtStatus;
}


int
PHY_RF6052_Config_8812(
	IN	PADAPTER		Adapter)
{
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);
	int					rtStatus = _SUCCESS;

	/* Initialize general global value */
	if (pHalData->rf_type == RF_1T1R)
		pHalData->NumTotalRFPath = 1;
	else
		pHalData->NumTotalRFPath = 2;

	/*  */
	/* Config BB and RF */
	/*  */
	rtStatus = phy_RF6052_Config_ParaFile_8812(Adapter);

	return rtStatus;

}


/* End of HalRf6052.c */
