#include "cat.h"

using namespace std;

// constants

const char Cat::CMD_LOCK_ON = 0x00;
const char Cat::CMD_SET_FREQUENCY = 0x01;
const char Cat::CMD_GET_FREQUENCY_MODE = 0x03;
const char Cat::CMD_SET_MODE = 0x07;
const char Cat::CMD_PTT_ON = 0x08;
const char Cat::CMD_LOCK_OFF = 0x80;
const char Cat::CMD_PTT_OFF = 0x88;
const char Cat::CMD_GET_RX_STATUS = 0xe7;
const char Cat::CMD_GET_TX_STATUS = 0xf7;

const char Cat::OP_MODE_LSB = 0x00;
const char Cat::OP_MODE_USB = 0x01;
const char Cat::OP_MODE_CW = 0x02;
const char Cat::OP_MODE_CWR = 0x03;
const char Cat::OP_MODE_AM = 0x04;
const char Cat::OP_MODE_WFM = 0x06;
const char Cat::OP_MODE_FM = 0x08;
const char Cat::OP_MODE_FMN = 0x88;
const char Cat::OP_MODE_DIG = 0x0a;
const char Cat::OP_MODE_PKT = 0x0c;

const map<string, char> Cat::OP_MODES {
	{"LSB", OP_MODE_LSB},
	{"USB", OP_MODE_USB},
	{"CW", OP_MODE_CW},
	{"CWR", OP_MODE_CWR},
	{"AM", OP_MODE_AM},
	{"WFM", OP_MODE_WFM},
	{"FM", OP_MODE_FM},
	{"FMN", OP_MODE_FMN},
	{"DIG", OP_MODE_DIG},
	{"PKT", OP_MODE_PKT},
};

// constructor

Cat::Cat(int port_speed, const char * port_name)
{
	uart0_filestream = open(port_name, O_RDWR | O_NOCTTY);

	if (uart0_filestream == -1) {
		cout << "Can't open" << port_name << endl;
	} else {
		if (verbose)
			cout << "Port " << port_name << " successfully opened" << endl;
	}

	struct termios options;
	tcgetattr(uart0_filestream, &options);

	cfsetispeed(&options, port_speed);
	cfsetospeed(&options, port_speed);

	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag = port_speed | CS8 | CLOCAL | CREAD | CSTOPB;
	options.c_iflag = IGNPAR;
	options.c_oflag = 0;
	options.c_lflag = 0;
	options.c_cc[VMIN] = 5;
	options.c_cc[VTIME] = 5;
	tcflush(uart0_filestream, TCIFLUSH);
	tcsetattr(uart0_filestream, TCSANOW, &options);
}

// destructor

Cat::~Cat()
{
	if (verbose)
		cout << "Closing port" << endl;

	close(uart0_filestream);
}

// getters / setters

/**
 * Set verbose flag on
 * @param bool v
 * @return void
 */
void Cat::setVerbose(bool v)
{
	verbose = v;
}

// private methods

/**
 * Send CAT packet to tcvr
 * @param char[5] packet Five bytes to write to tcvr
 * @return char Byte count
 */
char Cat::SendPacket(char packet[5])
{
	char byte_count = 0;

	for (int i = 0; i < 5; i++) {
		byte_count += write(uart0_filestream, &packet[i], 1);
	}

	return byte_count;
}

/**
 * Read response packet from tcvr. Timeout after 3 seconds.
 * @param char* packet
 * @return char
 */
char Cat::ReadPacket(char * packet)
{
	char byte_count = 0;

	// initialize file descriptor sets
	fd_set read_fds, write_fds, except_fds;
	FD_ZERO(&read_fds);
	FD_ZERO(&write_fds);
	FD_ZERO(&except_fds);
	FD_SET(uart0_filestream, &read_fds);

	// set timeout
	struct timeval timeout;
	timeout.tv_sec = 3;
	timeout.tv_usec = 0;

	if (select(uart0_filestream + 1, &read_fds, &write_fds, &except_fds, &timeout) == 1) {
		byte_count = read(uart0_filestream, (void*)packet, 5);

		if (byte_count < 0) {
			cout << "Error reading from serial device." << endl;
		} else if (byte_count == 0) {
			cout << "No data was read from serial device." << endl;
		} else {
			if (verbose)
				cout << "Bytes: " << (int)packet[0] << " "<< (int)packet[1] << " "<< (int)packet[2] << " "<< (int)packet[3] << " "<< (int)packet[4] << endl;
		}
	}

	return byte_count;
}

