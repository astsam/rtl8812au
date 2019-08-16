/******************************************************************************
 *
 * Copyright(c) 2007 - 2017 Realtek Corporation.
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
 *****************************************************************************/

#ifdef CONFIG_GPIO_WAKEUP
#include <linux/gpio.h>
#endif

#include <drv_types.h>

#if defined(RTW_ENABLE_WIFI_CONTROL_FUNC)
#include <linux/platform_device.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
	#include <linux/wlan_plat.h>
#else
	#include <linux/wifi_tiwlan.h>
#endif
#endif /* defined(RTW_ENABLE_WIFI_CONTROL_FUNC) */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
#define strnicmp	strncasecmp
#endif /* Linux kernel >= 4.0.0 */

#ifdef CONFIG_GPIO_WAKEUP
#include <linux/interrupt.h>
#include <linux/irq.h>
#endif

#include "rtw_version.h"

extern void macstr2num(u8 *dst, u8 *src);

const char *android_wifi_cmd_str[ANDROID_WIFI_CMD_MAX] = {
	"START",
	"STOP",
	"SCAN-ACTIVE",
	"SCAN-PASSIVE",
	"RSSI",
	"LINKSPEED",
	"RXFILTER-START",
	"RXFILTER-STOP",
	"RXFILTER-ADD",
	"RXFILTER-REMOVE",
	"BTCOEXSCAN-START",
	"BTCOEXSCAN-STOP",
	"BTCOEXMODE",
	"SETSUSPENDMODE",
	"SETSUSPENDOPT",
	"P2P_DEV_ADDR",
	"SETFWPATH",
	"SETBAND",
	"GETBAND",
	"COUNTRY",
	"P2P_SET_NOA",
	"P2P_GET_NOA",
	"P2P_SET_PS",
	"SET_AP_WPS_P2P_IE",

	"MIRACAST",

#ifdef CONFIG_PNO_SUPPORT
	"PNOSSIDCLR",
	"PNOSETUP",
	"PNOFORCE",
	"PNODEBUG",
#endif

	"MACADDR",

	"BLOCK_SCAN",
	"BLOCK",
	"WFD-ENABLE",
	"WFD-DISABLE",
	"WFD-SET-TCPPORT",
	"WFD-SET-MAXTPUT",
	"WFD-SET-DEVTYPE",
	"SET_DTIM",
	"HOSTAPD_SET_MACADDR_ACL",
	"HOSTAPD_ACL_ADD_STA",
	"HOSTAPD_ACL_REMOVE_STA",
#if defined(CONFIG_GTK_OL) && (LINUX_VERSION_CODE < KERNEL_VERSION(3, 1, 0))
	"GTK_REKEY_OFFLOAD",
#endif /* CONFIG_GTK_OL */
/*	Private command for	P2P disable*/
	"P2P_DISABLE",
	"SET_AEK",
	"EXT_AUTH_STATUS",
	"DRIVER_VERSION"
};

#ifdef CONFIG_PNO_SUPPORT
#define PNO_TLV_PREFIX			'S'
#define PNO_TLV_VERSION			'1'
#define PNO_TLV_SUBVERSION		'2'
#define PNO_TLV_RESERVED		'0'
#define PNO_TLV_TYPE_SSID_IE	'S'
#define PNO_TLV_TYPE_TIME		'T'
#define PNO_TLV_FREQ_REPEAT		'R'
#define PNO_TLV_FREQ_EXPO_MAX	'M'

typedef struct cmd_tlv {
	char prefix;
	char version;
	char subver;
	char reserved;
} cmd_tlv_t;

#ifdef CONFIG_PNO_SET_DEBUG
char pno_in_example[] = {
	'P', 'N', 'O', 'S', 'E', 'T', 'U', 'P', ' ',
	'S', '1', '2', '0',
	'S',	/* 1 */
	0x05,
	'd', 'l', 'i', 'n', 'k',
	'S',	/* 2 */
	0x06,
	'B', 'U', 'F', 'B', 'U', 'F',
	'S',	/* 3 */
	0x20,
	'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '!', '@', '#', '$', '%', '^',
	'S',	/* 4 */
	0x0a,
	'!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
	'T',
	'0', '5',
	'R',
	'2',
	'M',
	'2',
	0x00
};
#endif /* CONFIG_PNO_SET_DEBUG */
#endif /* PNO_SUPPORT */

