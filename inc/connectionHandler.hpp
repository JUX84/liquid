#ifndef CONNECTIONHANDLER_HPP
#define CONNECTIONHANDLER_HPP

#include <string>
#include <memory>
#include <arpa/inet.h>
#include <ev++.h>

#define BUFFER_SIZE 512
#define MAX_REQUEST_SIZE 4096
#define TIMEOUT 10

class ConnectionHandler
{
public:
	ConnectionHandler(int socket);

	void readRequest(ev::io& w, int revents);
	void writeResponse(ev::io& w, int revents);
	void timeout(ev::timer& w, int revents);

private:
	void setSocketOptions() const;
	void getPeerInfo();
	void destroy();
	std::string getClientIp() const;

	int sock;
	sockaddr_in client;
	std::string request;
	std::string response;
	size_t sent;

	ev::io readWatcher;
	ev::io writeWatcher;
	ev::timer timeoutWatcher;

	std::unique_ptr<ConnectionHandler> ptr;
};

#endif
