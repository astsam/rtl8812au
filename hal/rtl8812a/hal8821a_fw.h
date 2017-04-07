
#ifdef CONFIG_RTL8821A
#ifndef _FW_HEADER_8821A_H
#define _FW_HEADER_8821A_H

#ifdef LOAD_FW_HEADER_FROM_DRIVER
#if (defined(CONFIG_AP_WOWLAN) || (DM_ODM_SUPPORT_TYPE & (ODM_AP)))
extern u1Byte Array_MP_8821A_FW_AP[17154];
extern u4Byte ArrayLength_MP_8821A_FW_AP;
#endif

#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN)) || (DM_ODM_SUPPORT_TYPE & (ODM_CE))
extern u1Byte Array_MP_8821A_FW_NIC[31924];
extern u4Byte ArrayLength_MP_8821A_FW_NIC;

extern u1Byte Array_MP_8821A_FW_NIC_BT[32334];
extern u4Byte ArrayLength_MP_8821A_FW_NIC_BT;

extern u1Byte Array_MP_8821A_FW_WoWLAN[28460];
extern u4Byte ArrayLength_MP_8821A_FW_WoWLAN;
#endif
#endif /* end of LOAD_FW_HEADER_FROM_DRIVER*/

#endif
#endif /* end of HWIMG_SUPPORT*/


