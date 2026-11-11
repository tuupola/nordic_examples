## Build

Make sure you are in SDK shell to have access to the toolchain.
XIAO nRF54LM20A needs udev rules and recent NCS.

```
$ cat /etc/udev/rules.d/99-seeed-xiao.rules

SUBSYSTEM=="hidraw", ATTRS{idVendor}=="2886", ATTRS{idProduct}=="0068", MODE="0660", GROUP="plugdev"
SUBSYSTEM=="usb", ATTR{idVendor}=="2886", ATTR{idProduct}=="0068", MODE="0666"
```

```
$ nrfutil sdk-manager toolchain launch --ncs-version v3.5.0-preview4 --shell
(v3.5.0-preview4) host% source ~/ncs/v3.5.0-preview4/zephyr/zephyr-env.sh
(v3.5.0-preview4) host% west build -b xiao_nrf54lm20a/nrf54lm20a/cpuapp --pristine
(v3.5.0-preview4) host% west flash
```

## Connect

```
$ picocom -b 115200 /dev/ttyACM0
Terminal ready
*** Booting nRF Connect SDK v3.4.99-0581fccc95ff ***
*** Using Zephyr OS v4.4.99-474bc1411799 ***
[00:07:02.830,489] <inf> ble_scan: board=xiao_nrf54lm20a soc=nrf54lm20a
[00:07:02.830,537] <inf> bt_sdc_hci_driver: SoftDevice Controller build revision:
                                            7f 6d 61 ce 98 3f 7f 5f  3b 71 53 6d 22 19 9e 51 |.ma..?._ ;qSm"..Q
                                            50 13 73 79                                      |P.sy
[00:07:02.831,435] <inf> bt_hci_core: HW Platform: Nordic Semiconductor (0x0002)
[00:07:02.831,447] <inf> bt_hci_core: HW Variant: nRF54Lx (0x0005)
[00:07:02.831,457] <inf> bt_hci_core: Firmware: Standard Bluetooth controller (0x00) Version 127.24941 Build 2134874318
[00:07:02.831,818] <inf> bt_hci_core: HCI transport: SDC
[00:07:02.831,867] <inf> bt_hci_core: Identity: FC:6C:DB:6E:76:11 (random)
[00:07:02.831,879] <inf> bt_hci_core: HCI: version 6.3 (0x11) revision 0x30a1, manufacturer 0x0059
[00:07:02.831,889] <inf> bt_hci_core: LMP: version 6.3 (0x11) subver 0x30a1
[00:07:02.832,312] <inf> ble_scan: Bluetooth LE scan started...
...
ctrl-a ctrl-x
```
