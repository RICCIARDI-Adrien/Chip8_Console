/** @file Video_Player.c
 * See Video_Player.h for description.
 * @author Adrien RICCIARDI
 */
#include <Display.h>
#include <FAT.h>
#include <Keyboard.h>
#include <Log.h>
#include <Shared_Buffer.h>
#include <string.h>
#include <Video_Player.h>
#include <xc.h>

//-------------------------------------------------------------------------------------------------
// Private constants
//-------------------------------------------------------------------------------------------------
/** Set to 1 to enable the log messages, set to 0 to disable them. */
#define VIDEO_PLAYER_IS_LOGGING_ENABLED 1

//-------------------------------------------------------------------------------------------------
// Private functions
//-------------------------------------------------------------------------------------------------
// TODO
static inline unsigned char VideoPlayerFindFile(char *Pointer_String_File_Name, TFATFileDescriptor *Pointer_File_Descriptor)
{
	TFATFileInformation File_Information;

	// Begin listing the files
	if (FATListStart("/") != 0)
	{
		LOG(VIDEO_PLAYER_IS_LOGGING_ENABLED, "FATListStart() failed.");
		return 1;
	}

	// Search for the video file
	while (FATListNext(&File_Information) == 0)
	{
		LOG(VIDEO_PLAYER_IS_LOGGING_ENABLED, "File found : name=\"%s\", is directory=%u, size=%lu, first cluster=%lu.",
			File_Information.String_Short_Name,
			File_Information.Is_Directory,
			File_Information.Size,
			File_Information.First_Cluster_Number);

		// We are searching for a file, discard any directory
		if (File_Information.Is_Directory)
		{
			LOG(VIDEO_PLAYER_IS_LOGGING_ENABLED, "This is a directory, skipping it.");
			continue;
		}

		// Is this the searched file ?
		if (strcmp((char *) File_Information.String_Short_Name, Pointer_String_File_Name) == 0)
		{
			LOG(VIDEO_PLAYER_IS_LOGGING_ENABLED, "Found the video file.");
			FATReadSectorsStart(&File_Information, Pointer_File_Descriptor);
			return 0;
		}
	}

	LOG(VIDEO_PLAYER_IS_LOGGING_ENABLED, "No video file found.");
	return 1;
}

//-------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------
void VideoPlayer(void)
{
	TFATFileDescriptor File_Descriptor;
	unsigned long w = 0;

	// Retrieve the movie file to play
	if (VideoPlayerFindFile("VIDEO.BIN", &File_Descriptor) != 0)
	{
		LOG(VIDEO_PLAYER_IS_LOGGING_ENABLED, "No video file found.");
		DisplayDrawTextMessage(Shared_Buffer_Display, "SD card", "No video file found.\nReplace the SDcard\nand press Menu.");
		while (!KeyboardIsMenuKeyPressed());
		return;
	}

	// TEST
	while (FATReadSectorsNext(&File_Descriptor, 2, Shared_Buffers.Buffer) == 0)
	{
		// Check on each frame if the user wants to exit
		if (KeyboardIsMenuKeyPressed())
		{
			LOG(VIDEO_PLAYER_IS_LOGGING_ENABLED, "User has quit.");
			return;
		}

		DisplayDrawFullSizeBuffer(Shared_Buffers.Buffer);
		// TODO
		__delay_ms(20);
	}
}
