/*
 *
 *
 *    localtypes.h
 *
 *    Copyright (C) 2018 by Matt Roberts,
 *    All rights reserved.
 *
 *    License: GNU GPL3 (www.gnu.org)
 *
 *
 */


#ifndef __FDVCORE_LOCALTYPES_H
#define __FDVCORE_LOCALTYPES_H

// the version number
#define VERSION_NUMBER "1.0 (beta-20180615)"

// the version text
#define VERSION_TEXT "fdvcore " VERSION_NUMBER

// the default text stream content
#define DEFAULT_TEXT "fdvcore " VERSION_NUMBER " by KK5JY\r"

// the point at which clipping detection will fire
#define CLIP_LIMIT (0.90)

// define the sampling rate of the sound card and the modem algorithm
//              TODO: make these rates adaptable
#define CARD_FS 48000
#define MODEM_FS 8000

// the length of the FIR decimation and interpolation filters
#define FILTER_LEN 15

// the filter cutoff (in Hz)
#define FILTER_COF 2800

//
//  Device Modes
//
typedef enum : uint8_t {
	Mute, // audio input and output muted
	Pass, // audio input pass-through with no modification
	RX,   // receive with Codec2 decoding
	TX,   // transmit with Codec2 encoding

	// limits
	MinValue = Mute,
	MaxValue = TX,
} ModesDV;


//
//  basic modem stats
//
struct basic_stats {
	float snr;
	bool sync;

	// ctor
	basic_stats() : snr(0), sync(false) { /* nop */ }
};


#include <exception>

//
//  local_exception
//
class local_exception : public std::exception {
	private:
		const std::string msg;
	public:
		local_exception(const std::string &s) : msg(s) {
			// nop
		}

		virtual const char* what() const noexcept {
			return msg.c_str();
		}
};


//
//  callback state
//
//  Used to track the TX string state and the number of protocol calls made
//  by the modem.
//
struct local_callback_state {
    // the string data
	char  tx_str[128];
	// the next character to transmit
	char *ptx_str;

	// the number of times called
	size_t calls;

	//
	//  ctor
	//
	local_callback_state() {
		 strcpy(tx_str, "fdvcore...");
		 ptx_str = tx_str;
		 calls = 0;
	}
};

#endif
