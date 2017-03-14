#include "Log.hxx"

#include <iostream>
#include <iomanip>

namespace Module {

class LogRegistrant {
public:
	LogRegistrant() {
		struct ModuleInfo mi = {
			.factory = &Log::factory_method,
			.usage = &Log::usage,
		};
		ModuleRegistry::get_instance().add("log", mi);
		std::cout << "Registered module `log`\n";
	}
};
extern "C" {
	/* auto-constructed when this compilation unit is loaded */
	LogRegistrant DummyName;
}

void Log::usage(std::ostream &out) {
	out << "file=<filename>\n";
}

Log::Log(std::string params) :
	m_log_nc(true), m_log_disc(true), m_log_error(true),
	m_log_tx(false), m_log_rx(true)
{
	// TODO: parse params to activate/deactivate logs
	// Maybe output to file?
}

Log::~Log() {
}

void Log::new_connection(
	int id,
	const SockAddr::SockAddr &addr
) {
	if( ! m_log_nc ) return;
	std::cout << timestamp() << " " << addr.string() << " NewCon " << "\n" << std::flush;
}

void Log::disconnect(
	int id,
	const SockAddr::SockAddr &addr
) {
	if( ! m_log_disc ) return;
	std::cout << timestamp() << " " << addr.string() << " Close " << "\n" << std::flush;
}

void Log::error(
	int id,
	const SockAddr::SockAddr &addr,
	const Errno &e
) {
	if( ! m_log_error ) return;
	std::cout << timestamp() << " " << addr.string() << " Error " << " " << e.error_number() << " " << e.what() << "\n" << std::flush;
}

void Log::rx_data(
	int id,
	const SockAddr::SockAddr &addr,
	std::vector< std::string > &msg
) {
	if( ! m_log_rx ) return;
	std::string now( timestamp() );
	for(auto i = msg.begin(); i != msg.end(); i++) {
		std::cout << now << " " << addr.string() << " Rx";
		for(size_t j = 0; j < i->length(); j++) {
			std::cout << " " << std::hex << std::setfill('0') << std::setw(2) << (i->at(j)&0xff);
		}
		std::cout << "\n" << std::flush;
	}
}

void Log::tx_data(
	int srcid,
	int dstid,
	const SockAddr::SockAddr &src,
	const SockAddr::SockAddr &dst,
	std::vector< std::string > &msg
) {
	if( ! m_log_tx ) return;
	std::string now( timestamp() );
	for(auto i = msg.begin(); i != msg.end(); i++) {
		std::cout << now << " " << dst.string() << " Tx";
		for(size_t j = 0; j < i->length(); j++) {
			std::cout << " " << std::hex << std::setfill('0') << std::setw(2) << (i->at(j)&0xff);
		}
		std::cout << "\n" << std::flush;
	}
}

} // namespace
