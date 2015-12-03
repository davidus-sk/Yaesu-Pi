/**
 *
 * How to compile: g++ -O3 -std=c++0x -o yaesu yaesu.cpp cat.cpp
 */

#include "cat.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <algorithm>
#include <termios.h>

using namespace std;

void show_help(char *s);

int main(int argc, char **argv)
{
	Cat * cat = new Cat(B9600, "/dev/ttyUSB0");
	cat->Lock(false);
	cat->SetFrequency(14.25513);
	cat->GetFrequencyModeStatus();
	delete cat;

	return 1;
}
