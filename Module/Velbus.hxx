#pragma once

#include "Module.hxx"

#include <string>
#include <map>

namespace Module {

class Velbus: public Module {
protected:
	std::map< int, std::string > m_buf;
	/* max Velbus message = 4B header + [0;8]B body + 2B
	 * but length is 3 bits, so technically up to 15B body
	 */	

public:
	Velbus(std::string params);
	static Module* factory_method(std::string params) { return new Velbus(params); }

	static void usage(std::ostream& out);

	virtual ~Velbus();

	virtual void rx_data(
		int id,
		const SockAddr::SockAddr &addr,
		std::vector< std::string > &msg
	);

	virtual void disconnect(
		int id,
		const SockAddr::SockAddr &addr
	);
};

} // namespace
