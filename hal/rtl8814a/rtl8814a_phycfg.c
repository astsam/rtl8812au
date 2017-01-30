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
#define _RTL8814A_PHYCFG_C_

//#include <drv_types.h>

#include <rtl8814a_hal.h>
#include "hal_com_h2c.h"

/*---------------------Define local function prototype-----------------------*/

/*----------------------------Function Body----------------------------------*/
//1   1. BB register R/W API

u32
PHY_QueryBBReg8814A(
	IN	PADAPTER	Adapter,
	IN	u32		RegAddr,
	IN	u32		BitMask
	)
{
	u32	ReturnValue = 0, OriginalValue, BitShift;

#if (DISABLE_BB_RF == 1)
	return 0;
#endif

#if(SIC_ENABLE == 1)
	return SIC_QueryBBReg(Adapter, RegAddr, BitMask);
#endif

	OriginalValue = rtw_read32(Adapter, RegAddr);
	BitShift = PHY_CalculateBitShift(BitMask);
	ReturnValue = (OriginalValue & BitMask) >> BitShift;

	//DBG_871X("BBR MASK=0x%x Addr[0x%x]=0x%x\n", BitMask, RegAddr, OriginalValue);

	return (ReturnValue);
}


VOID
PHY_SetBBReg8814A(
	IN	PADAPTER	Adapter,
	IN	u32		RegAddr,
	IN	u32		BitMask,
	IN	u32		Data
	)
{
	u32			OriginalValue, BitShift;

#if (DISABLE_BB_RF == 1)
	return;
#endif

#if(SIC_ENABLE == 1)
	SIC_SetBBReg(Adapter, RegAddr, BitMask, Data);
	return; 
#endif

	if(BitMask!= bMaskDWord)
	{//if not "double word" write
		OriginalValue = rtw_read32(Adapter, RegAddr);
		BitShift = PHY_CalculateBitShift(BitMask);
		Data = ((OriginalValue) & (~BitMask)) |( ((Data << BitShift)) & BitMask);
	}

	rtw_write32(Adapter, RegAddr, Data);
	
	//DBG_871X("BBW MASK=0x%x Addr[0x%x]=0x%x\n", BitMask, RegAddr, Data);
}



static	u32
phy_RFRead_8814A(
	IN	PADAPTER			Adapter,
	IN	u8				eRFPath,
	IN	u32				RegAddr,
	IN	u32				BitMask
	)
{
	u32	DataAndAddr = 0;
	u32	Readback_Value, Direct_Addr;	
	
	RegAddr &= 0xff;
	switch(eRFPath){
		case ODM_RF_PATH_A:
			Direct_Addr = 0x2800+RegAddr*4;
		break;
		case ODM_RF_PATH_B:
			Direct_Addr = 0x2c00+RegAddr*4;
		break;
		case ODM_RF_PATH_C:
			Direct_Addr = 0x3800+RegAddr*4;
		break;
		case ODM_RF_PATH_D:
			Direct_Addr = 0x3c00+RegAddr*4;
		break;
		default: //pathA
			Direct_Addr = 0x2800+RegAddr*4;
		break;
	}
	

	BitMask &= bRFRegOffsetMask;
	
	Readback_Value = PHY_QueryBBReg(Adapter, Direct_Addr, BitMask);		
	//DBG_871X("RFR-%d Addr[0x%x]=0x%x\n", eRFPath, RegAddr, Readback_Value);

	return Readback_Value;
}


static	VOID
phy_RFWrite_8814A(
	IN	PADAPTER			Adapter,
	IN	u8				eRFPath,
	IN	u32				Offset,
	IN	u32				Data
	)
{
	u32						DataAndAddr = 0;
	HAL_DATA_TYPE				*pHalData = GET_HAL_DATA(Adapter);
	BB_REGISTER_DEFINITION_T		*pPhyReg = &pHalData->PHYRegDef[eRFPath];

	// 2009/06/17 MH We can not execute IO for power save or other accident mode.
	//if(RT_CANNOT_IO(Adapter))
	//{
		//RT_DISP(FPHY, PHY_RFW, ("phy_RFSerialWrite stop\n"));
		//return;
	//}

	Offset &= 0xff;

	// Shadow Update
	//PHY_RFShadowWrite(Adapter, eRFPath, Offset, Data);

	// Put write addr in [27:20]  and write data in [19:00]
	DataAndAddr = ((Offset<<20) | (Data&0x000fffff)) & 0x0fffffff;	

	// Write Operation 
	PHY_SetBBReg(Adapter, pPhyReg->rf3wireOffset, bMaskDWord, DataAndAddr);
	//DBG_871X("RFW-%d Addr[0x%x]=0x%x\n", eRFPath, pPhyReg->rf3wireOffset, DataAndAddr);
}


u32
PHY_QueryRFReg8814A(
	IN	PADAPTER			Adapter,
	IN	u8				eRFPath,
	IN	u32				RegAddr,
	IN	u32				BitMask
	)
{
	u32	Readback_Value;	

#if (DISABLE_BB_RF == 1)
	return 0;
#endif
	
	Readback_Value = phy_RFRead_8814A(Adapter, eRFPath, RegAddr, BitMask);

	return (Readback_Value);
}


VOID
PHY_SetRFReg8814A(
	IN	PADAPTER			Adapter,
	IN	u8				eRFPath,
	IN	u32				RegAddr,
	IN	u32				BitMask,
	IN	u32				Data
	)
{

#if (DISABLE_BB_RF == 1)
	return;
#endif

	if(BitMask == 0)
		return;

	RegAddr &= 0xff;
	// RF data is 20 bits only
	if (BitMask != bLSSIWrite_data_Jaguar) {
		u32			Original_Value, BitShift;

		Original_Value = phy_RFRead_8814A(Adapter, eRFPath, RegAddr, bLSSIWrite_data_Jaguar);
		BitShift =  PHY_CalculateBitShift(BitMask);
		Data = ((Original_Value) & (~BitMask)) | (Data<< BitShift);
	}
	
	phy_RFWrite_8814A(Adapter, eRFPath, RegAddr, Data);


}

//
// 3. Initial MAC/BB/RF config by reading MAC/BB/RF txt.
//

s32 PHY_MACConfig8814(PADAPTER Adapter)
{
	int				rtStatus = _FAIL;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	s8				*pszMACRegFile;
	s8				sz8814MACRegFile[] = RTL8814A_PHY_MACREG;

	pszMACRegFile = sz8814MACRegFile;


	//
	// Config MAC
	//
#ifdef CONFIG_LOAD_PHY_PARA_FROM_FILE
	rtStatus = phy_ConfigMACWithParaFile(Adapter, pszMACRegFile);
	if (rtStatus == _FAIL)
#endif //CONFIG_LOAD_PHY_PARA_FROM_FILE
	{
#ifdef CONFIG_EMBEDDED_FWIMG
		ODM_ConfigMACWithHeaderFile(&pHalData->odmpriv);
		rtStatus = _SUCCESS;
#endif//CONFIG_EMBEDDED_FWIMG
	}

	return rtStatus;
}


static	VOID
phy_InitBBRFRegisterDefinition(
	IN	PADAPTER		Adapter
)
{
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);	

	// RF Interface Sowrtware Control
	pHalData->PHYRegDef[ODM_RF_PATH_A].rfintfs = rFPGA0_XAB_RFInterfaceSW; // 16 LSBs if read 32-bit from 0x870
	pHalData->PHYRegDef[ODM_RF_PATH_B].rfintfs = rFPGA0_XAB_RFInterfaceSW; // 16 MSBs if read 32-bit from 0x870 (16-bit for 0x872)

	// RF Interface Output (and Enable)
	pHalData->PHYRegDef[ODM_RF_PATH_A].rfintfo = rFPGA0_XA_RFInterfaceOE; // 16 LSBs if read 32-bit from 0x860
	pHalData->PHYRegDef[ODM_RF_PATH_B].rfintfo = rFPGA0_XB_RFInterfaceOE; // 16 LSBs if read 32-bit from 0x864

	// RF Interface (Output and)  Enable
	pHalData->PHYRegDef[ODM_RF_PATH_A].rfintfe = rFPGA0_XA_RFInterfaceOE; // 16 MSBs if read 32-bit from 0x860 (16-bit for 0x862)
	pHalData->PHYRegDef[ODM_RF_PATH_B].rfintfe = rFPGA0_XB_RFInterfaceOE; // 16 MSBs if read 32-bit from 0x864 (16-bit for 0x866)

	if(IS_HARDWARE_TYPE_JAGUAR_AND_JAGUAR2(Adapter))
	{
		pHalData->PHYRegDef[ODM_RF_PATH_A].rf3wireOffset = rA_LSSIWrite_Jaguar; //LSSI Parameter
		pHalData->PHYRegDef[ODM_RF_PATH_B].rf3wireOffset = rB_LSSIWrite_Jaguar;

		pHalData->PHYRegDef[ODM_RF_PATH_A].rfHSSIPara2 = rHSSIRead_Jaguar;  //wire control parameter2
		pHalData->PHYRegDef[ODM_RF_PATH_B].rfHSSIPara2 = rHSSIRead_Jaguar;  //wire control parameter2
	}
	else
	{
		pHalData->PHYRegDef[ODM_RF_PATH_A].rf3wireOffset = rFPGA0_XA_LSSIParameter; //LSSI Parameter
		pHalData->PHYRegDef[ODM_RF_PATH_B].rf3wireOffset = rFPGA0_XB_LSSIParameter;

		pHalData->PHYRegDef[ODM_RF_PATH_A].rfHSSIPara2 = rFPGA0_XA_HSSIParameter2;  //wire control parameter2
		pHalData->PHYRegDef[ODM_RF_PATH_B].rfHSSIPara2 = rFPGA0_XB_HSSIParameter2;  //wire control parameter2
	}

	if(IS_HARDWARE_TYPE_8814A(Adapter))
	{
		pHalData->PHYRegDef[ODM_RF_PATH_C].rf3wireOffset = rC_LSSIWrite_Jaguar2; //LSSI Parameter
		pHalData->PHYRegDef[ODM_RF_PATH_D].rf3wireOffset = rD_LSSIWrite_Jaguar2;

		pHalData->PHYRegDef[ODM_RF_PATH_C].rfHSSIPara2 = rHSSIRead_Jaguar;  //wire control parameter2
		pHalData->PHYRegDef[ODM_RF_PATH_D].rfHSSIPara2 = rHSSIRead_Jaguar;  //wire control parameter2
	}
	
	if(IS_HARDWARE_TYPE_JAGUAR_AND_JAGUAR2(Adapter))
	{
		 // Tranceiver Readback LSSI/HSPI mode 
		pHalData->PHYRegDef[ODM_RF_PATH_A].rfLSSIReadBack = rA_SIRead_Jaguar;
		pHalData->PHYRegDef[ODM_RF_PATH_B].rfLSSIReadBack = rB_SIRead_Jaguar;
		pHalData->PHYRegDef[ODM_RF_PATH_A].rfLSSIReadBackPi = rA_PIRead_Jaguar;
		pHalData->PHYRegDef[ODM_RF_PATH_B].rfLSSIReadBackPi = rB_PIRead_Jaguar;
	}
	else
	{
		// Tranceiver Readback LSSI/HSPI mode 
		pHalData->PHYRegDef[ODM_RF_PATH_A].rfLSSIReadBack = rFPGA0_XA_LSSIReadBack;
		pHalData->PHYRegDef[ODM_RF_PATH_B].rfLSSIReadBack = rFPGA0_XB_LSSIReadBack;
		pHalData->PHYRegDef[ODM_RF_PATH_A].rfLSSIReadBackPi = TransceiverA_HSPI_Readback;
		pHalData->PHYRegDef[ODM_RF_PATH_B].rfLSSIReadBackPi = TransceiverB_HSPI_Readback;
	}

	if(IS_HARDWARE_TYPE_8814A(Adapter))
	{
		// Tranceiver Readback LSSI/HSPI mode 
		pHalData->PHYRegDef[ODM_RF_PATH_C].rfLSSIReadBack = rC_SIRead_Jaguar2;
		pHalData->PHYRegDef[ODM_RF_PATH_D].rfLSSIReadBack = rD_SIRead_Jaguar2;
		pHalData->PHYRegDef[ODM_RF_PATH_C].rfLSSIReadBackPi = rC_PIRead_Jaguar2;
		pHalData->PHYRegDef[ODM_RF_PATH_D].rfLSSIReadBackPi = rD_PIRead_Jaguar2;
	}

	//pHalData->bPhyValueInitReady=TRUE;
}


int
PHY_BBConfig8814(
	IN	PADAPTER	Adapter
	)
{
	int	rtStatus = _SUCCESS;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	u8	TmpU1B=0;

	phy_InitBBRFRegisterDefinition(Adapter);

    // . APLL_EN,,APLL_320_GATEB,APLL_320BIAS,  auto config by hw fsm after pfsm_go (0x4 bit 8) set
	TmpU1B = PlatformEFIORead1Byte(Adapter, REG_SYS_FUNC_EN_8814A);

	if(IS_HARDWARE_TYPE_8814AU(Adapter))
		TmpU1B |= FEN_USBA;
	else if(IS_HARDWARE_TYPE_8814AE(Adapter))
		TmpU1B |= FEN_PCIEA;

	PlatformEFIOWrite1Byte(Adapter, REG_SYS_FUNC_EN, TmpU1B);

	TmpU1B = PlatformEFIORead1Byte(Adapter, 0x1002);
	PlatformEFIOWrite1Byte(Adapter, 0x1002, (TmpU1B|FEN_BB_GLB_RSTn|FEN_BBRSTB));//same with 8812

	//6. 0x1f[7:0] = 0x07 PathA RF Power On
	PlatformEFIOWrite1Byte(Adapter, REG_RF_CTRL0_8814A , 0x07);//RF_SDMRSTB,RF_RSTB,RF_EN same with 8723a
	//7. 0x20[7:0] = 0x07 PathB RF Power On
	//8. 0x21[7:0] = 0x07 PathC RF Power On
	PlatformEFIOWrite2Byte(Adapter, REG_RF_CTRL1_8814A , 0x0707);//RF_SDMRSTB,RF_RSTB,RF_EN same with 8723a    
	//9. 0x76[7:0] = 0x07 PathD RF Power On
	PlatformEFIOWrite1Byte(Adapter, REG_RF_CTRL3_8814A , 0x7);

	//
	// Config BB and AGC
	//
	rtStatus = phy_BB8814A_Config_ParaFile(Adapter);

	hal_set_crystal_cap(Adapter, pHalData->CrystalCap);
	
	switch (Adapter->registrypriv.rf_config) {
	case RF_1T1R:
	case RF_2T4R:
	case RF_3T3R:
		/*RX CCK disable 2R CCA*/
		PHY_SetBBReg(Adapter, rCCK0_FalseAlarmReport+2, BIT2|BIT6, 0);
		/*pathB tx on, path A/C/D tx off*/
		PHY_SetBBReg(Adapter, rCCK_RX_Jaguar, 0xf0000000, 0x4);
		/*pathB rx*/
		PHY_SetBBReg(Adapter, rCCK_RX_Jaguar, 0x0f000000, 0x5);
		break;
	default:
		/*RX CCK disable 2R CCA*/
		PHY_SetBBReg(Adapter, rCCK0_FalseAlarmReport+2, BIT2|BIT6, 0);
		/*pathB tx on, path A/C/D tx off*/
		PHY_SetBBReg(Adapter, rCCK_RX_Jaguar, 0xf0000000, 0x4);
		/*pathB rx*/
		PHY_SetBBReg(Adapter, rCCK_RX_Jaguar, 0x0f000000, 0x5);
		DBG_871X("%s, unknown rf_config: %d\n", __func__, Adapter->registrypriv.rf_config);
		break;
	}
	
	return rtStatus;	
}

s32
phy_BB8814A_Config_ParaFile(
	IN	PADAPTER	Adapter
	)
{
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);
	int			rtStatus = _SUCCESS;

	s8				sz8814ABBRegFile[] = RTL8814A_PHY_REG;	
	s8				sz8814AAGCTableFile[] = RTL8814A_AGC_TAB;
	s8				sz8814ABBRegPgFile[] = RTL8814A_PHY_REG_PG;
	s8				sz8814RFTxPwrLmtFile[] = RTL8814A_TXPWR_LMT;
	s8				*pszBBRegFile = NULL, *pszAGCTableFile = NULL, 
						*pszBBRegPgFile = NULL, *pszRFTxPwrLmtFile = NULL, *pszBBRegMpFile=NULL;
	
	//DBG_871X("==>phy_BB8814A_Config_ParaFile\n");

	pszBBRegFile=sz8814ABBRegFile ;
	pszAGCTableFile =sz8814AAGCTableFile;
	pszBBRegPgFile = sz8814ABBRegPgFile;
	pszRFTxPwrLmtFile = sz8814RFTxPwrLmtFile;

	DBG_871X( "EEEPROMRegulatory %d\n", pHalData->EEPROMRegulatory );
	//RT_TRACE( COMP_INIT, DBG_LOUD, ( "Custom tx power limit file: %s\n", pMgntInfo->RegPwrLimitFile.Octet ) );
	//RT_TRACE( COMP_INIT, DBG_LOUD, ( "Custom power by rate file: %s\n", pMgntInfo->RegPwrByRateFile.Octet ) );

	//DBG_871X(" ===> phy_BB8814A_Config_ParaFile() phy_reg:%s\n",pszBBRegFile);
	//DBG_871X(" ===> phy_BB8814A_Config_ParaFile() phy_reg_pg:%s\n",pszBBRegPgFile);
	//DBG_871X(" ===> phy_BB8814A_Config_ParaFile() agc_table:%s\n",pszAGCTableFile);


	// Read Tx Power Limit 
	PHY_InitTxPowerLimit( Adapter );
	if ( Adapter->registrypriv.RegEnableTxPowerLimit == 1 || 
	     ( Adapter->registrypriv.RegEnableTxPowerLimit == 2 && pHalData->EEPROMRegulatory == 1 ) )
	{
#ifdef CONFIG_LOAD_PHY_PARA_FROM_FILE
		if (PHY_ConfigRFWithPowerLimitTableParaFile( Adapter, pszRFTxPwrLmtFile )== _FAIL)
#endif
		{
#ifdef CONFIG_EMBEDDED_FWIMG
			if (HAL_STATUS_SUCCESS != ODM_ConfigRFWithHeaderFile(&pHalData->odmpriv, CONFIG_RF_TXPWR_LMT, (ODM_RF_RADIO_PATH_E)0))
				rtStatus = _FAIL;
#endif
		}

		if(rtStatus != _SUCCESS){
			DBG_871X("%s():Read Tx power limit fail\n",__FUNCTION__	);
			goto phy_BB_Config_ParaFile_Fail;
		}
	}

	// Read PHY_REG.TXT BB INIT!!
#ifdef CONFIG_LOAD_PHY_PARA_FROM_FILE
	if (phy_ConfigBBWithParaFile(Adapter, pszBBRegFile, CONFIG_BB_PHY_REG) == _FAIL)
