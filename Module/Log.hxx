#pragma once

#include "Module.hxx"
#include <iostream>
#include <fstream>

namespace Module {

class Log: public Module {
protected:
	bool m_log_nc, m_log_disc, m_log_error, m_log_tx, m_log_rx;
	std::ostream m_output;

	std::string m_filename;
	std::ofstream m_outfile;

public:
	Log(std::string params);
	static Module* factory_method(std::string params) { return new Log(params); }

	static void usage(std::ostream &out);

	virtual ~Log();

	virtual void sighup();
	void reopen_file(const std::string &filename);

	virtual void new_connection(
		int id,
		const SockAddr::SockAddr &addr
	);

	virtual void disconnect(
		int id,
		const SockAddr::SockAddr &addr
	);

	virtual void error(
		int id,
		const SockAddr::SockAddr &addr,
		const Errno &e
	);

	virtual void rx_data(
		int id,
		const SockAddr::SockAddr &addr,
		std::vector< std::string > &msg
	);

	virtual void tx_data(
		int srcid,
		int dstid,
		const SockAddr::SockAddr &src,
		const SockAddr::SockAddr &dst,
		std::vector< std::string > &msg
	);
};

} // namespace
