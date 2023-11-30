/*
 *
 *
 *    FirFilter.h
 *
 *    Filtering classes; translated from C#
 *
 *    Copyright (C) 2013-2018 by Matt Roberts, KK5JY.
 *    All rights reserved.
 *
 *    License: GNU GPL3 (www.gnu.org)
 *    
 *
 */

#ifndef KK5JY_FIRFILTER_H
#define KK5JY_FIRFILTER_H

#include <cmath>
#include <cstdlib>
#include <exception>
#include <string>
#include <deque>
#include <algorithm>

#include "IFilter.h"

// for debugging only
//#define VERBOSE_DEBUG
#ifdef VERBOSE_DEBUG
#include <iostream>
#endif

namespace KK5JY {
	namespace DSP {

		// define specific function pointer types
		typedef double (*WindowFunction)(int n, int N);
		typedef double (*CoefFunction1)(double d, int i, int N);
		typedef double (*CoefFunction2)(double d1, double d2, int i, int N);

		//
		//  Exception class for reporting errors from the filter configuration
		//
		class FirFilterException : public std::exception {
			private:
				std::string userMsg;
			public:
				FirFilterException(const std::string &s) : userMsg(s) { /* nop */ }
				~FirFilterException() throw() { /* nop */ }
				const char *what() const throw() { return userMsg.c_str(); }
		};


		//
		//  Utilities for computing filter coefficients, and generic filter implementation
		//
		class FirFilterUtils {
			public:
				/// <summary>
				/// Generate Hamming coefficiencts for an odd-length window centered at zero, of length N.
				/// </summary>
				/// <param name="n">The sample number.</param>
				/// <param name="N">The window length.</param>
				static double HammingWindow(int n, int N) {
					return 0.54 - (0.46 * cos((2.0 * M_PI * (n + (N / 2))) / (N - 1)));
				}

				/// <summary>
				/// Generate Hann coefficiencts for an odd-length window centered at zero, of length N.
				/// </summary>
				/// <param name="n">The sample number.</param>
				/// <param name="N">The window length.</param>
				static double HannWindow(int n, int N) {
					return 0.50 * (1.0 - cos((2.0 * M_PI * (n + (N / 2))) / (N - 1)));
				}

				/// <summary>
				/// Generate Blackman coefficiencts for an odd-length window centered at zero, of length N.
				/// </summary>
				/// <param name="n">The sample number.</param>
				/// <param name="N">The window length.</param>
				static double BlackmanWindow(int n, int N) {
					return (7938.0 / 18608.0)
						- ((9240.0 / 18608.0) * cos((2.0 * M_PI * (n + (N / 2))) / (N - 1)))
						+ ((1430.0 / 18608.0) * cos((4.0 * M_PI * (n + (N / 2))) / (N - 1)));
				}

				/// <summary>
				/// Generate Nuttall coefficiencts for an odd-length window centered at zero, of length N.
				/// </summary>
				/// <param name="n">The sample number.</param>
				/// <param name="N">The window length.</param>
				static double NuttallWindow(int n, int N) {
					return 0.355768
						- (0.487396 * cos((2.0 * M_PI * (n + (N / 2))) / (N - 1)))
						+ (0.144232 * cos((4.0 * M_PI * (n + (N / 2))) / (N - 1)))
						- (0.012604 * cos((6.0 * M_PI * (n + (N / 2))) / (N - 1)));
				}

				/// <summary>
				/// Generate Blackman-Nuttall coefficiencts for an odd-length window centered at zero, of length N.
				/// </summary>
				/// <param name="n">The sample number.</param>
				/// <param name="N">The window length.</param>
				static double BlackmanNuttallWindow(int n, int N) {
					return 0.3635819
						- (0.4891775 * cos((2.0 * M_PI * (n + (N / 2))) / (N - 1)))
						+ (0.1365995 * cos((4.0 * M_PI * (n + (N / 2))) / (N - 1)))
						- (0.0106511 * cos((6.0 * M_PI * (n + (N / 2))) / (N - 1)));
				}

				/// <summary>
				/// Generate Blackman-Harris coefficiencts for an odd-length window centered at zero, of length N.
				/// </summary>
				/// <param name="n">The sample number.</param>
				/// <param name="N">The window length.</param>
				static double BlackmanHarrisWindow(int n, int N) {
					return 0.35875
						- (0.48829 * cos((2.0 * M_PI * (n + (N / 2))) / (N - 1)))
						+ (0.14128 * cos((4.0 * M_PI * (n + (N / 2))) / (N - 1)))
						- (0.01168 * cos((6.0 * M_PI * (n + (N / 2))) / (N - 1)));
				}