#endif
	{
#ifdef CONFIG_EMBEDDED_FWIMG
		if (HAL_STATUS_SUCCESS != ODM_ConfigBBWithHeaderFile(&pHalData->odmpriv, CONFIG_BB_PHY_REG))
			rtStatus = _FAIL;
#endif
	}

	if(rtStatus != _SUCCESS){
		DBG_871X("%s(): CONFIG_BB_PHY_REG Fail!!\n",__FUNCTION__	);
		goto phy_BB_Config_ParaFile_Fail;
	}

	// Read PHY_REG_MP.TXT BB INIT!!
#if (MP_DRIVER == 1)
	if (Adapter->registrypriv.mp_mode == 1) {
#ifdef CONFIG_LOAD_PHY_PARA_FROM_FILE
		if (phy_ConfigBBWithMpParaFile(Adapter, pszBBRegMpFile) == _FAIL)
#endif
		{
#ifdef CONFIG_EMBEDDED_FWIMG
			if (HAL_STATUS_SUCCESS != ODM_ConfigBBWithHeaderFile(&pHalData->odmpriv, CONFIG_BB_PHY_REG_MP))
				rtStatus = _FAIL;
#endif
		}

		if(rtStatus != _SUCCESS){
			DBG_871X("phy_BB8812_Config_ParaFile():Write BB Reg MP Fail!!\n");
			goto phy_BB_Config_ParaFile_Fail;
		}
	}
#endif	// #if (MP_DRIVER == 1)

	// If EEPROM or EFUSE autoload OK, We must config by PHY_REG_PG.txt
	PHY_InitTxPowerByRate( Adapter );
	if ( Adapter->registrypriv.RegEnableTxPowerByRate == 1 || 
	     ( Adapter->registrypriv.RegEnableTxPowerByRate == 2 && pHalData->EEPROMRegulatory != 2 ) )
	{
#ifdef CONFIG_LOAD_PHY_PARA_FROM_FILE
		if (phy_ConfigBBWithPgParaFile(Adapter, pszBBRegPgFile) == _FAIL)
#endif
		{
#ifdef CONFIG_EMBEDDED_FWIMG
			if (HAL_STATUS_SUCCESS != ODM_ConfigBBWithHeaderFile(&pHalData->odmpriv, CONFIG_BB_PHY_REG_PG))
				rtStatus = _FAIL;
#endif
		}

		if ( pHalData->odmpriv.PhyRegPgValueType == PHY_REG_PG_EXACT_VALUE )
			PHY_TxPowerByRateConfiguration( Adapter );

		if ( Adapter->registrypriv.RegEnableTxPowerLimit == 1 || 
	         ( Adapter->registrypriv.RegEnableTxPowerLimit == 2 && pHalData->EEPROMRegulatory == 1 ) )
			PHY_ConvertTxPowerLimitToPowerIndex( Adapter );

		if(rtStatus != _SUCCESS){
			DBG_871X("%s(): CONFIG_BB_PHY_REG_PG Fail!!\n",__FUNCTION__	);
			goto phy_BB_Config_ParaFile_Fail;
		}
	}

	// BB AGC table Initialization
#ifdef CONFIG_LOAD_PHY_PARA_FROM_FILE
	if (phy_ConfigBBWithParaFile(Adapter, pszAGCTableFile, CONFIG_BB_AGC_TAB) == _FAIL)
#endif
	{
#ifdef CONFIG_EMBEDDED_FWIMG
		if (HAL_STATUS_SUCCESS != ODM_ConfigBBWithHeaderFile(&pHalData->odmpriv, CONFIG_BB_AGC_TAB))
			rtStatus = _FAIL;
#endif
	}

	if(rtStatus != _SUCCESS){
		DBG_871X("%s(): CONFIG_BB_AGC_TAB Fail!!\n",__FUNCTION__	);
	}

phy_BB_Config_ParaFile_Fail:

	return rtStatus;

}


VOID
phy_ADC_CLK_8814A(
	IN	PADAPTER	Adapter
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	u32			MAC_REG_520, BB_REG_8FC, BB_REG_808, RXIQC[4];
	u32			Search_index = 0, MAC_Active = 1;
	u32			RXIQC_REG[2][4] = {{0xc10, 0xe10, 0x1810, 0x1a10}, {0xc14, 0xe14, 0x1814, 0x1a14}} ;

	if (GET_CVID_CUT_VERSION(pHalData->VersionID) != A_CUT_VERSION)
		return;

//1 Step1. MAC TX pause
	MAC_REG_520 = PHY_QueryBBReg( Adapter, 0x520, bMaskDWord);
	BB_REG_8FC = PHY_QueryBBReg( Adapter, 0x8fc, bMaskDWord);
	BB_REG_808 = PHY_QueryBBReg( Adapter, 0x808, bMaskDWord);
	PHY_SetBBReg(Adapter, 0x520, bMaskByte2, 0x3f);
	
//1 Step 2. Backup RXIQC & RXIQC = 0
	for(Search_index = 0; Search_index<4; Search_index++){
		RXIQC[Search_index] = PHY_QueryBBReg( Adapter, RXIQC_REG[0][Search_index], bMaskDWord);
		PHY_SetBBReg(Adapter, RXIQC_REG[0][Search_index], bMaskDWord, 0x0);
		PHY_SetBBReg(Adapter, RXIQC_REG[1][Search_index], bMaskDWord, 0x0);
	}
	PHY_SetBBReg(Adapter, 0xa14, 0x00000300, 0x3);
	Search_index = 0;
	
//1 Step 3. Monitor MAC IDLE
	PHY_SetBBReg(Adapter, 0x8fc, bMaskDWord, 0x0);
	while(MAC_Active){
		MAC_Active = PHY_QueryBBReg( Adapter, 0xfa0, bMaskDWord) & (0x803e0008);
		Search_index++;
		if(Search_index>1000){
			break;
		}
	}

//1 Step 4. ADC clk flow
	PHY_SetBBReg(Adapter, 0x808, bMaskByte0, 0x11);
	PHY_SetBBReg(Adapter, 0x90c, BIT(13), 0x1);
	PHY_SetBBReg(Adapter, 0x764, BIT(10)|BIT(9), 0x3);
	PHY_SetBBReg(Adapter, 0x804, BIT(2), 0x1);

	// 0xc1c/0xe1c/0x181c/0x1a1c[4] must=1 to ensure table can be written when bbrstb=0         
	// 0xc60/0xe60/0x1860/0x1a60[15] always = 1 after this line              
	// 0xc60/0xe60/0x1860/0x1a60[14] always = 0 bcz its error in A-cut          

	// power_off/clk_off @ anapar_state=idle mode
	PHY_SetBBReg(Adapter, 0xc60, bMaskDWord, 0x15800002);	//0xc60       0x15808002    
	PHY_SetBBReg(Adapter, 0xc60, bMaskDWord, 0x01808003);	//0xc60       0x01808003
	PHY_SetBBReg(Adapter, 0xe60, bMaskDWord, 0x15800002);	//0xe60      0x15808002    
	PHY_SetBBReg(Adapter, 0xe60, bMaskDWord, 0x01808003);	//0xe60      0x01808003    
	PHY_SetBBReg(Adapter, 0x1860, bMaskDWord, 0x15800002);	//0x1860    0x15808002    
	PHY_SetBBReg(Adapter, 0x1860, bMaskDWord, 0x01808003);	//0x1860    0x01808003    
	PHY_SetBBReg(Adapter, 0x1a60, bMaskDWord, 0x15800002);	//0x1a60    0x15808002        
	PHY_SetBBReg(Adapter, 0x1a60, bMaskDWord, 0x01808003);	//0x1a60    0x01808003     
 
	PHY_SetBBReg(Adapter, 0x764, BIT(10), 0x0);
	PHY_SetBBReg(Adapter, 0x804, BIT(2), 0x0);
	PHY_SetBBReg(Adapter, 0xc5c, bMaskDWord,  0x0D080058);	//0xc5c       0x00080058  // [19] =1 to turn off ADC  
	PHY_SetBBReg(Adapter, 0xe5c, bMaskDWord,  0x0D080058);	//0xe5c       0x00080058  // [19] =1 to turn off ADC     
	PHY_SetBBReg(Adapter, 0x185c, bMaskDWord,  0x0D080058);	//0x185c     0x00080058 // [19] =1 to turn off ADC    
	PHY_SetBBReg(Adapter, 0x1a5c, bMaskDWord,  0x0D080058);	//0x1a5c     0x00080058 // [19] =1 to turn off ADC       
 
	// power_on/clk_off 
	//PHY_SetBBReg(Adapter, 0x764, BIT(10), 0x1);
	PHY_SetBBReg(Adapter, 0xc5c, bMaskDWord,  0x0D000058);	//0xc5c       0x0D000058   // [19] =0 to turn on ADC    
	PHY_SetBBReg(Adapter, 0xe5c, bMaskDWord,  0x0D000058);	//0xe5c       0x0D000058  // [19] =0 to turn on ADC     
	PHY_SetBBReg(Adapter, 0x185c, bMaskDWord,  0x0D000058);	//0x185c     0x0D000058  // [19] =0 to turn on ADC     
	PHY_SetBBReg(Adapter, 0x1a5c, bMaskDWord,  0x0D000058);	//0x1a5c     0x0D000058 // [19] =0 to turn on ADC       

	// power_on/clk_on @ anapar_state=BT mode
	PHY_SetBBReg(Adapter, 0xc60, bMaskDWord, 0x05808032);	//0xc60 0x05808002 
	PHY_SetBBReg(Adapter, 0xe60, bMaskDWord, 0x05808032);	//0xe60 0x05808002           
	PHY_SetBBReg(Adapter, 0x1860, bMaskDWord, 0x05808032);	//0x1860 0x05808002    
	PHY_SetBBReg(Adapter, 0x1a60, bMaskDWord, 0x05808032);	//0x1a60 0x05808002            
       PHY_SetBBReg(Adapter, 0x764, BIT(10), 0x1);
	PHY_SetBBReg(Adapter, 0x804, BIT(2), 0x1);

	// recover original setting @ anapar_state=BT mode                              
	PHY_SetBBReg(Adapter, 0xc60, bMaskDWord, 0x05808032);	//0xc60       0x05808036    
	PHY_SetBBReg(Adapter, 0xe60, bMaskDWord, 0x05808032);	//0xe60      0x05808036    
	PHY_SetBBReg(Adapter, 0x1860, bMaskDWord, 0x05808032);	//0x1860    0x05808036    
	PHY_SetBBReg(Adapter, 0x1a60, bMaskDWord, 0x05808032);	//0x1a60    0x05808036        

	PHY_SetBBReg(Adapter, 0xc60, bMaskDWord, 0x05800002);	//0xc60       0x05800002    
	PHY_SetBBReg(Adapter, 0xc60, bMaskDWord, 0x07808003);	//0xc60       0x07808003
	PHY_SetBBReg(Adapter, 0xe60, bMaskDWord, 0x05800002);	//0xe60      0x05800002    
	PHY_SetBBReg(Adapter, 0xe60, bMaskDWord, 0x07808003);	//0xe60      0x07808003    
	PHY_SetBBReg(Adapter, 0x1860, bMaskDWord, 0x05800002);	//0x1860    0x05800002    
	PHY_SetBBReg(Adapter, 0x1860, bMaskDWord, 0x07808003);	//0x1860    0x07808003    
	PHY_SetBBReg(Adapter, 0x1a60, bMaskDWord, 0x05800002);	//0x1a60    0x05800002        
	PHY_SetBBReg(Adapter, 0x1a60, bMaskDWord, 0x07808003);	//0x1a60    0x07808003     

	PHY_SetBBReg(Adapter, 0x764, BIT(10)|BIT(9), 0x0);
	PHY_SetBBReg(Adapter, 0x804, BIT(2), 0x0);
	PHY_SetBBReg(Adapter, 0x90c, BIT(13), 0x0);

//1 Step 5. Recover MAC TX & IQC
	PHY_SetBBReg(Adapter, 0x520, bMaskDWord, MAC_REG_520);
	PHY_SetBBReg(Adapter, 0x8fc, bMaskDWord, BB_REG_8FC);
	PHY_SetBBReg(Adapter, 0x808, bMaskDWord, BB_REG_808);
	for(Search_index = 0; Search_index<4; Search_index++){
		PHY_SetBBReg(Adapter, RXIQC_REG[0][Search_index], bMaskDWord, RXIQC[Search_index]);
		PHY_SetBBReg(Adapter, RXIQC_REG[1][Search_index], bMaskDWord, 0x01000000);
	}
	PHY_SetBBReg(Adapter, 0xa14, 0x00000300, 0x0);
}

VOID
PHY_ConfigBB_8814A(
	IN	PADAPTER	Adapter
	)
{

	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);

	RT_TRACE(COMP_INIT, DBG_LOUD, (" ===> PHY_ConfigBB_8814A() \n"));
	PHY_SetBBReg(Adapter, rOFDMCCKEN_Jaguar, bOFDMEN_Jaguar|bCCKEN_Jaguar, 0x3);
}



//2 3.3 RF Config

s32
PHY_RFConfig8814A(
	IN	PADAPTER	Adapter
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	int	rtStatus = _SUCCESS;

	//vivi added this, 20100610
	if (rtw_is_surprise_removed(Adapter))
		return _FAIL;

	switch(pHalData->rf_chip)
	{
		case RF_PSEUDO_11N:
			DBG_871X("%s(): RF_PSEUDO_11N\n",__FUNCTION__);
			break;
		default: 
			rtStatus = PHY_RF6052_Config_8814A(Adapter);
			break;
	}

	return rtStatus;
}

#if (MP_DRIVER == 1)

RT_STATUS PHY_BBConfigMP_8814A(PADAPTER	Adapter)
{

	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);
 	RT_STATUS			rtStatus = RT_STATUS_FAILURE;
	s1Byte				sz8814ABBRegMpFile[] = RTL8814A_PHY_REG_MP;
	ps1Byte 			pszBBRegMpFile=NULL;

	pszBBRegMpFile = sz8814ABBRegMpFile;

	DBG_871X(" ===> PHY_BBConfigMP_8814A() phy_reg_mp:%s\n",pszBBRegMpFile);
	
	//3 Read PHY_REG_MP.TXT BB INIT!!
//#if	LOAD_PHY_PARA_FROM_HEADER
#ifdef CONFIG_EMBEDDED_FWIMG
	ODM_ConfigBBWithHeaderFile(&pHalData->odmpriv, CONFIG_BB_PHY_REG_MP);
	rtStatus = RT_STATUS_SUCCESS;
#else
	rtStatus = phy_ConfigBBWithMpParaFile(Adapter,pszBBRegMpFile);
#endif

	return rtStatus;
}

#endif


//1 4. RF State setting API

