#include "Velbus.hxx"

#include <iostream>

namespace Module {

class VelbusRegistrant {
public:
	VelbusRegistrant() {
		ModuleRegistry::get_instance().add("velbus", &Velbus::factory_method);
		std::cout << "Registered module `velbus`\n";
	}
};
extern "C" {
	/* auto-constructed when this compilation unit is loaded */
	VelbusRegistrant DummyName;
}

Velbus::Velbus(std::string params) {
}

Velbus::~Velbus() {
}

void Velbus::rx_data(const SockAddr::SockAddr &addr, std::vector< std::string > &msg) {
	// Join messages in to 1 string
	std::string d;
	for( auto i = msg.begin(); i != msg.end(); i++ ) {
		d.append(*i);
	}

	// find Velbus messages
	// POSSIBLE BUG: we assume we receive full messages
}

} // namespace
