import os
import platform
import sys
import toml
from setuptools import setup, Extension
from typing import Any, Dict, List, Optional

ir_native: Extension = Extension(
    name="clp_ffi_py.ir.native",
    language="c++",
    include_dirs=[
        "src",
        "src/GSL/include",
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

        project_name: str = "clp_ffi_py"
        description: str = "CLP FFI Python Interface"
        extension_modules: List[Extension] = [ir_native]
        if (3, 7) > sys.version_info:
            # For Python3.6, setuptools doesn't automatically include submodules
            # and .pyi/.type files, so we need to explicitly specify the
            # packages and the files to include per package (package_data).
            packages: List[str] = ["clp_ffi_py", "clp_ffi_py.ir"]
            data_to_include: List[str] = ["*.py", "*.pyi", "*.typed"]
            package_data: Dict[str, List[str]] = {package: data_to_include for package in packages}
            setup(
                name=project_name,
                description=description,
                ext_modules=extension_modules,
                packages=packages,
                package_data=package_data,
                version=version,
            )
        else:
            setup(
                name=project_name,
                description=description,
                ext_modules=extension_modules,
                packages=["clp_ffi_py"],
                version=version,
            )

    except Exception as e:
        sys.exit(f"Error: {e}")
