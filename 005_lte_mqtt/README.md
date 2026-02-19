## Build

Make sure you are in SDK shell to have access to the toolchain.

```
$ nrfutil sdk-manager toolchain launch --ncs-version v3.2.1 --shell
(v3.2.1) host% source ~/ncs/v3.2.1/zephyr/zephyr-env.sh
(v3.2.1) host% west build -b thingy91x/nrf9151/ns --pristine
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
[00:00:00.291,748] <inf> main: Initializing the modem firmware.
[00:00:00.627,593] <inf> main: Connecting to LTE network.
[00:00:04.616,668] <inf> main: Connected to LTE network.
[00:00:05.957,061] <inf> main: MQTT connected.
[00:00:05.958,374] <inf> main: Published: Hello #0
[00:00:06.403,045] <inf> main: Subscribed (msg id 1)
[00:00:15.968,017] <inf> main: Published: Hello #1
[00:00:25.974,822] <inf> main: Published: Hello #2
[00:03:58.508,850] <inf> main: Received: foo on topic: thingy91x/command
...
ctrl-a ctrl-x
```

## Listen to MQTT messages
```
$ mosquitto_sub -h broker.emqx.io -t "thingy91x/hello"
Hello #1
Hello #2
...
```

## Sending a MQTT command to the device

```
$ mosquitto_pub -h broker.emqx.io -t "thingy91x/command" -m "foo"
```