typedef struct android_wifi_priv_cmd {
	char *buf;
	int used_len;
	int total_len;
} android_wifi_priv_cmd;

#ifdef CONFIG_COMPAT
typedef struct compat_android_wifi_priv_cmd {
	compat_uptr_t buf;
	int used_len;
	int total_len;
} compat_android_wifi_priv_cmd;
#endif /* CONFIG_COMPAT */

/**
 * Local (static) functions and variables
 */

/* Initialize g_wifi_on to 1 so dhd_bus_start will be called for the first
 * time (only) in dhd_open, subsequential wifi on will be handled by
 * wl_android_wifi_on
 */
static int g_wifi_on = _TRUE;

unsigned int oob_irq = 0;
unsigned int oob_gpio = 0;

#ifdef CONFIG_PNO_SUPPORT
/*
 * rtw_android_pno_setup
 * Description:
 * This is used for private command.
 *
 * Parameter:
 * net: net_device
 * command: parameters from private command
 * total_len: the length of the command.
 *
 * */
static int rtw_android_pno_setup(struct net_device *net, char *command, int total_len)
{
	pno_ssid_t pno_ssids_local[MAX_PNO_LIST_COUNT];
	int res = -1;
	int nssid = 0;
	cmd_tlv_t *cmd_tlv_temp;
	char *str_ptr;
	int tlv_size_left;
	int pno_time = 0;
	int pno_repeat = 0;
	int pno_freq_expo_max = 0;
	int cmdlen = strlen(android_wifi_cmd_str[ANDROID_WIFI_CMD_PNOSETUP_SET]) + 1;

#ifdef CONFIG_PNO_SET_DEBUG
	int i;
	char *p;
	p = pno_in_example;

	total_len = sizeof(pno_in_example);
	str_ptr = p + cmdlen;
#else
	str_ptr = command + cmdlen;
#endif

	if (total_len < (cmdlen + sizeof(cmd_tlv_t))) {
		RTW_INFO("%s argument=%d less min size\n", __func__, total_len);
		goto exit_proc;
	}

	tlv_size_left = total_len - cmdlen;

	cmd_tlv_temp = (cmd_tlv_t *)str_ptr;
	memset(pno_ssids_local, 0, sizeof(pno_ssids_local));

	if ((cmd_tlv_temp->prefix == PNO_TLV_PREFIX) &&
	    (cmd_tlv_temp->version == PNO_TLV_VERSION) &&
	    (cmd_tlv_temp->subver == PNO_TLV_SUBVERSION)) {

		str_ptr += sizeof(cmd_tlv_t);
		tlv_size_left -= sizeof(cmd_tlv_t);

		nssid = rtw_parse_ssid_list_tlv(&str_ptr, pno_ssids_local,
			     MAX_PNO_LIST_COUNT, &tlv_size_left);
		if (nssid <= 0) {
			RTW_INFO("SSID is not presented or corrupted ret=%d\n", nssid);
			goto exit_proc;
		} else {
			if ((str_ptr[0] != PNO_TLV_TYPE_TIME) || (tlv_size_left <= 1)) {
				RTW_INFO("%s scan duration corrupted field size %d\n",
					 __func__, tlv_size_left);
				goto exit_proc;
			}
			str_ptr++;
			pno_time = simple_strtoul(str_ptr, &str_ptr, 16);
			RTW_INFO("%s: pno_time=%d\n", __func__, pno_time);

			if (str_ptr[0] != 0) {
				if ((str_ptr[0] != PNO_TLV_FREQ_REPEAT)) {
					RTW_INFO("%s pno repeat : corrupted field\n",
						 __func__);
					goto exit_proc;
				}
				str_ptr++;
				pno_repeat = simple_strtoul(str_ptr, &str_ptr, 16);
				RTW_INFO("%s :got pno_repeat=%d\n", __FUNCTION__, pno_repeat);
				if (str_ptr[0] != PNO_TLV_FREQ_EXPO_MAX) {
					RTW_INFO("%s FREQ_EXPO_MAX corrupted field size\n",
						 __func__);
					goto exit_proc;
				}
				str_ptr++;
				pno_freq_expo_max = simple_strtoul(str_ptr, &str_ptr, 16);
				RTW_INFO("%s: pno_freq_expo_max=%d\n",
					 __func__, pno_freq_expo_max);
			}
		}
	} else {
		RTW_INFO("%s get wrong TLV command\n", __FUNCTION__);
		goto exit_proc;
	}

	res = rtw_dev_pno_set(net, pno_ssids_local, nssid, pno_time, pno_repeat, pno_freq_expo_max);

#ifdef CONFIG_PNO_SET_DEBUG
	rtw_dev_pno_debug(net);
#endif

exit_proc:
	return res;
}

