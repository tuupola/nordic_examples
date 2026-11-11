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
[00:27:02.500,715] <inf> board_info: board=xiao_nrf54lm20a soc=nrf54lm20a
[00:27:02.500,722] <inf> board_info: part=0x054bc20a variant=AAB0
[00:27:02.500,728] <inf> board_info: package=0x00005041 ram=512 kB rram=2036 kB
...
ctrl-a ctrl-x
```
