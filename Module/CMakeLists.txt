#SOCKADDR_HAS_LEN_FIELD

# compiled-in modules:
add_library(Modules OBJECT Module.cxx Log.cxx)

# Dynamically loaded modules:
#add_library(log MODULE Log.cxx)
#set_target_properties(log PROPERTIES PREFIX "")

add_library(velbus MODULE Velbus.cxx)
set_target_properties(velbus PROPERTIES PREFIX "")

install(FILES velbus.pl DESTINATION lib/tcpbus)