/*
 * rtw_android_cfg80211_pno_setup
 * Description:
 * This is used for cfg80211 sched_scan.
 *
 * Parameter:
 * net: net_device
 * request: cfg80211_request
 * */

int rtw_android_cfg80211_pno_setup(struct net_device *net,
		   struct cfg80211_ssid *ssids, int n_ssids, int interval)
{
	int res = -1;
	int nssid = 0;
	int pno_time = 0;
	int pno_repeat = 0;
	int pno_freq_expo_max = 0;
	int index = 0;
	pno_ssid_t pno_ssids_local[MAX_PNO_LIST_COUNT];

	if (n_ssids > MAX_PNO_LIST_COUNT || n_ssids < 0) {
		RTW_INFO("%s: nssids(%d) is invalid.\n", __func__, n_ssids);
		return -EINVAL;
	}

	memset(pno_ssids_local, 0, sizeof(pno_ssids_local));

	nssid = n_ssids;

	for (index = 0 ; index < nssid ; index++) {
		pno_ssids_local[index].SSID_len = ssids[index].ssid_len;
		memcpy(pno_ssids_local[index].SSID, ssids[index].ssid,
		       ssids[index].ssid_len);
	}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 2, 0)
	if(ssids)
		rtw_mfree((u8 *)ssids, (n_ssids * sizeof(struct cfg80211_ssid)));
#endif
	pno_time = (interval / 1000);

	RTW_INFO("%s: nssids: %d, pno_time=%d\n", __func__, nssid, pno_time);

	res = rtw_dev_pno_set(net, pno_ssids_local, nssid, pno_time,
			      pno_repeat, pno_freq_expo_max);

#ifdef CONFIG_PNO_SET_DEBUG
	rtw_dev_pno_debug(net);
#endif
exit_proc:
	return res;
}

int rtw_android_pno_enable(struct net_device *net, int pno_enable)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(net);
	struct pwrctrl_priv *pwrctl = adapter_to_pwrctl(padapter);

	if (pwrctl) {
		pwrctl->wowlan_pno_enable = pno_enable;
		RTW_INFO("%s: wowlan_pno_enable: %d\n", __func__, pwrctl->wowlan_pno_enable);
		if (pwrctl->wowlan_pno_enable == 0) {
			if (pwrctl->pnlo_info != NULL) {
				rtw_mfree((u8 *)pwrctl->pnlo_info, sizeof(pno_nlo_info_t));
				pwrctl->pnlo_info = NULL;
			}
			if (pwrctl->pno_ssid_list != NULL) {
				rtw_mfree((u8 *)pwrctl->pno_ssid_list, sizeof(pno_ssid_list_t));
				pwrctl->pno_ssid_list = NULL;
			}
			if (pwrctl->pscan_info != NULL) {
				rtw_mfree((u8 *)pwrctl->pscan_info, sizeof(pno_scan_info_t));
				pwrctl->pscan_info = NULL;
			}
		}
		return 0;
	} else
		return -1;
}
#endif /* CONFIG_PNO_SUPPORT */

int rtw_android_cmdstr_to_num(char *cmdstr)
{
	int cmd_num;
	for (cmd_num = 0 ; cmd_num < ANDROID_WIFI_CMD_MAX; cmd_num++)
		if (0 == strnicmp(cmdstr , android_wifi_cmd_str[cmd_num], strlen(android_wifi_cmd_str[cmd_num])))
			break;

	return cmd_num;
}

