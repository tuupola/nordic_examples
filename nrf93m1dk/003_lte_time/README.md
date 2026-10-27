## Build

NRF93M1DK needs recent NCS. Make sure you are in the SDK shell.

```
$ nrfutil sdk-manager toolchain launch --ncs-version v3.5.0-preview4 --shell
(v3.5.0-preview4) host% source ~/ncs/v3.5.0-preview4/zephyr/zephyr-env.sh
(v3.5.0-preview4) host% west build -b nrf93m1dk/nrf54l15/cpuapp --pristine
(v3.5.0-preview4) host% west flash
```

## Connect

```
$ picocom -b 115200 /dev/ttyACM0
...
*** Booting nRF Connect SDK v3.4.99-0581fccc95ff ***
*** Using Zephyr OS v4.4.99-474bc1411799 ***
[00:00:00.028,578] <inf> main: Connecting to LTE network.
[00:00:00.528,279] <inf> power_domain_gpio: modem_dcdc is now ON
[00:00:00.628,461] <inf> main: Waiting for LTE.
[00:00:05.173,647] <inf> main: Connected to LTE network.
[00:00:05.173,663] <wrn> date_time: Valid time not currently available
[00:00:05.173,667] <wrn> main: Time not available: -61
[00:00:06.173,750] <inf> main: UTC: 2026-10-22 14:10:45
[00:00:07.173,870] <inf> main: UTC: 2026-10-22 14:10:46
[00:00:08.173,978] <inf> main: UTC: 2026-10-22 14:10:47
[00:00:09.174,079] <inf> main: UTC: 2026-10-22 14:10:48
```