				/// <summary>
				/// Generate flat-top coefficiencts for an odd-length window centered at zero, of length N.
				/// </summary>
				/// <param name="n">The sample number.</param>
				/// <param name="N">The window length.</param>
				static double FlatTopWindow(int n, int N) {
					return 1.0
						- (1.93 * cos((2.0 * M_PI * (n + (N / 2))) / (N - 1)))
						+ (1.29 * cos((4.0 * M_PI * (n + (N / 2))) / (N - 1)))
						- (0.388 * cos((6.0 * M_PI * (n + (N / 2))) / (N - 1)))
						+ (0.028 * cos((6.0 * M_PI * (n + (N / 2))) / (N - 1)));
				}

				/// <summary>
				/// Generate Rectangle coefficiencts for an odd-length window centered at zero, of length N.
				/// </summary>
				/// <param name="n">The sample number.</param>
				/// <param name="N">The window length.</param>
				static double RectangleWindow(int n, int N) {
					return 1.0;
				}

				/// <summary>
				/// Generate low-pass filter coefficients for an odd-length filter of length N.
				/// </summary>
				/// <param name="omega_c">Normalized cutoff frequency.</param>
				/// <param name="n">The sample number.</param>
				/// <param name="N">The window length.</param>
				static double IdealLowPass(double omega_c, int n, int N) {
					if (n == 0) {
						return omega_c / M_PI;
					} else {
						return sin(omega_c * n) / (M_PI * n);
					}
				}

				/// <summary>
				/// Generate high-pass filter coefficients for an odd-length filter of length N.
				/// </summary>
				/// <param name="omega_c">Normalized cutoff frequency.</param>
				/// <param name="n">The sample number.</param>
				/// <param name="N">The window length.</param>
				static double IdealHighPass(double omega_c, int n, int N) {
					if (n == 0) {
						return 1.0 - (omega_c / M_PI);
					} else {
						return -1.0 * sin(omega_c * n) / (M_PI * n);
					}
				}

				/// <summary>
				/// Generate band-pass filter coefficients for an odd-length filter of length N.
				/// </summary>
				/// <param name="omega_1">Normalized lower cutoff frequency.</param>
				/// <param name="omega_2">Normalized upper cutoff frequency.</param>
				/// <param name="n">The sample number.</param>
				/// <param name="N">The window length.</param>
				static double IdealBandPass(double omega_1, double omega_2, int n, int N) {
					if (n == 0) {
						return (omega_2 - omega_1) / M_PI;
					} else {
						return (sin(omega_2 * n) / (M_PI * n)) - (sin(omega_1 * n) / (M_PI * n));
					}
				}

				/// <summary>
				/// Generate band-stop filter coefficients for an odd-length filter of length N.
				/// </summary>
				/// <param name="omega_1">Normalized lower cutoff frequency.</param>
				/// <param name="omega_2">Normalized upper cutoff frequency.</param>
				/// <param name="n">The sample number.</param>
				/// <param name="N">The window length.</param>
				static double IdealBandStop(double omega_1, double omega_2, int n, int N) {
					if (n == 0) {
						return 1.0 - ((omega_2 - omega_1) / M_PI);
					} else {
						return (sin(omega_1 * n) / (M_PI * n)) - (sin(omega_2 * n) / (M_PI * n));
					}
				}


			private:
				// Generate windowed coefficients for single-cutoff filters
				static double *GenerateCoefficients(WindowFunction f, CoefFunction1 g, int length, double omega_c) {
					int limit = length / 2;
					double* result = new double[length];
					for (int i = -limit; i <= limit; ++i) {
						double wn = f(i, length);
						double hn = g(omega_c, i, length);
						result[i + limit] = (wn * hn);
						#ifdef VERBOSE_DEBUG
						std::cerr << "h[" << i << "]: " << hn << "; w[" << i << "]: " << wn << "; coef[" << i + limit << "]: " << result[i + limit] << std::endl;
						#endif
					}
					return result;
				}

				// Generate windowed coefficients for single-cutoff filters
				static double *GenerateCoefficients(WindowFunction f, CoefFunction2 g, int length, double omega_c1, double omega_c2) {
					int limit = length / 2;
					double* result = new double[length];
					for (int i = -limit; i <= limit; ++i) {
						double wn = f(i, length);
						double hn = g(omega_c1, omega_c2, i, length);
						result[i + limit] = (wn * hn);
						#ifdef VERBOSE_DEBUG
						std::cerr << "h[" << i << "]: " << hn << "; w[" << i << "]: " << wn << "; coef[" << i + limit << "]: " << result[i + limit] << std::endl;
						#endif
					}
					return result;
				}

