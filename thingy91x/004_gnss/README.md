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
[00:00:00.291,748] <inf> gnss: Initializing the modem firmware.
[00:00:00.627,593] <inf> gnss: GNSS started.
...
[00:02:06.423,645] <inf> gnss: NRF_MODEM_GNSS_EVT_PVT
[00:02:06.689,758] <inf> gnss:   SV  25: cn0=28.4 el=0 az=0 *
[00:02:06.689,788] <inf> gnss:   SV  12: cn0=27.7 el=0 az=0 *
[00:02:06.689,849] <inf> gnss:   SV   5: cn0=27.9 el=0 az=0 *
[00:02:06.689,910] <inf> gnss:   SV  29: cn0=38.4 el=0 az=0 *
...
[00:04:41.452,331] <inf> gnss: NRF_MODEM_GNSS_EVT_FIX
[00:04:41.452,331] <inf> gnss: NRF_MODEM_GNSS_EVT_PVT
[00:04:41.742,126] <inf> gnss:   lat=62.738685 lon=22.082958
[00:04:41.742,156] <inf> gnss:   SV  25: cn0=32.2 el=53 az=126 *
[00:04:41.742,218] <inf> gnss:   SV  12: cn0=30.4 el=12 az=113 *
[00:04:41.742,248] <inf> gnss:   SV   5: cn0=27.0 el=10 az=114 *
[00:04:41.742,309] <inf> gnss:   SV  29: cn0=38.3 el=60 az=205 *
...
ctrl-a ctrl-x
```
