//----------------------------------------------------------------------------------------------
// MFFile.h
// Copyright (C) 2014 Dumonteil David
//
// MFNode is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// MFNode is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//----------------------------------------------------------------------------------------------
#ifndef MFFILE_H
#define MFFILE_H

class CMFFile{

  public:

				CMFFile() : m_hFileRead(INVALID_HANDLE_VALUE), m_pData(NULL), m_dwFileSize(0){}
				~CMFFile(){ Close(); }

				// Open file completely...
				BOOL MFOpenFile(const WCHAR* wszFile){

						BOOL bRet = FALSE;
						Close();

						m_hFileRead = CreateFile(wszFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

						if(m_hFileRead == INVALID_HANDLE_VALUE){
								IF_ERROR_RETURN(bRet);
						}

						// Does not handle large file...
						m_dwFileSize = GetFileSize(m_hFileRead, NULL);

						if(m_dwFileSize == INVALID_FILE_SIZE){
								IF_ERROR_RETURN(bRet);
						}

						m_pData = new (std::nothrow)BYTE[m_dwFileSize];

						if(m_pData == NULL){
								IF_ERROR_RETURN(bRet);
						}

						DWORD dwRead;

						if(!ReadFile(m_hFileRead, (LPVOID)m_pData, m_dwFileSize, &dwRead, 0) || dwRead == 0){
								IF_ERROR_RETURN(bRet);
						}

						return bRet = TRUE;
				}

				// Expect OpenFile succeeded : nothing check...
				BYTE* GetFileBuffer(){ return m_pData; }
				DWORD GetSizeFile() const{ return m_dwFileSize; }

				void Close(){ CLOSE_HANDLE_IF(m_hFileRead); SAFE_DELETE_ARRAY(m_pData); m_dwFileSize = 0; }

		private:

				HANDLE m_hFileRead;
				BYTE* m_pData;
				DWORD m_dwFileSize;
};

class CMFWriteFile{

  public:

				CMFWriteFile() : m_hFile(INVALID_HANDLE_VALUE){}
				~CMFWriteFile(){ CLOSE_HANDLE_IF(m_hFile); }

				BOOL MFCreateFile(const WCHAR* wszFile){

						BOOL bRet = FALSE;
						CLOSE_HANDLE_IF(m_hFile);

						m_hFile = CreateFile(wszFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

						if(m_hFile == INVALID_HANDLE_VALUE){
								IF_ERROR_RETURN(bRet);
						}

						return bRet = TRUE;
				}

				BOOL MFWriteFile(const BYTE* pData, const DWORD dwLength){

						BOOL bRet = FALSE;
		    DWORD dwWritten;

		    if(!WriteFile(m_hFile, (LPCVOID)pData, dwLength, &dwWritten, 0) || dwWritten != dwLength){
				    IF_ERROR_RETURN(bRet);
      }

		    return bRet = TRUE;
				}

				static BOOL MFCreateAndWriteFile(const WCHAR* wszFile, const BYTE* pData, const DWORD dwLength){

						BOOL bRet = FALSE;
						HANDLE hFile = INVALID_HANDLE_VALUE;
		    DWORD dwWritten;

						hFile = CreateFile(wszFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

						if(hFile == INVALID_HANDLE_VALUE){
								IF_ERROR_RETURN(bRet);
						}

						bRet = WriteFile(hFile, (LPCVOID)pData, dwLength, &dwWritten, 0);

						CLOSE_HANDLE_IF(hFile);

						if(bRet && dwWritten != dwLength){
								bRet = FALSE;
								IF_ERROR_RETURN(bRet);
						}

		    return bRet;
				}

				// Not tested yet...
				static BOOL MFCreateAndWriteRGB24File(const WCHAR* wszFile, const BYTE* pData, const DWORD dwLength, const UINT uiWidth, const UINT uiHeight){

						BOOL bRet = FALSE;
						HANDLE hFile = INVALID_HANDLE_VALUE;
		    DWORD dwWritten;

						hFile = CreateFile(wszFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

						if(hFile == INVALID_HANDLE_VALUE){
								IF_ERROR_RETURN(bRet);
						}

						BITMAPFILEHEADER bmfHeader;
		    BITMAPINFOHEADER bi;

		    bi.biSize = sizeof(BITMAPINFOHEADER);
		    bi.biWidth = uiWidth;
		    bi.biHeight = uiHeight;
		    bi.biPlanes = 1;
		    bi.biBitCount = 24;
		    bi.biCompression = BI_RGB;
		    bi.biSizeImage = 0;
		    bi.biXPelsPerMeter = 0;
		    bi.biYPelsPerMeter = 0;
		    bi.biClrUsed = 0;
		    bi.biClrImportant = 0;

						DWORD dwSizeofDIB = dwLength + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
						bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);
						bmfHeader.bfSize = dwSizeofDIB;
						bmfHeader.bfType = 0x4D42;

						bRet = WriteFile(hFile, (LPSTR)&bmfHeader, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);

						if(bRet){
								bRet = WriteFile(hFile, (LPSTR)&bi, sizeof(BITMAPINFOHEADER), &dwWritten, NULL);
						}

						if(bRet){
								bRet = WriteFile(hFile, (LPCVOID)pData, dwLength, &dwWritten, 0);
						}

						CLOSE_HANDLE_IF(hFile);

						if(bRet && dwWritten != dwLength){
								bRet = FALSE;
								IF_ERROR_RETURN(bRet);
						}

		    return bRet;
				}

				void CloseFile(){ CLOSE_HANDLE_IF(m_hFile); }

				const BOOL IsOpened() const{ return m_hFile != INVALID_HANDLE_VALUE; }

		private:

				HANDLE m_hFile;
};

#endif