				// convert double array to float array
				template <typename sample_t>
				static sample_t *fromDouble(double *d, size_t len) {
					sample_t *result = new sample_t[len];
					for (size_t i = 0; i != len; ++i) {
						result[i] = d[i];
					}
					delete[] d;
					return result;
				}

				// convert double array to float array
				static float *toFloat(double *d, size_t len) {
					return fromDouble<float>(d, len);
				}

			public:
				template <typename T>
				static T* GenerateLowPassCoefficients(WindowFunction f, int length, double omega_c);
				template <typename T>
				static T* GenerateHighPassCoefficients(WindowFunction f, int length, double omega_c);
				template <typename T>
				static T* GenerateBandPassCoefficients(WindowFunction f, int length, double omega_c1, double omega_c2);
				template <typename T>
				static T* GenerateBandStopCoefficients(WindowFunction f, int length, double omega_c1, double omega_c2);

			public:
				//
				// FIR filter core (single precision).
				//
				static float Filter(float input, int &inputpos, float * const history, size_t hist_len, float * const coefs, size_t coef_len) {
					float output = 0.0f;

					// calculate the end pointers
					float* hist_end = history + hist_len;
					float* coef_end = coefs + coef_len;

					// calculate the initial array pointers
					float* hp = history + inputpos;
					float* cp = coefs;

					// store the input sample and wrap the history pointer
					*hp++ = input;
					if (hp == hist_end) {
						hp = history;
					}

					// run the filter
					while (cp != coef_end) {
						output += *hp++ * *cp++;

						// and make sure to wrap the history pointer
						if (hp == hist_end) {
							hp = history;
						}
					}

					// move the input index to the next value
					inputpos = (inputpos + 1) % hist_len;

					// return the result
					return output;
				}

				//
				// FIR filter core (double precision).
				//
				static double Filter(double input, int &inputpos, double * const history, size_t hist_len, double * const coefs, size_t coef_len) {
					double output = 0.0f;

					// calculate the end pointers
					double* hist_end = history + hist_len;
					double* coef_end = coefs + coef_len;

					// calculate the initial array pointers
					double* hp = history + inputpos;
					double* cp = coefs;

					// store the input sample and wrap the history pointer
					*hp++ = input;
					if (hp == hist_end) {
						hp = history;
					}

					// run the filter
					while (cp != coef_end) {
						output += *hp++ * *cp++;

						// and make sure to wrap the history pointer
						if (hp == hist_end) {
							hp = history;
						}
					}

					// move the input index to the next value
					inputpos = (inputpos + 1) % hist_len;

					// return the result
					return output;
				}
		};

		///
		/// Generate windowed low-pass coefficients (double precision).
		///
		template <>
		inline double* FirFilterUtils::GenerateLowPassCoefficients<double>(WindowFunction f, int length, double omega_c) {
			return GenerateCoefficients(f, IdealLowPass, length, omega_c);
		}

		///
		/// Generate windowed high-pass coefficients (double precision).
		///
		template <>
		inline double* FirFilterUtils::GenerateHighPassCoefficients<double>(WindowFunction f, int length, double omega_c) {
			return GenerateCoefficients(f, IdealHighPass, length, omega_c);
		}

		///
		/// Generate windowed band-pass coefficients (double precision).
		///
		template <>
		inline double* FirFilterUtils::GenerateBandPassCoefficients<double>(WindowFunction f, int length, double omega_c1, double omega_c2) {
			return GenerateCoefficients(f, IdealBandPass, length, omega_c1, omega_c2);
		}

		///
		/// Generate windowed band-stop coefficients (double precision).
		///
		template <>
		inline double* FirFilterUtils::GenerateBandStopCoefficients<double>(WindowFunction f, int length, double omega_c1, double omega_c2) {
			return GenerateCoefficients(f, IdealBandStop, length, omega_c1, omega_c2);
		}

		///
		/// Generate windowed low-pass coefficients (single precision).
		///
		template <>
		inline float* FirFilterUtils::GenerateLowPassCoefficients<float>(WindowFunction f, int length, double omega_c) {
			return toFloat(GenerateCoefficients(f, IdealLowPass, length, omega_c), length);
		}

		///
		/// Generate windowed high-pass coefficients (single precision).
		///
		template <>
		inline float* FirFilterUtils::GenerateHighPassCoefficients<float>(WindowFunction f, int length, double omega_c) {
			return toFloat(GenerateCoefficients(f, IdealHighPass, length, omega_c), length);
		}

