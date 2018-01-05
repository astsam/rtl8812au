# RTL8812AU/21AU and RTL8814AU drivers
# with monitor mode and frame injection

## DKMS
This driver can be installed using [DKMS]. This is a system which will automatically recompile and install a kernel module when a new kernel gets installed or updated. To make use of DKMS, install the `dkms` package, which on Debian (based) systems is done like this:
```
sudo apt install dkms
```

## Installation of Driver
In order to install the driver open a terminal in the directory with the source code and execute the following command:
```
sudo ./dkms-install.sh
```

## Removal of Driver
In order to remove the driver from your system open a terminal in the directory with the source code and execute the following command:
```
sudo ./dkms-remove.sh
```

## Make
For building & installing the RTL8812AU driver with 'make' use
```
make
make install
```
and for building & installing the RTL8814AU driver with 'make' use
```
make RTL8814=1
make install RTL8814=1
```

## Notes
Download
```
git clone -b v5.1.5 https://github.com/aircrack-ng/rtl8812au.git
cd rtl*
```
Package / Build dependencies
```
sudo apt-get install build-essential
sudo apt-get install bc
sudo apt-get install linux-headers-`uname -r`
```
For setting monitor mode
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
For setting TX power
```
sudo iwconfig wlan0 txpower 30
```
or
```
sudo iw wlan0 set txpower fixed 3000
```
Newer versions of NetworkManager switches to random MAC address. Some users would prefer to use a fixed address. 
Simply add these lines below
```
[device]
wifi.scan-rand-mac-address=no
```
at the end of file /etc/NetworkManager/NetworkManager.conf and restart NetworkManager with the command:
```
sudo service NetworkManager restart
```

## LED Parameter
```
We've added the "realtek-leds.conf" in build directory, 
with this you may change the leds to 
"2: Allways On", "1: Normal" or "0: Allways Off" with placing the file in "/etc/modprobe.d/

Manual modprobe will override this file if option value also included at the command line, e.g.,
$ sudo modprobe -r 8812au
$ sudo modprobe 8812au rtw_led_ctrl=1
```

## Credits
```
astsam    - for the main work + monitor/injection support        - https://github.com/astsam
evilphish - for great patching (USB3, VHT + txpower control +++) - https://github.com/evilphish
```
