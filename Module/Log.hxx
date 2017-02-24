#pragma once

#include "Module.hxx"

namespace Module {

class Log: public Module {
protected:
	bool m_log_nc, m_log_disc, m_log_error, m_log_tx, m_log_rx;

public:
	Log(std::string params);
	static Module* factory_method(std::string params) { return new Log(params); }

	virtual ~Log();

	virtual void new_connection(
		const SockAddr::SockAddr &addr
	);

	virtual void disconnect(
		const SockAddr::SockAddr &addr
	);

	virtual void error(
		const SockAddr::SockAddr &addr,
		const Errno &e
	);

	virtual void rx_data(
		const SockAddr::SockAddr &addr,
		std::vector< std::string > &msg
	);

	virtual void tx_data(
		const SockAddr::SockAddr &src,
		const SockAddr::SockAddr &dst,
		std::vector< std::string > &msg
	);
};

} // namespace
