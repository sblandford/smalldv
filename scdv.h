/*
 *
 *
 *    scdv.h
 *
 *    SoundCardDV class.
 *
 *    Portions adapted from Codec2, by David Rowe.
 *
 *    Copyright (C) 2018 by Matt Roberts,
 *    All rights reserved.
 *
 *    License: GNU GPL3 (www.gnu.org)
 *
 *
 */

#ifndef __FDVCORE_SCDV_H
#define __FDVCORE_SCDV_H

// import 'freedv' type
#include <codec2/freedv_api.h>

// needed for buffer types
#include <deque>

// import sound card interface
#include "sc.h"

// import FIR filter type
#include "FirFilter.h"

// local data types
#include "localtypes.h"


//
//
//   class SoundCardDV - interface to the sound device
//
//
class SoundCardDV : public SoundCard {
	private:
		ModesDV mMode;

		// FreeDV API fields
		local_callback_state cb_state;
		short *modem_in;
		short *modem_out;
		int n_speech_samples;
		int n_nom_modem_samples;
		int n_max_modem_samples;
		freedv * m_freedv;
		int sql_en;
		float sql_th;
		volatile uint64_t m_Frames;
		volatile bool clipping;

		// buffers between FreeDV and the sound card
		std::deque<int16_t> in_buffer;
		std::deque<int16_t> out_buffer;

		// decimation counter
		uint8_t dec_ctr;

		// decimation and interpolation filters
		KK5JY::DSP::FirFilter<float> m_DecFilter;
		KK5JY::DSP::FirFilter<float> m_IntFilter;

	private: // callbacks
		//  callback - returns the next TX data byte to send
		static char local_get_next_tx_char(void *callback_state);
		//  TX data callback - updates the callback counter
		static void local_get_next_proto(void *callback_state, char *proto_bits);
		//  RX modem callback -- for safety
		static void local_datarx(void *callback_state, unsigned char *packet, size_t size);
		//  TX modem callback
		static void local_datatx(void *callback_state, unsigned char *packet, size_t *size);

	public: // [cd]tors
		SoundCardDV(int modem, int id, int win = 0);
		virtual ~SoundCardDV();

	public: // accessors
		// set mode
		bool mode(const ModesDV &newMode) {
			if (newMode < ModesDV::MinValue || newMode > ModesDV::MaxValue) {
				return false;
			}

			// set the new mode
			mMode = newMode;

			return true;
		}

		// set squelch threshold
		void threshold(float value) {
			sql_th = value;
			freedv_set_snr_squelch_thresh(m_freedv, sql_th);
		}

		// set squelch state
		void squelch(bool value) {
			sql_en = value;
			freedv_set_squelch_en(m_freedv, sql_en);
		}

		// set text
		void text(const std::string &s) {
			strcpy(cb_state.tx_str, s.empty() ? DEFAULT_TEXT : s.c_str());
			cb_state.ptx_str = cb_state.tx_str;
		}

		// get text
		std::string text() const {
			return cb_state.tx_str;
		}

		// get mode
		ModesDV mode() const {
			return mMode;
		}

		// get squelch threshold
		float threshold() const {
			return sql_th;
		}

		// get squelch state
		bool squelch() const {
			return sql_en;
		}

		// returns true if the input is clipping
		bool clipped() {
			bool value = clipping;
			clipping = false;
			return value;
		}

		// returns the frame count
		uint64_t frames() const {
			return m_Frames;
		}

		// returns basic stats pair
		basic_stats stats();

		// returns sync value
		bool sync();

		// returns SNR value
		float snr();

		// returns delta-freq value
		float df();

	protected:
		//  sound event handler
		virtual void event(float *in, float *out, size_t count);
};

#endif
