import os
import platform
import sys
import toml
from setuptools import setup, Extension
from typing import Any, Dict, Optional

ir: Extension = Extension(
    name="clp_ffi_py.ir",
    language="c++",
    include_dirs=[
        "src",
        "src/GSL/include",
        "src/clp/components/core/submodules",
    ],
    sources=[
        "src/clp/components/core/src/ffi/ir_stream/decoding_methods.cpp",
        "src/clp/components/core/src/ffi/ir_stream/encoding_methods.cpp",
        "src/clp/components/core/src/ffi/encoding_methods.cpp",
        "src/clp/components/core/src/string_utils.cpp",
        "src/clp/components/core/src/TraceableException.cpp",
        "src/clp_ffi_py/ir/decoding_methods.cpp",
        "src/clp_ffi_py/ir/encoding_methods.cpp",
        "src/clp_ffi_py/ir/Metadata.cpp",
        "src/clp_ffi_py/ir/PyDecoder.cpp",
        "src/clp_ffi_py/ir/PyDecoderBuffer.cpp",
        "src/clp_ffi_py/ir/PyFourByteEncoder.cpp",
        "src/clp_ffi_py/ir/PyLogEvent.cpp",
        "src/clp_ffi_py/ir/PyMetadata.cpp",
        "src/clp_ffi_py/ir/PyQuery.cpp",
        "src/clp_ffi_py/ir/Query.cpp",
        "src/clp_ffi_py/modules/ir.cpp",
        "src/clp_ffi_py/Py_utils.cpp",
        "src/clp_ffi_py/utils.cpp",
    ],
    extra_compile_args=[
        "-std=c++17",
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

        config: Dict[str, Any] = toml.load("pyproject.toml")
        version: Optional[str] = config.get("project", {}).get("version", None)
        if None is version:
            sys.exit("Error: The version number was not found in pyproject.toml")

        setup(
            name="clp_ffi_py",
            description="CLP FFI Python Interface",
            ext_modules=[ir],
            packages=["clp_ffi_py"],
            version=version,
        )

    except Exception as e:
        sys.exit(f"Error: {e}")
