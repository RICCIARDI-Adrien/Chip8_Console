/** @file Log.h
 * Provide debug logging facilities that can be disabled in the release build.
 * @author Adrien RICCIARDI
 */
#ifndef H_LOG_H
#define H_LOG_H

#include <stdio.h>

//-------------------------------------------------------------------------------------------------
// Constants and macros
//-------------------------------------------------------------------------------------------------
/** Print a printf() like message on the serial port preceded by the function name and the line.
 * @param Is_Enabled Set to 1 to compile the message in, set to 0 do remove the message at the compilation time.
 * @param Format A printf() like format string.
 * @note The internal buffer is limited to 256 bytes, do not write too long strings.
 */
#ifdef LOG_IS_ENABLED
	#define LOG(Is_Enabled, Format, ...) do { if (Is_Enabled) printf("[%s:%d] " Format "\r\n", __FUNCTION__, __LINE__, ##__VA_ARGS__); } while (0)
#else
	#define LOG(Is_Enabled, Format, ...) do {} while (0)
#endif

#endif
