/*
 *
 *   sc.h - soundcard interface
 *
 *   This is a class library that wraps an RtAudio soundcard object
 *   and simplifies the interface.
 *
 *   Copyright (C) 2015-2018 by Matt Roberts, KK5JY,
 *   All rights reserved.
 *
 *   License: GNU GPL3 (www.gnu.org)
 *
 */

#ifndef __KK5JY_SC_H
#define __KK5JY_SC_H

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <string>
#include <rtaudio/RtAudio.h>


//
//  SoundCard - simple mono full-duplex interface to the sound card
//
class SoundCard {
	public:
		typedef enum {
			S16,
			Float
		} Formats;

		RtAudio adc;
		RtAudio::StreamParameters paramsIn;
		RtAudio::StreamParameters paramsOut;
		Formats mFormat;
		unsigned mCard;
		unsigned mRate;
		unsigned mWin;
	
	public:
		SoundCard(unsigned id, unsigned rate, unsigned short win = 256);
		SoundCard(unsigned id, unsigned rate, Formats format, unsigned short win = 256);
		virtual ~SoundCard() { };

	public:
		virtual bool start();
		virtual void stop();

	public:
		static void showDevices();
		static unsigned deviceCount();

		uint16_t channelsIn() const { return paramsIn.nChannels; }
		uint16_t channelsOut() const { return paramsOut.nChannels; }
	
	protected:
		virtual void event(float *inBuffer, float *outBuffer, size_t samples) { }
		virtual void event(int16_t *inBuffer, int16_t *outBuffer, size_t samples) { }

	private:
		static int handler(
			void *outputBuffer,
			void *inputBuffer,
			unsigned int nBufferFrames,
			double streamTime,
			RtAudioStreamStatus status,
			void *userData );
};


/*
 *
 *  SoundCard::ctor(...)
 *
 */
inline SoundCard::SoundCard(unsigned id, unsigned rate, unsigned short win)
	: adc(RtAudio::LINUX_ALSA),
	  mFormat(Formats::Float),
	  mCard(id),
	  mRate(rate),
	  mWin(win) {

	// read the caps of the sound card to configure channel counts
	RtAudio::DeviceInfo info = adc.getDeviceInfo(id);

	paramsOut.deviceId = mCard;
	paramsOut.nChannels = info.outputChannels;
	paramsOut.firstChannel = 0;

	paramsIn.deviceId = mCard;
	paramsIn.nChannels = info.inputChannels;
	paramsIn.firstChannel = 0;
}


/*
 *
 *  SoundCard::ctor(...)
 *
 */
inline SoundCard::SoundCard(unsigned id, unsigned rate, SoundCard::Formats format, unsigned short win)
	: adc(RtAudio::LINUX_ALSA),
	  mFormat(format),
	  mCard(id),
	  mRate(rate),
	  mWin(win) {

	// read the caps of the sound card to configure channel counts
	RtAudio::DeviceInfo info = adc.getDeviceInfo(id);

	paramsOut.deviceId = mCard;
	paramsOut.nChannels = info.outputChannels;
	paramsOut.firstChannel = 0;

	paramsIn.deviceId = mCard;
	paramsIn.nChannels = info.inputChannels;
	paramsIn.firstChannel = 0;
}


/*
 *
 *  SoundCard::start()
 *
 */
inline bool SoundCard::start() {
	// open the sound card
	try {
		int format = RTAUDIO_FLOAT32;
		switch (mFormat) {
			case Float: format = RTAUDIO_FLOAT32; break;
			case S16: format = RTAUDIO_SINT16; break;
		}
		adc.openStream(
			&paramsOut, // output
			&paramsIn,  // input
			format, // format
			mRate,  // rate
			&mWin, // buffer size
			&handler,     // callback
			this);        // callback user data
		// start
		adc.startStream();
	}
	catch ( RtAudioError& e ) {
		return false;
	}
	return true;
}

/*
 *
 *  SoundCard::stop()
 *
 */
inline void SoundCard::stop() {
	if ( adc.isStreamOpen() )
		adc.stopStream();
}

/*
 *
 *   SoundCard::handler(...)
 *
 */
inline int SoundCard::handler(
		void *outputBuffer,
		void *inputBuffer,
		unsigned int nBufferFrames,
		double streamTime,
		RtAudioStreamStatus status,
		void *sc) {
	#ifdef _DEBUG
	if (status)
		std::cout << "[sc:ov!]";
	#endif

	// extract appropriate pointers
	SoundCard *thisPtr = (SoundCard*)(sc);
	if (thisPtr == 0) return 0;

	switch (thisPtr->mFormat) {
		case Float: {
			float *inData = (float*)(inputBuffer);
			if (inData == 0) return 0;
			float *outData = (float*)(outputBuffer);
			if (outData == 0) return 0;

			// call the user's handler
			thisPtr->event(inData, outData, nBufferFrames);
		} break;
		case S16: {
			int16_t *inData = (int16_t*)(inputBuffer);
			if (inData == 0) return 0;
			int16_t *outData = (int16_t*)(outputBuffer);
			if (outData == 0) return 0;

			// call the user's handler
			thisPtr->event(inData, outData, nBufferFrames);
		} break;
	}

	// return success
	return 0;
}


/*
 *
 *   channelsToString(...)
 *
 */
static std::string channelsToString(unsigned count) {
	switch(count) {
		case 0: return "None";
		case 1: return "Mono";
		case 2: return "Stereo";
		default: return "Multi";
	}
}

/*
 *
 *   ratesToString(...)
 *
 */
static std::string ratesToString(std::vector<unsigned> rates) {
	std::stringstream result;
	for (unsigned i = 0; i != rates.size(); ++i) {
		if (result.str().size() != 0)
			result << ", ";
		result << rates[i];
	}
	return result.str();
}

/*
 *
 *   showDevices()
 *
 */
inline void SoundCard::showDevices (void) {
	RtAudio adc(RtAudio::LINUX_ALSA);

	// Determine the number of devices available
	unsigned int devices = adc.getDeviceCount();
	if (devices == 0) {
		std::cout << "No audio devices found." << std::endl;
		return;
	}

	// Scan through devices for various capabilities
	RtAudio::DeviceInfo info;
	std::cout << "Valid devices:" << std::endl;
	for (unsigned int i = 0; i < devices; i++) {
		info = adc.getDeviceInfo(i);
		if (info.probed == true) {
			// Print, for example, the maximum number of output channels for each device
			std::cout << " + Device ID = " << i;
			std::cout << ": \"" << info.name << "\"";
			std::cout << ", inputs = " << channelsToString(info.inputChannels);
			std::cout << ", outputs = " << channelsToString(info.outputChannels);
			std::cout << ", rates = " << ratesToString(info.sampleRates);
			std::cout << "\n";
		}
	}
}

/*
 *
 *   deviceCount()
 *
 */
inline unsigned SoundCard::deviceCount() {
	RtAudio adc(RtAudio::LINUX_ALSA);
	return adc.getDeviceCount();
}

#endif // __KK5JY_SC_H
