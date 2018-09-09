//----------------------------------------------------------------------------------------------
// Http_Definition.h
//----------------------------------------------------------------------------------------------
#ifndef HTTP_DEFINITION_H
#define HTTP_DEFINITION_H

#define DEFAULT_PORT      "13754"
#define HTTP_BUFFER_SIZE  512

#define BOUNDARY "boundarydonotcross"

#define STD_HEADER "HTTP/1.0 200 OK\r\n" \
		                 "Connection: close\r\n" \
                   "Server: David\r\n" \
                   "Cache-Control: no-store, no-cache, must-revalidate, pre-check=0, post-check=0, max-age=0\r\n" \
                   "Pragma: no-cache\r\n" \
                   "Content-Type: multipart/x-mixed-replace;boundary=" BOUNDARY "\r\n" \
                   "\r\n" \
                   "--" BOUNDARY "\r\n"

#define BOUNDARY_ALL "\r\n--boundarydonotcross\r\n"

#endif