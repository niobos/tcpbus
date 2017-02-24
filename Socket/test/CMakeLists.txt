add_executable(getifaddrs EXCLUDE_FROM_ALL getifaddrs.cxx $<TARGET_OBJECTS:Socket>)
add_test(getifaddrs getifaddrs)
add_dependencies(check getifaddrs)
