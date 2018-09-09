//----------------------------------------------------------------------------------------------
// HttpServer.h
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
#ifndef HTTPSERVER_H
#define HTTPSERVER_H 

class CHttpServer{

		public:

				CHttpServer() : m_ListenSocket(INVALID_SOCKET), m_ClientSocket(INVALID_SOCKET), m_bWsaIsInit(FALSE), m_NetState(NETWORK_CLOSE){}
				~CHttpServer(){ Release(); }

				HRESULT Initialize();
				void Release();
				HRESULT SendSample(IMFSample*);

		private:

				SOCKET m_ListenSocket;
				SOCKET m_ClientSocket;
				BOOL m_bWsaIsInit;

				enum NETWORK_STATE{ NETWORK_CLOSE, NETWORK_LISTEN, NETWORK_READY, NETWORK_ERROR };

				NETWORK_STATE m_NetState;

				static DWORD WINAPI ListenSocketThread(LPVOID);
				void ListenCallBack(NETWORK_STATE State){ m_NetState = State; }
};

#endif