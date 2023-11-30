/*
 *
 *
 *    fdvcore.cc
 *
 *    Half-duplex embedded modem for FreeDV.
 *
 *    Portions adapted from Codec2, by David Rowe.
 *
 *    Copyright (C) 2018 by Matt Roberts.
 *    License: GNU GPL3 (www.gnu.org)
 *
 *    Project started: 2018-05-06
 *
 *
 */


#include <iostream>
#include <cstring>
#include <string>
#include "stype.h"
#include "localtypes.h"
#include "SplitCommand.h"
#include "scdv.h"

#ifdef FREEDV_MODE_700D
#define FDV_MODES "1600, 800XA, 700, 700B, 700C, 700D"
#else
#define FDV_MODES "1600, 800XA, 700, 700B, 700C"
#endif

// this determines the number of frames that will be processed
//   per sound card event cycle
#define SCDV_WINDOW_SIZE (512)


/*
 *
 *   usage()
 *
 */
void usage() {
	std::cerr << std::endl;
	std::cerr <<  "Usage: fdvcore <dev> <modem>" << std::endl;
	std::cerr <<  "       fdvcore -l" << std::endl;
	std::cerr << std::endl;
	std::cerr <<  "       <dev>   - device ID" << std::endl;
	std::cerr <<  "       <modem> - the Codec2 modem { " FDV_MODES  " }" << std::endl;
	std::cerr << std::endl;
}


/*
 *
 *   main()
 *
 */