/*  todo
#if (DEV_BUS_TYPE == RT_PCI_INTERFACE)

//
// 2009/11/03 MH add for LPS mode power save sequence.
// 2009/11/03 According to document V10.
// 2009/11/24 According to document V11. by tynli.
//
VOID
phy_SetRTL8814ERfOn(
	IN	PADAPTER			Adapter
)
{
	rtw_write8(Adapter, REG_SPS0_CTRL_8814A, 0x2b);

	// c.	For PCIE: SYS_FUNC_EN 0x02[7:0] = 0xE3	//enable BB TRX function
	//	For USB: SYS_FUNC_EN 0x02[7:0] = 0x17
#if (DEV_BUS_TYPE == RT_PCI_INTERFACE)
	rtw_write8(Adapter, REG_SYS_FUNC_EN_8814A, 0xE3);
#else
	rtw_write8(Adapter, REG_SYS_FUNC_EN_8814A, 0x17);
#endif

	// RF_ON_EXCEP(d~g):
	// d.	APSD_CTRL 0x600[7:0] = 0x00
	//rtw_write8(Adapter, REG_APSD_CTRL, 0x00);

	// e.	For PCIE: SYS_FUNC_EN 0x02[7:0] = 0xE2	//reset BB TRX function again
	//f.	For PCIE: SYS_FUNC_EN 0x02[7:0] = 0xE3	//enable BB TRX function
#if (DEV_BUS_TYPE == RT_PCI_INTERFACE)
	rtw_write8(Adapter, REG_SYS_FUNC_EN_8814A, 0xE2);
	rtw_write8(Adapter, REG_SYS_FUNC_EN_8814A, 0xE3);
#else
	// e.For USB: SYS_FUNC_EN 0x02[7:0] = 0x16
	rtw_write8(Adapter, REG_SYS_FUNC_EN_8814A, 0x16);
	// f. For USB: SYS_FUNC_EN 0x02[7:0] = 0x17
	rtw_write8(Adapter, REG_SYS_FUNC_EN_8814A, 0x17);
#endif

	// g.	TXPAUSE 0x522[7:0] = 0x00				//enable MAC TX queue
	rtw_write8(Adapter, REG_TXPAUSE_8814A, 0x00);
}	// phy_SetRTL8188EERfSleep


BOOLEAN
phy_SetRFPowerState_8814E(
	IN	PADAPTER			Adapter,
	IN	rt_rf_power_state	eRFPowerState
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	PMGNT_INFO		pMgntInfo = &(Adapter->MgntInfo);
	BOOLEAN			bResult = TRUE;
	u8			i, QueueID;
	PRT_POWER_SAVE_CONTROL	pPSC = GET_POWER_SAVE_CONTROL(pMgntInfo);
	
	pHalData->SetRFPowerStateInProgress = TRUE;

	switch( eRFPowerState )
	{
		//
		// SW radio on/IPS site survey call will execute all flow
		// HW radio on
		//
		case eRfOn:
			{
			#if(MUTUAL_AUTHENTICATION == 1)
				if(pHalData->MutualAuthenticationFail)
					break;
			#endif
				if((pHalData->eRFPowerState == eRfOff) && RT_IN_PS_LEVEL(Adapter, RT_RF_OFF_LEVL_HALT_NIC))
				{ // The current RF state is OFF and the RF OFF level is halting the NIC, re-initialize the NIC.
					s32 rtstatus;
					u32 InitializeCount = 0;
					do
					{	
						InitializeCount++;
						rtstatus = NicIFEnableNIC( Adapter );
					}while( (rtstatus != _SUCCESS) &&(InitializeCount <10) );
					RT_ASSERT(rtstatus == _SUCCESS,("Nic Initialize Fail\n"));
					RT_CLEAR_PS_LEVEL(Adapter, RT_RF_OFF_LEVL_HALT_NIC);
				}
				else
				{ // This is the normal case, we just turn on the RF.
					phy_SetRTL8814ERfOn(Adapter);
				}
		
				// Turn on RF we are still linked, which might happen when 
				// we quickly turn off and on HW RF. 2006.05.12, by rcnjko.
				if( pMgntInfo->bMediaConnect == TRUE )
					Adapter->HalFunc.LedControlHandler(Adapter, LED_CTL_LINK);
				else // Turn off LED if RF is not ON.
					Adapter->HalFunc.LedControlHandler(Adapter, LED_CTL_NO_LINK);
			}
			break;

		// Card Disable/SW radio off/HW radio off/IPS enter call
		case eRfOff:
			{					
				// Make sure BusyQueue is empty befor turn off RFE pwoer.
				for(QueueID = 0, i = 0; QueueID < MAX_TX_QUEUE; )
				{
					if(RTIsListEmpty(&Adapter->TcbBusyQueue[QueueID]))
					{
						QueueID++;
						continue;
					}
					else if(IsLowPowerState(Adapter))
					{
							RT_TRACE((COMP_POWER|COMP_RF), DBG_LOUD, 
							("eRf Off/Sleep: %d times TcbBusyQueue[%d] !=0 but lower power state!\n", (i+1), QueueID));
						break;
					}
					else
					{
						RT_TRACE((COMP_POWER|COMP_RF), DBG_LOUD, 
							("eRf Off/Sleep: %d times TcbBusyQueue[%d] !=0 before doze!\n", (i+1), QueueID));
							PlatformStallExecution(10);
						i++;
					}
					
					if(i >= MAX_DOZE_WAITING_TIMES_9x)
					{
						RT_TRACE((COMP_POWER|COMP_RF), DBG_WARNING, ("\n\n\n SetZebraRFPowerState8185B(): eRfOff: %d times TcbBusyQueue[%d] != 0 !!!\n\n\n", MAX_DOZE_WAITING_TIMES_9x, QueueID));
						break;
					}
				}

				if(pPSC->RegRfPsLevel & RT_RF_OFF_LEVL_HALT_NIC)
				{ // Disable all components.
					NicIFDisableNIC(Adapter);

					if(IS_HARDWARE_TYPE_8814AE(Adapter))
						NicIFEnableInterrupt(Adapter);
					RT_SET_PS_LEVEL(Adapter, RT_RF_OFF_LEVL_HALT_NIC);
				} 
				else
				{ // Normal case.
					//If Rf off reason is from IPS, Led should blink with no link, by Maddest 071015
					if(pMgntInfo->RfOffReason==RF_CHANGE_BY_IPS )
						Adapter->HalFunc.LedControlHandler(Adapter,LED_CTL_NO_LINK); 
					else // Turn off LED if RF is not ON.
						Adapter->HalFunc.LedControlHandler(Adapter, LED_CTL_POWER_OFF); 
				}
			}
			break;

		default:
		case eRfSleep:// Not used LPS is running on FW
			bResult = FALSE;
			RT_ASSERT(FALSE, ("phy_SetRFPowerState_8814E(): unknow state to set: 0x%X!!!\n", eRFPowerState));
			break;
	} 

	if(bResult)
	{
		// Update current RF state variable.
		pHalData->eRFPowerState = eRFPowerState;
	}
	
	pHalData->SetRFPowerStateInProgress = FALSE;

	return bResult;
}

#elif (DEV_BUS_TYPE == RT_USB_INTERFACE)

BOOLEAN
phy_SetRFPowerState_8814U(
	IN	PADAPTER			Adapter,
	IN	rt_rf_power_state	eRFPowerState
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	PMGNT_INFO		pMgntInfo = &(Adapter->MgntInfo);
	BOOLEAN			bResult = TRUE;
	u8			i, QueueID;
	PRT_USB_DEVICE				pDevice = GET_RT_USB_DEVICE(Adapter);
	
	if(pHalData->SetRFPowerStateInProgress == TRUE)
		return FALSE;
	
	pHalData->SetRFPowerStateInProgress = TRUE;
	RT_TRACE(COMP_INIT, DBG_LOUD, ("======> phy_SetRFPowerState_8814U .\n"));

	switch( eRFPowerState )
	{
		case eRfOn:
			if((pHalData->eRFPowerState == eRfOff) &&
				RT_IN_PS_LEVEL(Adapter, RT_RF_OFF_LEVL_HALT_NIC))
			{ // The current RF state is OFF and the RF OFF level is halting the NIC, re-initialize the NIC.
				RT_TRACE(COMP_RF, DBG_LOUD, ("======> phy_SetRFPowerState_8814U-eRfOn .\n"));			

				if(!Adapter->bInHctTest)
				{
					// 2010/09/01 MH For 92CU, we do not make sure the RF B short initialize sequence
					// So disable the different RF on/off sequence for hidden AP.
					NicIFEnableNIC(Adapter);
					RT_CLEAR_PS_LEVEL(Adapter, RT_RF_OFF_LEVL_HALT_NIC);
				}
			}
			break;
	    
		// 
		// In current solution, RFSleep=RFOff in order to save power under 802.11 power save.
		// By Bruce, 2008-01-16.
		//
		case eRfSleep:
			{
				// ToDo:
			}
			break;
			
		case eRfOff:
			// HW setting had been configured.
			// Both of these RF configures are the same, configuring twice may cause HW abnormal.
			if(pHalData->eRFPowerState == eRfSleep || pHalData->eRFPowerState== eRfOff)
				break;
			rtw_write8(Adapter, 0xf015, 0x40);  //page added for usb3 bus
			// Make sure BusyQueue is empty befor turn off RFE pwoer.
			for(QueueID = 0, i = 0; QueueID < MAX_TX_QUEUE; )
			{
				if(RTIsListEmpty(&Adapter->TcbBusyQueue[QueueID]))
				{
					QueueID++;
					continue;
				}					
				else
				{
					RT_TRACE(COMP_POWER, DBG_LOUD, ("eRf Off/Sleep: %d times TcbBusyQueue[%d] !=0 before doze!\n", (i+1), QueueID));
					PlatformSleepUs(10);
					i++;
				}
				
				if(i >= MAX_DOZE_WAITING_TIMES_9x)
				{
					RT_TRACE(COMP_POWER, DBG_LOUD, ("\n\n\n SetZebraRFPowerState8185B(): eRfOff: %d times TcbBusyQueue[%d] != 0 !!!\n\n\n", MAX_DOZE_WAITING_TIMES_9x, QueueID));
					break;
				}
			}				

			// 
			//RF Off/Sleep sequence. Designed/tested from SD4 Scott, SD1 Grent and Jonbon.
			// Added by 
			//
			//==================================================================				
			// CU will call card disable flow to set RF off, such that we call halt directly
			// and set the PS_LEVEL to HALT_NIC or we might call halt twice in N6usbHalt in some cases.
			// 2010.03.05. Added by tynli. 				
			if(pMgntInfo->RfOffReason & RF_CHANGE_BY_IPS ||
				pMgntInfo->RfOffReason & RF_CHANGE_BY_HW ||
				pMgntInfo->RfOffReason & RF_CHANGE_BY_SW)
			{	//for HW/Sw radio off and IPS flow
				//RT_TRACE(COMP_INIT, DBG_LOUD, ("======> CardDisableWithoutHWSM -eRfOff.\n"));				
				if(!Adapter->bInHctTest)
				{
					// 2010/09/01 MH For 92CU, we do not make sure the RF B short initialize sequence
					// So disable the different RF on/off sequence for hidden AP.
					NicIFDisableNIC(Adapter);
					RT_SET_PS_LEVEL(Adapter, RT_RF_OFF_LEVL_HALT_NIC);
				}
			}				
			break;

		default:
			bResult = FALSE;
			RT_ASSERT(FALSE, ("phy_SetRFPowerState_8814U(): unknow state to set: 0x%X!!!\n", eRFPowerState));
			break;
	} 
			
	if(bResult)
	{
		// Update current RF state variable.
		pHalData->eRFPowerState = eRFPowerState;
		
		switch(pHalData->rf_chip )
		{
			default:		
			switch(pHalData->eRFPowerState)
			{
				case eRfOff:
					//
					//If Rf off reason is from IPS, Led should blink with no link, by Maddest 071015
					//
					if(pMgntInfo->RfOffReason==RF_CHANGE_BY_IPS )
						Adapter->HalFunc.LedControlHandler(Adapter,LED_CTL_NO_LINK); 
					else		// Turn off LED if RF is not ON.
						Adapter->HalFunc.LedControlHandler(Adapter, LED_CTL_POWER_OFF); 
					break;
        		
				case eRfOn:
					// Turn on RF we are still linked, which might happen when 
					// we quickly turn off and on HW RF. 2006.05.12, by rcnjko.
					if( pMgntInfo->bMediaConnect == TRUE )
						Adapter->HalFunc.LedControlHandler(Adapter, LED_CTL_LINK); 
					else		// Turn off LED if RF is not ON.
						Adapter->HalFunc.LedControlHandler(Adapter, LED_CTL_NO_LINK);
					break;
        		
				default:
					// do nothing.
					break;
			}// Switch RF state

			break;
		}// Switch rf_chip
	}
	
	pHalData->SetRFPowerStateInProgress = FALSE;
	RT_TRACE(COMP_INIT, DBG_LOUD, ("<====== phy_SetRFPowerState_8814U .\n"));
	return bResult;
}

#elif DEV_BUS_TYPE == RT_SDIO_INTERFACE

BOOLEAN
phy_SetRFPowerState_8814Sdio(
	IN	PADAPTER			Adapter,
	IN	rt_rf_power_state	eRFPowerState
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	PMGNT_INFO		pMgntInfo = &(Adapter->MgntInfo);
	BOOLEAN			bResult = TRUE;
	u8			i, QueueID;
	PRT_SDIO_DEVICE				pDevice = GET_RT_SDIO_DEVICE(Adapter);
	
	if(pHalData->SetRFPowerStateInProgress == TRUE)
		return FALSE;
	
	pHalData->SetRFPowerStateInProgress = TRUE;
	RT_TRACE(COMP_INIT, DBG_LOUD, ("======> phy_SetRFPowerState_8814Sdio .\n"))
		
	switch( eRFPowerState )
	{
		case eRfOn:
			if((pHalData->eRFPowerState == eRfOff) &&
				RT_IN_PS_LEVEL(Adapter, RT_RF_OFF_LEVL_HALT_NIC))
			{ // The current RF state is OFF and the RF OFF level is halting the NIC, re-initialize the NIC.
				RT_TRACE(COMP_RF, DBG_LOUD, ("======> phy_SetRFPowerState_8814Sdio-eRfOn .\n"));			

				if(!Adapter->bInHctTest)
				{													
					// 2010/09/01 MH For 92CU, we do not make sure the RF B short initialize sequence
					// So disable the different RF on/off sequence for hidden AP.
					NicIFEnableNIC(Adapter);
					RT_CLEAR_PS_LEVEL(Adapter, RT_RF_OFF_LEVL_HALT_NIC);
				}
			}

			// 2010/08/26 MH Prevent IQK to send out packet.
			if(pHalData->bIQKInitialized )
				PHY_IQCalibrate_8814A(Adapter, TRUE);
			else
			{
				PHY_IQCalibrate_8814A(Adapter,FALSE);
				pHalData->bIQKInitialized = _TRUE;
			}
			break;
	    
		// 
		// In current solution, RFSleep=RFOff in order to save power under 802.11 power save.
		// By Bruce, 2008-01-16.
		//
		case eRfSleep:
			{
				// ToDo:
			}
			break;
			
		case eRfOff:
			// HW setting had been configured.
			// Both of these RF configures are the same, configuring twice may cause HW abnormal.
			if(pHalData->eRFPowerState == eRfSleep || pHalData->eRFPowerState== eRfOff)
				break;

			// Make sure BusyQueue is empty befor turn off RFE pwoer.
			for(QueueID = 0, i = 0; QueueID < MAX_TX_QUEUE; )
			{
				if(RTIsListEmpty(&Adapter->TcbBusyQueue[QueueID]))
				{
					//DbgPrint("QueueID = %d", QueueID);
					QueueID++;
					continue;
				}					
				else
				{
					RT_TRACE(COMP_POWER, DBG_LOUD, ("eRf Off/Sleep: %d times TcbBusyQueue[%d] !=0 before doze!\n", (i+1), QueueID));
					PlatformSleepUs(10);
					i++;
				}
				
				if(i >= MAX_DOZE_WAITING_TIMES_9x)
				{
					RT_TRACE(COMP_POWER, DBG_LOUD, ("\n\n\n SetZebraRFPowerState8185B(): eRfOff: %d times TcbBusyQueue[%d] != 0 !!!\n\n\n", MAX_DOZE_WAITING_TIMES_9x, QueueID));
					break;
				}
			}				

			// 
			//RF Off/Sleep sequence. Designed/tested from SD4 Scott, SD1 Grent and Jonbon.
			// Added by 
			//
			//==================================================================				
			// CU will call card disable flow to set RF off, such that we call halt directly
			// and set the PS_LEVEL to HALT_NIC or we might call halt twice in N6usbHalt in some cases.
			// 2010.03.05. Added by tynli. 				
			if(pMgntInfo->RfOffReason & RF_CHANGE_BY_IPS ||
				pMgntInfo->RfOffReason & RF_CHANGE_BY_HW ||
				pMgntInfo->RfOffReason & RF_CHANGE_BY_SW)
			{	//for HW/Sw radio off and IPS flow
				//RT_TRACE(COMP_INIT, DBG_LOUD, ("======> CardDisableWithoutHWSM -eRfOff.\n"));				
				if(!Adapter->bInHctTest)
				{											
					// 2010/09/01 MH For 92CU, we do not make sure the RF B short initialize sequence
					// So disable the different RF on/off sequence for hidden AP.
					NicIFDisableNIC(Adapter);		
					
					RT_SET_PS_LEVEL(Adapter, RT_RF_OFF_LEVL_HALT_NIC);
				}
			}				
			break;

		default:
			bResult = FALSE;
			RT_ASSERT(FALSE, ("phy_SetRFPowerState_8814Sdio(): unknow state to set: 0x%X!!!\n", eRFPowerState));
			break;
	} 
			
	if(bResult)
	{
		// Update current RF state variable.
		pHalData->eRFPowerState = eRFPowerState;
		
		switch(pHalData->rf_chip )
		{
			default:		
			switch(pHalData->eRFPowerState)
			{
				case eRfOff:
					//
					//If Rf off reason is from IPS, Led should blink with no link, by Maddest 071015
					//
					if(pMgntInfo->RfOffReason==RF_CHANGE_BY_IPS )
						Adapter->HalFunc.LedControlHandler(Adapter,LED_CTL_NO_LINK); 
					else		// Turn off LED if RF is not ON.
						Adapter->HalFunc.LedControlHandler(Adapter, LED_CTL_POWER_OFF); 
					break;
        		
				case eRfOn:
					// Turn on RF we are still linked, which might happen when 
					// we quickly turn off and on HW RF. 2006.05.12, by rcnjko.
					if( pMgntInfo->bMediaConnect == TRUE )
						Adapter->HalFunc.LedControlHandler(Adapter, LED_CTL_LINK); 
					else		// Turn off LED if RF is not ON.
						Adapter->HalFunc.LedControlHandler(Adapter, LED_CTL_NO_LINK); 
					break;
        		
				default:
					// do nothing.
					break;
			}// Switch RF state

				break;
		}// Switch rf_chip
	}
	
	pHalData->SetRFPowerStateInProgress = FALSE;

	return bResult;
}

#endif



BOOLEAN
PHY_SetRFPowerState8814A(
	IN	PADAPTER			Adapter, 
	IN	rt_rf_power_state	eRFPowerState
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	BOOLEAN			bResult = FALSE;

	RT_TRACE(COMP_RF, DBG_LOUD, ("---------> PHY_SetRFPowerState8814(): eRFPowerState(%d)\n", eRFPowerState));
	if(eRFPowerState == pHalData->eRFPowerState)
	{
		RT_TRACE(COMP_RF, DBG_LOUD, ("<--------- PHY_SetRFPowerState8814(): discard the request for eRFPowerState(%d) is the same.\n", eRFPowerState));
		return bResult;
	}
#if (DEV_BUS_TYPE == RT_PCI_INTERFACE)
	bResult = phy_SetRFPowerState_8814E(Adapter, eRFPowerState);
#elif (DEV_BUS_TYPE == RT_USB_INTERFACE)
	bResult = phy_SetRFPowerState_8814U(Adapter, eRFPowerState);
#elif (DEV_BUS_TYPE == RT_SDIO_INTERFACE)
	bResult = phy_SetRFPowerState_8814Sdio(Adapter, eRFPowerState);
#endif
		
	RT_TRACE(COMP_RF, DBG_LOUD, ("<--------- PHY_SetRFPowerState8814(): bResult(%d)\n", bResult));

	return bResult;
}
todo */
//1 5. Tx  Power setting API


VOID
PHY_GetTxPowerLevel8814(
	IN	PADAPTER		Adapter,
	OUT ps4Byte    		powerlevel
	)
{
#if 0
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	PMGNT_INFO		pMgntInfo = &(Adapter->MgntInfo);
	s4Byte			TxPwrDbm = 13;

	if ( pMgntInfo->ClientConfigPwrInDbm != UNSPECIFIED_PWR_DBM )
		*powerlevel = pMgntInfo->ClientConfigPwrInDbm;
	else
		*powerlevel = TxPwrDbm;
#endif //0
}

VOID
PHY_SetTxPowerLevel8814(
	IN	PADAPTER		Adapter,
	IN	u8			Channel
	)
{
	u32			i, j, k = 0;
	u32			value[264]={0};
	u32			path = 0, PowerIndex, txagc_table_wd = 0x00801000;

	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);

	u8	jaguar2Rates[][4] =	{ {MGN_1M, MGN_2M, MGN_5_5M, MGN_11M}, 
								{MGN_6M, MGN_9M, MGN_12M, MGN_18M}, 
						  		{MGN_24M, MGN_36M, MGN_48M, MGN_54M}, 
						  		{MGN_MCS0, MGN_MCS1, MGN_MCS2, MGN_MCS3},
						  		{MGN_MCS4, MGN_MCS5, MGN_MCS6, MGN_MCS7}, 
						  		{MGN_MCS8, MGN_MCS9, MGN_MCS10, MGN_MCS11},
						   		{MGN_MCS12, MGN_MCS13, MGN_MCS14, MGN_MCS15},
						   		{MGN_MCS16, MGN_MCS17, MGN_MCS18, MGN_MCS19}, 
								{MGN_MCS20, MGN_MCS21, MGN_MCS22, MGN_MCS23},
						   		{MGN_VHT1SS_MCS0, MGN_VHT1SS_MCS1, MGN_VHT1SS_MCS2, MGN_VHT1SS_MCS3}, 
						   		{MGN_VHT1SS_MCS4, MGN_VHT1SS_MCS5, MGN_VHT1SS_MCS6, MGN_VHT1SS_MCS7}, 
						 		{MGN_VHT2SS_MCS8, MGN_VHT2SS_MCS9, MGN_VHT2SS_MCS0, MGN_VHT2SS_MCS1}, 
						 		{MGN_VHT2SS_MCS2, MGN_VHT2SS_MCS3, MGN_VHT2SS_MCS4, MGN_VHT2SS_MCS5}, 
								{MGN_VHT2SS_MCS6, MGN_VHT2SS_MCS7, MGN_VHT2SS_MCS8, MGN_VHT2SS_MCS9},
						   		{MGN_VHT3SS_MCS0, MGN_VHT3SS_MCS1, MGN_VHT3SS_MCS2, MGN_VHT3SS_MCS3}, 
						   		{MGN_VHT3SS_MCS4, MGN_VHT3SS_MCS5, MGN_VHT3SS_MCS6, MGN_VHT3SS_MCS7}, 
						 		{MGN_VHT3SS_MCS8, MGN_VHT3SS_MCS9, 0, 0}};	

	
	for( path = ODM_RF_PATH_A; path <= ODM_RF_PATH_D; ++path )
	{
			PHY_SetTxPowerLevelByPath(Adapter, Channel, (u8)path);
	}
