#include "Velbus.hxx"

#include <iostream>

namespace Module {

class VelbusRegistrant {
public:
	VelbusRegistrant() {
		struct ModuleInfo mi = {
			.factory = &Velbus::factory_method,
			.usage = &Velbus::usage,
		};
		ModuleRegistry::get_instance().add("velbus", mi);
		std::cout << "Registered module `velbus`\n";
	}
};
extern "C" {
	/* auto-constructed when this compilation unit is loaded */
	VelbusRegistrant DummyName;
}

void Velbus::usage(std::ostream& out) {
}

Velbus::Velbus(std::string params) {
}

Velbus::~Velbus() {
}

void Velbus::rx_data(
	int id,
	const SockAddr::SockAddr &addr,
	std::vector< std::string > &msg
) {
	// Join messages in to 1 string
	for( auto i = msg.begin(); i != msg.end(); i++ ) {
		m_buf[id].append(*i);
	}
	msg.clear();

	std::string now = timestamp();
parse:
	if( m_buf[id].length() < 7 ) return;

	if( m_buf[id][0] != 0x0f ) {
		printf("%s %s Velbus: Malformed header ([0]=0x%02x != 0x0f), skipping 1 byte\n",
		       now.c_str(), addr.string().c_str(), m_buf[id][0]&0xff);
		m_buf[id].erase(0,1);
		goto parse;
	}

	if( (m_buf[id][1] & 0xfc) != 0xf8 ) {
		printf("%s %s Velbus: Malformed header ([1]=0x%02x & 0xfc != 0xf8), skipping 1 byte\n",
		       now.c_str(), addr.string().c_str(), m_buf[id][1]&0xff);
		m_buf[id].erase(0,1);
		goto parse;
	}

	int len = m_buf[id][3] & 0x0f;
	if( m_buf[id].length() < 4 + len + 2 ) return;

	if( m_buf[id][4 + len + 1] != 0x04 ) {
		printf("%s %s Velbus: Malformed ETX ([%d]=0x%02x != 0x04), skipping 1 byte\n",
		       now.c_str(), addr.string().c_str(), 4+len+1, m_buf[id][4 + len + 1]&0xff );
		m_buf[id].erase(0,1);
		goto parse;
	}

	int sum = 0;
	for(int i=0; i < 4 + len; i++ ) {
		sum += m_buf[id][i];
	}
	sum = (-sum)&0xff;
	if( (unsigned char)(m_buf[id][4+len]) != sum ) {
		printf("%s %s Velbus: Malformed checksum ([%d]=0x%02x != 0x%02x), skipping 1 byte\n",
		       now.c_str(), addr.string().c_str(), 4+len, m_buf[id][4+len]&0xff, sum);
		m_buf[id].erase(0,1);
		goto parse;
	}

	msg.push_back( m_buf[id].substr(0, 4+len+2) );
	m_buf[id].erase(0, 4+len+2);
	goto parse;
}

void Velbus::disconnect(
	int id,
	const SockAddr::SockAddr &addr
) {
	if( m_buf[id].length() ) {
		printf("%s %s Velbus: Disconnect with non-empty buffer:",
		       timestamp().c_str(), addr.string().c_str());
		for( int i = 0; i <  m_buf[id].length(); i++ ) {
			printf(" %02x", m_buf[id][i]&0xff);
		}
		printf("\n");
	}
	m_buf.erase(id);
}

} // namespace