int main(int argc, char **argv) {
	// must have at least one argument
	if (argc < 2) {
		usage();
		return 1;
	}

	// if no cards, bail
	if ( SoundCard::deviceCount() < 1 ) {
		std::cerr << "\nNo audio devices found!\n";
		return 1;
	}

	// List (-l) option
	if (argc == 2 && strcmp(argv[1], "-l") == 0) {
		RtAudio adc(RtAudio::LINUX_ALSA);

		// Scan through devices for various capabilities
		RtAudio::DeviceInfo info;
		std::cout << "Valid devices:" << std::endl;
		unsigned int devices = adc.getDeviceCount();
		for (unsigned int i = 0; i < devices; i++) {
			info = adc.getDeviceInfo(i);
			if (info.probed == true) {
				std::cout << " + Device ID = " << i;
				std::cout << ": \"" << info.name << "\"";
				std::cout << std::endl;
			}
		}

		return 0;
	}

	// from this point forward, there must be two args
	if (argc < 3) {
		usage();
		return 1;
	}

	// read the device name
	size_t id = atoi(argv[1]);

	// parse the modem type
	int modem = -1;
	const int modemIndex = 2;
	if (!strcmp(argv[modemIndex],"1600"))
		modem = FREEDV_MODE_1600;
/*	if (!strcmp(argv[modemIndex],"700"))
		modem = FREEDV_MODE_700;
	if (!strcmp(argv[modemIndex],"700B"))
		modem = FREEDV_MODE_700B; */
	if (!strcmp(argv[modemIndex],"700C"))
		modem = FREEDV_MODE_700C;
	#ifdef FREEDV_MODE_700D
	if (!strcmp(argv[modemIndex],"700D"))
		modem = FREEDV_MODE_700D;
	#endif
	#if 0 // not supported yet
	if (!strcmp(argv[modemIndex],"2400A"))
		modem = FREEDV_MODE_2400A;
	if (!strcmp(argv[modemIndex],"2400B"))
		modem = FREEDV_MODE_2400B;
	#endif
	if (!strcmp(argv[modemIndex],"800XA"))
		modem = FREEDV_MODE_800XA;
	if (modem == -1) {
		usage();
		return 1;
	}

	// open the sound card
	SoundCardDV *adc = 0;
	try {
		adc = new SoundCardDV(modem, id, SCDV_WINDOW_SIZE);
		adc->start();
	}
	catch ( RtAudioError& e ) {
		if (adc)
			try { adc->stop(); } catch (const std::exception &e) { /* nop */ }
		e.printMessage();
		return 1;
	}

	// DEBUG: output debugging info about the card
	std::cerr << "DEBUG: using " << adc->channelsIn() << " input channels." << std::endl;
	std::cerr << "DEBUG: using " << adc->channelsOut() << " output channels." << std::endl;

	// wait for commands
	std::string line;
	std::string cmd, arg;
	try {
		while (std::cin) {
			// read one line from std input
			std::getline(std::cin, line);
			if (!std::cin)
				break;

			// trim off whitespace
			line = my::strip(line);

			// split the command from the argument (if any)
			SplitCommand(line, cmd, arg);
			cmd = my::toUpper(cmd);

			// COMMAND: QUIT
			if (cmd == "QUIT") {
				break;
			} else 

			// COMMAND: QUIT
			if (cmd == "VERSION") {
				std::cout << "OK:VERSION=" << VERSION_TEXT << std::endl;
				continue;
			} else 

			// COMMAND: TEXT
			if (cmd == "TEXT") {
				if (arg.empty()) {
					std::cout << "OK:TEXT=" << adc->text() << std::endl;
					continue;
				} else {
					adc->text(arg);
					std::cout << "OK:TEXT=" << arg << std::endl;
					continue;
				}
			} else 

			// COMMAND: CLIP CHECK
			if (cmd == "CLIP") {
				if (arg.empty()) {
					std::cout << "OK:CLIP=" << static_cast<int>(adc->clipped()) << std::endl;
					continue;
				}
			} else 

			// COMMAND: FRAME COUNT
			if (cmd == "FRAMES") {
				if (arg.empty()) {
					std::cout << "OK:FRAMES=" << adc->frames() << std::endl;
					continue;
				}
			} else 

			// COMMAND: SQUELCH ENABLE
			if (cmd == "SQEN") {
				if (arg.empty()) {
					std::cout << "OK:SQEN=" << static_cast<int>(adc->squelch()) << std::endl;
					continue;
				} else {
					int value = atoi(arg.c_str()) ? 1 : 0;
					adc->squelch(value);
					std::cout << "OK:SQEN=" << value << std::endl;
					continue;
				}
			} else 

			// COMMAND: SQUELCH THRESHOLD
			if (cmd == "SQTH") {
				if (arg.empty()) {
					std::cout << "OK:SQTH=" << adc->threshold() << std::endl;
					continue;
				} else {
					float value = atof(arg.c_str());
					adc->threshold(value);
					std::cout << "OK:SQTH=" << value << std::endl;
					continue;
				}
			} else 

			// COMMAND: DF - frequency offset estimate
			if (cmd == "DF" && arg.empty()) {
				float value = adc->df();
				std::cout << "OK:DF=" << value << std::endl;
				continue;
			}

			// COMMAND: SNR - return S/N value
			if (cmd == "STAT" && arg.empty()) {
				basic_stats bs = adc->stats();
				std::cout << "OK:STAT=" << bs.snr << ':' << (bs.sync ? "SYNC" : "NO_SYNC") << std::endl;
				continue;
			}

			// COMMAND: SNR - return S/N value
			if (cmd == "SNR" && arg.empty()) {
				float value = adc->snr();
				std::cout << "OK:SNR=" << value << std::endl;
				continue;
			}

			// COMMAND: SYNC - returns modem RX sync state
			if (cmd == "SYNC" && arg.empty()) {
				bool value = adc->sync();
				std::cout << "OK:SYNC=" << (value ? "1" : "0") << std::endl;
				continue;
			}

			// COMMAND: MODE
			if (cmd == "MODE") {
				if (arg.empty()) {
					switch (adc->mode()) {
						case ModesDV::Mute:
							std::cout << "OK:MODE=MUTE" << std::endl;
							break;
						case ModesDV::Pass:
							std::cout << "OK:MODE=PASS" << std::endl;
							break;
						case ModesDV::RX:
							std::cout << "OK:MODE=RX" << std::endl;
							break;
						case ModesDV::TX:
							std::cout << "OK:MODE=TX" << std::endl;
							break;
						default:
							goto no_good;
					}
					continue;
				} else {
					arg = my::toUpper(arg);
					if (arg == "MUTE") {
						if (!adc->mode(ModesDV::Mute)) goto no_good;
						std::cout << "OK:MODE=MUTE" << std::endl;
					} else if (arg == "PASS") {
						if (!adc->mode(ModesDV::Pass)) goto no_good;
						std::cout << "OK:MODE=PASS" << std::endl;
					} else if (arg == "RX") {
						if (!adc->mode(ModesDV::RX)) goto no_good;
						std::cout << "OK:MODE=RX" << std::endl;
					} else if (arg == "TX") {
						if (!adc->mode(ModesDV::TX)) goto no_good;
						std::cout << "OK:MODE=TX" << std::endl;
					} else {
						goto no_good;
					}
				}
			} else {
				goto no_good;
			}

			continue;
			
		no_good:
			std::cout << "ERR" << std::endl;
		} // ... while (cin)

		// Stop the stream
		adc->stop();
		adc = 0;
	}
	catch (RtAudioError& e) {
		e.printMessage();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
	return 0;
}

// EOF
