# Yaesu-Pi - Raspberry Pi and Yaesu FT8xx fusion

*Phase 1: Control transciever from command line and remotely over TCP/IP socket*

*Phase 2: Transmit audio from and to transciever over TCP/IP*

*Phase 3: Build a remote controling device using Raspberry Pi*

## Command line control: yaesu
Compile code using `g++ -O3 -std=c++0x -o yaesu yaesu.cpp cat.cpp`. Once compiled you can use the binary to control your radio from command line or remotely with a simple PHP (or other web-based language) wrapper.

**Your transciever is controlled using various parameters:**

* `-d <serial device>` Please supply path to your serial device, for example /dev/ttyUSB0. [required]
* `-b <serial speed>` Default is set to 9600 baud. You can set it to 2400, 4800 and 9600 (default). [optional]
* `-p <on/off>` Key transmitter. Allowed values are "on" and "off". [optional]
* `-l <on/off>` Lock/unlosk the front control panel of the transceiver. [optional]
* `-m <mode>` Set operating mode, for example USB, LSB, CW, DIG, PKT, FM, AM. [optional]
* `-f <frequency>` Set operating frequency in MHz, for example 14.190. [optional]
* `-r` Get receiver status such as signal strength. [optional]
* `-t` Get transmitter status such as power output. [optional]
* `-s` Get operating frequency and mode. [optional]
* `-v` Output various debug information. [optional]
* `-j` Output status fields in JSON format. [optional]

**Examples:**

Set operating mode and frequency: `./yaesu -d /dev/ttyUSB0 -f 14.190 -m USB`.

Get receiver status in JSON format: `./yaesu -d /dev/ttyUSB0 -r -s -j`.

### Controlling via PHP
Using simple PHP script you can control your transceiver from a website. Remember you have to have web-server installed on the Pi connected to your transceiver and then you can do something like this: `http://pi_address/yaesu.php?f=14.190&m=USB`.

```
<?php
// collect data from GET request
$frequency = $_GET['f'] * 1;
$mode = $_GET['m'];

// allowed modes
if (in_array($mode, array('USB', 'LSB', 'CW', 'AM', 'FM', 'PKT', 'DIG'))) {
	// pass it to the binary
	$output = `/path/to/yaesu -d /dev/ttyUSB0 -f $frequency -m $mode -r -s -j`;
	$data = json_decode($output);

	// output status to browser
	if ($data) {
	  echo "Operating frequency: " . $data->tcvr_frequency . " MHz<br />";
	  echo "Operating mode: " . $data->tcvr_mode . "<br />";
	  echo "Signal: " . $data->rx_signal . "<br />";
	} else {
		echo "Unable to parse JSON output.<br />";
	}
}
```

This sample PHP code is very simple and needs to be strengthened. Use this example only as basis for your own much more robust and secure application.

### Troubleshooting

If you are unable to control your Yaesu transciever please make sure that it is connected to your Raspberry Pi via serial connection and that you are using the correct serial device with the compiled binary. Also make sure you are setting the correct baud rate. Check you transciever settings (Menu 019 CAT RATE) and adjust it to match your setup (e.g. use  9600 on both sides). Simple setup diagram:

```
[ Raspberry Pi ] --- serial-to-USB --- <> --- Yaesu data cable --- [ Yaesu Tcvr ]

OR

[ Raspberry Pi ] --- RX/TX/GND Pins --- [ Yaesu Tcvr ]
```

Remember Yaesu's native serial output is at TTL levels. The Yaesu data cable will change this to RS-232 levels. Thus if you use this cable, you need to use cheap Serail-to-USB cable. Do not connect the Yaesu serial cable directly to Pi's RX/TX pins!
