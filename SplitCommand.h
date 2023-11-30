/*
 *
 *
 *    SplitCommand.h
 *
 *    License: GNU General Public License Version 3.0.
 *    
 *    Copyright (C) 2016-2018 by Matt Roberts, KK5JY.
 *    All rights reserved.
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *    
 *    This program is distributed in the hope that it will be useful, but
 *    WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    General Public License for more details.
 *    
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see: http://www.gnu.org/licenses/
 *
 *
 */

#ifndef __SPLIT_COMMAND_H
#define __SPLIT_COMMAND_H

#include "stype.h"

//
//  SplitCommand(...) - split a command from its argument
//
inline void SplitCommand(std::string &input, std::string &cmd, std::string &arg) {
	bool inCmd = true;
	cmd.clear();
	arg.clear();
	for (unsigned i = 0; i != input.length(); ++i) {
		char ch = input.at(i);
		if (ch == '=') {
			inCmd = false;
			continue;
		}
		if (inCmd) {
			cmd += ch;
		} else {
			arg += ch;
		}
	}

	// clean up the input in case it needs to be used in a response
	input.clear();
	cmd = my::toUpper(cmd);
	input += cmd;
	if (arg.size() != 0) {
		input += '=';
		input += arg;
	}
}

#endif // __SPLIT_COMMAND_H
