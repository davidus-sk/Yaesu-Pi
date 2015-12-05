/**
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
	string serial_port;
	bool lock = false, unlock = false, status = false, rx_status = false, tx_status = false, verbose = false, json = false;

	while ((option_char = getopt(argc, argv, ":f:m:d:luhtrsvj")) != -1) {
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
				serial_port = optarg;
				struct stat buffer;

				if (stat (serial_port.c_str(), &buffer) != 0) {
					cout << argv[0] << ": Invalid serial device: " << serial_port << endl << endl;
					return -1;
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

	// create CAT object
	Cat * cat = new Cat(B9600, serial_port.c_str());
	cat->setVerbose(verbose);
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

	if (json) {
		cat->Json();
	}

	delete cat;

	return 1;
}

void show_help(char *s)
{
	cout << "Usage: " << s << " -d <to callsign> -s <from callsign> [-p <path>] [-vj]" << endl << endl;
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
	cout << " -j output JSON formatted text" << endl;
}
