# RTL8812AU/21AU and RTL8814AU driver with monitor mode and frame injection


## DKMS
This driver can be installed using [DKMS](http://linux.dell.com/dkms/). This is a system which will automatically recompile and install a kernel module when a new kernel gets installed or updated. To make use of DKMS, install the `dkms` package, which on Debian (based) systems is done like this:
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
For building the RTL8812AU driver with 'make' use
```
make 
```
and for building the RTL8814AU driver with 'make' use
```
make RTL8814=1
```
but for installing both these drivers use these commands on e.g Kali on kernel v4.12
```
make
make RTL8814=1
make install
cp 8814au.ko /lib/modules/4.12.0-kali1-amd64/kernel/drivers/net/wireless/
```

## Notes
For Ubuntu 17.04 add the following lines
```
[device]
wifi.scan-rand-mac-address=no
```
at the end of file /etc/NetworkManager/NetworkManager.conf and restart NetworkManager with the command:
```
sudo service NetworkManager restart