/**
 * Find map key by value
 * @param map dictionary
 * @param char value
 * @return string
 */
string Cat::FindKeyByValue(const map<string, char> dictionary, char value)
{
	for (auto it = dictionary.begin(); it != dictionary.end(); ++it) {
		if (it->second == value) {
			return it->first;
		}
	}

	return "";
}

/**
 * Convert int which is really hex to int
 * @param doube value
 * @param char base
 * @return char
 */
char Cat::ConvertToBase(double value, char base)
{
	ostringstream strs;
	strs << value;
	return stoi(strs.str(), nullptr, base);
}

/**
 * Convert char array into double representing frequency
 * @param char* bytes
 * @return double
 */ 
double Cat::BytesToFrequency(char * bytes)
{
	stringstream strs;
	strs << setfill('0') << setw(2) << std::hex << (int)bytes[0] << setw(2) << std::hex << (int)bytes[1] << setw(2) << std::hex << (int)bytes[2] << setw(2) << std::hex << (int)bytes[3];

	double x;
	strs >> x;
	return x / 100000;
}

// public methods

void Cat::Json()
{
	cout << "{";

	for (auto it = tcvr_status.begin(); it != tcvr_status.end(); ++it) {
		cout << "\"" << it->first << "\":\"" << it->second << "\"";
	}

	cout << "}";
}

