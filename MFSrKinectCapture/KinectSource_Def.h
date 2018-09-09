//----------------------------------------------------------------------------------------------
// KinectSource_Def.h
//----------------------------------------------------------------------------------------------
#ifndef KINECTSOURCE_DEF_H
#define KINECTSOURCE_DEF_H

#define KINECT_IMAGE_RESOLUTION_640x480
//#define KINECT_IMAGE_RESOLUTION_1280x960

#ifdef KINECT_IMAGE_RESOLUTION_640x480

#define KINECT_VIDEO_WIDTH        640
#define KINECT_VIDEO_HEIGHT       480
#define KINECT_VIDEO_NUMERATOR     30
#define KINECT_VIDEO_DENOMINATOR    1
#define KINECT_IMAGE_RESOLUTION   NUI_IMAGE_RESOLUTION_640x480

#elif defined KINECT_IMAGE_RESOLUTION_1280x960

#define KINECT_VIDEO_WIDTH        1280
#define KINECT_VIDEO_HEIGHT       960
#define KINECT_VIDEO_NUMERATOR     15
#define KINECT_VIDEO_DENOMINATOR    1
#define KINECT_IMAGE_RESOLUTION   NUI_IMAGE_RESOLUTION_1280x960

#else

#error Must define an image resolution

#endif

#endif