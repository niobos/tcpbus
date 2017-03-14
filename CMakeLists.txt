cmake_minimum_required(VERSION 3.5)
project("tcpbus" C CXX)
set(PACKAGE_VERSION "0.0.0")

option(ENABLE_IPV6 "Enable IPv6 support" ON)

set(CMAKE_CXX_STANDARD 11)

find_package(Git)
if(Git_FOUND)
	execute_process(
		COMMAND "${GIT_EXECUTABLE}" "rev-parse" "HEAD"
		RESULT_VARIABLE exitcode
		OUTPUT_VARIABLE PACKAGE_GITREVISION
		OUTPUT_STRIP_TRAILING_WHITESPACE
		ERROR_QUIET
	)
	message(STATUS "Git commit: ${PACKAGE_GITREVISION}")
else()
	set(PACKAGE_GITREVISION "unknown")
endif()

configure_file(config.h.in config.h ESCAPE_QUOTES @ONLY)
include_directories( ${PROJECT_BINARY_DIR} )

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")


enable_testing()
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND})

add_subdirectory("Module")
add_subdirectory("Socket")

find_package(libev REQUIRED)
include_directories(${LIBEV_INCLUDE_DIRS})
add_executable(tcpbus src/tcpbus.cxx $<TARGET_OBJECTS:Modules> $<TARGET_OBJECTS:Socket>)
target_link_libraries(tcpbus ${CMAKE_DL_LIBS} -rdynamic) # dynamic linking support
# -rdynamic exposes global symbols toward libraries (Registy's)
target_link_libraries(tcpbus ${LIBEV_LIBRARIES})

install(TARGETS tcpbus
        RUNTIME DESTINATION bin
)

add_subdirectory("test")