#if 0 //todo H2C_TXPOWER_INDEX_OFFLOAD ?
	if(Adapter->MgntInfo.bScanInProgress == FALSE &&  pHalData->RegFWOffload == 2)
	{
		HalDownloadTxPowerLevel8814(Adapter, value);
	}
#endif //0
}

/**************************************************************************************************************
 *   Description: 
 *       The low-level interface to get the FINAL Tx Power Index , called  by both MP and Normal Driver.
 *
 *                                                                                    <20120830, Kordan>
 **************************************************************************************************************/
u8
PHY_GetTxPowerIndex_8814A(
	IN	PADAPTER			pAdapter,
	IN	u8				RFPath,
	IN	u8				Rate,	
	IN	CHANNEL_WIDTH		BandWidth,	
	IN	u8				Channel
	)
{
	PHAL_DATA_TYPE		pHalData = GET_HAL_DATA(pAdapter);
	s8				powerDiffByRate = 0;
	s8				txPower = 0, limit = 0;
	u8				tx_num = MgntQuery_NssTxRate(Rate );
	BOOLEAN				bIn24G = FALSE;

	/* DBG_871X( "===>%s\n", __FUNCTION__ ); */
	
	txPower = (s8) PHY_GetTxPowerIndexBase( pAdapter, RFPath, Rate, BandWidth, Channel, &bIn24G );

	powerDiffByRate = PHY_GetTxPowerByRate( pAdapter, (u8)(!bIn24G), RFPath, tx_num, Rate );

	limit = PHY_GetTxPowerLimit( pAdapter, pAdapter->registrypriv.RegPwrTblSel, (u8)(!bIn24G), pHalData->CurrentChannelBW, RFPath, Rate, pHalData->CurrentChannel);

	powerDiffByRate = powerDiffByRate > limit ? limit : powerDiffByRate;
	/* DBG_871X("Rate-0x%x: (TxPower, PowerDiffByRate Path-%c) = (0x%X, %d)\n", Rate, ((RFPath==0)?'A':(RFPath==1)?'B':(RFPath==2)?'C':'D'), txPower, powerDiffByRate); */

	txPower += powerDiffByRate;
	
	//txPower += PHY_GetTxPowerTrackingOffset( pAdapter, RFPath, Rate );
#if 0 //todo ?
#if CCX_SUPPORT
	CCX_CellPowerLimit( pAdapter, Channel, Rate, &txPower );
#endif
#endif
	if(txPower > MAX_POWER_INDEX)
		txPower = MAX_POWER_INDEX;

	//if (Adapter->registrypriv.mp_mode==0 && 
		//(pHalData->bautoload_fail_flag || pHalData->EfuseMap[EFUSE_INIT_MAP][EEPROM_TX_PWR_INX_JAGUAR] == 0xFF))
		//txPower = 0x12;

	/* DBG_871X("Final Tx Power(RF-%c, Channel: %d) = %d(0x%X)\n", ((RFPath==0)?'A':(RFPath==1)?'B':(RFPath==2)?'C':'D'), Channel,
		txPower, txPower); */

	return (u8) txPower;	
}


VOID
PHY_SetTxPowerIndex_8814A(
	IN	PADAPTER			Adapter,
	IN	u32				PowerIndex,
	IN	u8				RFPath,	
	IN	u8				Rate
	)
{
	u32	txagc_table_wd = 0x00801000;

	txagc_table_wd |= (RFPath << 8) | MRateToHwRate(Rate) | (PowerIndex << 24);
	PHY_SetBBReg(Adapter, 0x1998, bMaskDWord, txagc_table_wd);
	/* DBG_871X("txagc_table_wd %x\n", txagc_table_wd); */
	if (Rate == MGN_1M) {
		PHY_SetBBReg(Adapter, 0x1998, bMaskDWord, txagc_table_wd);	/* first time to turn on the txagc table */
										/* second to write the addr0 */
	}
}


BOOLEAN
PHY_UpdateTxPowerDbm8814A(
	IN	PADAPTER	Adapter,
	IN	s4Byte		powerInDbm
	)
{
	return TRUE;
}


u32 
PHY_GetTxBBSwing_8814A(
	IN	PADAPTER	Adapter,
	IN	BAND_TYPE 	Band,
	IN 	u8 		RFPath
	)
{
    HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(GetDefaultAdapter(Adapter));
    PDM_ODM_T		pDM_Odm = &pHalData->odmpriv;
    PODM_RF_CAL_T  	pRFCalibrateInfo = &(pDM_Odm->RFCalibrateInfo);
    s8 			bbSwing_2G = -1 * GetRegTxBBSwing_2G(Adapter);
    s8 			bbSwing_5G = -1 * GetRegTxBBSwing_5G(Adapter);
    u32          		out = 0x200;
    const s8		AUTO = -1;

	RT_TRACE(COMP_MP, DBG_LOUD, ("===> PHY_GetTxBBSwing_8814A, bbSwing_2G: %d, bbSwing_5G: %d\n", 
										  (s4Byte)bbSwing_2G, (s4Byte)bbSwing_5G));

    if ( pHalData->bautoload_fail_flag )
    {
		if ( Band == BAND_ON_2_4G ) 
		{
			pRFCalibrateInfo->BBSwingDiff2G = bbSwing_2G;
			if      (bbSwing_2G == 0)  out = 0x200; //  0 dB
			else if (bbSwing_2G == -3) out = 0x16A; // -3 dB
			else if (bbSwing_2G == -6) out = 0x101; // -6 dB
			else if (bbSwing_2G == -9) out = 0x0B6; // -9 dB
			else 
			{
				if ( pHalData->ExternalPA_2G ) 
				{
					pRFCalibrateInfo->BBSwingDiff2G = -3;
					out = 0x16A;
				} 
				else  
				{
					pRFCalibrateInfo->BBSwingDiff2G = 0;
					out = 0x200;
				}
	            }
	        } 
		else if ( Band == BAND_ON_5G ) 
		{
			pRFCalibrateInfo->BBSwingDiff5G = bbSwing_5G;
			if(bbSwing_5G == 0)  out = 0x200; //  0 dB
			else if (bbSwing_5G == -3) out = 0x16A; // -3 dB
			else if (bbSwing_5G == -6) out = 0x101; // -6 dB
			else if (bbSwing_5G == -9) out = 0x0B6; // -9 dB
			else 
			{
				if (pHalData->ExternalPA_5G) 
				{
					pRFCalibrateInfo->BBSwingDiff5G = -3;
					out = 0x16A;
				} 
				else 
				{
					pRFCalibrateInfo->BBSwingDiff5G = 0;
					out = 0x200;
				}
			}
		}
		else  
		{
			pRFCalibrateInfo->BBSwingDiff2G = -3;
			pRFCalibrateInfo->BBSwingDiff5G = -3;			
			out = 0x16A; // -3 dB
		}
	}
	else
	{
		u32 swing = 0, onePathSwing = 0;

		if (Band == BAND_ON_2_4G)
		{
			if (GetRegTxBBSwing_2G(Adapter) == AUTO)
			{
				EFUSE_ShadowRead(Adapter, 1, EEPROM_TX_BBSWING_2G_8814, (u32 *)&swing);
				if (swing == 0xFF) 
				{
					if(bbSwing_2G ==  0) swing = 0x00; //  0 dB
					else if (bbSwing_2G == -3) swing = 0x55; // -3 dB
					else if (bbSwing_2G == -6) swing = 0xAA; // -6 dB
					else if (bbSwing_2G == -9) swing = 0xFF; // -9 dB
					else swing = 0x00;					
				}
			}			
			else if (bbSwing_2G ==  0) swing = 0x00; //  0 dB
			else if (bbSwing_2G == -3) swing = 0x55; // -3 dB
			else if (bbSwing_2G == -6) swing = 0xAA; // -6 dB
			else if (bbSwing_2G == -9) swing = 0xFF; // -9 dB
			else swing = 0x00;
		}
		else
		{
			if (GetRegTxBBSwing_5G(Adapter) == AUTO)
			{
				EFUSE_ShadowRead(Adapter, 1, EEPROM_TX_BBSWING_5G_8814, (u32 *)&swing);
				if (swing == 0xFF) 
				{
					if(bbSwing_5G ==  0) swing = 0x00; //  0 dB
					else if (bbSwing_5G == -3) swing = 0x55; // -3 dB
					else if (bbSwing_5G == -6) swing = 0xAA; // -6 dB
					else if (bbSwing_5G == -9) swing = 0xFF; // -9 dB
					else swing = 0x00;
				}
			}
			else if (bbSwing_5G ==  0) swing = 0x00; //  0 dB
      			else if (bbSwing_5G == -3) swing = 0x55; // -3 dB
			else if (bbSwing_5G == -6) swing = 0xAA; // -6 dB
			else if (bbSwing_5G == -9) swing = 0xFF; // -9 dB
			else swing = 0x00;
		}

		if (RFPath == ODM_RF_PATH_A)
			onePathSwing = (swing & 0x3) >> 0; // 0xC6/C7[1:0]
		else if(RFPath == ODM_RF_PATH_B)		
			onePathSwing = (swing & 0xC) >> 2; // 0xC6/C7[3:2]
		else if(RFPath == ODM_RF_PATH_C)
			onePathSwing = (swing & 0x30) >> 4; // 0xC6/C7[5:4]
		else if(RFPath == ODM_RF_PATH_D)
			onePathSwing = (swing & 0xC0) >> 6; // 0xC6/C7[7:6]

		if (onePathSwing == 0x0) 
		{
			if (Band == BAND_ON_2_4G) 
				pRFCalibrateInfo->BBSwingDiff2G = 0;
			else
				pRFCalibrateInfo->BBSwingDiff5G = 0;
			out = 0x200; // 0 dB
		} 
		else if (onePathSwing == 0x1) 
		{
			if (Band == BAND_ON_2_4G) 
				pRFCalibrateInfo->BBSwingDiff2G = -3;
			else
				pRFCalibrateInfo->BBSwingDiff5G = -3;
			out = 0x16A; // -3 dB
		} 
		else if (onePathSwing == 0x2) 
		{
			if (Band == BAND_ON_2_4G) 
				pRFCalibrateInfo->BBSwingDiff2G = -6;
			else
				pRFCalibrateInfo->BBSwingDiff5G = -6;
			out = 0x101; // -6 dB
		} 
		else if (onePathSwing == 0x3) 
		{
			if (Band == BAND_ON_2_4G) 
				pRFCalibrateInfo->BBSwingDiff2G = -9;
			else
				pRFCalibrateInfo->BBSwingDiff5G = -9;
			out = 0x0B6; // -9 dB
		}
	}
	RT_TRACE(COMP_MP, DBG_LOUD,("<=== PHY_GetTxBBSwing_8814A, out = 0x%X\n", out));
	return out;
}


//1 7. BandWidth setting API

VOID
phy_SetBwRegAdc_8814A(
	IN	PADAPTER		Adapter,
	IN	u8			Band,
	IN	CHANNEL_WIDTH 	CurrentBW
)	
{
	switch(CurrentBW)
	{
		case CHANNEL_WIDTH_20:
			if(Band == BAND_ON_5G)
			{
				PHY_SetBBReg(Adapter, rRFMOD_Jaguar, BIT(1)|BIT(0), 0x0); 	// 0x8ac[28, 21,20,16, 9:6,1,0]=10'b10_0011_0000
			}
			else
			{
				PHY_SetBBReg(Adapter, rRFMOD_Jaguar, BIT(1)|BIT(0), 0x0); 	// 0x8ac[28, 21,20,16, 9:6,1,0]=10'b10_0101_0000
			}
			break;

		case CHANNEL_WIDTH_40:
			if(Band == BAND_ON_5G)
			{
				PHY_SetBBReg(Adapter, rRFMOD_Jaguar, BIT(1)|BIT(0), 0x1);		// 0x8ac[17, 11, 10, 7:6,1,0]=7'b100_0001
			}
			else
			{
				PHY_SetBBReg(Adapter, rRFMOD_Jaguar, BIT(1)|BIT(0), 0x1);		// 0x8ac[17, 11, 10, 7:6,1,0]=7'b101_0001
			}
			break;

		case CHANNEL_WIDTH_80:
			PHY_SetBBReg(Adapter, rRFMOD_Jaguar, BIT(1)|BIT(0), 0x02);				// 0x8ac[7:6,1,0]=4'b0010
			break;

		default:
			RT_DISP(FPHY, PHY_BBW, ("phy_SetBwRegAdc_8814A():	unknown Bandwidth: %#X\n",CurrentBW));
			break;
	}
}


VOID
phy_SetBwRegAgc_8814A(
	IN	PADAPTER		Adapter,
	IN	u8			Band,
	IN	CHANNEL_WIDTH 	CurrentBW
)	
{
	u32 AgcValue = 7;
	switch(CurrentBW)
	{
		case CHANNEL_WIDTH_20:
			if(Band == BAND_ON_5G)
				AgcValue = 6;
			else
				AgcValue = 6;
			break;

		case CHANNEL_WIDTH_40:
			if(Band == BAND_ON_5G)
				AgcValue = 8;
			else
				AgcValue = 7;
			break;

		case CHANNEL_WIDTH_80:
			AgcValue = 3;
			break;

		default:
			RT_DISP(FPHY, PHY_BBW, ("phy_SetBwRegAgc_8814A():	unknown Bandwidth: %#X\n",CurrentBW));
			break;
	}

	PHY_SetBBReg(Adapter, rAGC_table_Jaguar, 0xf000, AgcValue);	// 0x82C[15:12] = AgcValue				
}


BOOLEAN
phy_SwBand8814A(
	IN	PADAPTER	pAdapter,
	IN	u8			channelToSW)
{
	u8			u1Btmp; 
	BOOLEAN		ret_value = _TRUE;
	u8			Band = BAND_ON_5G, BandToSW;

	u1Btmp = rtw_read8(pAdapter, REG_CCK_CHECK_8814A);
	if(u1Btmp & BIT7)
		Band = BAND_ON_5G;
	else
		Band = BAND_ON_2_4G;

	// Use current channel to judge Band Type and switch Band if need.
	if(channelToSW > 14)
	{
		BandToSW = BAND_ON_5G;
	}
	else
	{
		BandToSW = BAND_ON_2_4G;
	}

	if(BandToSW != Band)
	{
		PHY_SwitchWirelessBand8814A(pAdapter,BandToSW);
	}
		
	return ret_value;
}


VOID
PHY_SetRFEReg8814A(
	IN PADAPTER		Adapter,
	IN BOOLEAN		bInit,
	IN u8		Band
)
{
	u8			u1tmp = 0;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);

	if(bInit)
	{	
		switch(pHalData->RFEType){
		case 2:case 1:
			PHY_SetBBReg(Adapter, 0x1994, 0xf, 0xf);								// 0x1994[3:0] = 0xf
			u1tmp = PlatformEFIORead1Byte(Adapter, REG_GPIO_IO_SEL_8814A);
			rtw_write8(Adapter, REG_GPIO_IO_SEL_8814A, u1tmp | 0xf0);	// 0x40[23:20] = 0xf
			break;
		case 0:
			PHY_SetBBReg(Adapter, 0x1994, 0xf, 0xf);								// 0x1994[3:0] = 0xf
			u1tmp = PlatformEFIORead1Byte(Adapter, REG_GPIO_IO_SEL_8814A);
			rtw_write8(Adapter, REG_GPIO_IO_SEL_8814A, u1tmp | 0xc0);	// 0x40[23:22] = 2b'11
			break;
		}
	}
	else if(Band == BAND_ON_2_4G)
	{
		switch(pHalData->RFEType){
			case 2:
			PHY_SetBBReg(Adapter, rA_RFE_Pinmux_Jaguar, bMaskDWord, 0x72707270);	// 0xCB0 = 0x72707270
			PHY_SetBBReg(Adapter, rB_RFE_Pinmux_Jaguar, bMaskDWord, 0x72707270);	// 0xEB0 = 0x72707270
			PHY_SetBBReg(Adapter, rC_RFE_Pinmux_Jaguar, bMaskDWord, 0x72707270);	// 0x18B4 = 0x72707270
			PHY_SetBBReg(Adapter, rD_RFE_Pinmux_Jaguar, bMaskDWord, 0x77707770);	// 0x1AB4 = 0x77707770
			PHY_SetBBReg(Adapter, 0x1ABC, 0x0ff00000, 0x72);						// 0x1ABC[27:20] = 0x72
			break;

		case 1:
			PHY_SetBBReg(Adapter, rA_RFE_Pinmux_Jaguar, bMaskDWord, 0x77777777);	// 0xCB0 = 0x77777777
			PHY_SetBBReg(Adapter, rB_RFE_Pinmux_Jaguar, bMaskDWord, 0x77777777);	// 0xEB0 = 0x77777777
			PHY_SetBBReg(Adapter, rC_RFE_Pinmux_Jaguar, bMaskDWord, 0x77777777);	// 0x18B4 = 0x77777777
			PHY_SetBBReg(Adapter, rD_RFE_Pinmux_Jaguar, bMaskDWord, 0x77777777);	// 0x1AB4 = 0x77777777
			PHY_SetBBReg(Adapter, 0x1ABC, 0x0ff00000, 0x77);						// 0x1ABC[27:20] = 0x77
			break;

		case 0:
		default:
			PHY_SetBBReg(Adapter, rA_RFE_Pinmux_Jaguar, bMaskDWord, 0x77777777);	// 0xCB0 = 0x77777777
			PHY_SetBBReg(Adapter, rB_RFE_Pinmux_Jaguar, bMaskDWord, 0x77777777);	// 0xEB0 = 0x77777777
			PHY_SetBBReg(Adapter, rC_RFE_Pinmux_Jaguar, bMaskDWord, 0x77777777);	// 0x18B4 = 0x77777777
			PHY_SetBBReg(Adapter, 0x1ABC, 0x0ff00000, 0x77);						// 0x1ABC[27:20] = 0x77
			break;
			
		}
	}
	else
	{
		switch(pHalData->RFEType){
		case 2:
			PHY_SetBBReg(Adapter, rA_RFE_Pinmux_Jaguar, bMaskDWord, 0x33173717);	// 0xCB0 = 0x33173717
			PHY_SetBBReg(Adapter, rB_RFE_Pinmux_Jaguar, bMaskDWord, 0x33173717);	// 0xEB0 = 0x33173717
			PHY_SetBBReg(Adapter, rC_RFE_Pinmux_Jaguar, bMaskDWord, 0x33173717);	// 0x18B4 = 0x33173717
			PHY_SetBBReg(Adapter, rD_RFE_Pinmux_Jaguar, bMaskDWord, 0x77177717);	// 0x1AB4 = 0x77177717
			PHY_SetBBReg(Adapter, 0x1ABC, 0x0ff00000, 0x37);						// 0x1ABC[27:20] = 0x37
			break;
			
		case 1:
			PHY_SetBBReg(Adapter, rA_RFE_Pinmux_Jaguar, bMaskDWord, 0x33173317);	// 0xCB0 = 0x33173317
			PHY_SetBBReg(Adapter, rB_RFE_Pinmux_Jaguar, bMaskDWord, 0x33173317);	// 0xEB0 = 0x33173317
			PHY_SetBBReg(Adapter, rC_RFE_Pinmux_Jaguar, bMaskDWord, 0x33173317);	// 0x18B4 = 0x33173317
			PHY_SetBBReg(Adapter, rD_RFE_Pinmux_Jaguar, bMaskDWord, 0x77177717);	// 0x1AB4 = 0x77177717
			PHY_SetBBReg(Adapter, 0x1ABC, 0x0ff00000, 0x33);						// 0x1ABC[27:20] = 0x33
			break;

		case 0:
		default:
			PHY_SetBBReg(Adapter, rA_RFE_Pinmux_Jaguar, bMaskDWord, 0x54775477);	// 0xCB0 = 0x54775477
			PHY_SetBBReg(Adapter, rB_RFE_Pinmux_Jaguar, bMaskDWord, 0x54775477);	// 0xEB0 = 0x54775477
			PHY_SetBBReg(Adapter, rC_RFE_Pinmux_Jaguar, bMaskDWord, 0x54775477);	// 0x18B4 = 0x54775477
			PHY_SetBBReg(Adapter, rD_RFE_Pinmux_Jaguar, bMaskDWord, 0x54775477);	// 0x1AB4 = 0x54775477
			PHY_SetBBReg(Adapter, 0x1ABC, 0x0ff00000, 0x54);						// 0x1ABC[27:20] = 0x54
			break;
		}
	}
}

