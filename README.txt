#
#
#   README.txt
#
#   Build instructions for the 'fdvcore' executable.
#
#   Copyright (C) 2018 by Matt Roberts, KK5JY.
#   All rights reserved.
#
#   License: GPL v3 (www.gnu.org)
#
#

This project is a software kit, and does require that the integrator or
hobbyist have some level of expertise in building and using custom
software packages.

This project depends on some system packages on Ubuntu, Mint, or Raspbian:

	build-essential
	libasound2-dev
	alsa-utils
	alsa-tools
	python

The project also depends on some packages that are only available in
source form:

	Codec2 (from https://freedv.org/)
	rtaudio v4 or v5 (from https://www.music.mcgill.ca/~gary/rtaudio/)

The details of building and installing those projects can be found on
their respective websites.

This project has two components, 'fdvcore', which is built from C++
source, and 'smalldv' which is a Python script.

To build the 'fdvcore' project:

	make

To build an executable that has no debugging symbols (yields smaller
and faster code):

	make strip

To install 'fdvcore' and 'smalldv' into /usr/local/bin:

	sudo make install

For more details about the project, see the website:

	http://www.kk5jy.net/smalldv-v1/

# EOF: README.txt
