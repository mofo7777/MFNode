//----------------------------------------------------------------------------------------------
// ScreenCapture_def.h
//----------------------------------------------------------------------------------------------
#ifndef SCREENCAPTURE_DEF_H
#define SCREENCAPTURE_DEF_H

// GetDIBits provide a reverse image.
// We can handle it if we define REVERSE_GDI_IMAGE.
// Otherwise, we can use the Video Renderer or MFT to handle it.
#define REVERSE_GDI_IMAGE

//#define MF_SCREEN_RESOLUTION_24
#define MF_SCREEN_RESOLUTION_32

#define VIDEO_WIDTH              400
#define VIDEO_HEIGHT             300
#define VIDEO_FRAME_RATE          10
#define VIDEO_FRAME_RATE_NUM      10 // for MFRatio
#define VIDEO_FRAME_RATE_DEN       1 // for MFRatio

#define VIDEO_DURATION    10000000 / VIDEO_FRAME_RATE

#if defined MF_SCREEN_RESOLUTION_24 && !defined MF_SCREEN_RESOLUTION_32
#define VideoFormatProject    MFVideoFormat_RGB24
#define VIDEO_BYTE_RGB        24
#define VIDEO_OCTET_RGB       3
#elif defined MF_SCREEN_RESOLUTION_32 && !defined MF_SCREEN_RESOLUTION_24
#define VideoFormatProject    MFVideoFormat_RGB32
#define VIDEO_BYTE_RGB        32
#define VIDEO_OCTET_RGB       4
#else
#error One screen resolution must be defined !
#endif

#endif