int rtw_android_get_rssi(struct net_device *net, char *command, int total_len)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(net);
	struct	mlme_priv	*pmlmepriv = &(padapter->mlmepriv);
	struct	wlan_network	*pcur_network = &pmlmepriv->cur_network;
	int bytes_written = 0;

	if (check_fwstate(pmlmepriv, _FW_LINKED) == _TRUE) {
		bytes_written += snprintf(&command[bytes_written], total_len, "%s rssi %d",
			pcur_network->network.Ssid.Ssid, padapter->recvpriv.rssi);
	}

	return bytes_written;
}

int rtw_android_get_link_speed(struct net_device *net, char *command, int total_len)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(net);
	int bytes_written = 0;
	u16 link_speed = 0;

	link_speed = rtw_get_cur_max_rate(padapter) / 10;
	bytes_written = snprintf(command, total_len, "LinkSpeed %d", link_speed);

	return bytes_written;
}

int rtw_android_get_macaddr(struct net_device *net, char *command, int total_len)
{
	int bytes_written = 0;

	bytes_written = snprintf(command, total_len, "Macaddr = "MAC_FMT, MAC_ARG(net->dev_addr));
	return bytes_written;
}

int rtw_android_set_country(struct net_device *net, char *command, int total_len)
{
	_adapter *adapter = (_adapter *)rtw_netdev_priv(net);
	char *country_code = command + strlen(android_wifi_cmd_str[ANDROID_WIFI_CMD_COUNTRY]) + 1;
	int ret = _FAIL;

	ret = rtw_set_country(adapter, country_code);

	return (ret == _SUCCESS) ? 0 : -1;
}

int rtw_android_get_p2p_dev_addr(struct net_device *net, char *command, int total_len)
{
	int bytes_written = 0;

	/* We use the same address as our HW MAC address */
	_rtw_memcpy(command, net->dev_addr, ETH_ALEN);

	bytes_written = ETH_ALEN;
	return bytes_written;
}

int rtw_android_set_block_scan(struct net_device *net, char *command, int total_len)
{
	_adapter *adapter = (_adapter *)rtw_netdev_priv(net);
	char *block_value = command + strlen(android_wifi_cmd_str[ANDROID_WIFI_CMD_BLOCK_SCAN]) + 1;

#ifdef CONFIG_IOCTL_CFG80211
	adapter_wdev_data(adapter)->block_scan = (*block_value == '0') ? _FALSE : _TRUE;
#endif

	return 0;
}

int rtw_android_set_block(struct net_device *net, char *command, int total_len)
{
	_adapter *adapter = (_adapter *)rtw_netdev_priv(net);
	char *block_value = command + strlen(android_wifi_cmd_str[ANDROID_WIFI_CMD_BLOCK]) + 1;

#ifdef CONFIG_IOCTL_CFG80211
	adapter_wdev_data(adapter)->block = (*block_value == '0') ? _FALSE : _TRUE;
#endif

	return 0;
}

int rtw_android_setband(struct net_device *net, char *command, int total_len)
{
	_adapter *adapter = (_adapter *)rtw_netdev_priv(net);
	char *arg = command + strlen(android_wifi_cmd_str[ANDROID_WIFI_CMD_SETBAND]) + 1;
	u32 band = WIFI_FREQUENCY_BAND_AUTO;
	int ret = _FAIL;

	if (sscanf(arg, "%u", &band) >= 1)
		ret = rtw_set_band(adapter, band);

	return (ret == _SUCCESS) ? 0 : -1;
}

int rtw_android_getband(struct net_device *net, char *command, int total_len)
{
	_adapter *adapter = (_adapter *)rtw_netdev_priv(net);
	int bytes_written = 0;

	bytes_written = snprintf(command, total_len, "%u", adapter->setband);

	return bytes_written;
}

#ifdef CONFIG_WFD
int rtw_android_set_miracast_mode(struct net_device *net, char *command, int total_len)
{
	_adapter *adapter = (_adapter *)rtw_netdev_priv(net);
	struct wifi_display_info *wfd_info = &adapter->wfd_info;
	char *arg = command + strlen(android_wifi_cmd_str[ANDROID_WIFI_CMD_MIRACAST]) + 1;
	u8 mode;
	int num;
	int ret = _FAIL;

	num = sscanf(arg, "%hhu", &mode);

	if (num < 1)
		goto exit;

	switch (mode) {
	case 1: /* soruce */
		mode = MIRACAST_SOURCE;
		break;
	case 2: /* sink */
		mode = MIRACAST_SINK;
		break;
	case 0: /* disabled */
	default:
		mode = MIRACAST_DISABLED;
		break;
	}
	wfd_info->stack_wfd_mode = mode;
	RTW_INFO("stack miracast mode: %s\n", get_miracast_mode_str(wfd_info->stack_wfd_mode));

	ret = _SUCCESS;

exit:
	return (ret == _SUCCESS) ? 0 : -1;
}
#endif /* CONFIG_WFD */

