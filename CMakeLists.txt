# Siconos is a program dedicated to modeling, simulation and control
# of non smooth dynamical systems.
#
# Copyright 2024 INRIA.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
#===============================================================================
# cmake utility to compile and install siconos.
#===============================================================================

# ============= Global 'standard' cmake Settings =============

# Set minimum version for cmake
# We advise the most recent version of cmake.
# Use python3 -m pip install cmake to get it.
cmake_minimum_required(VERSION 3.14) 

cmake_policy(SET CMP0074 NEW) # find_package() uses <PackageName>_ROOT variables.
cmake_policy(SET CMP0086 NEW) # UseSWIG honors SWIG_MODULE_NAME via -module flag.
cmake_policy(SET CMP0079 NEW) # target_link_libraries() allows use with targets in other directories
cmake_policy(SET CMP0067 NEW) # Honor language standard in try_compile() source-file signature.

# Set cmake modules directories
# (i.e. the those which contain all user-defined FindXXX.cmake files among other things)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/docs/cmake)

# If ON, a compile_commands.json file is generated and might be read and used by your editor
# to improve static verif. of the code source.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "" FORCE)

# Improve cmake interface when current version is too old.
include(new_cmake_functions)

# Force out-of-source build
if ("${CMAKE_SOURCE_DIR}" MATCHES "${CMAKE_BINARY_DIR}")
  message (SEND_ERROR "In source building not supported (not recommanded indeed). You need to :
    * cleanup your source directory :  \"rm -rf ./CMakeFiles/ ./CMakeCache.txt\"
    * try configure process again in a new directory
    (e.g. mkdir <anywhere>/build ; cd <anywhere>/build ; cmake ${CMAKE_SOURCE_DIR}) ...")
  return()
endif()

# If not given, turn build type to release mode.
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: None Debug Release." FORCE)
endif()

# Some useful macros and functions
include(SiconosTools)
# Some python tools and functions, used to configure the project, called by cmake.
configure_file(scripts/buildtools.py share/buildtools.py COPYONLY)

# cmake project name
set(PROJECT_NAME siconos)

# Get current year (for licence and other cartridges)
find_program(DATE_COMMAND date)
if(DATE_COMMAND)
  execute_process(COMMAND date "+\"%Y\"" OUTPUT_VARIABLE YEAR)
  string(STRIP ${YEAR} YEAR)
endif()
# Read user option file, if provided on command line
if(USER_OPTIONS_FILE)
  # Check for tilde in file name (not handled properly by cmake)
  string(FIND ${USER_OPTIONS_FILE} "\~" res)
  if(res EQUAL 0)
    string(REPLACE "\~" "$ENV{HOME}" USER_OPTIONS_FILE ${USER_OPTIONS_FILE})
  endif()
  if(NOT IS_ABSOLUTE ${USER_OPTIONS_FILE})
    get_filename_component(USER_OPTIONS_FILE ${USER_OPTIONS_FILE} REALPATH
      BASE_DIR ${CMAKE_CURRENT_BINARY_DIR})
  endif()
  message("\n !!!!! Load user-defined options set from file ${USER_OPTIONS_FILE} !!!!! \n")
  # Configure to take into account any change in ${USER_OPTIONS_FILE}
  configure_file(${USER_OPTIONS_FILE} current_options.cmake COPYONLY)
  include(${CMAKE_CURRENT_BINARY_DIR}/current_options.cmake
    RESULT_VARIABLE _CONF_FOUND OPTIONAL)
  if(NOT _CONF_FOUND)
    message(FATAL_ERROR "include could not find requested file: ${USER_OPTIONS_FILE}")
  endif()
else()
  message("\n !!!!! Load default configuration set from file ${CMAKE_SOURCE_DIR}/config_samples/default.cmake !!!!! \n")
  include(${CMAKE_SOURCE_DIR}/config_samples/default.cmake)
endif()
# Read devel or advanced user options
include(cmake/advanced_options.cmake)

# Set target names from components list
if(NOT COMPONENTS)
  message(WARNING "Components list is empty - Nothing will be built.")
endif()

set(AVAILABLE_COMPONENTS "externals numerics kernel control mechanics mechanisms io")
foreach(component_name IN LISTS COMPONENTS)
  # Check and set components list
  string(TOUPPER ${component_name} upname)
  list(FIND ${AVAILABLE_COMPONENTS} ${component_name} comp_exists)
  if(NOT comp_exists)
    message(FATAL_ERROR "Unknown siconos component : ${component_name}")
  endif()
  set(HAVE_SICONOS_${upname} TRUE)
endforeach()
if(HAVE_SICONOS_KERNEL AND NOT WITH_CXX)
  message(FATAL_ERROR "You can not build kernel component without C++. Please set WITH_CXX=ON.")
endif()
  
# Get last component in the list;
# It might be usefull to set some dependencies in targets.
list(GET COMPONENTS -1 last_component)
# ============= The project =============
# Set project name and project languages
# Languages:
# - Fortran is optional, needed by ql0001, lsodar, hem5 solvers.
# - CXX is required for kernel and following components
# - C always required

if(WITH_CXX)
  project(${PROJECT_NAME} CXX C) # C or not C ?
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_STANDARD 17)
else()
  # we may need to compile only numerics and only in C.
  # Really???
  project(${PROJECT_NAME} C)
