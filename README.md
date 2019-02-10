## RTL8812AU/21AU and RTL8814AU drivers
### Supports Realtek 8811, 8812, 8814 and 8821 chipsets

[![Monitor mode](https://img.shields.io/badge/monitor%20mode-working-brightgreen.svg)](#)
[![Frame Injection](https://img.shields.io/badge/frame%20injection-working-brightgreen.svg)](#)
[![GitHub version](https://badge.fury.io/gh/aircrack-ng%2Frtl8812au.svg)](https://badge.fury.io/gh/aircrack-ng%2Frtl8812au)
[![GitHub issues](https://img.shields.io/github/issues/aircrack-ng/rtl8812au.svg)](https://github.com/aircrack-ng/rtl8812au/issues)
[![GitHub forks](https://img.shields.io/github/forks/aircrack-ng/rtl8812au.svg)](https://github.com/aircrack-ng/rtl8812au/network)
[![GitHub stars](https://img.shields.io/github/stars/aircrack-ng/rtl8812au.svg)](https://github.com/aircrack-ng/rtl8812au/stargazers)
[![GitHub license](https://img.shields.io/github/license/aircrack-ng/rtl8812au.svg)](https://github.com/aircrack-ng/rtl8812au/blob/master/LICENSE)
<br>
[![Kali](https://img.shields.io/badge/Kali-supported-blue.svg)](https://www.kali.org)
[![Arch](https://img.shields.io/badge/Arch-supported-blue.svg)](https://www.archlinux.org)
[![Armbian](https://img.shields.io/badge/Armbian-supported-blue.svg)](https://www.armbian.com)
[![ArchLinux](https://img.shields.io/badge/ArchLinux-supported-blue.svg)](https://img.shields.io/badge/ArchLinux-supported-blue.svg)
[![aircrack-ng](https://img.shields.io/badge/aircrack--ng-supported-blue.svg)](https://github.com/aircrack-ng/aircrack-ng)
[![wifite2](https://img.shields.io/badge/wifite2-supported-blue.svg)](https://github.com/derv82/wifite2)

### DKMS
This driver can be installed using [DKMS]. This is a system which will automatically recompile and install a kernel module when a new kernel gets installed or updated. To make use of DKMS, install the `dkms` package, which on Debian (based) systems is done like this:
```
$ sudo apt-get install dkms
```

### Installation of Driver
In order to install the driver open a terminal in the directory with the source code and execute the following command:
```
$ sudo ./dkms-install.sh
```

### Removal of Driver
In order to remove the driver from your system open a terminal in the directory with the source code and execute the following command:
```
$ sudo ./dkms-remove.sh
```

### Build / Install with Make
For building & installing the driver with 'make' use
```
$ make
$ make install
```

### Using hostapd?
Check the documents folder for more information on how to setup wpa_cli and hostapd
support on these drivers. <br>For 802.11ac (high speed), the driver also needs to be loaded
with: <br>"modprobe -r 88XXau" && "modprobe 88XXau rtw_vht_enable=2"
```
# For a bit more info on AP/STA mode configuration, see following links:
  - http://blog.fraggod.net/2017/04/27/wifi-hostapd-configuration-for-80211ac-networks.html
  - https://github.com/mk-fg/archlinux-pkgbuilds/issues/2#issuecomment-325991813
```

### Download / Build / Install
Download
```
$ git clone -b v5.2.20 https://github.com/aircrack-ng/rtl8812au.git
$ cd rtl*
```
Package / Build dependencies (Kali)
```
$ apt-get install build-essential
$ apt-get install bc
$ apt-get install libelf-dev
$ apt-get install linux-headers-`uname -r`
```
For Raspberry (RPI 2/3) you will need kernel sources
```
$ wget "https://raw.githubusercontent.com/notro/rpi-source/master/rpi-source" -O /usr/bin/rpi-source
$ chmod 755 /usr/bin/rpi-source
$ rpi-source 
```
Then you need to download and compile the driver on the RPI
```
$ git clone https://github.com/aircrack-ng/rtl8812au -b v5.2.20
$ cd rtl*
$ make
$ cp 8812au.ko /lib/modules/`uname -r`/kernel/drivers/net/wireless
$ depmod -a
$ modprobe 88XXau
```
then run this step to change platform in Makefile, For RPI 2/3:
```
$ sed -i 's/CONFIG_PLATFORM_I386_PC = y/CONFIG_PLATFORM_I386_PC = n/g' Makefile
$ sed -i 's/CONFIG_PLATFORM_ARM_RPI = n/CONFIG_PLATFORM_ARM_RPI = y/g' Makefile
```
But for RPI 3 B+ you will need to run those below
which builds the ARM64 arch driver:
```
$ sed -i 's/CONFIG_PLATFORM_I386_PC = y/CONFIG_PLATFORM_I386_PC = n/g' Makefile
$ sed -i 's/CONFIG_PLATFORM_ARM64_RPI = n/CONFIG_PLATFORM_ARM64_RPI = y/g' Makefile
```

### For setting monitor mode
  1. Fix problematic interference in monitor mode. 
  ```
  $ airmon-ng check kill
  ```
  You may also uncheck the box "Automatically connect to this network when it is avaiable" in nm-connection-editor. This only works if you have a saved wifi connection.
  
  2. Set interface down
  ```
  $ ip link set <wlan1> down
  ``` 
  3. Set monitor mode
  ```
  $  airmon-ng start <wlan1>
  or
  $ iw dev <wlan1> set type monitor
  ```
  4. Set interface up
  ```
  $ ip link set <wlan1> up
  ```
For setting TX power
```
$ iwconfig <wlan1> txpower 30
or
$ iw <wlan1> set txpower fixed 3000
```
### LED control

#### You can now control LED behaviour statically by Makefile, for example:

```sh
CONFIG_LED_ENABLE = n
```
value can be y or n

#### statically by module parameter in /etc/modprobe.d/8812au.conf or whatever, example:

```sh
options 88XXau rtw_led_enable=0
```
value can be 0 or 1

#### or dynamically by writing to /proc/net/rtl8812au/$(your interface name)/led_enable, for example:

```sh
$ echo "0" > /proc/net/rtl8812au/$(your interface name)/led_enable
```
value can be 0 or 1

#### check current value:

```sh
$ cat /proc/net/rtl8812au/$(your interface name)/led_enable
```

### Testing / Debug
```
Check "documents" and "tools" folders for information
We also got a Android APK tool for debugging several chipsets
```
![https://github.com/aircrack-ng/rtl8812au/blob/v5.2.20/documents/Screenshot_20190129-025608-01.jpeg](https://github.com/aircrack-ng/rtl8812au/blob/v5.2.20/documents/Screenshot_20190129-025608-01.jpeg)

### NetworkManager

Newer versions of NetworkManager got some options you might want to disable.
Simply add these lines into the NetworkManager.conf
```
[device]
wifi.scan-rand-mac-address=no

[ifupdown]
managed=false

[connection]
wifi.powersave=2
```
at the end of file /etc/NetworkManager/NetworkManager.conf and restart NetworkManager with the command:
```
$ sudo service NetworkManager restart
```
