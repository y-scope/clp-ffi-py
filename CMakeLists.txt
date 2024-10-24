# Minimum cmake version required by scikit-build-core: 3.15
cmake_minimum_required(VERSION 3.15)

set(CLP_FFI_PY_INSTALL_LIBS ON CACHE BOOL "Whether to install the built libraries." FORCE)

if(DEFINED SKBUILD_PROJECT_NAME)
    set(CLP_FFI_PY
        ${SKBUILD_PROJECT_NAME}
        CACHE STRING
        "Use scikit-build-core's project name."
        FORCE
    )
    set(CLP_FFI_PY_VERSION
        ${SKBUILD_PROJECT_VERSION}
        CACHE STRING
        "Use scikit-build-core's project version."
        FORCE
    )
else()
    # The project is configured outside of scikit-build-core. Normally this is for build testing
    # purpose, or loading the project into CLion. We don't install the built library in this config.
    message(WARNING "Configured outside of scikit-build-core. Library installation is disabled.")
    set(CLP_FFI_PY_INSTALL_LIBS
        OFF
        CACHE BOOL
        "Disable library installation when building outside of scikit-build-core."
        FORCE
    )
    set(CLP_FFI_PY "clp-ffi-py-local-build" CACHE STRING "Use local build name." FORCE)
    set(CLP_FFI_PY_VERSION "0.0.0" CACHE STRING "Version info is unused for local build." FORCE)
    set(CMAKE_EXPORT_COMPILE_COMMANDS
        ON
        CACHE BOOL
        "Enable/Disable output of compile commands during generation."
        FORCE
    )
endif()

project(
    ${CLP_FFI_PY}
    LANGUAGES
        C
        CXX
    VERSION ${CLP_FFI_PY_VERSION}
)

# C-extension modules will be compiled as shared libraries. To enable dynamic linking, all libraries
# (including the static ones) must be compiled with position-independent-code option.
set(CMAKE_POSITION_INDEPENDENT_CODE ON CACHE BOOL "Compile libraries using PIC.")

# Find Python interpreter, compiler and development environment.
find_package(
    Python
    REQUIRED
    COMPONENTS
        Interpreter
        Development.Module
)

set(CLP_FFI_PY_LIB_IR "native" CACHE STRING "The name of IR native library")
python_add_library(
    ${CLP_FFI_PY_LIB_IR}
    MODULE
    WITH_SOABI
)

target_compile_features(${CLP_FFI_PY_LIB_IR} PRIVATE cxx_std_20)

