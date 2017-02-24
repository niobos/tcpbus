#pragma once

#include "Module.hxx"

namespace Module {

class Velbus: public Module {
public:
	Velbus(std::string params);
	static Module* factory_method(std::string params) { return new Velbus(params); }

	virtual ~Velbus();

	virtual void rx_data(
		const SockAddr::SockAddr &addr,
		std::vector< std::string > &msg
	);
};

} // namespace
