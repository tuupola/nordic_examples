## Build

Make sure you are in SDK shell to have access to the toolchain.

```
$ nrfutil sdk-manager toolchain launch --ncs-version v3.2.1 --shell
(v3.2.1) host% source ~/ncs/v3.2.1/zephyr/zephyr-env.sh
```

Build the main firmwre for the nRF9151 chip.

```
(v3.2.1) host% west build -b thingy91x/nrf9151/ns
(v3.2.1) host% west --verbose flash --recover
```

Build the Bluetooth firmware for the nrf5340 chip.

```
(v3.2.1) host% cd nrf5340
(v3.2.1) host% west build -b thingy91x/nrf5340/cpuapp --pristine
(v3.2.1) host% west --verbose flash --recover
```

See the logs of either chip. Need to choose the chip with the physical
switch first.

```
$ JLinkRTTLogger -device NRF9151_XXCA -if SWD -speed 4000 -RTTChannel 0 /dev/stdout | grep -v "Transfer rate"
```

## Scan Bluetooth

```
$ bluetoothctl
[bluetooth]# power on
[bluetooth]# scan le
```

Then in other terminal

```
$ stdbuf -oL tshark -i bluetooth-monitor \
  -Y "btcommon.eir_ad.entry.uuid_16 == 0xfffa" \
  -T fields -e btcommon.eir_ad.entry.service_data | rid
  
Capturing on 'bluetooth-monitor'
{"counter": 113, "protocol_version": 2, "message_type": 0, "id_type": 1, "ua_type": 2, "uas_id": "1ABCD2345EF678XYZ"}
```
