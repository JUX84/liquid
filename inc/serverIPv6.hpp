#pragma once

#include "server.hpp"

class ServerIPv6 : public Server
{
	public:
		ServerIPv6(uint16_t port, bool ipv6Only = false);
		virtual ~ServerIPv6();

	protected:
		virtual void handle(int responseSock) const override;
		virtual void setSocketOptions() const override;

	private:
		const bool IPv6Only;
};
