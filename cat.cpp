#include "cat.h"

using namespace std;

// constants

const char Cat::CMD_LOCK_ON = 0x00;
const char Cat::CMD_LOCK_OFF = 0x80;
const char Cat::CMD_PTT_ON = 0x08;
const char Cat::CMD_PTT_OFF = 0x88;
const char Cat::CMD_SET_FREQUENCY = 0x01;
const char Cat::CMD_GET_FREQUENCY_MODE = 0x03;
const char Cat::CMD_SET_MODE = 0x07;

const char Cat::OP_MODE_LSB = 0x00;
const char Cat::OP_MODE_USB = 0x01;
const char Cat::OP_MODE_CW = 0x02;
const char Cat::OP_MODE_CWR = 0x03;
const char Cat::OP_MODE_AM = 0x04;
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
	{"FM", OP_MODE_FM},
	{"FMN", OP_MODE_FMN},
	{"DIG", OP_MODE_DIG},
	{"PKT", OP_MODE_PKT},
};

// constructor

Cat::Cat(int port_speed, const char * port_name)
{
	uart0_filestream = open(port_name, O_RDWR | O_NOCTTY | O_NDELAY);

	if (uart0_filestream == -1) {
		cout << "Can't open" << port_name << endl;
	} else {
		cout << "Port " << port_name << " successfully opened" << endl;
	}

	struct termios options;
	tcgetattr(uart0_filestream, &options);
	options.c_cflag = port_speed | CS8 | CLOCAL | CREAD | CSTOPB;
	options.c_iflag = IGNPAR;
	options.c_oflag = 0;
	options.c_lflag = 0;
	tcflush(uart0_filestream, TCIFLUSH);
	tcsetattr(uart0_filestream, TCSANOW, &options);
}

// destructor

Cat::~Cat()
{
	cout << "Closing port" << endl;
	close(uart0_filestream);
}

// private methods

/**
 * Send CAT packet to tcvr
 * @param char[5] packet Five bytes to write to tcvr
 * @return char Byte count
char Cat::SendPacket(char packet[5])
{
	char byte_count = 0;

	for (int i = 0; i < 5; i++) {
		byte_count += write(uart0_filestream, &packet[i], 1);
	}

	return byte_count;
}

/**
 * Read response packet from tcvr
 * @return char* 
 */
char Cat::ReadPacket()
{
	char packet[6] = {0};
	char byte_count = read(uart0_filestream, (void*)packet, 5);

	if (byte_count < 0) {
		//An error occured (will occur if there are no bytes)
	} else if (byte_count == 0) {
		//No data waiting
	} else {
		//Bytes received
		cout << "Bytes: " << (int)packet[0] << " "<< (int)packet[1] << " "<< (int)packet[2] << " "<< (int)packet[3] << " "<< (int)packet[4] << endl;
	}
}

// public methods

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

	double freq = frequency * 1000;
	packet[3] = modf(freq, &intpart) * 100;
	packet[3] = strtol(&packet[3], nullptr, 16);

	freq = freq / 100;
	packet[2] = modf(freq, &intpart) * 100;
	packet[2] = strtol(&packet[2], nullptr, 16);

	freq = freq / 100;
	packet[1] = modf(freq, &intpart) * 100;
	packet[1] = strtol(&packet[1], nullptr, 16);

	freq = freq / 100;
	packet[0] = modf(freq, &intpart) * 100;
	packet[0] = strtol(&packet[0], nullptr, 16);

	// send packet to device
	char count = SendPacket(packet);

	// check if we sent 5 bytes
	if (count != 5) {
		return false;
	}

	cout << "Command> SetFrequency: " << frequency << endl;

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

	cout << "Command: SetOperatingMode" << endl;

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
	ReadPacket();

	cout << "Command: GetFrequencyModeStatus" << endl;

	return true;
}
