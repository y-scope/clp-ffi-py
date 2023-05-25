import os
from setuptools import setup, Extension

clp_four_byte_encoder: Extension = Extension(
    name="clp_ffi_py.CLPFourByteEncoder",
    language="c++",
    sources=[
        "src/clp/components/core/src/ffi/ir_stream/encoding_methods.cpp",
        "src/clp/components/core/src/ffi/encoding_methods.cpp",
        "src/clp/components/core/src/string_utils.cpp",
        "src/clp/components/core/src/TraceableException.cpp",
        "src/clp_ffi/clp_four_byte_encoder.cpp",
        "src/clp_ffi/encoding_method.cpp"
    ],
    extra_compile_args=[
        '-std=c++17',
        "-O3"
    ],
    define_macros=[
        ("SOURCE_PATH_SIZE", str(len(os.path.abspath("./src/clp/components/core"))))
    ]
)


setup(
    name="clp_ffi_py",
    version="0.0",
    description="CLP FFI Python Interface",
    ext_modules=[clp_four_byte_encoder],
    packages=["clp_ffi_py"],
)
