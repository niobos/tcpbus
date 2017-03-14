#pragma once

#include <config.h>

#include "../Socket/SockAddr.hxx"
#include <vector>
#include <string>
#include "Registry.t"

namespace Module {

/* interface */ class Module {
protected:
	Module() {}

public:
	virtual ~Module() {}

	static std::string timestamp();

	virtual void new_connection(
		int id,
		const SockAddr::SockAddr &addr
	) {}
	/* A new client has connected to the bus.
	 * id is a unique id for this connection (valid for the duration of the connection)
	 * Be sure to watch the disconnect() event to clean up.
	 */

	virtual void disconnect(
		int id,
		const SockAddr::SockAddr &addr
	) {}
	/* A client has disconnected from the bus */

	virtual void error(
		int id,
		const SockAddr::SockAddr &addr,
		const Errno &e
	) {}
	/* An error has occured while interacting with the given addr
	 */

	virtual void rx_data(
		int id,
		const SockAddr::SockAddr &addr,
		std::vector< std::string > &msg
	) {}
	/* New data was received from a client.
	 * msg contains a (list of) messages.
	 * This function can alter the received data. Upon return, the msg-list
	 * will be sent as individual messages (which does not do anything special
 	 * with TCP, but does for SCTP)
	 */

	virtual void tx_data(
		int srcid,
		const SockAddr::SockAddr &src,
		int dstid,
		const SockAddr::SockAddr &dst,
		std::vector< std::string > &msg
	) {}
	/* New data (originally from src) is about to be sent to dst.
	 * This function can alter the data to be transmitted (to this destination).
	 * Upon return, the msg-list will be send as individual send() calls.
	 */
};

struct ModuleInfo {
	Module* (*factory)(std::string);
	void (*usage)(std::ostream&);
};

typedef ::Registry< std::string, struct ModuleInfo > ModuleRegistry;

}

/* Don't instanciate these (by default), this would create multiple singletons */
extern template class ::Registry< std::string, struct ModuleInfo >;
