MFNode Project

//----------------------------------------------------------------------------------------------------------------------
-> For this project, you need :


- Microsoft Windows Vista or later
- Visual Studio 2017 community
- Microsoft Kinect SDK (optionnal)
- Nvidia Cuda SDK (optionnal)


//----------------------------------------------------------------------------------------------------------------------
-> Project diretory


- Common : some files used by differents projects.

- LibA52Audio : a decoder for ac3 audio format.

- LibMpeg2 : http://sourceforge.net/projects/libmpeg2/ (version libmpeg2-0.5.1). It's the original
project, no modification have been made.

- LibMpegAudio : a decoder for mp1/mp2 audio format.

- MFNodePlayer : a player to handle wave mixer, JpegHttpStreamer, ScreenCapture, Flv file and video renderer.

- MFSkDxva2Renderer : a video sink renderer Dxva2 (handle nv12 format from cuda decoder).

- MFSkImageWriter: a MediaFoundation sink to save image as bmp file.

- MFSkJpegHttpStreamer : a MediaFoundation sink to stream http jpeg.

- MFSkVideoRenderer : a video sink renderer with shader and DirectX9 (handle yuv format from mpeg2 decoder).

- MFSrA52Decoder : a MediaFoundation source for ac3 format (alpha version).

- MFSrFlvSplitter : a MediaFoundation source for flv format (h264, vp6, aac, mp3).

- MFSrKinectCapture : a MediaFoundation source for Kinect.

- MFSrMpeg12Decoder : a MediaFoundation source to decode mp1/mp2 audio format.

- MFSrMpeg2Splitter : a basic MediaFoundation source to split mpeg1/2 video format.

- MFSrScreenCapture : a MediaFoundation source for screenshot.

- MFTA52Decoder : a MediaFoundation transform to decode ac3 audio (alpha version).

- MFTCudaDecoder : a MediaFoundation transform to decode mpeg1/2 video format using cuda (progressive frame only).

- MFTDxva2Decoder :  a MediaFoundation transform to decode mpeg1/2 video format using dxva2.

- MFTJpegEncoder : a MediaFoundation transform to encode rgb data to jpeg data.

- MFTMpeg12Decoder : a MediaFoundation transform to decode mp1/mp2 audio format.

- MFTMpeg2Decoder : a MediaFoundation transform to decode mpeg1/mpeg2 video.

- MFTVp6Decoder : a MediaFoundation transform to decode VP 6.2 video.

- MFTWaveMixer : a MediaFoundation transform to mix two audio file (need same format for both).
it can't be use with topoedit, because of a bug in topoedit. You can use MFNodePlayer.

- Mp3Decoding : a console program to decode mp3 to wave.


//----------------------------------------------------------------------------------------------------------------------
-> general purpose

With Visual Studio i use those parameters (Tools->Options->Editor Text->C/C++) :

- Tab size         = 1
- Withdrawals size = 2
- All project are only use in debug mode. I don't use release mode, because for now, MFNode Project is in Beta version.


//----------------------------------------------------------------------------------------------------------------------
-> Compilation for MFTMpeg2Decoder


- First you need to compile Mpeg2Lib. It's create a static library use by MFTMpeg2Decoder.

- Second you compile MFTMpeg2Decoder. It's create MFTMpeg2Decoder.dll. You need to register it with Regsrv32.


//----------------------------------------------------------------------------------------------------------------------
-> Compilation for MFSrMpeg12Decoder


- First you need to compile LibMpegAudio.lib It's create a static library use by MFSrMpeg12Decoder.

- Second you compile MFSrMpeg12Decoder. It's create MFSrMpeg12Decoder.dll. You need to register it with Regsrv32.


//----------------------------------------------------------------------------------------------------------------------
-> Compilation for MFSrKinectCapture

- you need Kinect SDK V1.7. if you have a different version, change the Directory in the project.
- Compile MFSrKinectCapture. It's create MFSrKinectCapture.dll. You need to register it with Regsrv32.


//----------------------------------------------------------------------------------------------------------------------
-> Compilation for MFSkJpegHttpStreamer, MFSrScreenCapture, MFTWaveMixer, MFTJpegEncoder and MFTMpeg12Decoder

- Just compile and register with Regsrv32.


//----------------------------------------------------------------------------------------------------------------------
-> Compilation for MFNodePlayer

- Compile MFTWaveMixer and register with Regsrv32.
- Compile MFNodePlayer.


//----------------------------------------------------------------------------------------------------------------------
-> Use : Mpeg2 Transform


To use the decoder, you need a Source Node that demux video frame. You can use the MediaFoundation sample mpeg1source :

(C:\Program Files\Microsoft SDKs\Windows\v7.1\Samples\multimedia\mediafoundation\mpeg1source\). This Source Node
only demux Mpeg1 video frame, not mpeg2 video frame (a mpeg2source is currently under developpement). Register the
mpeg1source.dll with Regsvr32.

Now you can also use MFSrMpeg2Splitter.


For testing you need :

- a video mpeg1 format.
- both MFTMpeg2Decoder.dll, mpeg1source.dll (Regsvr32)
- TopoEdit : (C:\Program Files\Microsoft SDKs\Windows\v7.1\Bin\topoedit.exe).


- Launch TopoEdit.
- Topology->Add Source (choose the mpeg1 video).
- Topology->Add Transform (Video Decoder -> MFNode Mpeg2 Decoder -> Add)
- Topology->Add EVR.
- Manually join the Node between them in this order : source->decoder->evr
- Topology->Resolve Topology.
- Button Play.


//----------------------------------------------------------------------------------------------------------------------
-> Use : Kinect source

- compile MFPlayer2 sample :
(C:\Program Files\Microsoft SDKs\Windows\v7.1\Samples\multimedia\mediafoundation\MFPlayer2\).

- Launch MFPlayer.exe.
- File->Open URL.
- tape : "kinect:"
(for audio, a good idea is to use headphone, because the sound will repeat indefinitely).


//----------------------------------------------------------------------------------------------------------------------
-> Use : MFSrScreenCapture

- Use MFNodePlayer or :

- compile MFPlayer2 sample :
(C:\Program Files\Microsoft SDKs\Windows\v7.1\Samples\multimedia\mediafoundation\MFPlayer2\).

- Launch MFPlayer.exe.
- File->Open URL.
- tape : "screen:"


//----------------------------------------------------------------------------------------------------------------------
-> Use : MFTWaveMixer

- You can to use it with MFNodePlayer.


//----------------------------------------------------------------------------------------------------------------------
-> Use : MFSrMpeg12Decoder

- You can use TopoEdit.
- Audio file extension is .mp1 or .mp2


//----------------------------------------------------------------------------------------------------------------------
-> Use : MFTMpeg12Decoder

- You can use TopoEdit or MFNodePlayer.


//----------------------------------------------------------------------------------------------------------------------
-> Use : MFTJpegEncoder

- You can to use it with MFNodePlayer.


//----------------------------------------------------------------------------------------------------------------------
-> Use : MFSkJpegHttpStreamer

- You can to use it with MFNodePlayer.
- Run HttpStreamer session.
- Use a tool like VLC and open a network stream : http://127.0.0.1:13754.

//----------------------------------------------------------------------------------------------------------------------
-> Todo : explanation for :

* MFSrMpeg2Splitter
* MFSkImageWriter
* MFTCudaDecoder
* MFTDxva2Decoder
* MFSrFlvSplitter
* MFTVp6Decoder
* MFSrA52Decoder
* MFTA52Decoder
* MFSkVideoRenderer
* MFSkDxva2Renderer

