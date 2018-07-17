# rtl8812au

## Realtek 8812AU driver v5.2.20.2 with monitor mode and frame injection

This driver supports 8812au & 8821au chipsets, not the 8814au chipset at this moment.

### Building / Installing

To build and install module manually:
```
$ sudo make
$ sudo make install
```

To use DKMS install:

```
$ sudo ./dkms-install.sh
```

To use DKMS uninstall and remove:

```
$ sudo ./dkms-remove.sh
```

### Notes
Download
```
git clone -b v5.2.20 https://github.com/aircrack-ng/rtl8812au.git
cd rtl*
```
Package / Build dependencies (Kali)
```
sudo apt-get install build-essential
sudo apt-get install bc
sudo apt-get install libelf-dev
sudo apt-get install dkms
sudo apt-get install linux-headers-`uname -r`
```
Building for  Raspberry (RPI) also needs
```
sudo apt install raspberrypi-kernel-headers
```

### For setting monitor mode
1. Fix problematic interference in monitor mode. 
```
airmon-ng check kill
```
You may also uncheck the box "Automatically connect to this network when it is avaiable" in nm-connection-editor. This only works if you have a saved wifi connection.

2. Set interface down
```
sudo ip link set wlan0 down
``` 
3. Set monitor mode
```
sudo iw dev wlan0 set type monitor
```
4. Set interface up
```
sudo ip link set wlan0 up
```

### TX Power Control

The txpower may be adjusted with these commands
```
ifconfig <adapter> down
iwconfig <adapter> txpower 30
```

### LED control

#### You can now control LED behaviour statically by Makefile, for example:

```sh
CONFIG_LED_ENABLE = n
```
value can be y or n

#### statically by module parameter in /etc/modprobe.d/8812au.conf or wherever, for example:

```sh
options 8812au rtw_led_enable=0
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

### NetworkManager

Newer versions of NetworkManager switches to random MAC address. Some users would prefer to use a fixed address. 
Simply add these lines below
```
[device]
wifi.scan-rand-mac-address=no
```
at the end of file /etc/NetworkManager/NetworkManager.conf and restart NetworkManager with the command:
```
sudo service NetworkManager restart
