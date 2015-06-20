#pragma once

#include <string>
#include <memory>
#include <arpa/inet.h>
#include <ev++.h>

class ConnectionHandler
{
public:
	ConnectionHandler(int socket);
	virtual ~ConnectionHandler();

	void readRequest(ev::io& w, int revents);
	void writeResponse(ev::io& w, int revents);
	void timeout(ev::timer& w, int revents);

protected:
	virtual void getPeerInfo() = 0;
	void init();
	std::string binIp;
	int sock;

private:
	void setSocketOptions() const;
	void destroy();

	std::string request;
	const unsigned int MAX_REQUEST_SIZE;
	const int BUFFER_SIZE;
	std::string response;
	size_t sent;

	ev::io readWatcher;
	ev::io writeWatcher;
	ev::timer timeoutWatcher;

	std::unique_ptr<ConnectionHandler> ptr;
};
