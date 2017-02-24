#pragma once

#include <config.h>

#include <string>
#include <stdexcept>
#include <errno.h>

class Errno : public std::runtime_error {
public:
	Errno(std::string what, int errno_value) throw();
	virtual const char* what() const throw();

	virtual int error_number() const throw() { return m_errno; }

protected:
	int m_errno;

	mutable char m_what[1024];
};
