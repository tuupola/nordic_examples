## Build

Make sure you are in SDK shell to have access to the toolchain.
XIAO nRF54LM20A needs udev rules and recent NCS.

```
$ cat /etc/udev/rules.d/99-seeed-xiao.rules

SUBSYSTEM=="hidraw", ATTRS{idVendor}=="2886", ATTRS{idProduct}=="0068", MODE="0660", GROUP="plugdev"
SUBSYSTEM=="usb", ATTR{idVendor}=="2886", ATTR{idProduct}=="0068", MODE="0666"
```

```
$ nrfutil sdk-manager toolchain launch --ncs-version v3.5.0-preview4 --shell
(v3.5.0-preview4) host% source ~/ncs/v3.5.0-preview4/zephyr/zephyr-env.sh
(v3.5.0-preview4) host% west build -b xiao_nrf54lm20a/nrf54lm20a/cpuapp --pristine
(v3.5.0-preview4) host% west flash
```

## Connect

```
$ picocom -b 115200 /dev/ttyACM0
Terminal ready
*** Booting nRF Connect SDK v3.4.99-0581fccc95ff ***
*** Using Zephyr OS v4.4.99-474bc1411799 ***
[00:03:57.692,674] <inf> main: GNSS ready
[00:03:58.611,659] <inf> main: lat=0 lon=0 fix=0 sats=0
gnss: gnss_info: {satellites_cnt: 0, hdop: 0.0, fix_status: NO_FIX, fix_quality: INVALID}
gnss: navigation_data: {latitude: 0.000000000, longitude : 0.000000000, bearing 0.000, speed 0.000, altitude: 0.000}
gnss: gnss_time: {hour: 0, minute: 0, millisecond 0, month_day 0, month: 0, century_year: 0}
gnss: gnss_satellite: {prn: 6, snr: 32, elevation 0, azimuth 0, system: GPS, is_tracked: 1}
gnss: gnss_satellite: {prn: 11, snr: 33, elevation 0, azimuth 0, system: GPS, is_tracked: 1}
gnss: gnss_satellite: {prn: 17, snr: 29, elevation 0, azimuth 0, system: GPS, is_tracked: 1}
gnss: gnss_satellite: {prn: 19, snr: 23, elevation 0, azimuth 0, system: GPS, is_tracked: 1}
gnss: gnss_satellite: {prn: 26, snr: 31, elevation 0, azimuth 0, system: SBAS, is_tracked: 1}
[00:03:59.610,377] <inf> main: lat=0 lon=0 fix=0 sats=0
...
ctrl-a ctrl-x
```