set(SRC_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/src" CACHE STRING "Root path of C++ source code")
set(CLP_FFI_PY_SRC_ROOT
    "${SRC_ROOT}/clp_ffi_py"
    CACHE STRING
    "Root path of clp-ffi-py's C++ source code"
)
set(CLP_CORE_ROOT "${SRC_ROOT}/clp/components/core" CACHE STRING "Root path of CLP core")

add_subdirectory(${CLP_CORE_ROOT}/src/clp/string_utils)

# Source files of clp-ffi-core
# Note: we don't add headers hear since they are not considered to be a part of the project.
set(SOURCE_FILES_CLP_FFI_CORE
    ${CLP_CORE_ROOT}/src/clp/BufferReader.cpp
    ${CLP_CORE_ROOT}/src/clp/ffi/ir_stream/decoding_methods.cpp
    ${CLP_CORE_ROOT}/src/clp/ffi/ir_stream/encoding_methods.cpp
    ${CLP_CORE_ROOT}/src/clp/ffi/ir_stream/utils.cpp
    ${CLP_CORE_ROOT}/src/clp/ffi/encoding_methods.cpp
    ${CLP_CORE_ROOT}/src/clp/ir/parsing.cpp
    ${CLP_CORE_ROOT}/src/clp/ReaderInterface.cpp
)

# Header files and source files of clp-ffi-py
# Note: headers should be included to allow CLion loads the project properly.
set(SOURCE_FILES_CLP_FFI_PY_LIB_IR
    ${CLP_FFI_PY_SRC_ROOT}/error_messages.hpp
    ${CLP_FFI_PY_SRC_ROOT}/ExceptionFFI.hpp
    ${CLP_FFI_PY_SRC_ROOT}/ir/native/deserialization_methods.cpp
    ${CLP_FFI_PY_SRC_ROOT}/ir/native/deserialization_methods.hpp
    ${CLP_FFI_PY_SRC_ROOT}/ir/native/LogEvent.hpp
    ${CLP_FFI_PY_SRC_ROOT}/ir/native/Metadata.cpp
    ${CLP_FFI_PY_SRC_ROOT}/ir/native/Metadata.hpp
    ${CLP_FFI_PY_SRC_ROOT}/ir/native/PyDeserializer.cpp
    ${CLP_FFI_PY_SRC_ROOT}/ir/native/PyDeserializer.hpp
    ${CLP_FFI_PY_SRC_ROOT}/ir/native/PyDeserializerBuffer.cpp
    ${CLP_FFI_PY_SRC_ROOT}/ir/native/PyDeserializerBuffer.hpp
    ${CLP_FFI_PY_SRC_ROOT}/ir/native/PyFourByteSerializer.cpp
    ${CLP_FFI_PY_SRC_ROOT}/ir/native/PyFourByteSerializer.hpp
    ${CLP_FFI_PY_SRC_ROOT}/ir/native/PyLogEvent.cpp
    ${CLP_FFI_PY_SRC_ROOT}/ir/native/PyLogEvent.hpp
    ${CLP_FFI_PY_SRC_ROOT}/ir/native/PyMetadata.cpp
    ${CLP_FFI_PY_SRC_ROOT}/ir/native/PyMetadata.hpp
    ${CLP_FFI_PY_SRC_ROOT}/ir/native/PyQuery.cpp
    ${CLP_FFI_PY_SRC_ROOT}/ir/native/PyQuery.hpp
    ${CLP_FFI_PY_SRC_ROOT}/ir/native/Query.cpp
    ${CLP_FFI_PY_SRC_ROOT}/ir/native/Query.hpp
    ${CLP_FFI_PY_SRC_ROOT}/ir/native/serialization_methods.cpp
    ${CLP_FFI_PY_SRC_ROOT}/ir/native/serialization_methods.hpp
    ${CLP_FFI_PY_SRC_ROOT}/modules/ir_native.cpp
    ${CLP_FFI_PY_SRC_ROOT}/Py_utils.cpp
    ${CLP_FFI_PY_SRC_ROOT}/Py_utils.hpp
    ${CLP_FFI_PY_SRC_ROOT}/PyObjectCast.hpp
    ${CLP_FFI_PY_SRC_ROOT}/PyObjectUtils.hpp
    ${CLP_FFI_PY_SRC_ROOT}/Python.hpp
    ${CLP_FFI_PY_SRC_ROOT}/utils.cpp
    ${CLP_FFI_PY_SRC_ROOT}/utils.hpp
)

target_sources(
    ${CLP_FFI_PY_LIB_IR}
    PRIVATE
        ${SOURCE_FILES_CLP_FFI_CORE}
        ${SOURCE_FILES_CLP_FFI_PY_LIB_IR}
)

target_include_directories(
    ${CLP_FFI_PY_LIB_IR}
    SYSTEM
    PRIVATE
        ${CLP_CORE_ROOT}/src
        ${CLP_CORE_ROOT}/submodules
        ${SRC_ROOT}
)

target_link_libraries(${CLP_FFI_PY_LIB_IR} PRIVATE clp::string_utils)

if(CLP_FFI_PY_INSTALL_LIBS)
    install(
        TARGETS
            ${CLP_FFI_PY_LIB_IR}
            DESTINATION
            ${CLP_FFI_PY}/ir
            COMPONENT
            PythonModule
    )
endif()
