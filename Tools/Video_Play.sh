#!/bin/sh

if [ "$#" -ne 1 ]
then
	echo "Usage : $0 Input_Video_File"
	echo "The input file must be of VD8 format."
	return 1
fi

# -an : remove sound
# See the conversion tool for more information on the options
ffplay -an -f rawvideo -pixel_format monob -video_size 128x64 -framerate 25 "${1}"
