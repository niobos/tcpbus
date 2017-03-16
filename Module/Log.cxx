#include "Log.hxx"

#include <iomanip>
#include <sstream>

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
	m_log_tx(false), m_log_rx(true),
	m_output( nullptr )
{
	m_output.rdbuf( std::cout.rdbuf() );

	while( params.length() ) {
		size_t i = params.find_first_of("=:"); // npos if not found
		std::string key = params.substr(0, i); // npos copies till end of string
		params.erase(0, i); // npos erases entire string
		std::string value;
		if( params[0] == '=' ) {
			i = params.find_first_of(":");
			value = params.substr(1, i); // skip the '='
			params.erase(0, i);
		}
		
		if( key.compare("file") == 0 ) {
			this->reopen_file(value);
		} else {
			std::cerr << "log: Ignored unrecognized option: " << key << "\n";
		}
	}
	// TODO: parse params to activate/deactivate logs
}

Log::~Log() {
}

void Log::sighup() {
	reopen_file(m_filename);
}

void Log::reopen_file(const std::string &filename) {
	m_filename = filename;
	m_outfile.close();
	m_outfile.open(m_filename, std::ofstream::out | std::ofstream::app);
	m_output.rdbuf( m_outfile.rdbuf() );
}

void Log::new_connection(
	int id,
	const SockAddr::SockAddr &addr
) {
	if( ! m_log_nc ) return;
	m_output << timestamp() << " " << addr.string() << " NewCon " << "\n" << std::flush;
}

void Log::disconnect(
	int id,
	const SockAddr::SockAddr &addr
) {
	if( ! m_log_disc ) return;
	m_output << timestamp() << " " << addr.string() << " Close " << "\n" << std::flush;
}

void Log::error(
	int id,
	const SockAddr::SockAddr &addr,
	const Errno &e
) {
	if( ! m_log_error ) return;
	m_output << timestamp() << " " << addr.string() << " Error " << " " << e.error_number() << " " << e.what() << "\n" << std::flush;
}

void Log::rx_data(
	int id,
	const SockAddr::SockAddr &addr,
	std::vector< std::string > &msg
) {
	if( ! m_log_rx ) return;
	std::string now( timestamp() );
	for(auto i = msg.begin(); i != msg.end(); i++) {
		m_output << now << " " << addr.string() << " Rx";
		for(size_t j = 0; j < i->length(); j++) {
			m_output << " " << std::hex << std::setfill('0') << std::setw(2) << (i->at(j)&0xff);
		}
		m_output << "\n" << std::flush;
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
		m_output << now << " " << dst.string() << " Tx";
		for(size_t j = 0; j < i->length(); j++) {
			m_output << " " << std::hex << std::setfill('0') << std::setw(2) << (i->at(j)&0xff);
		}
		m_output << "\n" << std::flush;
	}
}

} // namespace
