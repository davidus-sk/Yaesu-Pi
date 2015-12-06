/**
 * Raspberry Pi and Yeasu FT8xx fusion project
 *
 * This file is part of Yaesu-Pi
 *
 * (c) 2015 David Ponevac (david at davidus dot sk) www.davidus.sk
 *
 * https://github.com/davidus-sk/Yaesu-Pi
 *
 * You are free to use, modify, extend, do whatever you like.
 * Please add attribution to your code.
 *
 * How to compile: g++ -O3 -std=c++0x -o yaesu yaesu.cpp cat.cpp
 */
#include "cat.h"
#include <sys/stat.h>
#include <algorithm>
#include <cstring>

using namespace std;

void show_help(char *s);

int main(int argc, char **argv)
{
	int option_char;

	double frequency = -1;
	int mode = -1;
	string serial_device;
	int serial_speed;
	bool lock = false, unlock = false, status = false, rx_status = false, tx_status = false, verbose = false, json = false;

	while ((option_char = getopt(argc, argv, ":f:m:d:b:luhtrsvj")) != -1) {
		switch(option_char) {
			// set frequency
			case 'f':
				frequency = stod(optarg, nullptr);

				if (frequency <= 0 || frequency >= 1000) {
					cout << argv[0] << ": Invalid frequency: " << frequency << ". Allowed range: 0 < f < 1000 MHz." << endl;
					return -1;
				}

				break;

			// set operating mode
			case 'm': {
					string text_mode = optarg;

					try {
						mode = Cat::OP_MODES.at(text_mode);
					} catch (const out_of_range& oor) {
						cout << argv[0]  << ": Invalid operating mode: " << text_mode << endl << endl;
						return -1;
					}
				} break;

			// set serial device
			case 'd':
				serial_device = optarg;
				struct stat buffer;

				if (stat(serial_device.c_str(), &buffer) != 0) {
					cout << argv[0] << ": Invalid serial device: " << serial_device << endl << endl;
					return -1;
				}

				break;

			// set serial speed
			case 'b':
				serial_speed = stoi(optarg, nullptr);

				if (serial_speed != 4800 && serial_speed != 9600) {
					cout << argv[0] << ": Setting port speed to 9600 bauds." << endl << endl;
					serial_speed = 9600;
				}

				break;

			// lock tcvr
			case 'l':
				lock = true;
				break;

			// unlock tcvr
			case 'u':
				unlock = true;
				break;

			// get RX status
			case 'r':
				rx_status = true;
				break;

			case 't':
				tx_status = true;
				break;

			// receiver status
			case 's':
				status = true;
				break;

			// verbose output
			case 'v':
				verbose = true;
				break;

			// show help
			case 'h':
				show_help(argv[0]);
				return 0;

			case 'j':
				verbose = false;
				json = true;
				break;

			// missing required value
			case ':':
				cout << argv[0] << ": Missing required parameter!" << endl << endl;
				show_help(argv[0]);
				return -1;

			case '?':
				cout << argv[0] << ": Unknown parameter!" << endl << endl;
				show_help(argv[0]);
				return -1;
		}
	}

	if (serial_device.empty()) {
		cout << argv[0] << ": Please specify serial device attached to your transciever!" << endl << endl;
		return -1;
	}

	// create CAT object
	Cat * cat = new Cat();
	cat->SetVerbose(verbose && !json);
	cat->Connect(serial_device, serial_speed);

	// try to get status to check tcvr is reachable
	bool tcvr_responding = cat->GetFrequencyModeStatus();

	if (!tcvr_responding) {
		cout << argv[0] << ": Transciever is not responding!" << endl << endl;
		return -1;
	}

	// lock
	if (lock) {
		cat->Lock(true);
	}

	// unlock
	if (unlock) {
		cat->Lock(false);
	}

	// set mode
	if (mode >= 0) {
		cat->SetOperatingMode(mode);
	}

	// set frequency
	if (frequency > 0) {
		cat->SetFrequency(frequency);
	}

	// get frequency and mode status
	if (status) {
		cat->GetFrequencyModeStatus();
	}

	// get RX status
	if (rx_status) {
		cat->GetRxStatus();
	}

	// get TX status
	if (tx_status) {
		cat->GetTxStatus();
	}

	// output status message in JSON format
	if (json) {
		cat->Json();
	}

	delete cat;

	return 1;
}

/**
 * Show help message
 * @param char* s This executable
 * @return void
 */
void show_help(char *s)
{
	cout << "Raspberry Pi and Yeasu FT8xx fusion" << endl << endl;

	cout << "Usage: " << endl;
	cout << " " << s << " -d <serial device> [-f <frequency in MHz>] [-m <operating mode>]  [-lurtsvj]" << endl << endl;

	cout << "Options:" << endl;
	cout << " -d serial device (e.g. /dev/ttyUSB0)" << endl;
	cout << " -l lock transciever" << endl;
	cout << " -u unlock transciever" << endl;
	cout << " -m set operaring mode (CW, USB, LSB, ...)" << endl;
	cout << " -f set frequency in MHz (e.g. 14.190)" << endl;
	cout << " -r get receiver status" << endl;
	cout << " -t get transmitter status" << endl;
	cout << " -s get current frequency and mode" << endl;
	cout << " -v verbose output" << endl;
	cout << " -j output JSON formatted text" << endl << endl;

	cout << "Examples:" << endl;
	cout << " " << s << " -d /dev/ttyUSB0  -f 14.190 -m USB" << endl; 
}
