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
[00:00:00.291,748] <inf> lte_ping: Initializing modem.
[00:00:00.627,593] <inf> lte_ping: Connecting to LTE network.
[00:00:04.616,668] <inf> lte_ping: Connected to LTE network.
...
ctrl-a ctrl-x
```