VOID 
phy_SetBBSwingByBand_8814A(
	IN PADAPTER		Adapter,
	IN u8		Band,
	IN u8		PreviousBand
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);

	s8 			BBDiffBetweenBand = 0; 		
	PDM_ODM_T		pDM_Odm = &pHalData->odmpriv;
	PODM_RF_CAL_T  	pRFCalibrateInfo = &(pDM_Odm->RFCalibrateInfo);
	
	PHY_SetBBReg(Adapter, rA_TxScale_Jaguar, 0xFFE00000, 
				 PHY_GetTxBBSwing_8814A(Adapter, (BAND_TYPE)Band, ODM_RF_PATH_A)); // 0xC1C[31:21]
	PHY_SetBBReg(Adapter, rB_TxScale_Jaguar, 0xFFE00000, 
				 PHY_GetTxBBSwing_8814A(Adapter, (BAND_TYPE)Band, ODM_RF_PATH_B)); // 0xE1C[31:21]
	PHY_SetBBReg(Adapter, rC_TxScale_Jaguar2, 0xFFE00000, 
				 PHY_GetTxBBSwing_8814A(Adapter, (BAND_TYPE)Band, ODM_RF_PATH_C)); // 0x181C[31:21]
	PHY_SetBBReg(Adapter, rD_TxScale_Jaguar2, 0xFFE00000, 
				 PHY_GetTxBBSwing_8814A(Adapter, (BAND_TYPE)Band, ODM_RF_PATH_D)); // 0x1A1C[31:21]
				 
	// <20121005, Kordan> When TxPowerTrack is ON, we should take care of the change of BB swing.
	// That is, reset all info to trigger Tx power tracking.
		
	if (Band != PreviousBand) 
	{
		BBDiffBetweenBand = (pRFCalibrateInfo->BBSwingDiff2G - pRFCalibrateInfo->BBSwingDiff5G);
		BBDiffBetweenBand = (Band == BAND_ON_2_4G) ? BBDiffBetweenBand : (-1 * BBDiffBetweenBand);
		pRFCalibrateInfo->DefaultOfdmIndex += BBDiffBetweenBand*2;				
	}
	
	ODM_ClearTxPowerTrackingState(pDM_Odm);
}


s32
PHY_SwitchWirelessBand8814A(
	IN PADAPTER		Adapter,
	IN u8		Band
)
{
	HAL_DATA_TYPE	*pHalData	= GET_HAL_DATA(Adapter);
	u8	PreBand = pHalData->CurrentBandType, tepReg = 0;
			
	DBG_871X("==>PHY_SwitchWirelessBand8814() %s\n", ((Band==0)?"2.4G":"5G"));

	pHalData->CurrentBandType =(BAND_TYPE)Band;

	/*clear 0x1000[16],	When this bit is set to 0, CCK and OFDM are disabled, and clock are gated. Otherwise, CCK and OFDM are enabled. */
	tepReg = rtw_read8(Adapter, REG_SYS_CFG3_8814A+2);
	rtw_write8(Adapter, REG_SYS_CFG3_8814A+2, tepReg & (~BIT0));
	
	// STOP Tx/Rx
	//PHY_SetBBReg(Adapter, rOFDMCCKEN_Jaguar, bOFDMEN_Jaguar|bCCKEN_Jaguar, 0x00);	

	if(Band == BAND_ON_2_4G)
	{// 2.4G band

		// AGC table select 
		PHY_SetBBReg(Adapter, rAGC_table_Jaguar2, 0x1F, 0);									// 0x958[4:0] = 5b'00000

		PHY_SetRFEReg8814A(Adapter, FALSE, Band);

		// cck_enable
		//PHY_SetBBReg(Adapter, rOFDMCCKEN_Jaguar, bOFDMEN_Jaguar|bCCKEN_Jaguar, 0x3);

		if(Adapter->registrypriv.mp_mode == 0)
		{
			// 0x80C & 0xa04 should use same antenna.
			PHY_SetBBReg(Adapter, rTxPath_Jaguar, 0xf0, 0x2);
			PHY_SetBBReg(Adapter, rCCK_RX_Jaguar, 0x0f000000, 0x5);
		}

		PHY_SetBBReg(Adapter, rOFDMCCKEN_Jaguar, bOFDMEN_Jaguar|bCCKEN_Jaguar, 0x3);

		
		// CCK_CHECK_en
		rtw_write8(Adapter, REG_CCK_CHECK_8814A, 0x0);
		/* after 5G swicth 2G , set A82[2] = 0 */
		PHY_SetBBReg(Adapter, 0xa80, BIT18, 0x0);
			
	}
	else	//5G band
	{		
		// CCK_CHECK_en
		rtw_write8(Adapter, REG_CCK_CHECK_8814A, 0x80);
		/* Enable CCK Tx function, even when CCK is off */
		PHY_SetBBReg(Adapter, 0xa80, BIT18, 0x1);
	
		// AGC table select 
		// Postpone to channel switch
		//PHY_SetBBReg(Adapter, rAGC_table_Jaguar2, 0x1F, 1);									// 0x958[4:0] = 5b'00001

		PHY_SetRFEReg8814A(Adapter, FALSE, Band);

		if(Adapter->registrypriv.mp_mode == 0)
		{
			PHY_SetBBReg(Adapter, rTxPath_Jaguar, 0xf0, 0x0);
			PHY_SetBBReg(Adapter, rCCK_RX_Jaguar, 0x0f000000, 0xF);
		}

		PHY_SetBBReg(Adapter, rOFDMCCKEN_Jaguar, bOFDMEN_Jaguar|bCCKEN_Jaguar, 0x02);
		//DBG_871X("==>PHY_SwitchWirelessBand8814() BAND_ON_5G settings OFDM index 0x%x\n", pHalData->OFDM_index[0]);
	}

	phy_SetBBSwingByBand_8814A(Adapter, Band, PreBand);
	phy_SetBwRegAdc_8814A(Adapter, Band, pHalData->CurrentChannelBW);
	phy_SetBwRegAgc_8814A(Adapter, Band, pHalData->CurrentChannelBW);
	/* set 0x1000[16], When this bit is set to 0, CCK and OFDM are disabled, and clock are gated. Otherwise, CCK and OFDM are enabled.*/
	tepReg = rtw_read8(Adapter, REG_SYS_CFG3_8814A+2);
	rtw_write8(Adapter, REG_SYS_CFG3_8814A+2, tepReg | BIT0);
	
	DBG_871X("<==PHY_SwitchWirelessBand8814():Switch Band OK.\n");
	return _SUCCESS;	
}


u8 
phy_GetSecondaryChnl_8814A(
	IN	PADAPTER	Adapter
)
{
	u8						SCSettingOf40 = 0, SCSettingOf20 = 0;
	PHAL_DATA_TYPE				pHalData = GET_HAL_DATA(Adapter);

	//DBG_871X("SCMapping: Case: pHalData->CurrentChannelBW %d, pHalData->nCur80MhzPrimeSC %d, pHalData->nCur40MhzPrimeSC %d \n",pHalData->CurrentChannelBW,pHalData->nCur80MhzPrimeSC,pHalData->nCur40MhzPrimeSC);
	if(pHalData->CurrentChannelBW== CHANNEL_WIDTH_80)
	{
		if(pHalData->nCur80MhzPrimeSC == HAL_PRIME_CHNL_OFFSET_LOWER)
			SCSettingOf40 = VHT_DATA_SC_40_LOWER_OF_80MHZ;
		else if(pHalData->nCur80MhzPrimeSC == HAL_PRIME_CHNL_OFFSET_UPPER)
			SCSettingOf40 = VHT_DATA_SC_40_UPPER_OF_80MHZ;
		else
			DBG_871X("SCMapping: DONOT CARE Mode Setting\n");
		
		if((pHalData->nCur40MhzPrimeSC == HAL_PRIME_CHNL_OFFSET_LOWER) && (pHalData->nCur80MhzPrimeSC == HAL_PRIME_CHNL_OFFSET_LOWER))
			SCSettingOf20 = VHT_DATA_SC_20_LOWEST_OF_80MHZ;
		else if((pHalData->nCur40MhzPrimeSC == HAL_PRIME_CHNL_OFFSET_UPPER) && (pHalData->nCur80MhzPrimeSC == HAL_PRIME_CHNL_OFFSET_LOWER))
			SCSettingOf20 = VHT_DATA_SC_20_LOWER_OF_80MHZ;
		else if((pHalData->nCur40MhzPrimeSC == HAL_PRIME_CHNL_OFFSET_LOWER) && (pHalData->nCur80MhzPrimeSC == HAL_PRIME_CHNL_OFFSET_UPPER))
			SCSettingOf20 = VHT_DATA_SC_20_UPPER_OF_80MHZ;
		else if((pHalData->nCur40MhzPrimeSC == HAL_PRIME_CHNL_OFFSET_UPPER) && (pHalData->nCur80MhzPrimeSC == HAL_PRIME_CHNL_OFFSET_UPPER))
			SCSettingOf20 = VHT_DATA_SC_20_UPPERST_OF_80MHZ;
		else
		{
			if(pHalData->nCur80MhzPrimeSC == HAL_PRIME_CHNL_OFFSET_LOWER)
				SCSettingOf20 = VHT_DATA_SC_40_LOWER_OF_80MHZ;
			else if(pHalData->nCur80MhzPrimeSC == HAL_PRIME_CHNL_OFFSET_UPPER)
				SCSettingOf20 = VHT_DATA_SC_40_UPPER_OF_80MHZ;
			else
				DBG_871X("SCMapping: DONOT CARE Mode Setting\n");
		}	
	}
	else if(pHalData->CurrentChannelBW == CHANNEL_WIDTH_40)
	{
		DBG_871X("SCMapping: pHalData->CurrentChannelBW %d, pHalData->nCur40MhzPrimeSC %d \n",pHalData->CurrentChannelBW,pHalData->nCur40MhzPrimeSC);

		if(pHalData->nCur40MhzPrimeSC == HAL_PRIME_CHNL_OFFSET_UPPER)
			SCSettingOf20 = VHT_DATA_SC_20_UPPER_OF_80MHZ;
		else if(pHalData->nCur40MhzPrimeSC == HAL_PRIME_CHNL_OFFSET_LOWER)
			SCSettingOf20 = VHT_DATA_SC_20_LOWER_OF_80MHZ;
		else
			DBG_871X("SCMapping: DONOT CARE Mode Setting\n");
	}

	/*DBG_871X("SCMapping: SC Value %x\n", ((SCSettingOf40 << 4) | SCSettingOf20));*/
	return  ( (SCSettingOf40 << 4) | SCSettingOf20);
}


VOID
phy_SetBwRegMac_8814A(
	IN	PADAPTER		Adapter,
	CHANNEL_WIDTH 	CurrentBW
)	
{
	u16		RegRfMod_BW, u2tmp = 0;
	RegRfMod_BW = PlatformEFIORead2Byte(Adapter, REG_TRXPTCL_CTL_8814A);

	switch(CurrentBW)
	{
		case CHANNEL_WIDTH_20:
			PlatformEFIOWrite2Byte(Adapter, REG_TRXPTCL_CTL_8814A, (RegRfMod_BW & 0xFE7F)); // BIT 7 = 0, BIT 8 = 0
			break;

		case CHANNEL_WIDTH_40:
			u2tmp = RegRfMod_BW | BIT7;
			PlatformEFIOWrite2Byte(Adapter, REG_TRXPTCL_CTL_8814A, (u2tmp & 0xFEFF)); // BIT 7 = 1, BIT 8 = 0
			break;

		case CHANNEL_WIDTH_80:
			u2tmp = RegRfMod_BW | BIT8;
			PlatformEFIOWrite2Byte(Adapter, REG_TRXPTCL_CTL_8814A, (u2tmp & 0xFF7F)); // BIT 7 = 0, BIT 8 = 1
			break;

		default:
			RT_DISP(FPHY, PHY_BBW, ("phy_SetBwRegMac_8814A():	unknown Bandwidth: %#X\n",CurrentBW));
			break;
	}
}

void PHY_Set_SecCCATH_by_RXANT_8814A(PADAPTER	pAdapter,u4Byte	ulAntennaRx)
{
	PHAL_DATA_TYPE		pHalData	= GET_HAL_DATA(pAdapter);

	if ((pHalData->bSWToBW40M == TRUE) && (pHalData->CurrentChannelBW != CHANNEL_WIDTH_40)) {
		PHY_SetBBReg(pAdapter, rPwed_TH_Jaguar, 0x007c0000,pHalData->BackUp_BB_REG_4_2nd_CCA[0]);
		PHY_SetBBReg(pAdapter, rCCAonSec_Jaguar, 0x0000ff00,pHalData->BackUp_BB_REG_4_2nd_CCA[1]);
		PHY_SetBBReg(pAdapter, r_L1_SBD_start_time, 0x0f000000,pHalData->BackUp_BB_REG_4_2nd_CCA[2]);
		pHalData->bSWToBW40M = FALSE;
	}

	if ((pHalData->bSWToBW80M == TRUE) && (pHalData->CurrentChannelBW != CHANNEL_WIDTH_80)) {
		PHY_SetBBReg(pAdapter, r_L1_SBD_start_time, 0x0f000000, pHalData->BackUp_BB_REG_4_2nd_CCA[2]);
		pHalData->bSWToBW80M = FALSE;
	}

	/*1 Setting CCA TH 2nd CCA parameter by Rx Antenna*/
	if (pHalData->CurrentChannelBW == CHANNEL_WIDTH_80) {
		if (pHalData->bSWToBW80M == FALSE) {
			pHalData->BackUp_BB_REG_4_2nd_CCA[2] = PHY_QueryBBReg(pAdapter, r_L1_SBD_start_time, 0x0f000000);
		}

		pHalData->bSWToBW80M = TRUE;

		switch (ulAntennaRx) {
		case ANTENNA_A:
		case ANTENNA_B:
		case ANTENNA_C:
		case ANTENNA_D:
				PHY_SetBBReg(pAdapter, r_L1_SBD_start_time, 0x0f000000,0x0b);/* 0x844[27:24] = 0xb */
				PHY_SetBBReg(pAdapter, rCCAonSec_Jaguar, 0x00000001, 0x1); /* 0x838 Enable 2ndCCA */
				PHY_SetBBReg(pAdapter, rAGC_table_Jaguar, 0x00FF0000, 0x89); /* 0x82C[23:20] = 8, PWDB_TH_QB, 0x82C[19:16] = 9, PWDB_TH_HB*/
				PHY_SetBBReg(pAdapter, rCCAonSec_Jaguar, 0x0FFF0000, 0x887); /* 838[27:24]=8, RF80_secondary40, 838[23:20]=8, RF80_secondary20, 838[19:16]=7, RF80_primary*/
				PHY_SetBBReg(pAdapter, rL1_Weight_Jaguar, 0x0000F000, 0x7);	/* 840[15:12]=7, L1_square_Pk_weight_80M*/
		break;

		case ANTENNA_AB:
		case ANTENNA_AC:
		case ANTENNA_AD:	
		case ANTENNA_BC:
		case ANTENNA_BD:
		case ANTENNA_CD:
				PHY_SetBBReg(pAdapter, r_L1_SBD_start_time, 0x0f000000,0x0d);
				PHY_SetBBReg(pAdapter, rCCAonSec_Jaguar, 0x00000001, 0x1); /* Enable 2ndCCA*/
				PHY_SetBBReg(pAdapter, rAGC_table_Jaguar, 0x00FF0000, 0x78); /* 0x82C[23:20] = 7, PWDB_TH_QB, 0x82C[19:16] = 8, PWDB_TH_HB*/
				PHY_SetBBReg(pAdapter, rCCAonSec_Jaguar, 0x0FFF0000, 0x444); /* 838[27:24]=4, RF80_secondary40, 838[23:20]=4, RF80_secondary20, 838[19:16]=4, RF80_primary*/
				PHY_SetBBReg(pAdapter, rL1_Weight_Jaguar, 0x0000F000, 0x6); /* 840[15:12]=6, L1_square_Pk_weight_80M*/
		break;

		case ANTENNA_ABC:
		case ANTENNA_ABD:
		case ANTENNA_ACD:
		case ANTENNA_BCD:
				PHY_SetBBReg(pAdapter, r_L1_SBD_start_time, 0x0f000000,0x0d);
				PHY_SetBBReg(pAdapter, rCCAonSec_Jaguar, 0x00000001, 0x1); /* Enable 2ndCCA*/
				PHY_SetBBReg(pAdapter, rAGC_table_Jaguar, 0x00FF0000, 0x98); /* 0x82C[23:20] = 9, PWDB_TH_QB, 0x82C[19:16] = 8, PWDB_TH_HB*/
				PHY_SetBBReg(pAdapter, rCCAonSec_Jaguar, 0x0FFF0000, 0x666); /* 838[27:24]=6, RF80_secondary40, 838[23:20]=6, RF80_secondary20, 838[19:16]=6, RF80_primary*/
				PHY_SetBBReg(pAdapter, rL1_Weight_Jaguar, 0x0000F000, 0x6); /* 840[15:12]=6, L1_square_Pk_weight_80M*/
		break;

		case ANTENNA_ABCD:
				PHY_SetBBReg(pAdapter, r_L1_SBD_start_time, 0x0f000000,0x0d);
				PHY_SetBBReg(pAdapter, rCCAonSec_Jaguar, 0x00000001, 0x1); /*Enable 2ndCCA*/
				PHY_SetBBReg(pAdapter, rAGC_table_Jaguar, 0x00FF0000, 0x98); /* 0x82C[23:20] = 9, PWDB_TH_QB, 0x82C[19:16] = 8, PWDB_TH_HB*/
				PHY_SetBBReg(pAdapter, rCCAonSec_Jaguar, 0x0FFF0000, 0x666); /* 838[27:24]=6, RF80_secondary40, 838[23:20]=6, RF80_secondary20, 838[19:16]=6, RF80_primary*/
				PHY_SetBBReg(pAdapter, rL1_Weight_Jaguar, 0x0000F000, 0x7); /*840[15:12]=7, L1_square_Pk_weight_80M*/
		break;

		default:
				DBG_871X("Unknown Rx antenna.\n");
		break;
		}
	} else if(pHalData->CurrentChannelBW == CHANNEL_WIDTH_40) {
		if (pHalData->bSWToBW40M == FALSE) {
			pHalData->BackUp_BB_REG_4_2nd_CCA[0] = PHY_QueryBBReg(pAdapter, rPwed_TH_Jaguar, 0x007c0000);
			pHalData->BackUp_BB_REG_4_2nd_CCA[1] = PHY_QueryBBReg(pAdapter, rCCAonSec_Jaguar, 0x0000ff00);
			pHalData->BackUp_BB_REG_4_2nd_CCA[2] = PHY_QueryBBReg(pAdapter, r_L1_SBD_start_time, 0x0f000000);
		}

		switch (ulAntennaRx) {
		case ANTENNA_A:  /* xT1R*/
		case ANTENNA_B:
		case ANTENNA_C:
		case ANTENNA_D:
						PHY_SetBBReg(pAdapter, r_L1_SBD_start_time, 0x0f000000,0x0b);
						PHY_SetBBReg(pAdapter, rPwed_TH_Jaguar, 0x007c0000, 0xe);
						PHY_SetBBReg(pAdapter, rCCAonSec_Jaguar, 0x0000ff00, 0x43);
						PHY_SetBBReg(pAdapter, rCCAonSec_Jaguar, 0x00000001, 0x1);
						break;
		case ANTENNA_AB: /* xT2R*/
		case ANTENNA_AC:
		case ANTENNA_AD:
		case ANTENNA_BC:
		case ANTENNA_BD:
		case ANTENNA_CD:
						PHY_SetBBReg(pAdapter, r_L1_SBD_start_time, 0x0f000000,0x0d);
						PHY_SetBBReg(pAdapter, rPwed_TH_Jaguar, 0x007c0000, 0x8);
						PHY_SetBBReg(pAdapter, rCCAonSec_Jaguar, 0x0000ff00, 0x43);
						PHY_SetBBReg(pAdapter, rCCAonSec_Jaguar, 0x00000001, 0x1);
						break;
		case ANTENNA_ABC: /* xT3R*/
		case ANTENNA_ABD:
		case ANTENNA_ACD:
		case ANTENNA_BCD:
		case ANTENNA_ABCD:  /* xT4R*/
						PHY_SetBBReg(pAdapter, r_L1_SBD_start_time, 0x0f000000,0x0d);
						PHY_SetBBReg(pAdapter, rPwed_TH_Jaguar, 0x007c0000, 0xa);
						PHY_SetBBReg(pAdapter, rCCAonSec_Jaguar, 0x0000ff00, 0x43);
						PHY_SetBBReg(pAdapter, rCCAonSec_Jaguar, 0x00000001, 0x1);
						break;
		default:
						break;
		}
		pHalData->bSWToBW40M = TRUE;
	} else {
		PHY_SetBBReg(pAdapter, rCCAonSec_Jaguar, 0x00000001, 0x0); /* Enable 2ndCCA*/
		PHY_SetBBReg(pAdapter, rAGC_table_Jaguar, 0x00FF0000, 0x43); /* 0x82C[23:20] = 9, PWDB_TH_QB, 0x82C[19:16] = 8, PWDB_TH_HB*/
		PHY_SetBBReg(pAdapter, rCCAonSec_Jaguar, 0x0FFF0000, 0x7aa); /* 838[27:24]=6, RF80_secondary40, 838[23:20]=6, RF80_secondary20, 838[19:16]=6, RF80_primary*/
		PHY_SetBBReg(pAdapter, rL1_Weight_Jaguar, 0x0000F000, 0x7); /* 840[15:12]=7, L1_square_Pk_weight_80M*/
	}
	
}


