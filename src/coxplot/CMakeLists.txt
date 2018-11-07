project(coxplot)
cmake_minimum_required(VERSION 3.2)

find_package(f2c REQUIRED)

set(COXPLOT_INSTALL_BIN_DIR "${CMAKE_INSTALL_PREFIX}/bin/${project}")
set(COXPLOT_INSTALL_LIB_DIR "${CMAKE_INSTALL_PREFIX}/lib/${project}")
set(COXPLOT_INSTALL_STA_DIR "${CMAKE_INSTALL_PREFIX}/static/${project}")
set(COXPLOT_INSTALL_INCLUDE_DIR "${CMAKE_INSTALL_PREFIX}/include/${project}")


# Until I have setup modern use of target properties I will export this.
# set(COXPLOT_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
# if(NOT F2C_FOUND)
#   find_package(f2c REQUIRED)
# endif(NOT F2C_FOUND)

include(FindMotif)

file(GLOB ALLO RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.f")
foreach(filename ${ALLO})
	string(REGEX REPLACE "\.f$" ".c" output_file "${filename}")

	set(input_file "${CMAKE_CURRENT_SOURCE_DIR}/${filename}")
	set(output_file "${CMAKE_CURRENT_BINARY_DIR}/${output_file}")

	# add_custom_command(OUTPUT "${output_file}"
	# 		COMMAND ${F2C_BIN} "${input_file}"
	# 		MAIN_DEPENDENCY "${input_file}")

	list(APPEND ALLO_SRCS "${output_file}")
endforeach(filename)
list(APPEND ALLO_SRCS find_color_name.c)
file(GLOB PLOTPAK_SRCS "ppak_*.c")
file(GLOB PLOT_SRCS "plot_*.c")

add_library(coxplot ${ALLO_SRCS} ${PLOT_SRCS} ${PLOTPAK_SRCS})

target_link_libraries(coxplot m converted_from_fortran ${MOTIF_LIBRARIES})
target_include_directories(coxplot PRIVATE ${MOTIF_INCLUDE_DIR})
target_include_directories(coxplot PUBLIC ${COXPLOT_INCLUDE_DIRS})

set_target_properties(coxplot PROPERTIES
	PUBLIC_HEADER "coxplot.h"
	COMPILE_DEFINITIONS "HAVE_XDBE"
)

install(TARGETS coxplot
	EXPORT CoxplotTargets
	RUNTIME DESTINATION "${COXPLOT_INSTALL_BIN_DIR}" COMPONENT bin
	LIBRARY DESTINATION "${COXPLOT_INSTALL_LIB_DIR}" COMPONENT shlib
	ARCHIVE DESTINATION "${COXPLOT_INSTALL_STA_DIR}"
	PUBLIC_HEADER DESTINATION "${COXPLOT_INSTALL_INCLUDE_DIR}"
	COMPONENT dev
)
