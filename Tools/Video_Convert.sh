#!/bin/sh

if [ "$#" -ne 2 ]
then
	echo "Usage : $0 Input_Video_File Output_Video_File"
	echo "The output file .vd8 extension is automatically appended."
	return 1
fi

Input_File="$1"
Output_File="$2"

# -an : remove sound
# -pix_fmt monob : convert to black and white with dithering
# -y : overwrite the output file without confirmation
# -r : force the output frame rate
ffmpeg -i "${1}" -vf "scale=128:64" -an -y -f rawvideo -pix_fmt monob -r 25 "${2}.vd8"
