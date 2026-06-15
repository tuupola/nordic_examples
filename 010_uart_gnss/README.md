## Build

Make sure you are in SDK shell to have access to the toolchain.

```
$ nrfutil sdk-manager toolchain launch --ncs-version v3.2.1 --shell
(v3.2.1) host% source ~/ncs/v3.2.1/zephyr/zephyr-env.sh
(v3.2.1) host% west build -b thingy91x/nrf9151/ns --pristine
```

Flash with `west`. You do must be in SDK shell for this.

```
(v3.2.1) host% west --verbose flash --recover
```

Flash with `nrfutil`. You do not need to be in SDK shell for this.

```
$ SERIAL_NUMBER=$(nrfutil device list | grep THINGY)
$ nrfutil device program --firmware build/dfu_application.zip --serial-number $SERIAL_NUMBER
```

## Connect

```
$ JLinkRTTLogger -device NRF9151_XXCA -if SWD -speed 4000 -RTTChannel 0
...
[00:00:01.592,346] <inf> main: GNSS

                               2c 2c 2c 2c 2c 2c 2c 2c  2c 2c 4e 2c 56 2a 33 37 |,,,,,,,, ,,N,V*37
                               0d 0a 24 47 4e 56 54 47  2c 2c 2c 2c 2c 2c 2c 2c |..$GNVTG ,,,,,,,,
                               2c 4e 2a 32 45 0d 0a 24  47 4e 47 47 41 2c 2c 2c |,N*2E..$ GNGGA,,,
                               2c 2c 2c 30 2c 30 30 2c  39 39 2e 39 39 2c 2c 2c |,,,0,00, 99.99,,,
                               2c 2c 2c 2a 35 36 0d 0a  24 47 4e 47 53 41 2c 41 |,,,*56.. $GNGSA,A
                               2c 31 2c 2c 2c 2c 2c 2c  2c 2c 2c 2c 2c 2c 2c 39 |,1,,,,,, ,,,,,,,9
                               39 2e 39 39 2c 39 39 2e  39 39 2c 39 39 2e 39 39 |9.99,99. 99,99.99
                               2c 31 2a 33 33 0d 0a 24  47 4e 47 53 41 2c 41 2c |,1*33..$ GNGSA,A,
                               31 2c 2c 2c 2c 2c 2c 2c  2c 2c 2c 2c 2c 2c 39 39 |1,,,,,,, ,,,,,,99
                               2e 39 39 2c 39 39 2e 39  39 2c 39 39 2e 39 39 2c |.99,99.9 9,99.99,
                               33 2a 33 31 0d 0a 24 47  4e 47 53 41 2c 41 2c 31 |3*31..$G NGSA,A,1
                               2c 2c 2c 2c 2c 2c 2c 2c  2c 2c 2c 2c 2c 39 39 2e |,,,,,,,, ,,,,,99.
                               39 39 2c 39 39 2e 39 39  2c 39 39 2e 39 39 2c 34 |99,99.99 ,99.99,4
                               2a 33 36 0d 0a 24 47 4e  47 53 41 2c 41 2c 31 2c |*36..$GN GSA,A,1,
                               2c 2c 2c 2c 2c 2c 2c 2c  2c 2c 2c 2c 39 39 2e 39 |,,,,,,,, ,,,,99.9
                               39 2c 39 39 2e 39 39 2c  39 39 2e 39 39 2c 35 2a |9,99.99, 99.99,5*
```
