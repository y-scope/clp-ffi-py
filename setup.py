import os
import platform
import sys
from setuptools import setup, Extension
from typing import List, Optional

ir_native: Extension = Extension(
    name="clp_ffi_py.ir.native",
    language="c++",
    include_dirs=[
        "src",
        "src/clp/components/core/submodules",
    ],
    sources=[
        "src/clp/components/core/src/BufferReader.cpp",
        "src/clp/components/core/src/ffi/ir_stream/decoding_methods.cpp",
        "src/clp/components/core/src/ffi/ir_stream/encoding_methods.cpp",
        "src/clp/components/core/src/ffi/encoding_methods.cpp",
        "src/clp/components/core/src/ir/parsing.cpp",
        "src/clp/components/core/src/ReaderInterface.cpp",
        "src/clp/components/core/src/string_utils.cpp",
        "src/clp/components/core/src/TraceableException.cpp",
        "src/clp_ffi_py/ir/native/decoding_methods.cpp",
        "src/clp_ffi_py/ir/native/encoding_methods.cpp",
        "src/clp_ffi_py/ir/native/Metadata.cpp",
        "src/clp_ffi_py/ir/native/PyDecoder.cpp",
        "src/clp_ffi_py/ir/native/PyDecoderBuffer.cpp",
        "src/clp_ffi_py/ir/native/PyFourByteEncoder.cpp",
        "src/clp_ffi_py/ir/native/PyLogEvent.cpp",
        "src/clp_ffi_py/ir/native/PyMetadata.cpp",
        "src/clp_ffi_py/ir/native/PyQuery.cpp",
        "src/clp_ffi_py/ir/native/Query.cpp",
        "src/clp_ffi_py/modules/ir_native.cpp",
        "src/clp_ffi_py/Py_utils.cpp",
        "src/clp_ffi_py/utils.cpp",
    ],
    extra_compile_args=[
        "-std=c++20",
        "-O3",
    ],
    define_macros=[("SOURCE_PATH_SIZE", str(len(os.path.abspath("./src/clp/components/core"))))],
)

if "__main__" == __name__:
    try:
        if "Darwin" == platform.system():
            target: Optional[str] = os.environ.get("MACOSX_DEPLOYMENT_TARGET")
            if None is target or float(target) < 10.15:
                os.environ["MACOSX_DEPLOYMENT_TARGET"] = "10.15"

        project_name: str = "clp_ffi_py"
        description: str = "CLP FFI Python Interface"
        extension_modules: List[Extension] = [ir_native]
        if (3, 7) > sys.version_info:
            sys.exit(f"The minimum Python version required is Python3.7")
        else:
            setup(
                name=project_name,
                description=description,
                ext_modules=extension_modules,
                packages=["clp_ffi_py"],
            )

    except Exception as e:
        sys.exit(f"Error: {e}")
