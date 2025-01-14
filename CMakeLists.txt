cmake_minimum_required(VERSION 3.26...3.30)

# Define the variables necessary to build and install the project differently depending on whether
# the project is being configured from scikit-build-core. If configured from outside
# scikit-build-core, we need to use placeholder values for the variables that are usually defined
# by scikit-build-core.
if(DEFINED SKBUILD_PROJECT_NAME)
    set(CLP_FFI_PY_INSTALL_LIBS ON CACHE BOOL "Enable installing the built libraries." FORCE)
    set(CLP_FFI_PY_PROJECT_NAME
        ${SKBUILD_PROJECT_NAME}
        CACHE STRING
        "The name of the project parsed by scikit-build-core."
        FORCE
    )
    set(CLP_FFI_PY_VERSION
        ${SKBUILD_PROJECT_VERSION}
        CACHE STRING
        "The version of the project parsed by scikit-build-core."
        FORCE
    )
    set(CLP_FFI_PY_ENABLE_LINTING OFF)
else()
    set(CLP_FFI_PY_INSTALL_LIBS OFF CACHE BOOL "Disable installing the built libraries." FORCE)
    set(CLP_FFI_PY_PROJECT_NAME "clp-ffi-py" CACHE STRING "Use a placeholder project name." FORCE)
    set(CLP_FFI_PY_VERSION "0.0.0" CACHE STRING "Use a placeholder version." FORCE)
    set(CMAKE_EXPORT_COMPILE_COMMANDS
        ON
        CACHE BOOL
        "Enable/Disable output of compile commands during generation."
        FORCE
    )
    set(CLP_FFI_PY_ENABLE_LINTING ON)
endif()

project(${CLP_FFI_PY_PROJECT_NAME} LANGUAGES CXX VERSION ${CLP_FFI_PY_VERSION})

# C-extension modules will be compiled as shared libraries. To enable dynamic linking, all libraries
# (including the static ones) must be compiled with the position-independent-code option.
set(CMAKE_POSITION_INDEPENDENT_CODE ON CACHE BOOL "Compile libraries using PIC.")

find_package(
    Python
    REQUIRED
    COMPONENTS
        Interpreter
        Development.Module
)

set(CLP_FFI_PY_LIB_IR "native")
python_add_library(${CLP_FFI_PY_LIB_IR} MODULE WITH_SOABI)

target_compile_features(${CLP_FFI_PY_LIB_IR} PRIVATE cxx_std_20)

