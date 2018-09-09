//----------------------------------------------------------------------------------------------
// Mpeg2Definition.h
//----------------------------------------------------------------------------------------------
#ifndef MPEG2DEFINITION_H
#define MPEG2DEFINITION_H

class CMFMpeg2Source;
class CMFMpeg2Stream;

const DWORD READ_SIZE = 4096;
const DWORD SAMPLE_QUEUE = 2;

typedef CGrowableArray<CMFMpeg2Stream*> StreamList;
typedef CTinyMap<BYTE, DWORD> StreamMap;

typedef ComPtrList<IMFSample> SampleList;
typedef ComPtrList<IUnknown, true> TokenList;

#endif