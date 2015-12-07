# Yaesu-Pi - Raspberry Pi and Yeasu FT8xx fusion



## Command line control: yaesu
Compile code using `g++ -O3 -std=c++0x -o yaesu yaesu.cpp cat.cpp`. Once compiled you can use the binary to control your radio from command line or remotely with a simple PHP (or other web-based language) wrapper.

**Your transciever is controlled using various parameters:**

* `-d <serial device>` Please supply path to your serial device, for example /dev/ttyUSB0. [required]
* `-b <serial speed>` Default is set to 9600 baud. You can set it to 2400, 4800 and 9600. [optional]
* `-p <on/off>` Key transmitter. Allowed values are "on" and "off". [optional]
* `-l <on/off>` Lock/unlosk the front control panel of the transceiver. [optional]
* `-m <mode>` Set operating mode, for example USB, LSB, CW, DIG, PKT, FM, AM. [optional]
* `-f <frequency>` Set operating frequency, for example 14.190. [optional]
* `-r` Get receiver status such as signal strength. [optional]
* `-t` Get transmitter status such as power output. [optional]
* `-s` Get operating frequency and mode. [optional]
* `-v` Output various debug information. [optional]
* `-j` Output status fields in JSON format. [optional]

**Examples:**

Set operating mode and frequency: `./yaesu -d /dev/ttyUSB0 -f 14.190 -m USB`. Get receiver status in JSON format: `./yaesu -d /dev/ttyUSB0 -r -s -j`.

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

This code is very simple and needs to be strengthened. Use this example only as basis for your own much more robust and secure application.
