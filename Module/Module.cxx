#include "Module.hxx"

#include <sys/time.h>
#include <sstream>
#include <iomanip>

/* Explicit instanciation */
template class ::Registry< std::string, struct Module::ModuleInfo >;

namespace Module {

std::string Module::timestamp() {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        std::ostringstream o;
        o << tv.tv_sec << "."
          << std::setfill('0') << std::setw(6) << tv.tv_usec;
        return o.str();
}

} // namespace
