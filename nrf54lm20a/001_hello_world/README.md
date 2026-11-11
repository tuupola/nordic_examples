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
[00:00:00.000,879] <inf> hello_world: board=xiao_nrf54lm20a soc=nrf54lm20a
[00:00:00.000,883] <inf> hello_world: Hello world!
[00:00:01.000,966] <inf> hello_world: Hello world!
[00:00:02.001,045] <inf> hello_world: Hello world!
...
ctrl-a ctrl-x
```
