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
git clone https://github.com/aircrack-ng/rtl8812au.git
cd rtl*
```
To download v4.3.21
```
git clone -b v4.3.21 https://github.com/aircrack-ng/rtl8812au.git
```
Maybe you should install some packet to build it.
```
sudo apt-get install build-essential
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
For setting TX power (v4.3.21 branch only):
```
sudo iwconfig wlan0 txpower 30
```
or
```
sudo iw wlan0 set txpower fixed 3000
```
For Ubuntu 17.04 add the following lines
```
[device]
wifi.scan-rand-mac-address=no
```
at the end of file /etc/NetworkManager/NetworkManager.conf and restart NetworkManager with the command:
```
sudo service NetworkManager restart
```

## Credits
```
astsam - for the main work + monitor/injection support - https://github.com/astsam
```

## Other Sources
```
astsam     - https://github.com/astsam/rtl8812au
gnab       - https://github.com/gnab/rtl8812au
zebulon2   - https://github.com/zebulon2/rtl8812au
paspro     - https://github.com/paspro/rtl8812au
ulli-kroll - https://github.com/ulli-kroll/rtl8821au
tpircher   - https://github.com/tpircher/rtl8814AU
xxNull-lsk - https://github.com/xxNull-lsk/rtl8812AU
```
