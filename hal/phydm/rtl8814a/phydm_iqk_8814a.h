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
#ifndef	__PHYDM_IQK_8814A_H__
#define    __PHYDM_IQK_8814A_H__

/*--------------------------Define Parameters-------------------------------*/
#define 	MAC_REG_NUM_8814 2
#define	BB_REG_NUM_8814 13
#define 	RF_REG_NUM_8814 2
#define	LOK_delay 1
#define	WBIQK_delay 10
#define 	TX_IQK 0
#define 	RX_IQK 1
#define	NUM 4	
/*---------------------------End Define Parameters-------------------------------*/

typedef struct _IQK_INFORMATION{
	BOOLEAN		LOK_fail[NUM];
	BOOLEAN		IQK_fail[2][NUM];
	u4Byte		IQC_Matrix[2][NUM];
	u1Byte      IQKtimes;

}IQK_INFO, *PIQK_INFO;


#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
VOID 
DoIQK_8814A(
	PVOID	pDM_VOID,
	u1Byte		DeltaThermalIndex,
	u1Byte		ThermalValue,	
	u1Byte		Threshold
	);
#else
VOID 
DoIQK_8814A(
	PVOID		pDM_VOID,
	u1Byte 		DeltaThermalIndex,
	u1Byte		ThermalValue,	
	u1Byte 		Threshold
	);
#endif

VOID	
PHY_IQCalibrate_8814A(	
	IN	PVOID		pDM_VOID,
	IN	BOOLEAN 	bReCovery
	);

VOID
PHY_IQCalibrate_8814A_Init(
	IN	PVOID		pDM_VOID
	);

 #endif	/* #ifndef __PHYDM_IQK_8814A_H__*/
