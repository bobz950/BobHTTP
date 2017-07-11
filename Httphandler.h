#ifndef HTTPHANDLER_H
#define HTTPHANDLER_H

#include "Settings.h"
#include "ResponseHeaders.h"
#include "asio.hpp"
#include <string>
#include <fstream>

using namespace asio::ip;
class Httphandler {
public:
	Httphandler(tcp::socket* s);
	~Httphandler();
	void execute();
private:
	bool parseRequest(char* headers, size_t size);
	void parseParams(char* p, int size, ParamMap* map);
	const char* strGetLine(char** s, size_t& size);
	void parseUrlParams(char* p);

	void send404();
	void send400();

	RequestType rType;
	char* location;
	char mode[16];
	size_t resSize;
	tcp::socket* sock;
	ParamMap* reqHeadParams; //request header parameters - stored dynamically for better alignment
	ParamMap* reqParams; //request post params
	ParamMap* reqQueryParams; //request query parameters
};

#endif