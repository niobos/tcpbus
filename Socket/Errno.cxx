#include "Errno.hxx"

#include <sstream>
#include <string.h>

Errno::Errno(std::string what, int errno_value) throw() :
	std::runtime_error(what),
	m_errno(errno_value)
{
}

const char* Errno::what() const throw() {
	std::ostringstream e;
	e << std::runtime_error::what()
	  << ": " << strerror(m_errno);

	strncpy(m_what, e.str().c_str(), sizeof(m_what)-1);
	m_what[sizeof(m_what)-1] = '\0';

	return m_what;
}
