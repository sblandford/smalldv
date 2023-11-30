/*
 *
 *
 *    scdv.cc
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

#include "scdv.h"
#include <climits>
#include <algorithm>

// FreeDV headers
#include <codec2/freedv_api.h>
#include <codec2/modem_stats.h>
#include <codec2/codec2.h>

// define these to emit underflow messages
//#define INPUT_UNDERFLOW_DEBUG
//#define OUTPUT_UNDERFLOW_DEBUG

// define this to see total packet process counts
//#define EMIT_THROUGHPUT_COUNTS

//
//  callback - returns the next TX data byte to send
//
char SoundCardDV::local_get_next_tx_char(void *callback_state) {
	local_callback_state *pstate = (local_callback_state*)callback_state;
	char  c = *pstate->ptx_str++;

	if (*pstate->ptx_str == 0) {
		pstate->ptx_str = pstate->tx_str;
	}

	// DEBUG:
	//std::cerr << "DEBUG: sent data char: " << c << std::endl;

	return c;
}


//
//  TX data callback - updates the callback counter
//
void SoundCardDV::local_get_next_proto(void *callback_state, char *proto_bits) {
	local_callback_state* cb_states = (local_callback_state*)(callback_state);
	snprintf(proto_bits, 3, "%2lu", cb_states->calls);
	cb_states->calls = cb_states->calls + 1;
}


//
//  RX modem callback -- for safety
//
void SoundCardDV::local_datarx(void *callback_state, unsigned char *packet, size_t size) {
	// TODO: to be replaced when RX implemented
	std::cerr << "DEBUG: datarx callback called, this should not happen!" << std::endl;    
}


//
//  TX modem callback
//
/* Called when a new packet can be send */
void SoundCardDV::local_datatx(void *callback_state, unsigned char *packet, size_t *size) {
	#if 0
	static int data_toggle;

	/* Data could come from a network interface, here we just make up some */

	data_toggle = !data_toggle;
	if (data_toggle) {
		/* Send a packet with data */
		int i;
		for (i = 0; i < 64; i++)
			packet[i] = i;
		*size = i;
	} else {
		/* set size to zero, the freedv api will insert a header frame */
		*size = 0;
	}
	#else
	*size = 0;
	#endif
}


//
//  dynamic_window_size
//
static size_t dynamic_window_size(int modem) {
	size_t result = 0;
	switch (modem) {
		case FREEDV_MODE_1600: result = 320; break;
		case FREEDV_MODE_700D: result = 1280; break;
		default: result = 512; break;
	}
	return (result * CARD_FS) / MODEM_FS;
}


//
//  SoundCardDV::ctor
//
SoundCardDV::SoundCardDV(int modem, int id, int win)
	: SoundCard(id, CARD_FS, win ? win : dynamic_window_size(modem)),
	  mMode(ModesDV::Mute),
	  modem_in(0),
	  modem_out(0),
	  m_freedv(0),
	  m_Frames(0),
	  clipping(false),
	  dec_ctr(0),
	  m_DecFilter(KK5JY::DSP::FirFilter<float>::Types::LowPass, FILTER_LEN, FILTER_COF, CARD_FS),
	  m_IntFilter(KK5JY::DSP::FirFilter<float>::Types::LowPass, FILTER_LEN, FILTER_COF, CARD_FS) {
	
	// DEBUG:
	std::cerr << "DEBUG: Card ID = " << id << std::endl;
	std::cerr << "DEBUG: Modem   = " << modem << std::endl;
	std::cerr << "DEBUG: Window  = " << (win ? win : dynamic_window_size(modem)) << std::endl;

	// FreeDV SETUP begins ===========================================
	m_freedv = freedv_open(modem);
	if (!m_freedv) {
		throw local_exception("Could not start the Modem");
	}

	// set initial squelch valus and enable
	sql_en = 1;
	sql_th = -100.0;
	freedv_set_snr_squelch_thresh(m_freedv, sql_th);
	freedv_set_squelch_en(m_freedv, sql_en);

	n_speech_samples = freedv_get_n_speech_samples(m_freedv);
	n_nom_modem_samples = freedv_get_n_nom_modem_samples(m_freedv);
	n_max_modem_samples = freedv_get_n_max_modem_samples(m_freedv);
	size_t n = std::max(n_speech_samples, std::max(n_nom_modem_samples, n_max_modem_samples));
	modem_in = (short*)malloc(sizeof(short)*n);
	modem_out = (short*)malloc(sizeof(short)*n);
	if (!modem_in || !modem_out) {
		throw local_exception("Could not allocate buffers");
	}

	/* set up text buffer, and callback to service it */
	strcpy(cb_state.tx_str, DEFAULT_TEXT);
	cb_state.ptx_str = cb_state.tx_str;
	cb_state.calls = 0;
	freedv_set_callback_txt(m_freedv, NULL, &local_get_next_tx_char, &cb_state);

	/* set up callback for protocol bits */
	freedv_set_callback_protocol(m_freedv, NULL, &local_get_next_proto, &cb_state);

	/* set up callback for data packets */
	freedv_set_callback_data(m_freedv, local_datarx, local_datatx, &cb_state);

	// FreeDV SETUP complete ===========================================
}


