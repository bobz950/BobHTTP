#include "Settings.h"

#include <cstdio>

#include <thread>
#include "asio.hpp"
#include "Httphandler.h"

using namespace asio::ip;
class BobHttp {
public:
	BobHttp();
	static void runThread(tcp::socket* h);
	volatile void go();
private:
	asio::io_service* serverSock;
	tcp::acceptor acc;
};

BobHttp::BobHttp() : serverSock(new asio::io_service()), acc(*serverSock, tcp::endpoint(tcp::v4(), 6001)) {}

void BobHttp::runThread(tcp::socket* s) {
	Httphandler* handler = new Httphandler(s);
	handler->execute();
	delete handler;

	s->shutdown(tcp::socket::shutdown_both);
	s->close();
	delete s;
}

volatile void BobHttp::go() {

	while (true) {
		tcp::socket* clientSock = new tcp::socket(*serverSock); //create a client socket.
		asio::error_code err;
		tcp::endpoint ep;
		acc.accept(*clientSock, ep, err); //wait for incomming connection. Accept into clientSock 

		std::thread(&BobHttp::runThread, clientSock).detach(); //execute handler in new thread
	}
}

int main()
{
	BobHttp serv;
	serv.go();
    return 0;
}

