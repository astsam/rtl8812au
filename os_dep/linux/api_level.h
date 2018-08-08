#ifndef _API_LEVEL_H_
#define _API_LEVEL_H_
/*
* Use CFG80211_API_LEVEL instead of LINUX_VERSION_CODE to adopt API changes.
* Some older kernels may have backported cfg80211/nl80211 API.
*/

#ifndef CFG80211_API_LEVEL
#define CFG80211_API_LEVEL LINUX_VERSION_CODE
#endif /* CFG80211_API_LEVEL */
#endif

