#include <iostream>
#include <string>
#include <cstring>
#include <system_error>
#include <stdexcept>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h> // sockaddr_in
#include "logger.hpp"
#include "server.hpp"
#include "connectionHandler.hpp"
#include "requestHandler.hpp"
#include "config.hpp"

Server::Server()
	: watcher(EV_DEFAULT), timer(EV_DEFAULT), timer2(EV_DEFAULT)
{}

Server::~Server()
{}

void Server::init(int domain, uint16_t port, sockaddr* address, socklen_t addrlen)
{
	int opt = 1;

	if ((sock = socket(domain, SOCK_STREAM, IPPROTO_TCP)) == -1)
		throw std::system_error(errno, std::system_category());

	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
		throw std::system_error(errno, std::system_category());

	if (bind(sock, address, addrlen) == -1)
		throw std::system_error(errno, std::system_category());

	if (listen(sock, 1) == -1)
		throw std::system_error(errno, std::system_category());


	watcher.set<Server, &Server::acceptClient>(this);
	watcher.start(sock, ev::READ);

	timer.set<&RequestHandler::clearTorrentPeers>();
	timer.set(Config::getInt("clear_peers_interval"), Config::getInt("clear_peers_interval"));
	timer.start();

	timer2.set<&RequestHandler::flushSqlRecords>();
	timer2.set(Config::getInt("flush_records_interval"), Config::getInt("flush_records_interval"));
	timer2.start();
}

void Server::run()
{
	ev_run(EV_DEFAULT);
}

void Server::acceptClient(ev::io& w, int revents)
{
	int responseSock = accept(sock, nullptr, nullptr);

	if (responseSock != -1) {
		try {
			handle(responseSock);
		}
		catch (const std::exception& e) {
			LOG_ERROR(e.what());
		}
	}
	else {
		LOG_ERROR("accept() failed");
	}
}
