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

** Booting nRF Connect SDK v3.2.1-d8887f6f32df ***
*** Using Zephyr OS v4.2.99-ec78104f1569 ***
Attempting to boot slot 0.
Attempting to boot from address 0x8200.
I: Trying to get Firmware version
I: Verifying signature against key 0.
I: Hash: 0x3e...f9
I: Firmware signature verified.
Firmware version 2
[00:00:00.753,723] <inf> spi_nor: GD25LE255E@0: 32 MiBy flash
[00:00:00.756,103] <err> gpio_hogs: GPIO port npm1300_gpios not ready
*** Booting nRF Connect SDK v3.2.1-d8887f6f32df ***
*** Using Zephyr OS v4.2.99-ec78104f1569 ***
[00:00:00.756,225] <inf> main: Listening on UART1 (P0.18 RX / P0.19 TX) at 460800 baud
[00:00:00.780,029] <inf> main: ATTITUDE_QUATERNION id=31 from sys=1 comp=1
[00:00:00.780,151] <inf> main: ATTITUDE id=30 from sys=1 comp=1
[00:00:00.780,334] <inf> main: HIGHRES_IMU id=105 from sys=1 comp=1
[00:00:00.780,456] <inf> main: LOCAL_POSITION_NED id=32 from sys=1 comp=1
[00:00:00.780,578] <inf> main: ATTITUDE id=30 from sys=1 comp=1
[00:00:00.791,503] <inf> main: ATTITUDE_QUATERNION id=31 from sys=1 comp=1
[00:00:00.791,625] <inf> main: ATTITUDE id=30 from sys=1 comp=1
[00:00:00.791,717] <inf> main: TIMESYNC id=111 from sys=1 comp=1
[00:00:00.791,839] <inf> main: ALTITUDE id=141 from sys=1 comp=1
[00:00:00.791,992] <inf> main: ATTITUDE_TARGET id=83 from sys=1 comp=1
[00:00:00.792,053] <inf> main: EXTENDED_SYS_STATE id=245 from sys=1 comp=1
[00:00:00.792,205] <inf> main: POSITION_TARGET_LOCAL_NED id=85 from sys=1 comp=1
...
ctrl-a ctrl-x
```

Also if you check a NuttX shell in a connected PX4 flight controller you should see this 
device.

```
nsh> mavlink status
...
instance #1:
    mavlink chan: #1
    type:        GENERIC LINK OR RADIO
    flow control: OFF
    rates:
      tx: 18040.9 B/s
      txerr: 0.0 B/s
      tx rate mult: 0.814
      tx rate max: 23040 B/s
      rx: 21.0 B/s
      rx loss: 4.2%
    Received Messages:
      sysid:  1, compid:191, Total: 29 (lost: 121)
    FTP enabled: YES, TX enabled: YES
    mode: Onboard
    Forwarding: Off
    MAVLink version: 2
    transport protocol: serial (/dev/ttyS3 @460800)
...
```
