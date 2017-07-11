#include "Httphandler.h"

Httphandler::Httphandler(tcp::socket* s) : sock(s), reqHeadParams(0), location(0), reqQueryParams(0), reqParams(0) {}

Httphandler::~Httphandler() {
	if (this->reqHeadParams) {
		delete this->reqHeadParams;
		this->reqHeadParams = 0;
	}
	if (this->reqQueryParams) {
		delete this->reqQueryParams;
		this->reqQueryParams = 0;
	}
	if (this->reqParams) {
		delete this->reqQueryParams;
		this->reqQueryParams = 0;
	}
	if (this->location) {
		delete this->location;
		this->location = 0;
	}
}
//node: this increments the calling pointer. had to make this since strtok sucks
const char* Httphandler::strGetLine(char** s, size_t& size) {
	if (size < 2) return nullptr;
	size_t count = strcspn(*s, "\n"); // find newline char \n
	char* line = nullptr;
	if (count > 1) {
		line = new char[count];
		memcpy(line, *s, count);
		line[count - 1] = '\0'; // replace the \r with null terminator
	}
	*s += count + 1; //increment pointer to next line
	size -= count + 1; // decremement remaining size
	return line;
}
//parses the request headers into a map
bool Httphandler::parseRequest(char* headers, size_t size) {
	const char* var = this->strGetLine(&headers, size);
	if (var[0] == 'G') this->rType = RequestType::GET;
	else if (var[0] == 'P') this->rType = RequestType::POST;
	else {
		this->rType = RequestType::INVALID;
		return false;
	}
	int i;
	if (this->rType == RequestType::GET) i = 4;
	else i = 5;
	//get location string
	const size_t varlen = strlen(var) - i;
	char* p = (char*)var + i;
	int k;
	for (k = 0; k < varlen; k++)
		if (p[k] == ' ') break;
	//copy the location
	this->location = new char[k+1];
	memcpy(this->location, var+i, k);
	this->location[k] = '\0';
	//copy the mode (http1.2...)
	memcpy(this->mode, var + i + k, varlen - (i + k));
	this->mode[varlen - (i + k)] = '\0';

	delete[] var; //delete first line 
	var = 0;
	this->reqHeadParams = new ParamMap();

	while ((var = this->strGetLine(&headers, size)) != 0) {
		size_t vlen = strlen(var);
		size_t j;
		for (j = 0; j < vlen; j++)
			if (var[j] == ':') break;
		//get key from string
		char* key = new char[j+1];
		memcpy(key, var, j);
		key[j] = '\0';
		//j += 2;
		//get value from string
		char* value = new char[vlen - j + 1];
		memcpy(value, var + j, vlen - j);
		value[vlen - j] = '\0';

		std::string skey = key;
		std::string sval = value;
		this->reqHeadParams->insert(std::pair<std::string, std::string>(skey, sval));

		delete[] key;
		delete[] value;
		delete[] var;
	}
	if (this->rType == RequestType::POST) {
		this->reqParams = new ParamMap();
		this->parseParams(headers, atoi((*this->reqHeadParams)["Content-Length"].c_str() + 2), this->reqParams);
	}
	
	return true;
}

void Httphandler::parseParams(char* p, int size, ParamMap* map) {
	int c = strcspn(p, "=");
	char* key = new char[c+1];
	memcpy(key, p, c);
	key[c] = '\0';
	c+=1;
	int j = c;
	while (j < size) {
		if (p[j] == '&') break;
		j++;
	}
	int valsize = j - c;
	char* value = new char[valsize + 1];
	memcpy(value, p + c, valsize);
	value[valsize] = '\0';
	map->insert(std::pair<std::string, std::string>(key, value));
	delete[] key;
	delete[] value;
	if (j < size)
		this->parseParams(p + j + 1, size - j, map);
}

void Httphandler::parseUrlParams(char* p) {
	int size = strlen(p);
	int count = strcspn(p, "?");
	if (size == count) return;
	if (!this->reqQueryParams) 
		this->reqQueryParams = new ParamMap();

	this->parseParams(p + count + 1, size - count - 1, this->reqQueryParams);
	this->location[count] = '\0';
}

void Httphandler::execute() {
		char read[MAXBUF]; //read buffer
		std::string loc = HOMEDIR;
		try {
			int readl = sock->read_some(asio::buffer(read)); //read the request 

			for (int i = 0; i < (readl); i++)
				printf("%c", read[i]);

			if (!this->parseRequest(read, readl)) {
				//send 400 bad request
				this->send400();
				return;
			}

			//send back appropriate response
			this->parseUrlParams(this->location);
			if (strlen(this->location) == 1)
				loc += "index.html";
			else {
				loc += this->location;
			}
			
			const char* acceptField = (*this->reqHeadParams)["Accept"].c_str();
			if (strncmp(acceptField, ": text", 6) == 0 || strncmp(acceptField, ": image", 7) == 0) {
				//using c style file reading because its better
				FILE* f = fopen(loc.c_str(), "rb");
				if (!f) { //if file was not found send back 404 response
					this->send404();
					return;
				}
				char buffer[4096];
				int bytes_read;
				char msg[] = RES_OK;
				this->sock->write_some(asio::buffer(msg, strlen(msg))); //write back OK headers
				//read file and send back
				while ((bytes_read = fread(buffer, 1, 4096, f)) > 0)
					this->sock->write_some(asio::buffer(buffer, bytes_read));
				fclose(f);
			}
			//testing query params
			if (this->reqQueryParams) {
				
				ParamMap::iterator it;
				for (it = this->reqQueryParams->begin(); it != this->reqQueryParams->end(); it++) {
					this->sock->write_some(asio::buffer(it->first, it->first.length()));

					char co[] = ": ";
					this->sock->write_some(asio::buffer(co, 2));

					this->sock->write_some(asio::buffer(it->second, it->second.length()));

					char n[] = "<br>";
					this->sock->write_some(asio::buffer(n, strlen(n)));
				}

			}
			ParamMap::iterator it;
			if (this->reqParams) { //testing post params
				for (it = this->reqParams->begin(); it != this->reqParams->end(); it++) {
					this->sock->write_some(asio::buffer(it->first, it->first.length()));

					char co[] = ": ";
					this->sock->write_some(asio::buffer(co, 2));

					this->sock->write_some(asio::buffer(it->second, it->second.length()));

					char n[] = "<br>";
					this->sock->write_some(asio::buffer(n, strlen(n)));
				}
			}
			
		}
		catch (std::exception& ex) {
			printf("client disconnected %s\n", ex.what());
		}
		
}

void Httphandler::send404() {
	char msg[] = RES_NOT_FOUND;
	this->sock->write_some(asio::buffer(msg, strlen(msg))); //write back 404 headers
	char nf[] = "<b>Message from BobServer: Request not found!</b>";
	this->sock->write_some(asio::buffer(nf, strlen(nf)));
}

void Httphandler::send400() {
	char msg[] = RES_BAD_REQUEST;
	this->sock->write_some(asio::buffer(msg, strlen(msg))); //write back 400 headers
}