bool Cat::Lock(bool enabled)
{
	// form the packet
	char packet[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
	packet[4] = enabled ? CMD_LOCK_ON : CMD_LOCK_OFF;

	// send packet to device
	char count = SendPacket(packet);

	// check if we sent 5 bytes
	if (count != 5) {
		return false;
	}

	if (verbose)
		cout << "Command> Lock: " << enabled << endl;

	return true;
}

bool Cat::Ptt(bool enabled)
{
	// form the packet
	char packet[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
	packet[4] = enabled ? CMD_PTT_ON : CMD_PTT_OFF;

	// send packet to device
	char count = SendPacket(packet);

	// check if we sent 5 bytes
	if (count != 5) {
		return false;
	}

	if (verbose)
		cout << "Command> PTT: " << enabled << endl;

	return true;
}

/**
 * Set tcvr's opereating frequency
 * @param double frequency
 * @return bool
 */
bool Cat::SetFrequency(double frequency)
{
	// form the packet
	char packet[5] = {0x00, 0x00, 0x00, 0x00, CMD_SET_FREQUENCY};
	double intpart;

	double f = modf(frequency * 1000, &intpart) * 100;
	packet[3] = ConvertToBase(f, 16);

	f = modf(intpart / 100, &intpart) * 100;
	packet[2] = ConvertToBase(f, 16);

	f = modf(intpart / 100, &intpart) * 100;
	packet[1] = ConvertToBase(f, 16);

	f =  modf(intpart / 100, &intpart) * 100;
	packet[0] = ConvertToBase(f, 16);

	// send packet to device
	char count = SendPacket(packet);

	// check if we sent 5 bytes
	if (count != 5) {
		return false;
	}

	if (verbose)
		cout << "Command> SetFrequency: " << frequency << " MHz" << endl;

	return true;
}

/**
 * Set tcvr operating mode
 * @param char mode
 * @return bool
 * @see OP_MODE_XXXXXX
 */
bool Cat::SetOperatingMode(char mode)
{
	// form the packet
	char packet[5] = {mode, 0x00, 0x00, 0x00, CMD_SET_MODE};

	// send packet to device
	char count = SendPacket(packet);

	// check if we sent 5 bytes
	if (count != 5) {
		return false;
	}

	string text_mode = FindKeyByValue(OP_MODES, mode);

	if (verbose)
		cout << "Command> SetOperatingMode: " << text_mode << endl;

	return true;
}

/**
 * Set tcvr operating mode
 * @param string text_mode
 * @return bool
 * @see OP_MODES
 */
bool Cat::SetOperatingMode(string text_mode)
{
	try {
		char mode = OP_MODES.at(text_mode);

		return SetOperatingMode(mode);
	} catch (const out_of_range& oor) {
		return false;
	}
}

/**
 * Get tcvr's transmitter status
 * @return bool
 */
bool Cat::GetTxStatus()
{
	// form the packet
	char packet[5] = {0x00, 0x00, 0x00, 0x00, CMD_GET_TX_STATUS};

	// send packet to device
	char count = SendPacket(packet);

	// check if we sent 5 bytes
	if (count != 5) {
		return false;
	}

	// read packet
	char rx_packet[5] = {0};
	char bytes_read = ReadPacket(rx_packet);

	if (bytes_read && rx_packet[0] != 255) {
		int power = rx_packet[0] & 0x0f;
		bool split = ~(rx_packet[0] & 0x20);
		bool swr = rx_packet[0] & 0x40;
		bool ptt = rx_packet[0] & 0x80;

		if (verbose) {
			cout << "Command> GetTxStatus: Power: " << power << " Split: " << split << " SWR: " << swr << " PTT: " << ptt << endl;
		}

		tcvr_status["tx_power"] = to_string(power);
		tcvr_status["split"] = to_string(split);
		tcvr_status["swr_high"] = to_string(swr);
		tcvr_status["ptt_on"] = to_string(ptt);

		return true;
	}

	return false;
}

/**
 * Get tcvr's receiver status
 * @return bool
 */
bool Cat::GetRxStatus()
{
	// form the packet
	char packet[5] = {0x00, 0x00, 0x00, 0x00, CMD_GET_RX_STATUS};

	// send packet to device
	char count = SendPacket(packet);

	// check if we sent 5 bytes
	if (count != 5) {
		return false;
	}

	// read packet
	char rx_packet[5] = {0};
	char bytes_read = ReadPacket(rx_packet);

	if (bytes_read) {
		int signal = rx_packet[0] & 0x0f;
		bool centered = ~(rx_packet[0] & 0x20);
		bool ctcss_dcs = rx_packet[0] & 0x40;
		bool squelched = rx_packet[0] & 0x80;

		if (verbose) {
			cout << "Command> GetRxStatus: Signal: " << signal << " Centered: " << centered << " CTCSS/DCS: " << ctcss_dcs << " Squelched: " << squelched  << endl;
		}

		// add to map
		tcvr_status["rx_signal"] = to_string(signal);
		tcvr_status["centered"] = to_string(centered);
		tcvr_status["ctcss_dcs"] = to_string(ctcss_dcs);
		tcvr_status["rx_squelched"] = to_string(squelched);

		return true;
	}

	return false;
}

/**
 * Read current frequency and mode
 * @return bool
 */
bool Cat::GetFrequencyModeStatus()
{
	// form the packet
	char packet[5] = {0x00, 0x00, 0x00, 0x00, CMD_GET_FREQUENCY_MODE};

	// send packet to device
	char count = SendPacket(packet);

	// check if we sent 5 bytes
	if (count != 5) {
		return false;
	}

	// read packet
	char rx_packet[5] = {0};
	char bytes_read = ReadPacket(rx_packet);

	if (bytes_read) {
		string text_mode = FindKeyByValue(OP_MODES, rx_packet[4]);
		double frequency = BytesToFrequency(rx_packet);

		if (verbose) {
			cout << "Command> GetFrequencyModeStatus: Mode: " << text_mode << " Frequency: " << frequency << " MHz" << endl;
		}

		// add to map
		tcvr_status["tcvr_mode"] = text_mode;
		tcvr_status["tcvr_frequency"] = to_string(frequency);

		return true;
	}

	return false;
}
