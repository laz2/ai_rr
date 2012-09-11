
project(wave_find_path)

cmake_minimum_required(VERSION 2.6)

set(SRCS
	wave_find_path.cpp
)

find_library(libSC "libSC" PATHS_SUFFIXES "../../lib")
find_library(libSCd "libSCd" PATHS_SUFFIXES "../../lib")

include_directories("../../include/libSC" "../../include/libTGF" "../../include/libSys")
add_executable(wave_find_path ${SRCS})
target_link_libraries(wave_find_path debug ${libSCd})
target_link_libraries(wave_find_path optimized ${libSC})

# Configure the template file
if(MSVC)
	macro(configure_user_template PROJECT_NAME)
		# Find user and system name
		set(SYSTEM_NAME $ENV{USERDOMAIN} CACHE STRING SystemName)
		set(USER_NAME $ENV{USERNAME} CACHE STRING UserName)

		set(USER_FILE ${PROJECT_NAME}.vcproj.${SYSTEM_NAME}.${USER_NAME}.user)

		set(OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/${USER_FILE})

		configure_file(${CMAKE_CURRENT_SOURCE_DIR}/UserTemplate.user ${USER_FILE} @ONLY)
	endmacro()

	file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../bin" SC_CORE_BINARY_DIR_NATIVE)
	set(USERFILE_ENVIRONMENT "PATH=${SC_CORE_BINARY_DIR_NATIVE}")
	set(USERFILE_ENVIRONMENT_DEBUG ${USERFILE_ENVIRONMENT})
	set(USERFILE_ENVIRONMENT_RELEASE ${USERFILE_ENVIRONMENT})
	set(USERFILE_ENVIRONMENT_MINSIZEREL ${USERFILE_ENVIRONMENT})
	set(USERFILE_ENVIRONMENT_RELWITHDEBINFO ${USERFILE_ENVIRONMENT})
	set(USERFILE_PLATFORM "Win32")

	configure_user_template(wave_find_path)
endif()
