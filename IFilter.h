/*
 *
 *
 *    IFilter.h
 *
 *    Filter interface.
 *
 *    Copyright (C) 2016 by Matt Roberts,
 *    All rights reserved.
 *
 *    License: GNU GPL3 (www.gnu.org)
 *
 *
 */

#ifndef __KK5JY_IFILTER_H
#define __KK5JY_IFILTER_H

template <typename sample_t>
class IFilter {
	public:
		virtual sample_t filter(sample_t sample) = 0;
		virtual ~IFilter() { /* nop */ };
};

#endif // __KK5JY_IFILTER_H
