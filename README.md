# MFNode

The aim of this project is to explore MediaFoundation API. It provides some ways on how to use it.
This project is just experimental, and you can see it as an extension to the MediaFoundation samples.

You will find :

* audio/video player (win32)
* custom media session
* sequencer source
* audio/video capture
* screen capture
* kinect V1 capture
* wave audio mixer
* http streaming (winsock)
* jpeg encoding (Gdiplus)
* dxva2 technologie
* cuda decoding
* directX9 renderer and shader
* COM technologie
* different kind of mediafoundation Source, Sink and Transform (mpeg2, flv, a52, vp6...)
* mp3 to wave transcoder


## Project diretory details

* **Common** : some files used by differents projects.
* **LibA52Audio** : a decoder for ac3 audio format.
* LibMpeg2 : http://sourceforge.net/projects/libmpeg2/ (version libmpeg2-0.5.1). It's the original
project, no modification have been made.
* LibMpegAudio : a decoder for mp1/mp2 audio format.
* MFNodePlayer : a player to handle wave mixer, JpegHttpStreamer, ScreenCapture, Flv file and video renderer.
* MFSkDxva2Renderer : a video sink renderer Dxva2 (handle nv12 format from cuda decoder).
* MFSkImageWriter: a MediaFoundation sink to save image as bmp file.
* MFSkJpegHttpStreamer : a MediaFoundation sink to stream http jpeg.
* MFSkVideoRenderer : a video sink renderer with shader and DirectX9 (handle yuv format from mpeg2 decoder).
* MFSrA52Decoder : a MediaFoundation source for ac3 format (alpha version).
* MFSrFlvSplitter : a MediaFoundation source for flv format (h264, vp6, aac, mp3).
* MFSrKinectCapture : a MediaFoundation source for Kinect.
* MFSrMpeg12Decoder : a MediaFoundation source to decode mp1/mp2 audio format.
* MFSrMpeg2Splitter : a basic MediaFoundation source to split mpeg1/2 video format.
* MFSrScreenCapture : a MediaFoundation source for screenshot.
* MFTA52Decoder : a MediaFoundation transform to decode ac3 audio (alpha version).
* MFTCudaDecoder : a MediaFoundation transform to decode mpeg1/2 video format using cuda (progressive frame only).
* MFTDxva2Decoder :  a MediaFoundation transform to decode mpeg1/2 video format using dxva2.
* MFTJpegEncoder : a MediaFoundation transform to encode rgb data to jpeg data.
* MFTMpeg12Decoder : a MediaFoundation transform to decode mp1/mp2 audio format.
* MFTMpeg2Decoder : a MediaFoundation transform to decode mpeg1/mpeg2 video.
* MFTVp6Decoder : a MediaFoundation transform to decode VP 6.2 video.
* MFTWaveMixer : a MediaFoundation transform to mix two audio file (need same format for both).
it can't be use with topoedit, because of a bug in topoedit. You can use MFNodePlayer.
* Mp3Decoding : a console program to decode mp3 to wave.


See README.txt for more.