set(CLP_FFI_PY_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(CLP_FFI_PY_LIB_SRC_DIR "${CLP_FFI_PY_SRC_DIR}/clp_ffi_py")
set(CLP_FFI_PY_WRAPPED_FACADE_HEADERS_DIR "${CLP_FFI_PY_SRC_DIR}/wrapped_facade_headers")
set(CLP_FFI_PY_CLP_CORE_DIR "${CLP_FFI_PY_SRC_DIR}/clp/components/core")

# Add CLP's string_utils
add_subdirectory(${CLP_FFI_PY_CLP_CORE_DIR}/src/clp/string_utils)

# Add GSL
add_subdirectory(${CLP_FFI_PY_SRC_DIR}/GSL)

# Add msgpack
set(MSGPACK_CXX20 ON CACHE BOOL "Enable C++20 in msgpack" FORCE)
set(MSGPACK_USE_BOOST OFF CACHE BOOL "Disable Boost in msgpack" FORCE)
add_subdirectory(${CLP_FFI_PY_SRC_DIR}/msgpack EXCLUDE_FROM_ALL)

# NOTE: We don't add headers here since CLP core is technically a library we're using, not a part of
# this project.
set(CLP_FFI_PY_CLP_CORE_SOURCES
    ${CLP_FFI_PY_CLP_CORE_DIR}/src/clp/BufferReader.cpp
    ${CLP_FFI_PY_CLP_CORE_DIR}/src/clp/ffi/ir_stream/decoding_methods.cpp
    ${CLP_FFI_PY_CLP_CORE_DIR}/src/clp/ffi/ir_stream/encoding_methods.cpp
    ${CLP_FFI_PY_CLP_CORE_DIR}/src/clp/ffi/ir_stream/ir_unit_deserialization_methods.cpp
    ${CLP_FFI_PY_CLP_CORE_DIR}/src/clp/ffi/ir_stream/Serializer.cpp
    ${CLP_FFI_PY_CLP_CORE_DIR}/src/clp/ffi/ir_stream/utils.cpp
    ${CLP_FFI_PY_CLP_CORE_DIR}/src/clp/ffi/encoding_methods.cpp
    ${CLP_FFI_PY_CLP_CORE_DIR}/src/clp/ffi/SchemaTree.cpp
    ${CLP_FFI_PY_CLP_CORE_DIR}/src/clp/ffi/KeyValuePairLogEvent.cpp
    ${CLP_FFI_PY_CLP_CORE_DIR}/src/clp/ir/EncodedTextAst.cpp
    ${CLP_FFI_PY_CLP_CORE_DIR}/src/clp/ir/parsing.cpp
    ${CLP_FFI_PY_CLP_CORE_DIR}/src/clp/ReaderInterface.cpp
)

# NOTE: We include headers to ensure IDEs like CLion load the project properly.
set(CLP_FFI_PY_LIB_IR_SOURCES
    ${CLP_FFI_PY_LIB_SRC_DIR}/api_decoration.hpp
    ${CLP_FFI_PY_LIB_SRC_DIR}/error_messages.hpp
    ${CLP_FFI_PY_LIB_SRC_DIR}/ExceptionFFI.hpp
    ${CLP_FFI_PY_LIB_SRC_DIR}/ir/native/deserialization_methods.cpp
    ${CLP_FFI_PY_LIB_SRC_DIR}/ir/native/deserialization_methods.hpp
    ${CLP_FFI_PY_LIB_SRC_DIR}/ir/native/DeserializerBufferReader.cpp
    ${CLP_FFI_PY_LIB_SRC_DIR}/ir/native/DeserializerBufferReader.hpp
    ${CLP_FFI_PY_LIB_SRC_DIR}/ir/native/LogEvent.hpp
    ${CLP_FFI_PY_LIB_SRC_DIR}/ir/native/Metadata.cpp
    ${CLP_FFI_PY_LIB_SRC_DIR}/ir/native/Metadata.hpp
    ${CLP_FFI_PY_LIB_SRC_DIR}/ir/native/PyDeserializer.cpp
    ${CLP_FFI_PY_LIB_SRC_DIR}/ir/native/PyDeserializer.hpp
    ${CLP_FFI_PY_LIB_SRC_DIR}/ir/native/PyDeserializerBuffer.cpp
    ${CLP_FFI_PY_LIB_SRC_DIR}/ir/native/PyDeserializerBuffer.hpp
    ${CLP_FFI_PY_LIB_SRC_DIR}/ir/native/PyFourByteDeserializer.cpp
    ${CLP_FFI_PY_LIB_SRC_DIR}/ir/native/PyFourByteDeserializer.hpp
    ${CLP_FFI_PY_LIB_SRC_DIR}/ir/native/PyFourByteSerializer.cpp
    ${CLP_FFI_PY_LIB_SRC_DIR}/ir/native/PyFourByteSerializer.hpp
    ${CLP_FFI_PY_LIB_SRC_DIR}/ir/native/PyKeyValuePairLogEvent.cpp
    ${CLP_FFI_PY_LIB_SRC_DIR}/ir/native/PyKeyValuePairLogEvent.hpp
    ${CLP_FFI_PY_LIB_SRC_DIR}/ir/native/PyLogEvent.cpp
    ${CLP_FFI_PY_LIB_SRC_DIR}/ir/native/PyLogEvent.hpp
    ${CLP_FFI_PY_LIB_SRC_DIR}/ir/native/PyMetadata.cpp
    ${CLP_FFI_PY_LIB_SRC_DIR}/ir/native/PyMetadata.hpp
    ${CLP_FFI_PY_LIB_SRC_DIR}/ir/native/PyQuery.cpp
    ${CLP_FFI_PY_LIB_SRC_DIR}/ir/native/PyQuery.hpp
    ${CLP_FFI_PY_LIB_SRC_DIR}/ir/native/PySerializer.cpp
    ${CLP_FFI_PY_LIB_SRC_DIR}/ir/native/PySerializer.hpp
    ${CLP_FFI_PY_LIB_SRC_DIR}/ir/native/Query.cpp
    ${CLP_FFI_PY_LIB_SRC_DIR}/ir/native/Query.hpp
    ${CLP_FFI_PY_LIB_SRC_DIR}/ir/native/serialization_methods.cpp
    ${CLP_FFI_PY_LIB_SRC_DIR}/ir/native/serialization_methods.hpp
    ${CLP_FFI_PY_LIB_SRC_DIR}/modules/ir_native.cpp
    ${CLP_FFI_PY_LIB_SRC_DIR}/Py_utils.cpp
    ${CLP_FFI_PY_LIB_SRC_DIR}/Py_utils.hpp
    ${CLP_FFI_PY_LIB_SRC_DIR}/PyExceptionContext.hpp
    ${CLP_FFI_PY_LIB_SRC_DIR}/PyObjectCast.hpp
    ${CLP_FFI_PY_LIB_SRC_DIR}/PyObjectUtils.hpp
    ${CLP_FFI_PY_LIB_SRC_DIR}/utils.cpp
    ${CLP_FFI_PY_LIB_SRC_DIR}/utils.hpp
)

# NOTE: These headers are third-party facade headers wrapped with IWYU exports to resolve clang-tidy
# violations for missing direct includes.
set(CLP_FFI_PY_WRAPPED_FACADE_HEADERS
    ${CLP_FFI_PY_WRAPPED_FACADE_HEADERS_DIR}/msgpack.hpp
    ${CLP_FFI_PY_WRAPPED_FACADE_HEADERS_DIR}/Python.hpp
)

target_sources(
    ${CLP_FFI_PY_LIB_IR}
    PRIVATE
        ${CLP_FFI_PY_CLP_CORE_SOURCES}
        ${CLP_FFI_PY_LIB_IR_SOURCES}
        ${CLP_FFI_PY_WRAPPED_FACADE_HEADERS}
)

# NOTE: We mark the include directories below as system headers so that the compiler (including
# `clang-tidy`) doesn't generate warnings from them.
target_include_directories(
    ${CLP_FFI_PY_LIB_IR}
    SYSTEM
    PRIVATE
        ${CLP_FFI_PY_CLP_CORE_DIR}/src
        ${CLP_FFI_PY_CLP_CORE_DIR}/submodules
)

target_include_directories(${CLP_FFI_PY_LIB_IR} PRIVATE ${CLP_FFI_PY_SRC_DIR})

if(CLP_FFI_PY_ENABLE_LINTING)
    target_compile_definitions(${CLP_FFI_PY_LIB_IR} PRIVATE CLP_FFI_PY_ENABLE_LINTING)
endif()

target_link_libraries(
    ${CLP_FFI_PY_LIB_IR}
    PRIVATE
        clp::string_utils
        Microsoft.GSL::GSL
        msgpack-cxx
)

if(CLP_FFI_PY_INSTALL_LIBS)
    install(TARGETS ${CLP_FFI_PY_LIB_IR} DESTINATION ${CLP_FFI_PY_PROJECT_NAME}/ir)
endif()