//
//  SoundCardDV::dtor
//
SoundCardDV::~SoundCardDV() {
	if (modem_in) {
		free(modem_in);
		modem_in = 0;
	}
	if (modem_out) {
		free(modem_out);
		modem_out = 0;
	}
	if (m_freedv) {
		freedv_close(m_freedv);
		m_freedv = 0;
	}
}


//
//  SoundCardDV::stats() - returns basic statistics
//
basic_stats SoundCardDV::stats() {
	basic_stats result;
	int syncVal;
	freedv_get_modem_stats(m_freedv, &syncVal, &result.snr);
	result.sync = syncVal;
	return result;
}


//
//  SoundCardDV::sync() - returns sync value
//
bool SoundCardDV::sync() {
	int syncVal = 0;
	float snrVal = 0;
	freedv_get_modem_stats(m_freedv, &syncVal, &snrVal);
	return syncVal ? true : false;
}


//
//  SoundCardDV::snr() - returns SNR value
//
float SoundCardDV::snr() {
	int syncVal = 0;
	float snrVal = 0;
	freedv_get_modem_stats(m_freedv, &syncVal, &snrVal);
	return snrVal;
}


//
//  SoundCardDV::df() - returns delta-freq value
//
float SoundCardDV::df() {
	::MODEM_STATS stats;
	freedv_get_modem_extended_stats(m_freedv, &stats);
	return stats.foff;
}


