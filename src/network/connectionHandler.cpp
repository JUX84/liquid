#include <string>
#include <cstring>
#include <system_error>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "handler/requestHandler.hpp"
#include "misc/config.hpp"
#include "network/connectionHandler.hpp"

ConnectionHandler::ConnectionHandler(int socket)
	: ipv6(false), sock(socket), MAX_REQUEST_SIZE(Config::getInt("max_request_size")),
	BUFFER_SIZE(Config::getInt("read_buffer_size")), sent(0)
{}

void ConnectionHandler::init()
{
	setSocketOptions();
	getPeerInfo();

	request.reserve(MAX_REQUEST_SIZE);

	readWatcher.set<ConnectionHandler, &ConnectionHandler::readRequest>(this);
	readWatcher.start(sock, ev::READ);

	writeWatcher.set<ConnectionHandler, &ConnectionHandler::writeResponse>(this);
	writeWatcher.set(sock, ev::WRITE);

	timeoutWatcher.set<ConnectionHandler, &ConnectionHandler::timeout>(this);
	timeoutWatcher.set(Config::getInt("timeout"), 0);
	timeoutWatcher.start();

	ptr = std::unique_ptr<ConnectionHandler>(this);
}

ConnectionHandler::~ConnectionHandler()
{}

void ConnectionHandler::setSocketOptions() const
{
	int flags = fcntl(sock, F_GETFL);

	if (flags == -1 || fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1)
		throw std::system_error(errno, std::system_category());
}

void ConnectionHandler::readRequest(ev::io& w, int revents)
{
	char buffer[BUFFER_SIZE];
	memset(buffer, 0, BUFFER_SIZE);
	size_t size = recv(sock, buffer, BUFFER_SIZE, 0);

	if (size == 0) {
		destroy();
		return;
	}

	request.append(buffer, size);
	size = request.size();

	if (size > MAX_REQUEST_SIZE || (size >= 4 && request.compare(size - 4, std::string::npos, "\r\n\r\n") == 0)) {
		readWatcher.stop();

		if (size > MAX_REQUEST_SIZE) {
			shutdown(sock, SHUT_RD);
			response = "request too long";
		}
		else {
			response = RequestHandler::handle(request, IP, ipv6);
		}

		writeWatcher.start();
	}
}

void ConnectionHandler::writeResponse(ev::io& w, int revents)
{
	size_t responseSize = response.size();
#ifdef MSG_NOSIGNAL
	int flag = MSG_NOSIGNAL;
#else
	int flag = 0;
#endif

	ssize_t n = send(sock, response.data() + sent, responseSize - sent, flag);

	if (n == -1)
		return;

	sent += n;
	if (sent == responseSize)
		destroy();
}

void ConnectionHandler::timeout(ev::timer& w, int revents)
{
	destroy();
}

void ConnectionHandler::destroy()
{
	readWatcher.stop();
	writeWatcher.stop();
	timeoutWatcher.stop();

	close(sock);

	/* delete this; */
	ptr.reset();
}
