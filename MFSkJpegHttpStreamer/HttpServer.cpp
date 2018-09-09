//----------------------------------------------------------------------------------------------
// HttpServer.cpp
// Copyright (C) 2013 Dumonteil David
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
#include "StdAfx.h"

HRESULT CHttpServer::Initialize(){

	WSADATA WsaData;
	HRESULT hr;

	assert(m_bWsaIsInit == FALSE);

	int iResult = WSAStartup(MAKEWORD(2, 2), &WsaData);

	IF_FAILED_RETURN(hr = (iResult != 0 ? E_FAIL : S_OK));

	m_bWsaIsInit = TRUE;

	struct addrinfo* InfoResult = NULL;
	struct addrinfo InfoHints;

	ZeroMemory(&InfoHints, sizeof(InfoHints));
	InfoHints.ai_family = AF_INET;
	InfoHints.ai_socktype = SOCK_STREAM;
	InfoHints.ai_protocol = IPPROTO_TCP;
	InfoHints.ai_flags = AI_PASSIVE;

	iResult = getaddrinfo(NULL, DEFAULT_PORT, &InfoHints, &InfoResult);

	IF_FAILED_RETURN(hr = (iResult != 0 ? E_FAIL : S_OK));

	HANDLE hListenThread = NULL;

	try{

		m_ListenSocket = socket(InfoResult->ai_family, InfoResult->ai_socktype, InfoResult->ai_protocol);
		IF_FAILED_THROW(hr = (m_ListenSocket == INVALID_SOCKET ? E_FAIL : S_OK));

		iResult = bind(m_ListenSocket, InfoResult->ai_addr, static_cast<int>(InfoResult->ai_addrlen));
		IF_FAILED_THROW(hr = (iResult == SOCKET_ERROR ? E_FAIL : S_OK));

		m_NetState = NETWORK_LISTEN;

		hListenThread = CreateThread(NULL, 0, ListenSocketThread, this, 0, NULL);
		IF_FAILED_THROW(hr = (hListenThread == NULL ? E_FAIL : S_OK));

		IF_FAILED_THROW(hr = (CloseHandle(hListenThread) == 0 ? E_FAIL : S_OK));
	}
	catch(HRESULT){}

	if(InfoResult)
		freeaddrinfo(InfoResult);

	return hr;
}

void CHttpServer::Release(){

	if(m_NetState != NETWORK_CLOSE && m_ClientSocket != INVALID_SOCKET)
		shutdown(m_ClientSocket, SD_SEND);

	CLOSE_SOCKET_IF(m_ClientSocket);
	CLOSE_SOCKET_IF(m_ListenSocket);

	if(m_bWsaIsInit)
		WSACleanup();

	m_NetState = NETWORK_CLOSE;
	m_bWsaIsInit = FALSE;
}

HRESULT CHttpServer::SendSample(IMFSample* pSample){

	HRESULT hr = S_OK;

	IF_FAILED_RETURN(hr = (m_NetState == NETWORK_CLOSE) ? E_UNEXPECTED : S_OK);
	IF_FAILED_RETURN(hr = (m_NetState == NETWORK_ERROR) ? E_FAIL : S_OK);

	if(m_NetState == NETWORK_LISTEN)
		return hr;

	IMFMediaBuffer* pInput = NULL;
	BYTE* pInputData = NULL;
	DWORD dwLenght = 0;

	int iWsaResult = 1;
	char Buffer[HTTP_BUFFER_SIZE];

	try{

		IF_FAILED_THROW(hr = pSample->GetBufferByIndex(0, &pInput));
		//IF_FAILED_RETURN(hr = pInput->GetMaxLength(&dwLenght));

		// Todo : check dwLenght and max dwLenght
		IF_FAILED_THROW(hr = pInput->Lock(&pInputData, NULL, &dwLenght));

		// Todo : check/log sprintf_s return value
		sprintf_s(Buffer, HTTP_BUFFER_SIZE, "Content-Type: image/jpeg\r\n" \
			"Content-Length: %d\r\n" \
			"\r\n", dwLenght);

		// Todo : check/log strnlen return value
		iWsaResult = send(m_ClientSocket, Buffer, strnlen(Buffer, HTTP_BUFFER_SIZE), 0);
		IF_FAILED_THROW(hr = (iWsaResult <= 0 ? E_FAIL : S_OK));

		iWsaResult = send(m_ClientSocket, reinterpret_cast<const char*>(pInputData), dwLenght, 0);
		IF_FAILED_THROW(hr = (iWsaResult <= 0 ? E_FAIL : S_OK));

		iWsaResult = send(m_ClientSocket, BOUNDARY_ALL, strlen(BOUNDARY_ALL), 0);
		IF_FAILED_THROW(hr = (iWsaResult <= 0 ? E_FAIL : S_OK));

		LOG_HRESULT(hr = pInput->Unlock());
		pInputData = NULL;
	}
	catch(HRESULT){}

	if(pInput && pInputData){
		LOG_HRESULT(pInput->Unlock());
	}

	SAFE_RELEASE(pInput);

	if(iWsaResult <= 0){

		// Todo : check/log shutdown return value
		shutdown(m_ClientSocket, SD_SEND);
		m_NetState = NETWORK_ERROR;
	}

	return hr;
}

DWORD WINAPI CHttpServer::ListenSocketThread(LPVOID lpParam){

	CHttpServer* pServer = reinterpret_cast<CHttpServer*>(lpParam);

	HRESULT hr = S_OK;
	int iResult;
	char Buffer[HTTP_BUFFER_SIZE];

	try{

		iResult = listen(pServer->m_ListenSocket, SOMAXCONN);
		IF_FAILED_THROW(hr = (iResult == SOCKET_ERROR ? E_FAIL : S_OK));

		pServer->m_ClientSocket = accept(pServer->m_ListenSocket, NULL, NULL);
		IF_FAILED_THROW(hr = (pServer->m_ClientSocket == INVALID_SOCKET ? E_FAIL : S_OK));

		CLOSE_SOCKET_IF(pServer->m_ListenSocket);

		iResult = recv(pServer->m_ClientSocket, Buffer, HTTP_BUFFER_SIZE, 0);
		IF_FAILED_THROW(hr = (iResult <= 0 ? E_FAIL : S_OK));

		iResult = send(pServer->m_ClientSocket, STD_HEADER, strlen(STD_HEADER), 0);
		IF_FAILED_THROW(hr = (iResult == SOCKET_ERROR ? E_FAIL : S_OK));
	}
	catch(HRESULT){}

	if(SUCCEEDED(hr))
		pServer->ListenCallBack(NETWORK_READY);
	else
		pServer->ListenCallBack(NETWORK_ERROR);

	return 0;
}