VOID PHY_SetRXSC_by_TXSC_8814A(PADAPTER	Adapter, u1Byte SubChnlNum)	
{
	PHAL_DATA_TYPE pHalData = GET_HAL_DATA(Adapter);

	if (pHalData->CurrentChannelBW == CHANNEL_WIDTH_80) {
		if (SubChnlNum == 0)
			PHY_SetBBReg(Adapter, rRFMOD_Jaguar, 0x00000003c, 0x1);
		else if (SubChnlNum == 1)
			PHY_SetBBReg(Adapter, rRFMOD_Jaguar, 0x00000003c, 0x1);
		else if (SubChnlNum == 2)
			PHY_SetBBReg(Adapter, rRFMOD_Jaguar, 0x00000003c, 0x2);
		else if (SubChnlNum == 4)
			PHY_SetBBReg(Adapter, rRFMOD_Jaguar, 0x00000003c, 0x4);
		else if (SubChnlNum == 3)
			PHY_SetBBReg(Adapter, rRFMOD_Jaguar, 0x00000003c, 0x3);
		else if (SubChnlNum == 9)
			PHY_SetBBReg(Adapter, rRFMOD_Jaguar, 0x00000003c, 0x1);
		else if (SubChnlNum == 10)
			PHY_SetBBReg(Adapter, rRFMOD_Jaguar, 0x00000003c, 0x2);
	} else if (pHalData->CurrentChannelBW == CHANNEL_WIDTH_40) { 
		if (SubChnlNum == 1)
			PHY_SetBBReg(Adapter, rRFMOD_Jaguar, 0x00000003c, 0x1);
		else if (SubChnlNum == 2)
			PHY_SetBBReg(Adapter, rRFMOD_Jaguar, 0x00000003c, 0x2);
	} else
		PHY_SetBBReg(Adapter, rRFMOD_Jaguar, 0x00000003c, 0x0);
}


/* <20141230, James> A workaround to eliminate the 5280MHz & 5600MHz & 5760MHzspur of 8814A. (Asked by BBSD Neil.)*/
VOID phy_SpurCalibration_8814A(PADAPTER	Adapter)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);

	BOOLEAN		Reset_NBI_CSI = TRUE;
	PDM_ODM_T		pDM_Odm = &pHalData->odmpriv;

	/*DBG_871X("%s(),RFE Type =%d, CurrentCh = %d ,ChannelBW =%d\n", __func__, pHalData->RFEType, pHalData->CurrentChannel, pHalData->CurrentChannelBW);*/
	/*DBG_871X("%s(),Before RrNBI_Setting_Jaguar= %x\n", __func__, PHY_QueryBBReg(Adapter, rNBI_Setting_Jaguar, bMaskDWord));*/
	
	if (pHalData->RFEType == 0) {
		switch (pHalData->CurrentChannelBW) {
		case CHANNEL_WIDTH_40:
				if (pHalData->CurrentChannel == 54 || pHalData->CurrentChannel == 118) {
					PHY_SetBBReg(Adapter, rNBI_Setting_Jaguar, 0x000fe000, 0x3e >> 1);
					PHY_SetBBReg(Adapter, rCSI_Mask_Setting1_Jaguar, BIT(0), 1);
					PHY_SetBBReg(Adapter, rCSI_Fix_Mask0_Jaguar, bMaskDWord, 0);
					PHY_SetBBReg(Adapter, rCSI_Fix_Mask1_Jaguar, BIT(0), 1);
					PHY_SetBBReg(Adapter, rCSI_Fix_Mask6_Jaguar, bMaskDWord, 0);
					PHY_SetBBReg(Adapter, rCSI_Fix_Mask7_Jaguar, bMaskDWord, 0);
					Reset_NBI_CSI = FALSE;
				} else if (pHalData->CurrentChannel == 151) {
					PHY_SetBBReg(Adapter, rNBI_Setting_Jaguar, 0x000fe000, 0x1e >> 1);
					PHY_SetBBReg(Adapter, rCSI_Mask_Setting1_Jaguar, BIT(0), 1);
					PHY_SetBBReg(Adapter, rCSI_Fix_Mask0_Jaguar,  BIT(16), 1);
					PHY_SetBBReg(Adapter, rCSI_Fix_Mask1_Jaguar, bMaskDWord, 0);
					PHY_SetBBReg(Adapter, rCSI_Fix_Mask6_Jaguar, bMaskDWord, 0);
					PHY_SetBBReg(Adapter, rCSI_Fix_Mask7_Jaguar, bMaskDWord, 0);
					Reset_NBI_CSI = FALSE;
				}
		break;

		case CHANNEL_WIDTH_80:
				if (pHalData->CurrentChannel == 58 || pHalData->CurrentChannel == 122) {
					PHY_SetBBReg(Adapter, rNBI_Setting_Jaguar, 0x000fe000, 0x3a >> 1);
					PHY_SetBBReg(Adapter, rCSI_Mask_Setting1_Jaguar, BIT(0), 1);
					PHY_SetBBReg(Adapter, rCSI_Fix_Mask0_Jaguar, bMaskDWord, 0);
					PHY_SetBBReg(Adapter, rCSI_Fix_Mask1_Jaguar, bMaskDWord, 0);
					PHY_SetBBReg(Adapter, rCSI_Fix_Mask6_Jaguar, bMaskDWord, 0);
					PHY_SetBBReg(Adapter, rCSI_Fix_Mask7_Jaguar, BIT(0), 1);
					Reset_NBI_CSI = FALSE;
				} else if (pHalData->CurrentChannel == 155) {
					PHY_SetBBReg(Adapter, rNBI_Setting_Jaguar, 0x000fe000, 0x5a >> 1);
					PHY_SetBBReg(Adapter, rCSI_Mask_Setting1_Jaguar, BIT(0), 1);
					PHY_SetBBReg(Adapter, rCSI_Fix_Mask0_Jaguar, bMaskDWord, 0);
					PHY_SetBBReg(Adapter, rCSI_Fix_Mask1_Jaguar, bMaskDWord, 0);
					PHY_SetBBReg(Adapter, rCSI_Fix_Mask6_Jaguar, BIT(16), 1);
					PHY_SetBBReg(Adapter, rCSI_Fix_Mask7_Jaguar, bMaskDWord, 0);
					Reset_NBI_CSI = FALSE;
				}
		break;
		case CHANNEL_WIDTH_20:
				if (pHalData->CurrentChannel == 153) {
					PHY_SetBBReg(Adapter, rNBI_Setting_Jaguar, 0x000fe000, 0x1e >> 1);
					PHY_SetBBReg(Adapter, rCSI_Mask_Setting1_Jaguar, BIT(0), 1);
					PHY_SetBBReg(Adapter, rCSI_Fix_Mask0_Jaguar, bMaskDWord, 0);
					PHY_SetBBReg(Adapter, rCSI_Fix_Mask1_Jaguar, bMaskDWord, 0);
					PHY_SetBBReg(Adapter, rCSI_Fix_Mask6_Jaguar, bMaskDWord, 0);
					PHY_SetBBReg(Adapter, rCSI_Fix_Mask7_Jaguar, BIT(16), 1);
					Reset_NBI_CSI = FALSE;
					}
		break;
			
		default:
		break;
		}
	} else if (pHalData->RFEType == 1 || pHalData->RFEType == 2) {
			switch (pHalData->CurrentChannelBW) {
			case CHANNEL_WIDTH_20:
					if (pHalData->CurrentChannel == 153) {
						PHY_SetBBReg(Adapter, rNBI_Setting_Jaguar, 0x000fe000, 0x1E >> 1);
						PHY_SetBBReg(Adapter, rCSI_Mask_Setting1_Jaguar, BIT(0), 1);
						PHY_SetBBReg(Adapter, rCSI_Fix_Mask0_Jaguar, bMaskDWord, 0);
						PHY_SetBBReg(Adapter, rCSI_Fix_Mask1_Jaguar, bMaskDWord, 0);
						PHY_SetBBReg(Adapter, rCSI_Fix_Mask6_Jaguar, bMaskDWord, 0);
						PHY_SetBBReg(Adapter, rCSI_Fix_Mask7_Jaguar, BIT(16), 1);
						Reset_NBI_CSI = FALSE;
					}
			break;
			case CHANNEL_WIDTH_40:
					if (pHalData->CurrentChannel == 151) {
						PHY_SetBBReg(Adapter, rNBI_Setting_Jaguar, 0x000fe000, 0x1e >> 1);
						PHY_SetBBReg(Adapter, rCSI_Mask_Setting1_Jaguar, BIT(0), 1);
						PHY_SetBBReg(Adapter, rCSI_Fix_Mask0_Jaguar, BIT(16), 1);
						PHY_SetBBReg(Adapter, rCSI_Fix_Mask1_Jaguar, bMaskDWord, 0);
						PHY_SetBBReg(Adapter, rCSI_Fix_Mask6_Jaguar, bMaskDWord, 0);
						PHY_SetBBReg(Adapter, rCSI_Fix_Mask7_Jaguar, bMaskDWord, 0);
						Reset_NBI_CSI = FALSE;
					}
			break;
			case CHANNEL_WIDTH_80:
					if (pHalData->CurrentChannel == 155) {
						PHY_SetBBReg(Adapter, rNBI_Setting_Jaguar, 0x000fe000, 0x5a >> 1);
						PHY_SetBBReg(Adapter, rCSI_Mask_Setting1_Jaguar, BIT(0), 1);
						PHY_SetBBReg(Adapter, rCSI_Fix_Mask0_Jaguar, bMaskDWord, 0);
						PHY_SetBBReg(Adapter, rCSI_Fix_Mask1_Jaguar, bMaskDWord, 0);
						PHY_SetBBReg(Adapter, rCSI_Fix_Mask6_Jaguar, BIT(16), 1);
						PHY_SetBBReg(Adapter, rCSI_Fix_Mask7_Jaguar, bMaskDWord, 0);
						Reset_NBI_CSI = FALSE;
					}
			break;

			default:
			break;
			}
	}
	
	if (Reset_NBI_CSI) {
		PHY_SetBBReg(Adapter, rNBI_Setting_Jaguar, 0x000fe000, 0xfc >> 1);
		PHY_SetBBReg(Adapter, rCSI_Mask_Setting1_Jaguar, BIT(0), 0);
		PHY_SetBBReg(Adapter, rCSI_Fix_Mask0_Jaguar, bMaskDWord, 0);
		PHY_SetBBReg(Adapter, rCSI_Fix_Mask1_Jaguar, bMaskDWord, 0);
		PHY_SetBBReg(Adapter, rCSI_Fix_Mask6_Jaguar, bMaskDWord, 0);
		PHY_SetBBReg(Adapter, rCSI_Fix_Mask7_Jaguar, bMaskDWord, 0);
	}
	
	phydm_spur_nbi_setting_8814a(pDM_Odm);
	/*DBG_871X("%s(),After RrNBI_Setting_Jaguar= %x\n", __func__, PHY_QueryBBReg(Adapter, rNBI_Setting_Jaguar, bMaskDWord));*/
}


void phy_ModifyInitialGain_8814A(
	PADAPTER		Adapter)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	u8 			channel = pHalData->CurrentChannel;
	s1Byte		offset[4]; /*{A,B,C,D}*/
	u8			i = 0;
	u8			chnl_section = 0xff;

	if (channel <= 14 && channel > 0)
		chnl_section = 0; /*2G*/
	else if (channel <= 64 && channel >= 36)
		chnl_section = 1; /*5GL*/
	else if (channel <= 144 && channel >= 100)
		chnl_section = 2; /*5GM*/
	else if (channel <= 177 && channel >= 149)
		chnl_section = 3; /*5GH*/

	if (chnl_section > 3) {
		DBG_871X("%s: worng channel section\n", __func__);
		return;
	}

	for (i = 0; i < 4; i++) {
		u1Byte	hex_offset;

		hex_offset = (u1Byte)(pHalData->RxGainOffset[chnl_section] >> (12-4*i))&0x0f;
		DBG_871X("%s: pHalData->RxGainOffset[%d] = %x\n", __func__, chnl_section, pHalData->RxGainOffset[chnl_section]);
		DBG_871X("%s: hex_offset = %x\n", __func__, hex_offset);

		if (hex_offset == 0xf)
			offset[i] = 0;
		else if (hex_offset >= 0x8)
			offset[i] = 0x11 - hex_offset;
		else
		 	offset[i] = 0x0 - hex_offset;
		 offset[i] = (offset[i] / 2) * 2;
		 DBG_871X("%s: offset[%d] = %x\n", __func__, i, offset[i]);
		 DBG_871X("%s: BackUp_IG_REG_4_Chnl_Section[%d] = %x\n", __func__, i, pHalData->BackUp_IG_REG_4_Chnl_Section[i]);
	}

	if (pHalData->BackUp_IG_REG_4_Chnl_Section[0] != 0 &&
		pHalData->BackUp_IG_REG_4_Chnl_Section[1] != 0 &&
		pHalData->BackUp_IG_REG_4_Chnl_Section[2] != 0 &&
		pHalData->BackUp_IG_REG_4_Chnl_Section[3] != 0
		) {
		PHY_SetBBReg(Adapter, rA_IGI_Jaguar, 0x000000ff, pHalData->BackUp_IG_REG_4_Chnl_Section[0] + offset[0]);
		PHY_SetBBReg(Adapter, rB_IGI_Jaguar, 0x000000ff, pHalData->BackUp_IG_REG_4_Chnl_Section[1] + offset[1]);
		PHY_SetBBReg(Adapter, rC_IGI_Jaguar2, 0x000000ff, pHalData->BackUp_IG_REG_4_Chnl_Section[2] + offset[2]);
		PHY_SetBBReg(Adapter, rD_IGI_Jaguar2, 0x000000ff, pHalData->BackUp_IG_REG_4_Chnl_Section[3] + offset[3]);
	}
}


VOID phy_SetBwMode8814A(PADAPTER	Adapter)
{
	u8			SubChnlNum = 0;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);

	//3 Set Reg668 BW
	phy_SetBwRegMac_8814A(Adapter, pHalData->CurrentChannelBW);

	//3 Set Reg483
	SubChnlNum = phy_GetSecondaryChnl_8814A(Adapter);
	rtw_write8(Adapter, REG_DATA_SC_8814A, SubChnlNum);

	if(pHalData->rf_chip == RF_PSEUDO_11N)
	{
		DBG_871X("phy_SetBwMode8814A: return for PSEUDO \n");
		return;
	}

	//3 Set Reg8AC Reg8C4 Reg8C8
	phy_SetBwRegAdc_8814A(Adapter, pHalData->CurrentBandType, pHalData->CurrentChannelBW);
	//3 Set Reg82C
	phy_SetBwRegAgc_8814A(Adapter, pHalData->CurrentBandType, pHalData->CurrentChannelBW);

	//3 Set Reg848  RegA00
	switch(pHalData->CurrentChannelBW)
	{
		case CHANNEL_WIDTH_20:
			break;

		case CHANNEL_WIDTH_40:
			PHY_SetBBReg(Adapter, rRFMOD_Jaguar, 0x3C, SubChnlNum);			// 0x8ac[5:2]=1/2

			if(SubChnlNum == VHT_DATA_SC_20_UPPER_OF_80MHZ)					// 0xa00[4]=1/0
				PHY_SetBBReg(Adapter, rCCK_System_Jaguar, bCCK_System_Jaguar, 1);
			else
				PHY_SetBBReg(Adapter, rCCK_System_Jaguar, bCCK_System_Jaguar, 0);
			break;

		case CHANNEL_WIDTH_80:
			PHY_SetBBReg(Adapter, rRFMOD_Jaguar, 0x3C, SubChnlNum);			// 0x8ac[5:2]=1/2/3/4/9/10
			break;

		default:
			DBG_871X("%s():unknown Bandwidth:%#X\n", __func__, pHalData->CurrentChannelBW);
			break;
	}
	
