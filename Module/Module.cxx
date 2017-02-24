#include "Module.hxx"

/* Explicit instanciation */
template class ::Registry< std::string, Module::Module*(*)(std::string) >;

namespace Module {

} // namespace