endif()

if(LLVM_ANALYSE)
  set(CMAKE_C_OUTPUT_EXTENSION ".bc")
endif()

# Activate fortran, if needed.
if(WITH_FORTRAN)
  enable_language(Fortran)
  set(HAS_FORTRAN TRUE)
  set(CMAKE_Fortran_MODULE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Modules)
endif()

# Common setup
include(SiconosSetup)

# ========= Tests setup =========
if(WITH_TESTING)
  include(CTest)
  include(SiconosCTest)
  # log file for tests setup
  file(WRITE ${CMAKE_BINARY_DIR}/tests.log "--- Siconos tests setup --- \n")
endif()

# ========= Documentation setup =========
# This includes doxygen_warnings,
# doxygen doc from source files, sphinx doc
# and docstrings from doxygen, depending on the
# enabled options.
add_subdirectory(docs)

if(WITH_FCLIB)
  include(fclib_setup)
endif()

# ========= Setup each required component =========
foreach(COMPONENT IN LISTS COMPONENTS)
  message("\n--------------------------------------------------")
  add_subdirectory(${COMPONENT})
endforeach()

# ========= Python bindings =========
if(WITH_PYTHON_WRAPPER)
  add_subdirectory(wrap)
endif()

# ========= Documentation setup =========
# Finalize documentation setups.
# Must be called AFTER each component config.
if(WITH_DOCUMENTATION)
  finalize_doc()
endif()
  
include(FeatureSummary)
feature_summary(WHAT ALL)

# ============= Siconos Package configuration =============
# i.e. what should be generated/configured at install.
include(SiconosPackageSetup)

# SiconosConfig.h generation and include
if(EXISTS ${CMAKE_SOURCE_DIR}/config.h.cmake)
  # Warning : strongly depends on find_package(...) results.
  configure_file(${CMAKE_SOURCE_DIR}/config.h.cmake
    ${CMAKE_BINARY_DIR}/SiconosConfig.h)
  if(SHOW_ME_SICONOS_CONFIG)
    file(READ ${CMAKE_BINARY_DIR}/SiconosConfig.h SICONOS_CONFIG_H)
    message(${SICONOS_CONFIG_H})
  endif()
  install(FILES ${CMAKE_BINARY_DIR}/SiconosConfig.h DESTINATION include/${PROJECT_NAME})
endif()


# man pages
if(IS_DIRECTORY ${CMAKE_SOURCE_DIR}/man)
  configure_file(man/siconos.1.in man/siconos.1)
  install(FILES ${CMAKE_BINARY_DIR}/man/siconos.1 DESTINATION share/man/man1)
endif()