#if (MP_DRIVER == 1)
if (Adapter->registrypriv.mp_mode == 1) {
	/* 2 Set Reg 0x8AC */
	PHY_SetRXSC_by_TXSC_8814A(Adapter, (SubChnlNum & 0xf));
	PHY_Set_SecCCATH_by_RXANT_8814A(Adapter, pHalData->AntennaRxPath);
}
#endif	
	/* 3 Set RF related register */
	PHY_RF6052SetBandwidth8814A(Adapter, pHalData->CurrentChannelBW);

	phy_ADC_CLK_8814A(Adapter);
	phy_SpurCalibration_8814A(Adapter);
}



//1 6. Channel setting API

// <YuChen, 140529> Add for KFree Feature Requested by RF David.
// We need support ABCD four path Kfree

VOID
phy_SetKfreeToRF_8814A(
	IN	PADAPTER			Adapter,
	IN	u8				eRFPath,
	IN	u8				Data
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(GetDefaultAdapter(Adapter));	
	PDM_ODM_T	pDM_Odm = &pHalData->odmpriv;
	BOOLEAN bOdd;
	PODM_RF_CAL_T	pRFCalibrateInfo = &(pDM_Odm->RFCalibrateInfo);
	if((Data%2) != 0)		//odd -> positive
	{
		Data = Data - 1;
		PHY_SetRFReg(Adapter, eRFPath, rRF_TxGainOffset, BIT19, 1);
		bOdd = TRUE;
	}
	else		// even -> negative
	{
		PHY_SetRFReg(Adapter, eRFPath, rRF_TxGainOffset, BIT19, 0);
		bOdd = FALSE;
	}
	RT_TRACE(COMP_MP, DBG_LOUD, ("phy_ConfigKFree8814A(): RF_0x55[19]= %d\n", bOdd));
	switch(Data)
	{
		case 2:
			PHY_SetRFReg(Adapter, eRFPath, rRF_TxGainOffset, BIT14, 1);
			pRFCalibrateInfo->KfreeOffset[eRFPath] = 0;
		break;
		case 4:
			PHY_SetRFReg(Adapter, eRFPath, rRF_TxGainOffset, BIT17|BIT16|BIT15, 1);
			pRFCalibrateInfo->KfreeOffset[eRFPath] = 1;
		break;
		case 6:
			PHY_SetRFReg(Adapter, eRFPath, rRF_TxGainOffset, BIT14, 1);
			PHY_SetRFReg(Adapter, eRFPath, rRF_TxGainOffset, BIT17|BIT16|BIT15, 1);
			pRFCalibrateInfo->KfreeOffset[eRFPath] = 1;
		break;
		case 8:
			PHY_SetRFReg(Adapter, eRFPath, rRF_TxGainOffset, BIT17|BIT16|BIT15, 2);
			pRFCalibrateInfo->KfreeOffset[eRFPath] = 2;
		break;
		case 10:
			PHY_SetRFReg(Adapter, eRFPath, rRF_TxGainOffset, BIT14, 1);
			PHY_SetRFReg(Adapter, eRFPath, rRF_TxGainOffset, BIT17|BIT16|BIT15, 2);
			pRFCalibrateInfo->KfreeOffset[eRFPath] = 2;
		break;
		case 12:
			PHY_SetRFReg(Adapter, eRFPath, rRF_TxGainOffset, BIT17|BIT16|BIT15, 3);
			pRFCalibrateInfo->KfreeOffset[eRFPath] = 3;
		break;
		case 14:
			PHY_SetRFReg(Adapter, eRFPath, rRF_TxGainOffset, BIT14, 1);
			PHY_SetRFReg(Adapter, eRFPath, rRF_TxGainOffset, BIT17|BIT16|BIT15, 3);
			pRFCalibrateInfo->KfreeOffset[eRFPath] = 3;
		break;
		case 16:
			PHY_SetRFReg(Adapter, eRFPath, rRF_TxGainOffset, BIT17|BIT16|BIT15, 4);
			pRFCalibrateInfo->KfreeOffset[eRFPath] = 4;
		break;
		case 18:
			PHY_SetRFReg(Adapter, eRFPath, rRF_TxGainOffset, BIT14, 1);
			PHY_SetRFReg(Adapter, eRFPath, rRF_TxGainOffset, BIT17|BIT16|BIT15, 4);
			pRFCalibrateInfo->KfreeOffset[eRFPath] = 4;
		break;
		case 20:
			PHY_SetRFReg(Adapter, eRFPath, rRF_TxGainOffset, BIT17|BIT16|BIT15, 5);
			pRFCalibrateInfo->KfreeOffset[eRFPath] = 5;
		break;

		default:
		break;
	}

	if(bOdd == FALSE)			// that means Kfree offset is negative, we need to record it.
	{
		pRFCalibrateInfo->KfreeOffset[eRFPath] = (-1)*pRFCalibrateInfo->KfreeOffset[eRFPath];
		RT_TRACE(COMP_MP, DBG_LOUD, ("phy_ConfigKFree8814A(): KfreeOffset = %d\n", pRFCalibrateInfo->KfreeOffset[eRFPath]));
	}
	else
		RT_TRACE(COMP_MP, DBG_LOUD, ("phy_ConfigKFree8814A(): KfreeOffset = %d\n", pRFCalibrateInfo->KfreeOffset[eRFPath]));
	
}


VOID
phy_ConfigKFree8814A(
	IN	PADAPTER	Adapter,
	IN	u8 		channelToSW,
	IN	BAND_TYPE	bandType
	)
{
	u8			targetval_A = 0xFF;
	u8			targetval_B = 0xFF;
	u8			targetval_C = 0xFF;
	u8			targetval_D = 0xFF;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	
	//DBG_871X("===>phy_ConfigKFree8814A()\n");
	
	if (Adapter->registrypriv.RegRfKFreeEnable == 2)
	{
		//DBG_871X("phy_ConfigKFree8814A(): RegRfKFreeEnable == 2, Disable \n");
		return;
	}
	else if (Adapter->registrypriv.RegRfKFreeEnable == 1 || Adapter->registrypriv.RegRfKFreeEnable == 0)
	{
		DBG_871X("phy_ConfigKFree8814A(): RegRfKFreeEnable == TRUE \n");
		if (bandType == BAND_ON_2_4G) // 2G
		{
			DBG_871X("phy_ConfigKFree8814A(): bandType == BAND_ON_2_4G, channelToSW= %d  \n", channelToSW);
			if (channelToSW <= 14 && channelToSW >= 1)
			{
				efuse_OneByteRead(Adapter, 0x3F4, &targetval_A, FALSE);	// for Path A and B
				efuse_OneByteRead(Adapter, 0x3F5, &targetval_B, FALSE);	// for Path C and D
			}
			
		}
		else if (bandType == BAND_ON_5G)
		{
			DBG_871X("phy_ConfigKFree8814A(): bandType == BAND_ON_5G, channelToSW= %d  \n", channelToSW);
			if (channelToSW >= 36 && channelToSW < 50) // 5GLB_1
			{
				efuse_OneByteRead(Adapter, 0x3E0, &targetval_A, FALSE);
				efuse_OneByteRead(Adapter, 0x3E1, &targetval_B, FALSE);
				efuse_OneByteRead(Adapter, 0x3E2, &targetval_C, FALSE);
				efuse_OneByteRead(Adapter, 0x3E3, &targetval_D, FALSE);
			}
			else if (channelToSW >= 50 && channelToSW <= 64) // 5GLB_2
			{
				efuse_OneByteRead(Adapter, 0x3E4, &targetval_A, FALSE);
				efuse_OneByteRead(Adapter, 0x3E5, &targetval_B, FALSE);
				efuse_OneByteRead(Adapter, 0x3E6, &targetval_C, FALSE);
				efuse_OneByteRead(Adapter, 0x3E7, &targetval_D, FALSE);
			}
			else if (channelToSW >= 100 && channelToSW <= 118) // 5GMB_1
			{
				efuse_OneByteRead(Adapter, 0x3E8, &targetval_A, FALSE);
				efuse_OneByteRead(Adapter, 0x3E9, &targetval_B, FALSE);
				efuse_OneByteRead(Adapter, 0x3EA, &targetval_C, FALSE);
				efuse_OneByteRead(Adapter, 0x3EB, &targetval_D, FALSE);
			}
			else if (channelToSW >= 120 && channelToSW <= 140) // 5GMB_2
			{
				efuse_OneByteRead(Adapter, 0x3EC, &targetval_A, FALSE);
				efuse_OneByteRead(Adapter, 0x3ED, &targetval_B, FALSE);
				efuse_OneByteRead(Adapter, 0x3EE, &targetval_C, FALSE);
				efuse_OneByteRead(Adapter, 0x3EF, &targetval_D, FALSE);
			}
			else if (channelToSW >= 149 && channelToSW <= 165) // 5GHB
			{
				efuse_OneByteRead(Adapter, 0x3F0, &targetval_A, FALSE);
				efuse_OneByteRead(Adapter, 0x3F1, &targetval_B, FALSE);
				efuse_OneByteRead(Adapter, 0x3F2, &targetval_C, FALSE);
				efuse_OneByteRead(Adapter, 0x3F3, &targetval_D, FALSE);
			}
		}
		DBG_871X("phy_ConfigKFree8814A(): targetval_A= %#x \n", targetval_A);
		DBG_871X("phy_ConfigKFree8814A(): targetval_B= %#x \n", targetval_B);
		DBG_871X("phy_ConfigKFree8814A(): targetval_C= %#x \n", targetval_C);
		DBG_871X("phy_ConfigKFree8814A(): targetval_D= %#x \n", targetval_D);
		
		// Make sure the targetval is defined
		if ((Adapter->registrypriv.RegRfKFreeEnable == 1) && ((targetval_A != 0xFF) || (pHalData->RfKFreeEnable == TRUE)))
		{
			if (bandType == BAND_ON_2_4G) // 2G
			{
				RT_TRACE(COMP_MP, DBG_LOUD, ("phy_ConfigKFree8814A(): PATH_A: %#x \n", targetval_A&0x0F));
				phy_SetKfreeToRF_8814A(Adapter, ODM_RF_PATH_A, targetval_A&0x0F);
				RT_TRACE(COMP_MP, DBG_LOUD, ("phy_ConfigKFree8814A(): PATH_B: %#x \n", (targetval_A&0xF0)>>4));
				phy_SetKfreeToRF_8814A(Adapter, ODM_RF_PATH_B, (targetval_A&0xF0)>>4);
				RT_TRACE(COMP_MP, DBG_LOUD, ("phy_ConfigKFree8814A(): PATH_C: %#x \n", targetval_B&0x0F));
				phy_SetKfreeToRF_8814A(Adapter, ODM_RF_PATH_C, targetval_B&0x0F);
				RT_TRACE(COMP_MP, DBG_LOUD, ("phy_ConfigKFree8814A(): PATH_D: %#x \n", (targetval_B&0xF0)>>4));
				phy_SetKfreeToRF_8814A(Adapter, ODM_RF_PATH_D, (targetval_B&0xF0)>>4);
			}
			else if(bandType == BAND_ON_5G)
			{
				RT_TRACE(COMP_MP, DBG_LOUD, ("phy_ConfigKFree8814A(): PATH_A: %#x \n", targetval_A&0x1F));
				phy_SetKfreeToRF_8814A(Adapter, ODM_RF_PATH_A, targetval_A&0x1F);
				RT_TRACE(COMP_MP, DBG_LOUD, ("phy_ConfigKFree8814A(): PATH_B: %#x \n", targetval_B&0x1F));
				phy_SetKfreeToRF_8814A(Adapter, ODM_RF_PATH_B, targetval_B&0x1F);
				RT_TRACE(COMP_MP, DBG_LOUD, ("phy_ConfigKFree8814A(): PATH_C: %#x \n", targetval_C&0x1F));
				phy_SetKfreeToRF_8814A(Adapter, ODM_RF_PATH_C, targetval_C&0x1F);
				RT_TRACE(COMP_MP, DBG_LOUD, ("phy_ConfigKFree8814A(): PATH_D: %#x \n", targetval_D&0x1F));
				phy_SetKfreeToRF_8814A(Adapter, ODM_RF_PATH_D, targetval_D&0x1F);
			}
		}
		else
		{
			RT_TRACE(COMP_MP, DBG_LOUD, ("phy_ConfigKFree8814A(): targetval not defined, Don't execute KFree Process.\n"));
			return;
		}
	}
	RT_TRACE(COMP_MP, DBG_LOUD, ("<===phy_ConfigKFree8814A()\n"));
}

VOID
phy_SwChnl8814A(	
	IN	PADAPTER					pAdapter
	)
{
	u8			eRFPath = 0 , channelIdx = 0;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
#ifdef CONFIG_RF_GAIN_OFFSET
	struct kfree_data_t *kfree_data = &pHalData->kfree_data;
#endif
	u8 			channelToSW = pHalData->CurrentChannel;
	u32			RFValToWR , RFTmpVal, BitShift, BitMask;
	  
	//DBG_871X("[BW:CHNL], phy_SwChnl8814A(), switch to channel %d !!\n", channelToSW);

	if (phy_SwBand8814A(pAdapter, channelToSW) == FALSE)
	{
		DBG_871X("error Chnl %d", channelToSW);
	}

	if(pHalData->rf_chip == RF_PSEUDO_11N)
	{
		RT_TRACE(COMP_MLME, DBG_LOUD, ("phy_SwChnl8814A: return for PSEUDO\n"));
		return;
	}
	
#ifdef CONFIG_RF_GAIN_OFFSET
	/* <YuChen, 140529> Add for KFree Feature Requested by RF David. */
	if (kfree_data->flag & KFREE_FLAG_ON) {

		channelIdx = rtw_ch_to_bb_gain_sel(channelToSW);
	#if 0
		if (pHalData->RfKFree_ch_group != channelIdx) {
			/* Todo: wait for new phydm ready */
			phy_ConfigKFree8814A(pAdapter, channelToSW, pHalData->CurrentBandType);
			phydm_ConfigKFree(pDM_Odm, channelToSW, kfree_data->bb_gain);
			DBG_871X("RfKFree_ch_group =%d\n", channelIdx);
		}
	#endif

		pHalData->RfKFree_ch_group = channelIdx;

	}
#endif
	if(pHalData->RegFWOffload == 2)
	{
		FillH2CCmd_8814(pAdapter, H2C_CHNL_SWITCH_OFFLOAD, 1, &channelToSW);
	}
	else
	{
		// fc_area		
		if (36 <= channelToSW && channelToSW <= 48) 
			PHY_SetBBReg(pAdapter, rFc_area_Jaguar, 0x1ffe0000, 0x494); 
		else if (50 <= channelToSW && channelToSW <= 64) 
			PHY_SetBBReg(pAdapter, rFc_area_Jaguar, 0x1ffe0000, 0x453);  
		else if (100 <= channelToSW && channelToSW <= 116) 
			PHY_SetBBReg(pAdapter, rFc_area_Jaguar, 0x1ffe0000, 0x452);  
		else if (118 <= channelToSW) 
			PHY_SetBBReg(pAdapter, rFc_area_Jaguar, 0x1ffe0000, 0x412);  
		else
			PHY_SetBBReg(pAdapter, rFc_area_Jaguar, 0x1ffe0000, 0x96a);

		for(eRFPath = 0; eRFPath < pHalData->NumTotalRFPath; eRFPath++)
		{
			// RF_MOD_AG
			if (36 <= channelToSW && channelToSW <= 64)
				RFValToWR = 0x101;	 //5'b00101
			else if (100 <= channelToSW && channelToSW <= 140) 
				RFValToWR = 0x301; 	//5'b01101
			else if (140 < channelToSW) 
				RFValToWR = 0x501; 	//5'b10101
			else	
				RFValToWR = 0x000; 	//5'b00000

			// Channel to switch
			BitMask = BIT18|BIT17|BIT16|BIT9|BIT8;
			BitShift =  PHY_CalculateBitShift(BitMask);
			RFTmpVal = channelToSW | (RFValToWR << BitShift);

			BitMask = BIT18|BIT17|BIT16|BIT9|BIT8|bMaskByte0;

			PHY_SetRFReg(pAdapter, eRFPath, RF_CHNLBW_Jaguar, BitMask, RFTmpVal);
		}

		if (36 <= channelToSW && channelToSW <= 64)				// Band 1 & Band 2
			PHY_SetBBReg(pAdapter, rAGC_table_Jaguar2, 0x1F, 1);	// 0x958[4:0] = 0x1
		else if (100 <= channelToSW && channelToSW <= 144) 			// Band 3
			PHY_SetBBReg(pAdapter, rAGC_table_Jaguar2, 0x1F, 2);	// 0x958[4:0] = 0x2
		else	if(channelToSW >= 149)								// Band 4
			PHY_SetBBReg(pAdapter, rAGC_table_Jaguar2, 0x1F, 3);	// 0x958[4:0] = 0x3
	}

	if (pAdapter->registrypriv.mp_mode == 1) {
				if (!pHalData->bSetChnlBW)
					phy_ADC_CLK_8814A(pAdapter);
		phy_SpurCalibration_8814A(pAdapter);
		phy_ModifyInitialGain_8814A(pAdapter);
	}
	
	/* 2.4G CCK TX DFIR  */
	if (channelToSW >= 1 && channelToSW <= 11) {
		PHY_SetBBReg(pAdapter, rCCK0_TxFilter1, bMaskDWord, 0x1a1b0030);
		PHY_SetBBReg(pAdapter, rCCK0_TxFilter2, bMaskDWord, 0x090e1317);
		PHY_SetBBReg(pAdapter, rCCK0_DebugPort, bMaskDWord, 0x00000204);
	} else if (channelToSW >= 12 && channelToSW <= 13) {
		PHY_SetBBReg(pAdapter, rCCK0_TxFilter1, bMaskDWord, 0x1a1b0030);
		PHY_SetBBReg(pAdapter, rCCK0_TxFilter2, bMaskDWord, 0x090e1217);
		PHY_SetBBReg(pAdapter, rCCK0_DebugPort, bMaskDWord, 0x00000305);
	} else if (channelToSW == 14) {
		PHY_SetBBReg(pAdapter, rCCK0_TxFilter1, bMaskDWord, 0x1a1b0030);
		PHY_SetBBReg(pAdapter, rCCK0_TxFilter2, bMaskDWord, 0x00000E17);
		PHY_SetBBReg(pAdapter, rCCK0_DebugPort, bMaskDWord, 0x00000000);
	}

}

