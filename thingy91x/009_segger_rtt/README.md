## Build

Make sure you are in SDK shell to have access to the toolchain.

```
$ nrfutil sdk-manager toolchain launch --ncs-version v3.2.1 --shell
(v3.2.1) host% source ~/ncs/v3.2.1/zephyr/zephyr-env.sh
```

You can build either for the `nRF9151` or the `nRF5340` chip.

```
(v3.2.1) host% west build -b thingy91x/nrf9151/ns --pristine
(v3.2.1) host% west build -b thingy91x/nrf5340/cpuapp --pristine
```

Flash with `west`. Make sure the correct chip is selected in  the hardware 
chip select switch on the side of the Thingy91:X. If not you will get a 
flashing error.

```
(v3.2.1) host% west flash --recover 
```

## Connect 

Note that for some reason `-device NRF5340_XXAA_APP` does not work for `nRF5340`. 
Instead you must use `-device NRF9151_XXCA` for both.

```
$ JLinkRTTLogger -device NRF9151_XXCA -if SWD -speed 4000 -RTTChannel 0 /dev/stdout
SEGGER J-Link RTT Logger
Compiled Feb  6 2026 12:45:47
(c) 2016-2017 SEGGER Microcontroller GmbH, www.segger.com
         Solutions for real time microcontroller applications

Default logfile path: /home/user/.config/SEGGER

------------------------------------------------------------ 


------------------------------------------------------------ 
Connected to:
  SEGGER J-Link (unknown)
  S/N: 801006468

Searching for RTT Control Block...OK.
3 up-channels found:
0: Terminal
1: 
2: 
Selected RTT Channel description: 
  Index: 0
  Name:  Terminal
  Size:  1024 bytes.

Output file: /dev/stdout

Getting RTT data from target. Press any key to quit.
------------------------------------------------------------ 

Transfer rate: 0 Bytes/s Data written: 0 Bytes [00:00:00.255,462] <inf> spi_nor: GD25LE255E@0: 32 MiBy flash
*** Booting nRF Connect SDK v3.2.1-d8887f6f32df ***
*** Using Zephyr OS v4.2.99-ec78104f1569 ***
[00:00:00.000,457] <inf> segger_rtt: Running on thingy91x (nrf5340)
Transfer rate: 0 Bytes/s Data written: 176 Bytes 
[00:00:01.000,549] <inf> segger_rtt: Running on thingy91x (nrf5340)
[00:00:02.000,732] <inf> segger_rtt: Running on thingy91x (nrf5340)
Transfer rate: 0 Bytes/s Data written: 330 Bytes 
[00:00:03.000,823] <inf> segger_rtt: Running on thingy91x (nrf5340)
[00:00:04.001,007] <inf> segger_rtt: Running on thingy91x (nrf5340)
Transfer rate: 0 Bytes/s Data written: 484 Bytes 
[00:00:05.001,098] <inf> segger_rtt: Running on thingy91x (nrf5340)
[00:00:06.001,281] <inf> segger_rtt: Running on thingy91x (nrf5340)
Transfer rate: 0 Bytes/s Data written: 638 Bytes 
[00:00:07.001,373] <inf> segger_rtt: Running on thingy91x (nrf5340)
[00:00:08.001,556] <inf> segger_rtt: Running on thingy91x (nrf5340)
```
