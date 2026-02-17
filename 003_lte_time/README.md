## Build

Make sure you are in SDK shell to have access to the toolchain.

```
$ nrfutil sdk-manager toolchain launch --ncs-version v3.2.1 --shell
(v3.2.1) host% source ~/ncs/v3.2.1/zephyr/zephyr-env.sh
(v3.2.1) host% west build -b thingy91x/nrf9151/ns
```

Flash with `nrfutil`. You do not need to be in SDK shell for this.

```
$ SERIAL_NUMBER=$(nrfutil device list | grep THINGY)
$ nrfutil device program --firmware build/dfu_application.zip --serial-number $SERIAL_NUMBER
```

## Connect 

```
$ picocom -b 115200 /dev/ttyACM0
Terminal ready
*** Booting nRF Connect SDK v3.2.1-d8887f6f32df ***
*** Using Zephyr OS v4.2.99-ec78104f1569 ***
[00:00:00.291,839] <inf> lte_time: Initializing the modem firmware.
[00:00:00.628,051] <inf> lte_time: Connecting to LTE network.
[00:00:04.384,887] <inf> lte_time: Connected to LTE network.
[00:00:04.384,887] <wrn> date_time: Valid time not currently available
[00:00:04.384,918] <wrn> lte_time: Time not available: -61
[00:00:05.385,070] <inf> lte_time: UTC: 2026-02-17 11:09:23
[00:00:06.385,192] <inf> lte_time: UTC: 2026-02-17 11:09:24
[00:00:07.385,375] <inf> lte_time: UTC: 2026-02-17 11:09:25
[00:00:08.385,498] <inf> lte_time: UTC: 2026-02-17 11:09:26

...
ctrl-a ctrl-x
```
