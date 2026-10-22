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
[00:00:00.028,488] <inf> lte_getaddr: Connecting to LTE network.
[00:00:00.528,271] <inf> power_domain_gpio: modem_dcdc is now ON
[00:00:00.628,441] <inf> lte_getaddr: Waiting for LTE.
[00:00:05.230,894] <inf> lte_getaddr: Connected to LTE network.
[00:00:05.230,902] <inf> lte_getaddr: Resolving www.appelsiini.net.
[00:00:05.528,724] <inf> lte_getaddr: IP: 185.111.111.154
```