# ============= Summary =============
message(STATUS "\n============================================ Summary ============================================")
message(STATUS "${PROJECT_NAME} version ${SICONOS_VERSION} is now ready for compilation and installation.")
message(STATUS "To proceed run 'make' and 'make install' and optionaly 'make test'.")
message(STATUS "C++ Compiler : ${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "C++ flags : ${CMAKE_CXX_FLAGS}")
message(STATUS "C Compiler : ${CMAKE_C_COMPILER} ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "C flags : ${CMAKE_C_FLAGS}")
message(STATUS "Fortran Compiler : ${CMAKE_Fortran_COMPILER} ${CMAKE_Fortran_COMPILER_VERSION}")
message(STATUS "Fortran flags : ${CMAKE_Fortran_FLAGS}")
message(STATUS "Compilation mode is : ${CMAKE_BUILD_TYPE}")
message(STATUS "Code Sources are in : ${CMAKE_SOURCE_DIR}")
message(STATUS "Blas from ${BLAS_NAME}.")
message("    Blas libraries : ${BLAS_LIBRARIES}.")
message("    Blas headers : ${BLAS_HEADER} in ${BLAS_INCLUDE_DIR}.")
message(STATUS "Lapack config:")
message("    Lapack libraries : ${LAPACK_LIBRARIES}.")
message("    Lapack headers : ${LAPACK_HEADER} in ${LAPACK_INCLUDE_DIR}.")
if(WITH_FCLIB)
  message(STATUS "Use fclib installed in ${fclib_ROOT}.")
endif()
  message(STATUS "Use system-provided SuiteSparse library, with:")
  message("    - CXSparse headers path: ${SuiteSparse_CXSparse_INCLUDE_DIR}")
  message("    - CXSparse library : ${SuiteSparse_CXSparse_LIBRARY}")
  message("    - LDL headers path: ${SuiteSparse_LDL_INCLUDE_DIR}")
  message("    - LDL library : ${SuiteSparse_LDL_LIBRARY}")
message(STATUS "Python interpreter is ${Python_EXECUTABLE}")
if(WITH_PYTHON_WRAPPER)
  message(STATUS "Python libraries are ${Python_LIBRARIES}")
else()
  message("Siconos Python API is off\n")
endif()
if(NOT WITH_FORTRAN)
  message(STATUS "Siconos Fortran API is off. Some integrators won't be available (Lsodar and Hem5).\n")
endif()
message(STATUS "------------ Installation information ------------\n")
message("    Please check this info with care ...\n")
if(SICONOS_SYSTEM_WIDE_INSTALL)
  message(WARNING "\nYou asked for a system wide installation. This requires root privileges and means that you MUST run make install as root (or sudo).
Else, a correct behavior of python packages installation process is not guaranteed.\n")
endif()
if(SICONOS_CUSTOM_INSTALL AND ISOLATED_INSTALL)
  message(WARNING "You asked for a fully isolated installation in ${SICONOS_CUSTOM_INSTALL} at your own risks.\n")
endif()
message("    - The binaries, libraries and headers will be installed in ${CMAKE_INSTALL_PREFIX} \n")
if(WITH_PYTHON_WRAPPER)
  message("    - Siconos python packages will be installed in ${SICONOS_PYTHON_INSTALL_DIR}\n")
  message("    with the command ${Python_EXECUTABLE} -m pip install -U ${PIP_INSTALL_OPTIONS} ...\n")
  message("    If this path is not in the standard python environment, you'll probably have to fill PYTHONPATH.\n")
  message("    To check python known paths, run: python3 -c 'import sys; print(sys.path)' \n")
  message("    If siconos package path is not in sys.path, to add it, execute: export PYTHONPATH=${SICONOS_PYTHON_INSTALL_DIR}")
endif()
message(STATUS "------------------------------------- ------------\n")
message(STATUS "To get more information about dependencies, config or else, ")
message(STATUS "check CMakeCache.txt file or re-run cmake with -DPRINT_ENV=ON.")
message(STATUS "=================================================================================================\n")

# Log build info into ${CMAKE_BINARY_DIR}/Testing/Notes/Build
write_notes()