int get_int_from_command(char *pcmd)
{
	int i = 0;

	for (i = 0; i < strlen(pcmd); i++) {
		if (pcmd[i] == '=') {
			/*	Skip the '=' and space characters. */
			i += 2;
			break;
		}
	}
	return rtw_atoi(pcmd + i) ;
}

#if defined(CONFIG_GTK_OL) && (LINUX_VERSION_CODE < KERNEL_VERSION(3, 1, 0))
int rtw_gtk_offload(struct net_device *net, u8 *cmd_ptr)
{
	int i;
	/* u8 *cmd_ptr = priv_cmd.buf; */
	struct sta_info *psta;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(net);
	struct mlme_priv	*pmlmepriv = &padapter->mlmepriv;
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct security_priv *psecuritypriv = &(padapter->securitypriv);
	psta = rtw_get_stainfo(pstapriv, get_bssid(pmlmepriv));


	if (psta == NULL)
		RTW_INFO("%s, : Obtain Sta_info fail\n", __func__);
	else {
		/* string command length of "GTK_REKEY_OFFLOAD" */
		cmd_ptr += 18;

		_rtw_memcpy(psta->kek, cmd_ptr, RTW_KEK_LEN);
		cmd_ptr += RTW_KEK_LEN;
		/*
		printk("supplicant KEK: ");
		for(i=0;i<RTW_KEK_LEN; i++)
			printk(" %02x ", psta->kek[i]);
		printk("\n supplicant KCK: ");
		*/
		_rtw_memcpy(psta->kck, cmd_ptr, RTW_KCK_LEN);
		cmd_ptr += RTW_KCK_LEN;
		/*
		for(i=0;i<RTW_KEK_LEN; i++)
			printk(" %02x ", psta->kck[i]);
		*/
		_rtw_memcpy(psta->replay_ctr, cmd_ptr, RTW_REPLAY_CTR_LEN);
		psecuritypriv->binstallKCK_KEK = _TRUE;

		/* printk("\nREPLAY_CTR: "); */
		/* for(i=0;i<RTW_REPLAY_CTR_LEN; i++) */
		/* printk(" %02x ", psta->replay_ctr[i]); */
	}

	return _SUCCESS;
}
#endif /* CONFIG_GTK_OL */

#ifdef CONFIG_RTW_MESH_AEK
static int rtw_android_set_aek(struct net_device *ndev, char *command, int total_len)
{
#define SET_AEK_DATA_LEN (ETH_ALEN + 32)

	_adapter *adapter = (_adapter *)rtw_netdev_priv(ndev);
	u8 *addr;
	u8 *aek;
	int err = 0;

	if (total_len - strlen(android_wifi_cmd_str[ANDROID_WIFI_CMD_SET_AEK]) - 1 != SET_AEK_DATA_LEN) {
		err = -EINVAL;
		goto exit;
	}

	addr = command + strlen(android_wifi_cmd_str[ANDROID_WIFI_CMD_SET_AEK]) + 1;
	aek = addr + ETH_ALEN;

	RTW_PRINT(FUNC_NDEV_FMT" addr="MAC_FMT"\n"
		, FUNC_NDEV_ARG(ndev), MAC_ARG(addr));
	if (0)
		RTW_PRINT(FUNC_NDEV_FMT" aek="KEY_FMT KEY_FMT"\n"
			, FUNC_NDEV_ARG(ndev), KEY_ARG(aek), KEY_ARG(aek + 16));

	if (rtw_mesh_plink_set_aek(adapter, addr, aek) != _SUCCESS)
		err = -ENOENT;

exit:
	return err;
}
#endif /* CONFIG_RTW_MESH_AEK */

/**
 * Functions for Android WiFi card detection
 */
#if defined(RTW_ENABLE_WIFI_CONTROL_FUNC)

