#include "Log.hxx"

#include <iostream>
#include <sys/time.h>
#include <sstream>
#include <iomanip>

namespace Module {

class LogRegistrant {
public:
	LogRegistrant() {
		ModuleRegistry::get_instance().add("log", &Log::factory_method);
		std::cout << "Registered module `log`\n";
	}
};
extern "C" {
	/* auto-constructed when this compilation unit is loaded */
	LogRegistrant DummyName;
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

std::string timestamp() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	std::ostringstream o;
	o << tv.tv_sec << "."
	  << std::setfill('0') << std::setw(6) << tv.tv_usec;
	return o.str();
}

void Log::new_connection(const SockAddr::SockAddr &addr) {
	if( ! m_log_nc ) return;
	std::cout << timestamp() << " " << addr.string() << " NewCon " << "\n";
}

void Log::disconnect(const SockAddr::SockAddr &addr) {
	if( ! m_log_disc ) return;
	std::cout << timestamp() << " " << addr.string() << " Close " << "\n";
}

void Log::error(const SockAddr::SockAddr &addr, const Errno &e) {
	if( ! m_log_error ) return;
	std::cout << timestamp() << " " << addr.string() << " Error " << " " << e.error_number() << " " << e.what() << "\n";
}

void Log::rx_data(const SockAddr::SockAddr &addr, std::vector< std::string > &msg) {
	if( ! m_log_rx ) return;
	std::string now( timestamp() );
	for(auto i = msg.begin(); i != msg.end(); i++) {
		std::cout << now << " " << addr.string() << " Rx";
		for(size_t j = 0; j < i->length(); j++) {
			std::cout << " " << std::hex << std::setfill('0') << std::setw(2) << (int)i->at(j);
		}
		std::cout << "\n";
	}
}

void Log::tx_data(const SockAddr::SockAddr &src, const SockAddr::SockAddr &dst, std::vector< std::string > &msg) {
	if( ! m_log_tx ) return;
	std::string now( timestamp() );
	for(auto i = msg.begin(); i != msg.end(); i++) {
		std::cout << now << " " << dst.string() << " Tx";
		for(size_t j = 0; j < i->length(); j++) {
			std::cout << " " << std::hex << std::setfill('0') << std::setw(2) << (int)i->at(j);
		}
		std::cout << "\n";
	}
}

} // namespace