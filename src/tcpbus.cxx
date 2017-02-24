#include <config.h>

#include "../Module/Module.hxx"
#include "../Socket/Socket.hxx"

#include <stdio.h>
#include <sysexits.h>
#include <getopt.h>
#include <signal.h>
#include <ev.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <dirent.h>

#include <vector>
#include <list>
#include <memory>

static const int MAX_CONN_BACKLOG = 32;

void received_sigint(EV_P_ ev_signal *w, int revents) {
	fprintf(stderr, "Received SIGINT, exiting\n");
	ev_break(EV_A_ EVUNLOOP_ALL);
}
void received_sigterm(EV_P_ ev_signal *w, int revents) {
	fprintf(stderr, "Received SIGTERM, exiting\n");
	ev_break(EV_A_ EVUNLOOP_ALL);
}

Socket s_listen;

class Connection {
public:
	Socket socket;
	std::unique_ptr<SockAddr::SockAddr> addr;
	ev_io read_ready;

	Connection() {};
	~Connection() {
		ev_io_stop(EV_DEFAULT_ &read_ready);
	}

	/* ev_io structure should not have its address changed: prevent moving/copying: */
	Connection(const Connection &copy) =delete;
	Connection(Connection &&move) =delete;
	Connection& operator =(const Connection &copy) =delete;
	Connection& operator =(Connection &&move) =delete;
};
std::list< std::unique_ptr<Connection> > connections;

std::vector< std::unique_ptr<Module::Module> > modules;

void kill_connection(Connection *con) {
	for(auto c = connections.begin(); c != connections.end(); c++ ) {
		if( c->get() == con ) {
			for(auto m = modules.begin(); m != modules.end(); m++) {
				(**m).disconnect( *con->addr );
			}
			connections.erase(c);
			// the destructor of the Connection will close the socket
			return;
		}
	}
}

void ready_to_read(EV_P_ ev_io *w, int revents) {
	Connection *con = static_cast<Connection*>( w->data );

	try {
		std::vector< std::string > data;
		data.push_back( con->socket.recv(4096) );

		if( data[0].length() == 0 ) { // EOF
			kill_connection(con);
			return;
		}

		for(auto i = modules.begin(); i != modules.end(); i++) {
			(**i).rx_data( *con->addr, data );
		}

		for(auto c = connections.begin(); c != connections.end(); c++ ) {
			if( c->get() == con ) {
				// Don't loopback to the input socket
				continue;
			} 

			std::vector< std::string > txdata( data ); // copy

			for(auto m = modules.begin(); m != modules.end(); m++) {
				(**m).tx_data( *con->addr, *(**c).addr, txdata );
			}

			for(auto s = txdata.begin(); s != txdata.end(); s++ ) {
				(**c).socket.send(*s);
			}
		}

	} catch(Errno e) {
		for(auto i = modules.begin(); i != modules.end(); i++) {
			(**i).error( *con->addr, e );
		}
		kill_connection(con);
		return;
	}

}

void incomming_connection(EV_P_ ev_io *w, int revents) {
	std::unique_ptr<Connection> c( new Connection );

	c->socket = s_listen.accept(&c->addr);

	int flags = fcntl(c->socket, F_GETFL);
	if( flags == -1 ) {
		return;
	}
	int rv = fcntl(c->socket, F_SETFL, flags | O_NONBLOCK);
	if( rv == -1 ) {
		return;
	}

	for(auto i = modules.begin(); i != modules.end(); i++) {
		(**i).new_connection( *c->addr );
	}

	ev_io_init( &c->read_ready, ready_to_read, c->socket, EV_READ);
	c->read_ready.data = c.get();
	ev_io_start(EV_A_ &c->read_ready);

	connections.push_back( std::move(c) );
}

int ends_with(const char* haystack, const char* suffix) {
	size_t nh = strlen(haystack);
	size_t ns = strlen(suffix);
	if( nh < nh ) return 0;
	return ! strcmp( haystack + nh - ns, suffix);
}

