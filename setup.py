import os
from setuptools import setup, Extension

ir: Extension = Extension(
    name="clp_ffi_py.ir",
    language="c++",
    include_dirs=[
        "src"
    ],
    sources=[
        "src/clp/components/core/src/ffi/ir_stream/decoding_methods.cpp",
        "src/clp/components/core/src/ffi/ir_stream/encoding_methods.cpp",
        "src/clp/components/core/src/ffi/encoding_methods.cpp",
        "src/clp/components/core/src/string_utils.cpp",
        "src/clp/components/core/src/TraceableException.cpp",

        "src/clp_ffi_py/ir/encoding_methods.cpp",
        "src/clp_ffi_py/ir/Metadata.cpp",
        "src/clp_ffi_py/ir/PyFourByteEncoder.cpp",
        "src/clp_ffi_py/ir/PyLogEvent.cpp",
        "src/clp_ffi_py/ir/PyMetadata.cpp",
        "src/clp_ffi_py/modules/ir.cpp",
        "src/clp_ffi_py/Py_utils.cpp",
        "src/clp_ffi_py/utils.cpp",
    ],
    extra_compile_args=[
        '-std=c++17',
        "-O3",
    ],
    define_macros=[
        ("SOURCE_PATH_SIZE", str(len(os.path.abspath("./src/clp/components/core"))))
    ]
)

setup(
    name="clp_ffi_py",
    description="CLP FFI Python Interface",
    ext_modules=[ir],
    packages=["clp_ffi_py"],
)
