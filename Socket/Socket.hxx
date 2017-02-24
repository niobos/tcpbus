#pragma once

#include <config.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>

#include "Errno.hxx"
#include "SockAddr.hxx"

class Socket {
private:
	int m_socket;

public:
	/**
	 * Take responsibility of a socket
	 * Will close when this object is destroyed
	 */
	Socket(int const socket = -1) throw() : m_socket(socket) {}
	Socket(Socket &&s) throw() : m_socket( s.release() ) {}
	Socket(const Socket &s) =delete;

	~Socket() throw() { this->reset(); }

	/**
	 * Release responsibility of the socket
	 * The socket FD is returned.
	 */
	int release() throw() { int tmp = m_socket; m_socket = -1; return tmp; }

	/**
	 * Replace the socket with another socket
	 * The old socket (if any) is closed first
	 */
	void reset(int const socket = -1) throw() { if( m_socket != -1 ) close(m_socket); m_socket = socket; }

	/**
	 * Since we take responsability to close a socket, we can't be copied, only moved
	 */
	Socket & operator =(Socket &&rhs) throw() { this->reset( rhs.release() ); return *this; }
	Socket & operator =(const Socket &rhs) =delete;

	/*
	 * Factory methods
	 */
	static Socket socket(int const domain, int const type, int const protocol) throw(Errno);
	static Socket accept(int const socket, struct sockaddr *address, socklen_t *address_len) throw(Errno);

	operator int() throw() { return m_socket; }

	void bind(struct sockaddr const *address, socklen_t const address_len) throw(Errno);
	void bind(SockAddr::SockAddr const &addr) throw(Errno) {
		return this->bind(static_cast<struct sockaddr const*>(addr), addr.addr_len()); }

	void connect(struct sockaddr const *address, socklen_t const address_len) throw(Errno);
	void connect(SockAddr::SockAddr const &addr) throw(Errno) {
		return this->connect(static_cast<struct sockaddr const*>(addr), addr.addr_len()); }

	void listen(int const backlog) throw(Errno);

	/**
	 * Accept a new connection on this socket (must be in listening mode)
	 * The new socket FD is returned
	 * if client_address is not NULL, the target unique_ptr is reassigned to
	 * own the SockAddr of the client.
	 */
	Socket accept(std::unique_ptr<SockAddr::SockAddr> *client_address) throw(Errno);

	std::string recv(size_t const max_length = 4096) throw(Errno);
	ssize_t send(char const *data, size_t len) throw(Errno);
	void send(std::string const &data) throw(Errno,std::runtime_error);

	void shutdown(int how) throw(Errno);

	std::unique_ptr<SockAddr::SockAddr> getsockname() const throw(Errno);
	std::unique_ptr<SockAddr::SockAddr> getpeername() const throw(Errno);

	/**
	 * {set,get}sockopt calls
	 */
	void setsockopt(int const level, int const optname, const void *optval, socklen_t optlen) throw(Errno);
	void getsockopt(int const level, int const optname, void *optval, socklen_t *optlen) throw(Errno);

	void set_reuseaddr(bool state = true) throw(Errno) {
		int optval = state;
		return this->setsockopt(SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	}

	int getsockopt_so_error() throw(Errno) {
		int error;
		socklen_t error_len = sizeof(error);
		this->getsockopt(SOL_SOCKET, SO_ERROR, &error, &error_len);
		return error;
	}


	/**
	 * Get or set the non-blocking state of the socket
	 * Both functions return the non-blocking state at the moment of the call
	 * (i.e. before it is changed)
	 */
	bool non_blocking() throw(Errno);
	bool non_blocking(bool new_state) throw(Errno);
};
