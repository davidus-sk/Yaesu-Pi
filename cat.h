#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <iostream>
#include <string>
#include <math.h>
#include <map>
#include <stdexcept>
#include <sstream>
#include  <iomanip>

using namespace std;

#ifndef APRS_H
#define APRS_H

class Cat
{
	private:
		int uart0_filestream;
		bool verbose;
		map<string, string> tcvr_status;

		char SendPacket(char packet[5]);
		char ReadPacket(char * packet);
		string FindKeyByValue(const map<string, char> dictionary, char value);
		double BytesToFrequency(char * bytes);
		char ConvertToBase(double value, char base);

	public:
		Cat() : verbose(false) {}

		static const char CMD_LOCK_ON;
		static const char CMD_LOCK_OFF;
		static const char CMD_PTT_ON;
		static const char CMD_PTT_OFF;
		static const char CMD_SET_FREQUENCY;
		static const char CMD_GET_FREQUENCY_MODE;
		static const char CMD_SET_MODE;
		static const char CMD_GET_RX_STATUS;
		static const char CMD_GET_TX_STATUS;

		static const char OP_MODE_LSB;
		static const char OP_MODE_USB;
		static const char OP_MODE_CW;
		static const char OP_MODE_CWR;
		static const char OP_MODE_AM;
		static const char OP_MODE_WFM;
		static const char OP_MODE_FM;
		static const char OP_MODE_FMN;
		static const char OP_MODE_DIG;
		static const char OP_MODE_PKT;

		static const map<string, char> OP_MODES;

		Cat(int port_speed, const char * port_name);
		~Cat();

		void setVerbose(bool v);

		void Json();

		bool Lock(bool enabled);
		bool Ptt(bool enabled);
		bool SetFrequency(double frequency);
		bool SetOperatingMode(char mode);
		bool SetOperatingMode(string mode);
/*		void Clar(bool enabled);
		void SetClarFrequency(char offset, double frequency);
		void ToggleVfo();
		void Split(bool enabled);
		void SetRepeaterOffset(char offset);
		void SetRepeaterOffsetFrequency(double frequency);
		void SetCtcssDcsMode(string mode);
		void SetCtcssTone(double tx_frequency, double rx_frequency);
		void SetDcsCode(short tx_code, short rx_code);
*/
		bool GetTxStatus();
		bool GetRxStatus();
		bool GetFrequencyModeStatus();
};

#endif
