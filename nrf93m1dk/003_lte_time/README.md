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
[00:00:00.292,083] <inf> main: Initializing the modem firmware.
[00:00:00.627,746] <inf> main: Connecting to LTE network.
[00:00:02.048,156] <inf> main: Searching for LTE network...
[00:00:04.377,593] <inf> main: LTE network registered.
[00:00:04.377,685] <inf> main: Connected to LTE network.
[00:00:04.377,716] <wrn> date_time: Valid time not currently available
[00:00:04.377,716] <wrn> main: Time not available: -61
[00:00:05.377,868] <inf> main: UTC: 2026-02-19 14:30:31
[00:00:06.378,021] <inf> main: UTC: 2026-02-19 14:30:32
[00:00:07.378,204] <inf> main: UTC: 2026-02-19 14:30:33
[00:00:08.378,326] <inf> main: UTC: 2026-02-19 14:30:34
[00:00:09.378,509] <inf> main: UTC: 2026-02-19 14:30:35
[00:00:10.378,662] <inf> main: UTC: 2026-02-19 14:30:36
[00:00:11.378,845] <inf> main: UTC: 2026-02-19 14:30:37
[00:00:12.378,967] <inf> main: UTC: 2026-02-19 14:30:38
[00:00:13.379,150] <inf> main: UTC: 2026-02-19 14:30:39
[00:00:14.379,272] <inf> main: UTC: 2026-02-19 14:30:40
[00:00:15.379,455] <inf> main: UTC: 2026-02-19 14:30:41
[00:00:16.379,577] <inf> main: UTC: 2026-02-19 14:30:42
[00:00:17.379,760] <inf> main: UTC: 2026-02-19 14:30:43
[00:00:18.379,882] <inf> main: UTC: 2026-02-19 14:30:44
[00:00:19.380,065] <inf> main: UTC: 2026-02-19 14:30:45
[00:00:20.380,187] <inf> main: UTC: 2026-02-19 14:30:46
[00:00:21.380,371] <inf> main: UTC: 2026-02-19 14:30:47
[00:00:22.380,493] <inf> main: UTC: 2026-02-19 14:30:48
[00:00:23.380,676] <inf> main: UTC: 2026-02-19 14:30:49
[00:00:24.380,798] <inf> main: UTC: 2026-02-19 14:30:50
[00:00:25.380,981] <inf> main: UTC: 2026-02-19 14:30:51
[00:00:26.381,103] <inf> main: UTC: 2026-02-19 14:30:52
[00:00:27.381,286] <inf> main: UTC: 2026-02-19 14:30:53
[00:00:28.381,408] <inf> main: UTC: 2026-02-19 14:30:54
[00:00:29.381,591] <inf> main: UTC: 2026-02-19 14:30:55
[00:00:30.381,713] <inf> main: UTC: 2026-02-19 14:30:56
[00:00:31.381,896] <inf> main: UTC: 2026-02-19 14:30:57
[00:00:32.382,019] <inf> main: UTC: 2026-02-19 14:30:58
[00:00:33.382,202] <inf> main: UTC: 2026-02-19 14:30:59
[00:00:33.382,202] <inf> main: Forcing LTE disconnect for testing.
[00:00:33.582,977] <wrn> main: LTE disconnected (status: 0).
[00:00:34.920,623] <wrn> main: LTE disconnected, reconnecting...
[00:00:34.920,654] <inf> main: Connecting to LTE network.
[00:00:36.260,742] <inf> main: Searching for LTE network...
[00:00:37.908,813] <inf> main: LTE network registered.
[00:00:37.908,905] <inf> main: Connected to LTE network.
[00:00:37.908,935] <inf> main: UTC: 2026-02-19 14:31:04
...
ctrl-a ctrl-x
```
