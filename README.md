# rtl8812au

## Realtek 8812AU driver v5.2.20 with monitor mode and packet injection

Only supports 8812AU chipset, not the 8814AU or the 8821AU.


### Building

To build and install module manually:
```sh
$ make
$ sudo make install
```

To use dkms install:

```sh
$ sudo ./dkms-install.sh
```

To use dkms uninstall and remove:

```sh
$ sudo ./dkms-remove.sh
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

As others have noted, people using NetworkManager need to add this stanza to /etc/NetworkManager/NetworkManager.conf

```sh
  [device]
  wifi.scan-rand-mac-address=no
```
