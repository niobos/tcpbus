#SOCKADDR_HAS_LEN_FIELD

# Linux does not have a sin_len field
# OSX (and probably *BSD as well) does have it
include(CheckCSourceCompiles)
check_c_source_compiles("
	#include <netinet/in.h>
	void main() {
		struct sockaddr_in sa;
		void* t = &sa.sin_len;
	}
	"
	SOCKADDR_HAS_LEN_FIELD
)

add_library(Socket OBJECT Socket.cxx SockAddr.cxx Errno.cxx)

add_subdirectory(test)
