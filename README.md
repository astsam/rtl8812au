# RTL8812AU linux driver with monitor mode and frame injection
This RTL8812AU driver based on https://github.com/ulli-kroll/rtl8821au branch v4.3.22-beta/rework.
According to rtw_version.c the real driver version is 4.3.20.

for building type

`$ make`

The branch v4.3.21 may be built for RTL8814AU or RTL8812AU/RTL8821AU. Edit Makefile accordingly. 

for setting monitor mode

1. Set interface down
`# ip link set wlan0 down`
2. Set monitor mode
`# iwconfig wlan0 mode monitor`
3. Set interface up
`# ip link set wlan0 up`

for switching channels (interface must be up)

Set channel 6, width 40 MHz
```
# iw wlan0 set channel 6 HT40-
```
Set channel 149, width 80 MHz
```
# iw wlan 0 set freq 5745 80 5775
```

to inject frames with b/g rates use the Rate field in the radiotap header

to inject frames with n rates use the MCS field in the radiotap header

to inject frames with ac rates use the VHT field in the radiotap header 

