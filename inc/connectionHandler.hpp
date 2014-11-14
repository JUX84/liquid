#pragma once

#include <string>
#include <memory>
#include <arpa/inet.h>
#include <ev++.h>

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
	const int MAX_REQUEST_SIZE;
	const int BUFFER_SIZE;
	std::string response;
	size_t sent;

	ev::io readWatcher;
	ev::io writeWatcher;
	ev::timer timeoutWatcher;

	std::unique_ptr<ConnectionHandler> ptr;
};