//
//  sound event handler
//
//     in    - input data
//     out   - output data
//     count - buffer lengths
//
//	NOTE: input and output are in stereo by default, L first, then R
//
void SoundCardDV::event(float *in, float *out, size_t count) {
	++m_Frames;

	switch (mMode) {
		//
		//  MODE == MUTE
		//
		case ModesDV::Mute: {
			// read the number of output channels
			const uint16_t co = channelsOut();

			// for each sample
			for (size_t i = 0; i != count; ++i) {
				// copy zero into ALL output channels
				for (size_t j = 0; j != co; ++j) {
					*out++ = 0;
				}
			}
		} break;

		//
		//  MODE == PASS
		//
		case ModesDV::Pass: {
			// read the number of input channels
			const uint16_t ci = channelsIn();

			// read the number of output channels
			const uint16_t co = channelsOut();

			// for each sample
			for (size_t i = 0; i != count; ++i) {
				float sample = *in;
				in += ci;

				// copy zero into ALL output channels
				for (size_t j = 0; j != co; ++j) {
					*out++ = sample;
				}
			}
		} break;

		//
		//  MODE == RX/TX
		//
		case ModesDV::RX:
		case ModesDV::TX: {
			//
			//  INPUT: read the sound card and downsample
			//

			// read the number of input channels
			const uint16_t ci = channelsIn();

			// read the number of output channels
			const uint16_t co = channelsOut();

			// the number of samples that the en/decoder expects
			size_t nin = (mMode == ModesDV::RX) ? freedv_nin(m_freedv) : n_speech_samples;

			#ifdef EMIT_THROUGHPUT_COUNTS
			uint16_t input_count = 0;
			#endif

			// for each sample
			for (size_t i = 0; (i != count) && (in_buffer.size() <= (10 * nin)); ++i) {
				#ifdef EMIT_THROUGHPUT_COUNTS
				++input_count;
				#endif

				float sample = *in; // LEFT input
				in += ci; // step to next sample, stepping over any other channels
				if (abs(sample) >= CLIP_LIMIT)
					clipping = true;
				sample = m_DecFilter.filter(sample);
				if (abs(sample) >= CLIP_LIMIT)
					clipping = true;
				if (++dec_ctr == (CARD_FS / MODEM_FS)) {
					dec_ctr = 0;
					in_buffer.push_back(SHRT_MAX * sample);
				}
			}
			#ifdef EMIT_THROUGHPUT_COUNTS
			std::cerr << "IN: " << input_count << "; " << in_buffer.size() << std::endl;
			#endif

			//
			//  MODEM: encode or decode data
			//
			if (in_buffer.size() >= nin) { // underflow check
				#ifdef EMIT_THROUGHPUT_COUNTS
				std::cerr << "MODEM_IN: " << nin << std::endl;
				#endif

				int16_t *toCopy = 0;

				// set up the input buffer
				toCopy = modem_in;
				for (size_t i = 0; i != nin; ++i) {
					*toCopy++ = in_buffer.front();
					in_buffer.pop_front();
				}

				// encode/decode
				size_t nout = 0;
				if (mMode == ModesDV::RX) {
					nout = freedv_rx(m_freedv, modem_out, modem_in);
				} else {
					freedv_tx(m_freedv, modem_out, modem_in);
					nout = n_nom_modem_samples;
				}

				// copy the modem output to the buffer
				toCopy = modem_out;
				for (size_t i = 0; (i != nout) && (out_buffer.size() <= (10 * nout)); ++i) {
					float sample = *toCopy++;
					// and upsample
					for (size_t j = 0; j != (CARD_FS / MODEM_FS); ++j) {
						out_buffer.push_back(m_IntFilter.filter(sample));
					}
				}

				#ifdef EMIT_THROUGHPUT_COUNTS
				std::cerr << "MODEM_OUT: " << nout << std::endl;
				#endif
			#ifdef INPUT_UNDERFLOW_DEBUG
				std::cerr << "DEBUG: modem(" << ((mMode == ModesDV::RX) ? "RX" : "TX") << ") buffer OK" << std::endl;
			} else {
				std::cerr << "DEBUG: modem(" << ((mMode == ModesDV::RX) ? "RX" : "TX") << ") underflow, needed " << nin << ", had " << in_buffer.size() << std::endl;
			#endif
			}

			//
			//  OUTPUT: upsample and write audio to the sound card
			//
			#ifdef EMIT_THROUGHPUT_COUNTS
			uint16_t output_count = 0;
			#endif

			if (out_buffer.size() >= count) {
				// for each sample
				for (size_t i = 0; i != count; ++i) {
					#ifdef EMIT_THROUGHPUT_COUNTS
					++output_count;
					#endif

					// calculate the sample value
					float out_sample = static_cast<float>(out_buffer.front()) / SHRT_MAX;

					// copy to output soundcard buffer, into ALL output channels
					for (size_t j = 0; j != co; ++j) {
						*out++ = out_sample;
					}

					// move to next sample
					out_buffer.pop_front();
				}

				#ifdef EMIT_THROUGHPUT_COUNTS
				std::cout << "OUT: " << output_count << "; " << out_buffer.size() << std::endl;
				#endif
				#ifdef OUTPUT_UNDERFLOW_DEBUG
				std::cerr << "DEBUG: output buffer OK" << std::endl;
				#endif
			} else {
				// copy to output soundcard buffer
				for (size_t i = 0; i != count; ++i) {
					for (size_t j = 0; j != co; ++j) {
						// mute ALL channels
						*out++ = 0;
					}
				}

				#ifdef OUTPUT_UNDERFLOW_DEBUG
				std::cerr << "DEBUG: output underflow, needed" << count << ", had " << out_buffer.size() << std::endl;
				#endif
			}
		} break;
	}
}

// EOF