static int g_wifidev_registered = 0;
static struct semaphore wifi_control_sem;
static struct wifi_platform_data *wifi_control_data = NULL;
static struct resource *wifi_irqres = NULL;

static int wifi_add_dev(void);
static void wifi_del_dev(void);

int rtw_android_wifictrl_func_add(void)
{
	int ret = 0;
	sema_init(&wifi_control_sem, 0);

	ret = wifi_add_dev();
	if (ret) {
		RTW_INFO("%s: platform_driver_register failed\n", __FUNCTION__);
		return ret;
	}
	g_wifidev_registered = 1;

	/* Waiting callback after platform_driver_register is done or exit with error */
	if (down_timeout(&wifi_control_sem,  msecs_to_jiffies(1000)) != 0) {
		ret = -EINVAL;
		RTW_INFO("%s: platform_driver_register timeout\n", __FUNCTION__);
	}

	return ret;
}

void rtw_android_wifictrl_func_del(void)
{
	if (g_wifidev_registered) {
		wifi_del_dev();
		g_wifidev_registered = 0;
	}
}

void *wl_android_prealloc(int section, unsigned long size)
{
	void *alloc_ptr = NULL;
	if (wifi_control_data && wifi_control_data->mem_prealloc) {
		alloc_ptr = wifi_control_data->mem_prealloc(section, size);
		if (alloc_ptr) {
			RTW_INFO("success alloc section %d\n", section);
			if (size != 0L)
				memset(alloc_ptr, 0, size);
			return alloc_ptr;
		}
	}

	RTW_INFO("can't alloc section %d\n", section);
	return NULL;
}

int wifi_get_irq_number(unsigned long *irq_flags_ptr)
{
	if (wifi_irqres) {
		*irq_flags_ptr = wifi_irqres->flags & IRQF_TRIGGER_MASK;
		return (int)wifi_irqres->start;
	}
#ifdef CUSTOM_OOB_GPIO_NUM
	return CUSTOM_OOB_GPIO_NUM;
#else
	return -1;
#endif
}

int wifi_set_power(int on, unsigned long msec)
{
	RTW_INFO("%s = %d\n", __FUNCTION__, on);
	if (wifi_control_data && wifi_control_data->set_power)
		wifi_control_data->set_power(on);
	if (msec)
		msleep(msec);
	return 0;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
int wifi_get_mac_addr(unsigned char *buf)
{
	RTW_INFO("%s\n", __FUNCTION__);
	if (!buf)
		return -EINVAL;
	if (wifi_control_data && wifi_control_data->get_mac_addr)
		return wifi_control_data->get_mac_addr(buf);
	return -EOPNOTSUPP;
}
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35)) */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 39)) || defined(COMPAT_KERNEL_RELEASE)
void *wifi_get_country_code(char *ccode)
{
	RTW_INFO("%s\n", __FUNCTION__);
	if (!ccode)
		return NULL;
	if (wifi_control_data && wifi_control_data->get_country_code)
		return wifi_control_data->get_country_code(ccode);
	return NULL;
}
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 39)) */

static int wifi_set_carddetect(int on)
{
	RTW_INFO("%s = %d\n", __FUNCTION__, on);
	if (wifi_control_data && wifi_control_data->set_carddetect)
		wifi_control_data->set_carddetect(on);
	return 0;
}

static int wifi_probe(struct platform_device *pdev)
{
	struct wifi_platform_data *wifi_ctrl =
		(struct wifi_platform_data *)(pdev->dev.platform_data);
	int wifi_wake_gpio = 0;

	RTW_INFO("## %s\n", __FUNCTION__);
	wifi_irqres = platform_get_resource_byname(pdev, IORESOURCE_IRQ, "bcmdhd_wlan_irq");

	if (wifi_irqres == NULL)
		wifi_irqres = platform_get_resource_byname(pdev,
				IORESOURCE_IRQ, "bcm4329_wlan_irq");
	else
		wifi_wake_gpio = wifi_irqres->start;

#ifdef CONFIG_GPIO_WAKEUP
	RTW_INFO("%s: gpio:%d wifi_wake_gpio:%d\n", __func__,
	       (int)wifi_irqres->start, wifi_wake_gpio);

	if (wifi_wake_gpio > 0) {
#ifdef CONFIG_PLATFORM_INTEL_BYT
		wifi_configure_gpio();
#else /* CONFIG_PLATFORM_INTEL_BYT */
		gpio_request(wifi_wake_gpio, "oob_irq");
		gpio_direction_input(wifi_wake_gpio);
		oob_irq = gpio_to_irq(wifi_wake_gpio);
#endif /* CONFIG_PLATFORM_INTEL_BYT */
		RTW_INFO("%s oob_irq:%d\n", __func__, oob_irq);
	} else if (wifi_irqres) {
		oob_irq = wifi_irqres->start;
		RTW_INFO("%s oob_irq:%d\n", __func__, oob_irq);
	}
#endif
	wifi_control_data = wifi_ctrl;

	wifi_set_power(1, 0);	/* Power On */
	wifi_set_carddetect(1);	/* CardDetect (0->1) */

	up(&wifi_control_sem);
	return 0;
}

