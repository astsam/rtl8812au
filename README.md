# rtl8812au

## Realtek 8812AU driver version 5.2.20

Only supports 8812AU chipset. 

Works fine with 4.15 kernel. Source now builds with no warnings or errors.

Added (cosmeticly edited) original Realtek_Changelog.txt, this README.md and dkms.conf.

Added device USB IDs, sorted by ID number.
Added LED control by Makefile, module parameter and dynamic /proc writing.
Added VHT extras.
Added regdb files.

### Building

To build and install module manually:
```sh
$ make
$ sudo make install
```

To use dkms install:

```sh
  (as root, or sudo) copy source folder contents to /usr/src/rtl8812au-5.2.20
```

```sh
$ sudo dkms add -m rtl8812au -v 5.2.20
$ sudo dkms build -m rtl8812au -v 5.2.20
$ sudo dkms install -m rtl8812au -v 5.2.20 
```

To use dkms uninstall and remove:

```sh
$ sudo dkms remove -m rtl8812au -v 5.2.20 --all
```

### LED control

Thanks to @dkadioglu and others for a start on this.

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

As others have noted, people using NetworkManager need to add this stanza to /etc/NetworkManager/NetworkManager.conf

```sh
  [device]
  wifi.scan-rand-mac-address=no
```

### Regdb files

If needed, copy the regulatory database files in regdb/ to /lib/firmware/

```sh
$ sudo cp ./regdb/* /lib/firmware/
```
