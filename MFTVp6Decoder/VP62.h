//-----------------------------------------------------------------------------------------------
// VP62.h
// This file comes from ffdshow project
//-----------------------------------------------------------------------------------------------
#ifndef VP62_H
#define VP62_H

class VP62{

  private:
				
				BYTE   *inputBuffer;	// Points to payload buffer

				// Frame characteristics
    int	    frameType;
    int	    tagFrame;
    int	    displayRows;
    int	    displayColumns;
    int	    movieWidth;
    int	    movieHeight;
    int	    lastRows;
    int	    lastCols;

    // Interlacing
    int	    interlacedBlock;
    bool    useInterlacing;
    int	    ilProb;

    int	    quantizer;
    int	    lastQuantizer;
    short   coeffScale[64];

    // Arithmetic Coding (AC)
    int	    acHigh;
    unsigned long acCodeWord;
    int	    acBits;
    BYTE   *acBuffer;
    BYTE   *acBufferEnd;

    // Buffers
    BYTE   *yuvCurrentFrame;
    BYTE   *yuvTaggedFrame;
    BYTE   *yuvLastFrame;

    int	    yStride;	// actual stride + 96
    int	    uvStride;   // actual stride + 48
    int	    ySize;	// Luma Y
    int	    uvSize;	// Chroma U or V
    int	    mbc;	// Number of horizontal MB
    int	    mbr;	// Number of vertical MB
    int	    orgPlaneStride[6];
    int	    planeStride[6];
    int	    pixelOffset[6]; // Offset to upper left corner pixel for each blocks

    // DC Predictors management
				struct AB{
						
						char  notNullDC;
						char  refFrame;
						short dcCoeff;
				}*aboveBlocks;	// Above blocks
				
				int aboveBlockIndex[6];

    struct AB prevBlock[4]; // Left blocks
    short   prevDCRefFrame[3][3];

    // Blocks / Macroblock
    int	    currentMbType;
    int	    prevMbType;
    short   block8x8[6][64];
    BYTE    block12x12[144];   // Intermediate 12x12 block for filtering
    int	    coeffIndexToPos[64];
    BYTE    coeffReorder[64];
    short   coeff420[6][64];  // DCT Coeff for each blocks
    
				struct MB{
						
						BYTE type;	// MB type
						short vx,vy;	// Motion vector
				}*macroblocks;

    // Vectors (Motion compensation)
    short   blockVector[6][2];	// Vectors for each block in MB
    short   prevFrameFirstVectorCandidate[2];
    short   prevFrameSecondVectorCandidate[2];
    int	    prevFrameFirstVectorCandidatePos;
    short   tagFrameFirstVectorCandidate[2];
    short   tagFrameSecondVectorCandidate[2];
    int	    tagFrameFirstVectorCandidatePos;

    // Predictors candidates
    int	    predOffset[12];

    // Filtering hints for moved blocks
    int	    blockCopyFiltering;
    int	    blockCopyFilterMode;
    int	    maxVectorLength;
    int	    sampleVarianceThreshold;
    int	    filterSelection;

    // AC Models
    BYTE    sigVectorModel[2];		    // Delta sign
    BYTE    dctVectorModel[2];		    // Delta Coding Types
    BYTE    pdvVectorModel[2][7];	    // Predefined Delta Values
    BYTE    fdvVectorModel[2][8];	    // 8 bit delta value definition
    BYTE    sameMbTypeModel[3][10];	    // Same as previous MB type
    BYTE    nextMbTypeModel[3][10][9];	    // Next MB type
    BYTE    dccvCoeffModel[2][11];	    // DC Coeff value
    BYTE    ractCoeffModel[2][3][6][11];    // Run/AC coding type and AC coeff value
    BYTE    dcctCoeffModel[2][3][5];	    // DC coeff coding type
    BYTE    runvCoeffModel[2][14];	    // Run value
    BYTE    mbTypesStats[3][2][10];	    // Contextual, next MbType statistics

    //////// METHODS
    void    decodeFrame();
    int	    parseHeader();
    void    allocateBuffers();
    void    initOffscreenBorders(BYTE *yuv);
    void    initCoeffScales();

    // Arithmetic coding
    void    acInit(BYTE *);
    int	    acGetBit(int prob);
    int	    acGetBit();
    int	    acGetBits(int bits);

    // Block blitters
    void    drawBlock(int block);
    void    drawDeltaBlockFromYuv(BYTE *yuv, int block);
    void    drawDeltaBlockFromMB(short *srcBlock, short *delta, int b);

    // Models
    void    defaultModelsInit();
    void    parseVectorModelsChanges();
    void    parseVectorAdjustment(short *v, int mbType);
    void    parseCoeffModelsChanges();
    void    parseMacroblockTypeModelsChanges();
    void    initCoeffOrderTable();

    // Parsing/Decoding
    void    decodeMacroBlock(int row, int col);
    int	    parseMacroblockType(int prevType, int index);
    void    parseCoeff();
    void    addPredictorsDC();
    void    decodeMbTypeAndVectors(int row, int col);
    void    decode4Vectors(int row, int col);

    // iDCT
    void    iDCT8x8(int block);

    // Motion compensation
    void    getBlock(int block, short *dstBlock, BYTE *yuv);
    int	    getVectorsPredictors(int row, int col, int refFrame);

    // Copy/filter
    void    fill12x12Block(BYTE *yuv, int pixOffset, int vx, int vy, int block);
    void    simpleBlockCopy(short *dstBlock, BYTE *yuv, int offset, int stride);
    void    fourPointFilterHV(short *dstBlock, BYTE *yuv, int offset, int stride, int delta, int *filter);
    void    fourPointFilterDiag(short *dstBlock, BYTE *yuv, int offset, int stride, int *hFilter, int *vFilter);
    void    aaFilterHV(short *dstBlock, BYTE *yuv, int offset, int stride, int delta, int *aa);
    void    aaFilterDiag(short *dstBlock, BYTE *yuv, int offset, int stride, int *hAA, int *vAA);
    void    filteredBlockCopy(short *dstBlock, BYTE *yuv, int iOffset, int oOffset, int stride, int x8, int y8, bool useEnhancedFilter, int select);
    void    edgeFilter(int n, int pixInc, int lineInc, int t);

  public:
				
				VP62();	// Constructor
				~VP62();	// Destructor

				int decodePacket(BYTE *payload, int length);
				void getImageSize(int *width, int *height);
				void getDisplaySize(int *width, int *height);   // Maybe different than image
				void getRGB(BYTE *rgb);	// Returns RGB32 image
				void getYUV(BYTE** yuv, int* pitch);
				void ReleaseDecoder();
				void getRGB32(BYTE *rgb);
};

inline void VP62::ReleaseDecoder(){

		if(yuvCurrentFrame){
		  free(yuvCurrentFrame);
				yuvCurrentFrame = NULL;
		}

		if(yuvTaggedFrame){
    free(yuvTaggedFrame);
				yuvTaggedFrame = NULL;
		}

		if(yuvLastFrame){
    free(yuvLastFrame);
				yuvLastFrame = NULL;
		}

		if(macroblocks){
    free(macroblocks);
				macroblocks = NULL;
		}

		if(aboveBlocks){
    free(aboveBlocks);
				aboveBlocks = NULL;
		}
}

#endif