#ifdef RTW_SUPPORT_PLATFORM_SHUTDOWN
extern PADAPTER g_test_adapter;

static void shutdown_card(void)
{
	u32 addr;
	u8 tmp8, cnt = 0;

	if (NULL == g_test_adapter) {
		RTW_INFO("%s: padapter==NULL\n", __FUNCTION__);
		return;
	}

#ifdef CONFIG_FWLPS_IN_IPS
	LeaveAllPowerSaveMode(g_test_adapter);
#endif /* CONFIG_FWLPS_IN_IPS */

#ifdef CONFIG_WOWLAN
#ifdef CONFIG_GPIO_WAKEUP
	/*default wake up pin change to BT*/
	RTW_INFO("%s:default wake up pin change to BT\n", __FUNCTION__);
	rtw_hal_switch_gpio_wl_ctrl(g_test_adapter, WAKEUP_GPIO_IDX, _FALSE);
#endif /* CONFIG_GPIO_WAKEUP */
#endif /* CONFIG_WOWLAN */

	/* Leave SDIO HCI Suspend */
	addr = 0x10250086;
	rtw_write8(g_test_adapter, addr, 0);
	do {
		tmp8 = rtw_read8(g_test_adapter, addr);
		cnt++;
		RTW_INFO(FUNC_ADPT_FMT ": polling SDIO_HSUS_CTRL(0x%x)=0x%x, cnt=%d\n",
			 FUNC_ADPT_ARG(g_test_adapter), addr, tmp8, cnt);

		if (tmp8 & BIT(1))
			break;

		if (cnt >= 100) {
			RTW_INFO(FUNC_ADPT_FMT ": polling 0x%x[1]==1 FAIL!!\n",
				 FUNC_ADPT_ARG(g_test_adapter), addr);
			break;
		}

		rtw_mdelay_os(10);
	} while (1);

	/* unlock register I/O */
	rtw_write8(g_test_adapter, 0x1C, 0);

	/* enable power down function */
	/* 0x04[4] = 1 */
	/* 0x05[7] = 1 */
	addr = 0x04;
	tmp8 = rtw_read8(g_test_adapter, addr);
	tmp8 |= BIT(4);
	rtw_write8(g_test_adapter, addr, tmp8);
	RTW_INFO(FUNC_ADPT_FMT ": read after write 0x%x=0x%x\n",
		FUNC_ADPT_ARG(g_test_adapter), addr, rtw_read8(g_test_adapter, addr));

	addr = 0x05;
	tmp8 = rtw_read8(g_test_adapter, addr);
	tmp8 |= BIT(7);
	rtw_write8(g_test_adapter, addr, tmp8);
	RTW_INFO(FUNC_ADPT_FMT ": read after write 0x%x=0x%x\n",
		FUNC_ADPT_ARG(g_test_adapter), addr, rtw_read8(g_test_adapter, addr));

	/* lock register page0 0x0~0xB read/write */
	rtw_write8(g_test_adapter, 0x1C, 0x0E);

	rtw_set_surprise_removed(g_test_adapter);
	RTW_INFO(FUNC_ADPT_FMT ": bSurpriseRemoved=%s\n",
		FUNC_ADPT_ARG(g_test_adapter), rtw_is_surprise_removed(g_test_adapter) ? "True" : "False");
}
#endif /* RTW_SUPPORT_PLATFORM_SHUTDOWN */

