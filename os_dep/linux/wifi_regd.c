/******************************************************************************
 *
 * Copyright(c) 2009-2010  Realtek Corporation.
 *
 *****************************************************************************/

#include <drv_types.h>

#ifdef CONFIG_IOCTL_CFG80211

#include <rtw_wifi_regd.h>

void rtw_reg_notify_by_driver(_adapter *adapter)
{
	return;
}

int rtw_regd_init(_adapter * padapter)
{
	return 0;
}
#endif //CONFIG_IOCTL_CFG80211