		///
		/// Generate windowed band-pass coefficients (single precision).
		///
		template <>
		inline float* FirFilterUtils::GenerateBandPassCoefficients<float>(WindowFunction f, int length, double omega_c1, double omega_c2) {
			return toFloat(GenerateCoefficients(f, IdealBandPass, length, omega_c1, omega_c2), length);
		}

		///
		/// Generate windowed band-stop coefficients (single precision).
		///
		template <>
		inline float* FirFilterUtils::GenerateBandStopCoefficients<float>(WindowFunction f, int length, double omega_c1, double omega_c2) {
			return toFloat(GenerateCoefficients(f, IdealBandStop, length, omega_c1, omega_c2), length);
		}


		//
		//  Generic FIR filter
		//
		template <typename sample_t>
		class FirFilter : public IFilter<sample_t> {
			public:
				typedef enum {
					LowPass,
					HighPass,
					BandPass,
					BandStop,
				} Types;

			private:
				int m_InputPos;
				sample_t* m_History;
				sample_t* m_Coefs;

			private:
				void CalculateGain(Types type) {
					std::deque<sample_t> items;
					for (int i = 0; i != Length; ++i) {
						m_History[i] = 0;
						items.push_back(abs(m_Coefs[i]));
					}
					std::sort(items.begin(), items.end());
					OverallGain = 0.0;
					for (size_t i = 0; i != items.size(); ++i) {
						OverallGain += items[i];
					}
					GainCorrection = 1.0;
					if (OverallGain != 0) {
						GainCorrection = 1.0 / OverallGain;
					}
					for (int i = 0; i != Length; ++i) {
						m_Coefs[i] *= GainCorrection;
					}
				}

			public:
				/// The filter length.
				int Length;

				/// The overall gain of the coefficients.
				sample_t OverallGain;

				/// The gain factor applied to filter outputs to compensate for
				/// filter loss.
				sample_t GainCorrection;

			public:
				// ctor for BPF and BSF
				FirFilter(Types type, int length, double f1, double f2, size_t fs, WindowFunction f = FirFilterUtils::HammingWindow) {
					// order must be odd
					if ((length % 2) == 0)
						++length;
					Length = length;
					double omega_c1 = 2 * M_PI * f1 / fs;
					double omega_c2 = 2 * M_PI * f2 / fs;

					// generate double coefs, then convert
					switch (type) {
						case BandPass:
							m_Coefs = FirFilterUtils::GenerateBandPassCoefficients<sample_t>(f, length, omega_c1, omega_c2);
							break;
						case BandStop:
							m_Coefs = FirFilterUtils::GenerateBandStopCoefficients<sample_t>(f, length, omega_c1, omega_c2);
							break;
						case LowPass:
						case HighPass:
							throw FirFilterException("This constructor is only for bandpass and bandstop filters");
						default:
							throw FirFilterException("Unknown filter type specified");
					}
					m_History = new sample_t[length];
					m_InputPos = 0;

					CalculateGain(type);
#ifdef VERBOSE_DEBUG
					for (int i = 0; i != length; ++i) {
						std::cerr << "Filter[" << i << "] = " << m_Coefs[i] << std::endl;
					}
					std::cerr << "Filter Gain = " << OverallGain << std::endl;
#endif
				}

				// ctor for LPF and HPF
				FirFilter(Types type, int length, double fc, size_t fs, WindowFunction f = FirFilterUtils::HammingWindow) {
					// order must be odd
					if ((length % 2) == 0)
						++length;
					Length = length;
					double omega_c = 2 * M_PI * fc / fs;

					// generate double coefs, then convert
					switch (type) {
						case LowPass:
							m_Coefs = FirFilterUtils::GenerateLowPassCoefficients<sample_t>(f, length, omega_c);
							break;
						case HighPass:
							m_Coefs = FirFilterUtils::GenerateHighPassCoefficients<sample_t>(f, length, omega_c);
							break;
						case BandPass:
						case BandStop:
							throw FirFilterException("This constructor cannot be used for bandpass and bandstop filters");
						default:
							throw FirFilterException("Unknown filter type specified");
					}
					m_History = new sample_t[length];
					m_InputPos = 0;

					CalculateGain(type);
#ifdef VERBOSE_DEBUG
					for (int i = 0; i != length; ++i) {
						std::cerr << "Filter[" << i << "] = " << m_Coefs[i] << std::endl;
					}
					std::cerr << "Filter Gain = " << OverallGain << std::endl;
#endif
				}

			public:
				//
				//  the sample-by-sample filter function
				//
				sample_t filter(sample_t sample) {
					return FirFilterUtils::Filter(sample, m_InputPos, m_History, Length, m_Coefs, Length);
				}
		};
	}
}
#endif // KK5JY_FIRFILTER_H