static int wifi_remove(struct platform_device *pdev)
{
	struct wifi_platform_data *wifi_ctrl =
		(struct wifi_platform_data *)(pdev->dev.platform_data);

	RTW_INFO("## %s\n", __FUNCTION__);
	wifi_control_data = wifi_ctrl;

	wifi_set_power(0, 0);	/* Power Off */
	wifi_set_carddetect(0);	/* CardDetect (1->0) */

	up(&wifi_control_sem);
	return 0;
}

#ifdef RTW_SUPPORT_PLATFORM_SHUTDOWN
static void wifi_shutdown(struct platform_device *pdev)
{
	struct wifi_platform_data *wifi_ctrl =
		(struct wifi_platform_data *)(pdev->dev.platform_data);


	RTW_INFO("## %s\n", __FUNCTION__);

	wifi_control_data = wifi_ctrl;

	shutdown_card();
	wifi_set_power(0, 0);	/* Power Off */
	wifi_set_carddetect(0);	/* CardDetect (1->0) */
}
#endif /* RTW_SUPPORT_PLATFORM_SHUTDOWN */

static int wifi_suspend(struct platform_device *pdev, pm_message_t state)
{
	RTW_INFO("##> %s\n", __FUNCTION__);
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 39)) && defined(OOB_INTR_ONLY)
	bcmsdh_oob_intr_set(0);
#endif
	return 0;
}

static int wifi_resume(struct platform_device *pdev)
{
	RTW_INFO("##> %s\n", __FUNCTION__);
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 39)) && defined(OOB_INTR_ONLY)
	if (dhd_os_check_if_up(bcmsdh_get_drvdata()))
		bcmsdh_oob_intr_set(1);
#endif
	return 0;
}

/* temporarily use these two */
static struct platform_driver wifi_device = {
	.probe          = wifi_probe,
	.remove         = wifi_remove,
	.suspend        = wifi_suspend,
	.resume         = wifi_resume,
#ifdef RTW_SUPPORT_PLATFORM_SHUTDOWN
	.shutdown       = wifi_shutdown,
#endif /* RTW_SUPPORT_PLATFORM_SHUTDOWN */
	.driver         = {
		.name   = "bcmdhd_wlan",
	}
};

static struct platform_driver wifi_device_legacy = {
	.probe          = wifi_probe,
	.remove         = wifi_remove,
	.suspend        = wifi_suspend,
	.resume         = wifi_resume,
	.driver         = {
		.name   = "bcm4329_wlan",
	}
};

static int wifi_add_dev(void)
{
	RTW_INFO("## Calling platform_driver_register\n");
	platform_driver_register(&wifi_device);
	platform_driver_register(&wifi_device_legacy);
	return 0;
}

static void wifi_del_dev(void)
{
	RTW_INFO("## Unregister platform_driver_register\n");
	platform_driver_unregister(&wifi_device);
	platform_driver_unregister(&wifi_device_legacy);
}
#endif /* defined(RTW_ENABLE_WIFI_CONTROL_FUNC) */

#ifdef CONFIG_GPIO_WAKEUP
#ifdef CONFIG_PLATFORM_INTEL_BYT
int wifi_configure_gpio(void)
{
	if (gpio_request(oob_gpio, "oob_irq")) {
		RTW_INFO("## %s Cannot request GPIO\n", __FUNCTION__);
		return -1;
	}
	gpio_export(oob_gpio, 0);
	if (gpio_direction_input(oob_gpio)) {
		RTW_INFO("## %s Cannot set GPIO direction input\n", __FUNCTION__);
		return -1;
	}
	oob_irq = gpio_to_irq(oob_gpio);
	if (oob_irq < 0) {
		RTW_INFO("## %s Cannot convert GPIO to IRQ\n", __FUNCTION__);
		return -1;
	}

	RTW_INFO("## %s OOB_IRQ=%d\n", __FUNCTION__, oob_irq);

	return 0;
}
#endif /* CONFIG_PLATFORM_INTEL_BYT */
void wifi_free_gpio(unsigned int gpio)
{
#ifdef CONFIG_PLATFORM_INTEL_BYT
	if (gpio)
		gpio_free(gpio);
#endif /* CONFIG_PLATFORM_INTEL_BYT */
}
#endif /* CONFIG_GPIO_WAKEUP */