int main(int argc, char* argv[]) {
	fprintf(stderr, "%s version %s (%s) starting up\n", PACKAGE_NAME, PACKAGE_VERSION, PACKAGE_GITREVISION);

	// Default options
	struct {
		std::string bind_addr_listen;
	} options = {
#ifdef ENABLE_IPV6
		/* bind_addr_listen = */ "[::]:[0]",
#else
		/* bind_addr_listen = */ "[127.0.0.1]:[0]",
#endif
		};

	{ // Parse options
		char optstring[] = "hVfp:b:B:l:m:M:H:";
		struct option longopts[] = {
			{"help",       no_argument,       NULL, 'h'},
			{"version",    no_argument,       NULL, 'V'},
			{"bind",       required_argument, NULL, 'b'},
			{"module",     required_argument, NULL, 'm'},
			{"moduledir",  required_argument, NULL, 'M'},
			{"hook",       required_argument, NULL, 'H'},
			{NULL, 0, 0, 0}
		};
		int longindex;
		int opt;
		while( (opt = getopt_long(argc, argv, optstring, longopts, &longindex)) != -1 ) {
			switch(opt) {
			case 'h':
			case '?':
				fprintf(stderr,
				//	>---------------------- Standard terminal width ---------------------------------<
					"Options:\n"
					"  -h, --help                      Displays this help message and exits\n"
					"  -V, --version                   Displays the version and exits\n"
					"  -b, --bind <host:port>          Bind to the specified address for incomming\n"
					"                                  connections.\n"
					"                                  host and port resolving can be bypassed by\n"
					"                                  placing [] around them\n"
					"  -m, --module <file>             Load the given module\n"
					"  -M, --moduledir <dir>           Load all .so files in the given directory\n"
					"\n"
					"  -H, --hook <module=config>\n" 
					"                                 Configure the given module and hook them in\n"
					"\n"
					"Current available modules:\n"
				);
				{
					auto l = Module::ModuleRegistry::get_instance().get_list();
					for( auto i = l.begin(); i != l.end(); i++ ) {
						fprintf(stderr, "  %s\n", i->c_str());
					}
				}

				if( opt == '?' ) exit(EX_USAGE);
				exit(EX_OK);
			case 'V':
				printf("%1$s version %2$s\n"
				       " Options:\n"
				       "   IPv6: %3$s\n"
				       "\n",
					 PACKAGE_NAME, PACKAGE_VERSION " (" PACKAGE_GITREVISION ")",
#ifdef ENABLE_IPV6
				         "yes"
#else
				         "no"
#endif
				         );
				exit(EX_OK);
			case 'b':
				options.bind_addr_listen = optarg;
				break;
			case 'm': {
				void* mod = dlopen(optarg, RTLD_LAZY);
				if( mod == NULL ) {
					fprintf(stderr, "Could not load module `%s`: %s\n", optarg, dlerror());
					exit(EX_NOINPUT);
				}
				} break;
			case 'M': {
					DIR* d = opendir(optarg);
					if( d == NULL ) {
						fprintf(stderr, "Could not open directory `%s`: %s\n", optarg, strerror(errno));
						exit(EX_NOINPUT);
					}
					while(struct dirent* e = readdir(d)) {
						if(e->d_name[0] == '.') continue; // hidden
						if(e->d_type != DT_REG) continue;
						if( ! ends_with(e->d_name, ".so") ) continue;
						std::string path(optarg);
						path.append("/");
						path.append(e->d_name);
						void* mod = dlopen(path.c_str(), RTLD_LAZY);
						if( mod == NULL ) {
							fprintf(stderr, "Warning: ignoring `%s/%s`: %s\n", optarg, e->d_name, dlerror());
						}
					}
					closedir(d);
				} break;
			case 'H':
				std::string mod(optarg), conf;
				auto p = mod.find("=");
				if( p != std::string::npos ) {
					conf = mod.substr(p+1);
					mod = mod.substr(0, p);
				}
				modules.push_back( std::unique_ptr<Module::Module>(
					Module::ModuleRegistry::get_instance().get(mod)(conf)
				) );
				break;
			}
		}
	}

	{ // Open listening socket
		std::string host, port;

		/* Address format is
		 *   - hostname:portname
		 *   - [numeric ip]:portname
		 *   - hostname:[portnumber]
		 *   - [numeric ip]:[portnumber]
		 */
		size_t c = options.bind_addr_listen.rfind(":");
		if( c == std::string::npos ) {
			/* TRANSLATORS: %1$s contains the string passed as option
			 */
			fprintf(stderr, "Invalid bind string \"%1$s\": could not find ':'\n", options.bind_addr_listen.c_str());
			exit(EX_DATAERR);
		}
		host = options.bind_addr_listen.substr(0, c);
		port = options.bind_addr_listen.substr(c+1);

		std::unique_ptr< boost::ptr_vector< SockAddr::SockAddr> > bind_sa
			= SockAddr::resolve( host, port, 0, SOCK_STREAM, 0);
		if( bind_sa->size() == 0 ) {
			fprintf(stderr, "Can not bind to \"%1$s\": Could not resolve\n", options.bind_addr_listen.c_str());
			exit(EX_DATAERR);
		} else if( bind_sa->size() > 1 ) {
			// TODO: allow this
			fprintf(stderr, "Can not bind to \"%1$s\": Resolves to multiple entries:\n", options.bind_addr_listen.c_str());
			for( typeof(bind_sa->begin()) i = bind_sa->begin(); i != bind_sa->end(); i++ ) {
				fprintf(stderr, "  %s\n", i->string().c_str());
			}
			exit(EX_DATAERR);
		}

		s_listen = Socket::socket( (*bind_sa)[0].proto_family() , SOCK_STREAM, 0);
		s_listen.set_reuseaddr();
		s_listen.bind((*bind_sa)[0]);
		s_listen.listen(MAX_CONN_BACKLOG);

		std::unique_ptr<SockAddr::SockAddr> bound_addr( s_listen.getsockname() );
		fprintf(stderr, "Listening on %s\n", bound_addr->string().c_str());
	}

	{
		struct sigaction act;
		if( sigaction(SIGPIPE, NULL, &act) == -1) {
			fprintf(stderr, "sigaction() failed: %s", strerror(errno));
			return -1;
		}
		act.sa_handler = SIG_IGN; // Ignore SIGPIPE (we'll handle the write()-error)
		if( sigaction(SIGPIPE, &act, NULL) == -1 ) {
			fprintf(stderr, "sigaction() failed: %s", strerror(errno));
			return -1;
		}
	}

	{
		ev_signal ev_sigint_watcher;
		ev_signal_init( &ev_sigint_watcher, received_sigint, SIGINT);
		ev_signal_start( EV_DEFAULT_ &ev_sigint_watcher);

		ev_signal ev_sigterm_watcher;
		ev_signal_init( &ev_sigterm_watcher, received_sigterm, SIGTERM);
		ev_signal_start( EV_DEFAULT_ &ev_sigterm_watcher);

		ev_io ev_listen;
		ev_io_init(&ev_listen, incomming_connection, s_listen, EV_READ);
		ev_io_start( EV_DEFAULT_ &ev_listen);

		fprintf(stderr, "Setup done, starting event loop\n");

		ev_run(EV_DEFAULT_ 0);
	}

	fprintf(stderr, "Cleaning up\n");

	return 0;
}