/*
VOID
PHY_SwChnlTimerCallback8814A(
	IN	PRT_TIMER		pTimer
	)
{
	PADAPTER		pAdapter = (PADAPTER)pTimer->Adapter;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	
	RT_TRACE(COMP_SCAN, DBG_LOUD, ("==>PHY_SwChnlTimerCallback8814A(), switch to channel %d\n", pHalData->CurrentChannel));
	
	if (rtw_is_drv_stopped(padapter))
		return;
	
	if(pHalData->rf_chip == RF_PSEUDO_11N)
	{
		pHalData->SwChnlInProgress=FALSE;
		return; 								//return immediately if it is peudo-phy	
	}


	PlatformAcquireSpinLock(pAdapter, RT_CHANNEL_AND_BANDWIDTH_SPINLOCK);
	pHalData->SwChnlInProgress=TRUE;
	PlatformReleaseSpinLock(pAdapter, RT_CHANNEL_AND_BANDWIDTH_SPINLOCK);
	
	phy_SwChnl8814A(pAdapter);

	PlatformAcquireSpinLock(pAdapter, RT_CHANNEL_AND_BANDWIDTH_SPINLOCK);
	pHalData->SwChnlInProgress=FALSE;
	PlatformReleaseSpinLock(pAdapter, RT_CHANNEL_AND_BANDWIDTH_SPINLOCK);
	
	RT_TRACE(COMP_SCAN, DBG_LOUD, ("<==PHY_SwChnlTimerCallback8814()\n"));
}


VOID
PHY_SwChnlWorkItemCallback8814A(
	IN PVOID            pContext
	)
{
	PADAPTER		pAdapter = (PADAPTER)pContext;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	RT_TRACE(COMP_SCAN, DBG_LOUD, ("==>PHY_SwChnlWorkItemCallback8814A(), switch to channel %d\n", pHalData->CurrentChannel));

	if(pAdapter->bInSetPower && RT_USB_CANNOT_IO(pAdapter))
	{
		RT_TRACE(COMP_SCAN, DBG_LOUD, ("<== PHY_SwChnlWorkItemCallback8814A() SwChnlInProgress FALSE driver sleep or unload\n"));
	
		pHalData->SwChnlInProgress = FALSE;		
		return;
	}
	
	if (rtw_is_drv_stopped(padapter))
		return;
	
	if(pHalData->rf_chip == RF_PSEUDO_11N)
	{
		pHalData->SwChnlInProgress=FALSE;
		return; 								//return immediately if it is peudo-phy	
	}

	PlatformAcquireSpinLock(pAdapter, RT_CHANNEL_AND_BANDWIDTH_SPINLOCK);
	pHalData->SwChnlInProgress=TRUE;
	PlatformReleaseSpinLock(pAdapter, RT_CHANNEL_AND_BANDWIDTH_SPINLOCK);
		
	phy_SwChnl8814A(pAdapter);

	PlatformAcquireSpinLock(pAdapter, RT_CHANNEL_AND_BANDWIDTH_SPINLOCK);	
	pHalData->SwChnlInProgress=FALSE;
	PlatformReleaseSpinLock(pAdapter, RT_CHANNEL_AND_BANDWIDTH_SPINLOCK);
	
	RT_TRACE(COMP_P2P, DBG_LOUD, ("PHY_SwChnlWorkItemCallback8814A(), switch to channel %d\n", pHalData->CurrentChannel));
	RT_TRACE(COMP_SCAN, DBG_LOUD, ("<==PHY_SwChnlWorkItemCallback8814A()\n"));
}


VOID
HAL_HandleSwChnl8814A(	// Call after initialization
	IN	PADAPTER	pAdapter,
	IN	u8		channel
	)
{
	PADAPTER Adapter =  GetDefaultAdapter(pAdapter);
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	RT_TRACE(COMP_SCAN | COMP_RM, DBG_LOUD, ("HAL_HandleSwChnl8814A()===>\n"));
	pHalData->CurrentChannel = channel;
	phy_SwChnl8814A(Adapter);


#if (MP_DRIVER == 1) 
	// <20120712, Kordan> IQK on each channel, asked by James.
	PHY_IQCalibrate_8814A(pAdapter, FALSE);
#endif

	RT_TRACE(COMP_SCAN | COMP_RM, DBG_LOUD, ("<==HAL_HandleSwChnl8814A()\n"));
}
*/

VOID
phy_SwChnlAndSetBwMode8814A(
	IN  PADAPTER		Adapter
)
{
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);
	PDM_ODM_T		pDM_Odm = &pHalData->odmpriv;

	//DBG_871X("phy_SwChnlAndSetBwMode8814A(): bSwChnl %d, bSetChnlBW %d \n", pHalData->bSwChnl, pHalData->bSetChnlBW);
	if ( Adapter->bNotifyChannelChange )
	{
		DBG_871X( "[%s] bSwChnl=%d, ch=%d, bSetChnlBW=%d, bw=%d\n", 
			__FUNCTION__, 
			pHalData->bSwChnl,
			pHalData->CurrentChannel,
			pHalData->bSetChnlBW,
			pHalData->CurrentChannelBW);
	}
	
	if (RTW_CANNOT_RUN(Adapter)) {
		pHalData->bSwChnlAndSetBWInProgress= FALSE;
		return;
	}

	if (pHalData->bSwChnl)
	{
		phy_SwChnl8814A(Adapter);
		pHalData->bSwChnl = FALSE;
	}	

	if (pHalData->bSetChnlBW)
	{
		phy_SetBwMode8814A(Adapter);
		pHalData->bSetChnlBW = FALSE;
	}	

	if (Adapter->registrypriv.mp_mode == 0) {
		ODM_ClearTxPowerTrackingState(pDM_Odm);
		PHY_SetTxPowerLevel8814(Adapter, pHalData->CurrentChannel);
		if (pHalData->bNeedIQK == _TRUE) {
			PHY_IQCalibrate_8814A(pDM_Odm, _FALSE);
			pHalData->bNeedIQK = _FALSE;
		}
	} else
		PHY_IQCalibrate_8814A(pDM_Odm, _FALSE);
#if 0 //todo
#if (AUTO_CHNL_SEL_NHM == 1)	
	if(IS_AUTO_CHNL_SUPPORT(Adapter) && 
		P2PIsSocialChannel(pHalData->CurrentChannel))
	{	
		RT_TRACE(COMP_SCAN, DBG_TRACE, ("[ACS] phy_SwChnlAndSetBwMode8723B(): CurrentChannel %d Reset NHM counter!!\n", pHalData->CurrentChannel));
		RT_TRACE(COMP_SCAN, DBG_TRACE, ("[ACS] phy_SwChnlAndSetBwMode8723B(): AutoChnlSelPeriod(%d)\n", 
			GetDefaultAdapter(Adapter)->MgntInfo.AutoChnlSel.AutoChnlSelPeriod));

		// Reset NHM counter		
    		odm_AutoChannelSelectReset(GET_PDM_ODM(Adapter));
		
		SET_AUTO_CHNL_STATE(Adapter, ACS_BEFORE_NHM);// Before NHM measurement
	}	
#endif
#endif //0
	pHalData->bSwChnlAndSetBWInProgress= FALSE;
}


VOID
PHY_SwChnlAndSetBWModeCallback8814A(
	IN PVOID            pContext
)
{
	PADAPTER		Adapter = (PADAPTER)pContext;
	phy_SwChnlAndSetBwMode8814A(Adapter);
}

/*
//
// Description:
//	Switch channel synchronously. Called by SwChnlByDelayHandler.
//
// Implemented by Bruce, 2008-02-14.
// The following procedure is operted according to SwChanlCallback8190Pci().
// However, this procedure is performed synchronously  which should be running under
// passive level.
// 
VOID
PHY_SwChnlSynchronously8814A(	// Only called during initialize
	IN	PADAPTER	Adapter,
	IN	u8		channel
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);

	RT_TRACE(COMP_SCAN | COMP_RM, DBG_LOUD, ("==>PHY_SwChnlSynchronously(), switch from channel %d to channel %d.\n", pHalData->CurrentChannel, channel));

	// Cannot IO.
	if(RT_CANNOT_IO(Adapter))
		return;

	// Channel Switching is in progress.
	if(pHalData->bSwChnlAndSetBWInProgress)
		return;
	
	//return immediately if it is peudo-phy
	if(pHalData->rf_chip == RF_PSEUDO_11N)
	{
		pHalData->bSwChnlAndSetBWInProgress=FALSE;
		return;
	}

	switch(pHalData->CurrentWirelessMode)
	{
		case WIRELESS_MODE_A:
		case WIRELESS_MODE_N_5G:
		case WIRELESS_MODE_AC_5G:
			//Get first channel error when change between 5G and 2.4G band.
			//FIX ME!!!
			if(channel <=14)
				return;
			RT_ASSERT((channel>14), ("WIRELESS_MODE_A but channel<=14"));		
			break;
			
		case WIRELESS_MODE_B:
		case WIRELESS_MODE_G:
		case WIRELESS_MODE_N_24G:
		case WIRELESS_MODE_AC_24G:
			//Get first channel error when change between 5G and 2.4G band.
			//FIX ME!!!
			if(channel > 14)
				return;
			RT_ASSERT((channel<=14), ("WIRELESS_MODE_G but channel>14"));
			break;

		default:
			RT_ASSERT(FALSE, ("Invalid WirelessMode(%#x)!!\n", pHalData->CurrentWirelessMode));
			break;
	
	}	

	pHalData->bSwChnlAndSetBWInProgress = TRUE;
	if( channel == 0)
		channel = 1;

	pHalData->bSwChnl = TRUE;
	pHalData->bSetChnlBW = FALSE;
	pHalData->CurrentChannel=channel;

	phy_SwChnlAndSetBwMode8814A(Adapter);

	RT_TRACE(COMP_SCAN | COMP_RM, DBG_LOUD, ("<==PHY_SwChnlSynchronously(), switch from channel %d to channel %d.\n", pHalData->CurrentChannel, channel));
	
}
*/

VOID
PHY_HandleSwChnlAndSetBW8814A(
	IN	PADAPTER			Adapter,
	IN	BOOLEAN				bSwitchChannel,
	IN	BOOLEAN				bSetBandWidth,
	IN	u8					ChannelNum,
	IN	CHANNEL_WIDTH		ChnlWidth,
	IN	u8					ChnlOffsetOf40MHz,
	IN	u8					ChnlOffsetOf80MHz,
	IN	u8					CenterFrequencyIndex1
)
{
	PADAPTER  			pDefAdapter =  GetDefaultAdapter(Adapter);
	PHAL_DATA_TYPE		pHalData = GET_HAL_DATA(pDefAdapter);
	u8					tmpChannel = pHalData->CurrentChannel;
	CHANNEL_WIDTH		tmpBW= pHalData->CurrentChannelBW;
	u8					tmpnCur40MhzPrimeSC = pHalData->nCur40MhzPrimeSC;
	u8					tmpnCur80MhzPrimeSC = pHalData->nCur80MhzPrimeSC;
	u8					tmpCenterFrequencyIndex1 =pHalData->CurrentCenterFrequencyIndex1;
	struct mlme_ext_priv	*pmlmeext = &Adapter->mlmeextpriv;

	//check is swchnl or setbw
	if(!bSwitchChannel && !bSetBandWidth)
	{
		DBG_871X("PHY_HandleSwChnlAndSetBW8812:  not switch channel and not set bandwidth \n");
		return;
	}

	//skip change for channel or bandwidth is the same
	if(bSwitchChannel)
	{
		if(pHalData->CurrentChannel != ChannelNum)
		{
			if (HAL_IsLegalChannel(Adapter, ChannelNum))
				pHalData->bSwChnl = _TRUE;
			else
				return;
		}
	}

	if(bSetBandWidth)
	{
		if(pHalData->bChnlBWInitialized == _FALSE)
		{
			pHalData->bChnlBWInitialized = _TRUE;
			pHalData->bSetChnlBW = _TRUE;
		}
		else if((pHalData->CurrentChannelBW != ChnlWidth) ||
			(pHalData->nCur40MhzPrimeSC != ChnlOffsetOf40MHz) || 
			(pHalData->nCur80MhzPrimeSC != ChnlOffsetOf80MHz) ||
			(pHalData->CurrentCenterFrequencyIndex1!= CenterFrequencyIndex1))
		{
			pHalData->bSetChnlBW = _TRUE;
		}
	}

	if(!pHalData->bSetChnlBW && !pHalData->bSwChnl)
	{
		//DBG_871X("<= PHY_HandleSwChnlAndSetBW8812: bSwChnl %d, bSetChnlBW %d \n",pHalData->bSwChnl,pHalData->bSetChnlBW);
		return;
	}


	if(pHalData->bSwChnl)
	{
		pHalData->CurrentChannel=ChannelNum;
		pHalData->CurrentCenterFrequencyIndex1 = ChannelNum;
	}
	

	if(pHalData->bSetChnlBW)
	{
		pHalData->CurrentChannelBW = ChnlWidth;
#if 0
		if(ExtChnlOffsetOf40MHz==EXTCHNL_OFFSET_LOWER)
			pHalData->nCur40MhzPrimeSC = HAL_PRIME_CHNL_OFFSET_UPPER;
		else if(ExtChnlOffsetOf40MHz==EXTCHNL_OFFSET_UPPER)
			pHalData->nCur40MhzPrimeSC = HAL_PRIME_CHNL_OFFSET_LOWER;
		else
			pHalData->nCur40MhzPrimeSC = HAL_PRIME_CHNL_OFFSET_DONT_CARE;

		if(ExtChnlOffsetOf80MHz==EXTCHNL_OFFSET_LOWER)
			pHalData->nCur80MhzPrimeSC = HAL_PRIME_CHNL_OFFSET_UPPER;
		else if(ExtChnlOffsetOf80MHz==EXTCHNL_OFFSET_UPPER)
			pHalData->nCur80MhzPrimeSC = HAL_PRIME_CHNL_OFFSET_LOWER;
		else
			pHalData->nCur80MhzPrimeSC = HAL_PRIME_CHNL_OFFSET_DONT_CARE;
#else
		pHalData->nCur40MhzPrimeSC = ChnlOffsetOf40MHz;
		pHalData->nCur80MhzPrimeSC = ChnlOffsetOf80MHz;
#endif

		pHalData->CurrentCenterFrequencyIndex1 = CenterFrequencyIndex1;		
	}

	//Switch workitem or set timer to do switch channel or setbandwidth operation
	if (!RTW_CANNOT_RUN(Adapter))
		phy_SwChnlAndSetBwMode8814A(Adapter);
	else {
		if(pHalData->bSwChnl)
		{
			pHalData->CurrentChannel = tmpChannel;
			pHalData->CurrentCenterFrequencyIndex1 = tmpChannel;
		}	
		if(pHalData->bSetChnlBW)
		{
			pHalData->CurrentChannelBW = tmpBW;
			pHalData->nCur40MhzPrimeSC = tmpnCur40MhzPrimeSC;
			pHalData->nCur80MhzPrimeSC = tmpnCur80MhzPrimeSC;
			pHalData->CurrentCenterFrequencyIndex1 = tmpCenterFrequencyIndex1;
		}
	}

	//DBG_871X("Channel %d ChannelBW %d ",pHalData->CurrentChannel, pHalData->CurrentChannelBW);
	//DBG_871X("40MhzPrimeSC %d 80MhzPrimeSC %d ",pHalData->nCur40MhzPrimeSC, pHalData->nCur80MhzPrimeSC);
	//DBG_871X("CenterFrequencyIndex1 %d \n",pHalData->CurrentCenterFrequencyIndex1);

	//DBG_871X("<= PHY_HandleSwChnlAndSetBW8812: bSwChnl %d, bSetChnlBW %d \n",pHalData->bSwChnl,pHalData->bSetChnlBW);

}


/*
//
//	Description:
//		Configure H/W functionality to enable/disable Monitor mode.
//		Note, because we possibly need to configure BB and RF in this function, 
//		so caller should in PASSIVE_LEVEL. 080118, by rcnjko.
//
VOID
PHY_SetMonitorMode8814A(
	IN	PADAPTER			pAdapter,
	IN	BOOLEAN				bEnableMonitorMode
	)
{
	HAL_DATA_TYPE		*pHalData	= GET_HAL_DATA(pAdapter);
	BOOLEAN				bFilterOutNonAssociatedBSSID = FALSE;

	//2 Note: we may need to stop antenna diversity.
	if(bEnableMonitorMode)
	{
		bFilterOutNonAssociatedBSSID = FALSE;
		RT_TRACE(COMP_RM, DBG_LOUD, ("PHY_SetMonitorMode8814A(): enable monitor mode\n"));

		pHalData->bInMonitorMode = TRUE;
		pAdapter->HalFunc.AllowAllDestAddrHandler(pAdapter, TRUE, TRUE);
		rtw_hal_set_hwreg(pAdapter, HW_VAR_CHECK_BSSID, (u8*)&bFilterOutNonAssociatedBSSID);
	}
	else
	{
		bFilterOutNonAssociatedBSSID = TRUE;
		RT_TRACE(COMP_RM, DBG_LOUD, ("PHY_SetMonitorMode8814A(): disable monitor mode\n"));

		pAdapter->HalFunc.AllowAllDestAddrHandler(pAdapter, FALSE, TRUE);
		pHalData->bInMonitorMode = FALSE;
		rtw_hal_set_hwreg(pAdapter, HW_VAR_CHECK_BSSID, (u8*)&bFilterOutNonAssociatedBSSID);
	}
}
*/

BOOLEAN
SetAntennaConfig8814A(
	IN	PADAPTER		pAdapter,
	IN	u8			DefaultAnt		// 0: Main, 1: Aux.
)
{
	return TRUE;
}

VOID
PHY_SetBWMode8814(
	IN	PADAPTER			Adapter,
	IN	CHANNEL_WIDTH	Bandwidth,	// 20M or 40M
	IN	u8					Offset		// Upper, Lower, or Don't care
)
{
	PHAL_DATA_TYPE		pHalData = GET_HAL_DATA(Adapter);

	//DBG_871X("%s()===>\n",__FUNCTION__);

	PHY_HandleSwChnlAndSetBW8814A(Adapter, _FALSE, _TRUE, pHalData->CurrentChannel, Bandwidth, Offset, Offset, pHalData->CurrentChannel);

	//DBG_871X("<==%s()\n",__FUNCTION__);
}

VOID
PHY_SwChnl8814(
	IN	PADAPTER	Adapter,
	IN	u8			channel
	)
{
	//DBG_871X("%s()===>\n",__FUNCTION__);

	PHY_HandleSwChnlAndSetBW8814A(Adapter, _TRUE, _FALSE, channel, 0, 0, 0, channel);

	//DBG_871X("<==%s()\n",__FUNCTION__);
}

VOID
PHY_SetSwChnlBWMode8814(
	IN	PADAPTER			Adapter,
	IN	u8					channel,
	IN	CHANNEL_WIDTH		Bandwidth,
	IN	u8					Offset40,
	IN	u8					Offset80
)
{
	//DBG_871X("%s()===>\n",__FUNCTION__);

	PHY_HandleSwChnlAndSetBW8814A(Adapter, _TRUE, _TRUE, channel, Bandwidth, Offset40, Offset80, channel);

	//DBG_871X("<==%s()\n",__FUNCTION__);
}

