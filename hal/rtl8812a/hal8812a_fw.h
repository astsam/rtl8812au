
#ifdef CONFIG_RTL8812A
#ifndef _FW_HEADER_8812A_H
#define _FW_HEADER_8812A_H

#ifdef LOAD_FW_HEADER_FROM_DRIVER
#if (defined(CONFIG_AP_WOWLAN) || (DM_ODM_SUPPORT_TYPE & (ODM_AP)))
extern u1Byte Array_MP_8812A_FW_AP[23716];
extern u4Byte ArrayLength_MP_8812A_FW_AP;
#endif

#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN)) || (DM_ODM_SUPPORT_TYPE & (ODM_CE))
extern u1Byte Array_MP_8812A_FW_NIC[32654];
extern u4Byte ArrayLength_MP_8812A_FW_NIC;

extern u1Byte Array_MP_8812A_FW_NIC_BT[29398];
extern u4Byte ArrayLength_MP_8812A_FW_NIC_BT;

extern u1Byte Array_MP_8812A_FW_WoWLAN[29956];
extern u4Byte ArrayLength_MP_8812A_FW_WoWLAN;
#endif
#endif /* end of LOAD_FW_HEADER_FROM_DRIVER*/

#endif
#endif /* end of HWIMG_SUPPORT*/


