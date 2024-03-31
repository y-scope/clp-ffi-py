import distutils.ccompiler
import multiprocessing.pool
import os
import platform
import sys
from setuptools import setup, Extension
from typing import List, Optional, Tuple

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
        "/std:c++20",
        "/O2,"
    ] if "nt" == os.name else [
        "-std=c++20",
        "-O3",
    ],
    define_macros=[("SOURCE_PATH_SIZE", str(len(os.path.abspath("./src/clp/components/core"))))],
)

def _parallel_compile(
    self: distutils.ccompiler.CCompiler,
    sources: List[str],
    output_dir: Optional[str] = None,
    macros: Optional[List[Tuple[str, Optional[str]]]] = None,
    include_dirs: Optional[List[str]] = None,
    debug: int = 0,
    extra_preargs: Optional[List[str]] = None,
    extra_postargs: Optional[List[str]] = None,
    depends: Optional[List[str]] = None,
) -> List[str]:
    """
    A replacement for distutils.ccompiler.CCompiler.compile that compiles each source file
    concurrently.
    """
    macros, objects, extra_postargs, pp_opts, build = self._setup_compile(
        output_dir, macros, include_dirs, sources, depends, extra_postargs
    )
    cc_args = self._get_cc_args(pp_opts, debug, extra_preargs)

    def _compile_single_file(obj: str) -> None:
        try:
            src, ext = build[obj]
        except KeyError:
            return
        self._compile(obj, src, ext, cc_args, extra_postargs, pp_opts)

    num_cores: int = multiprocessing.cpu_count()
    with multiprocessing.pool.ThreadPool(num_cores) as pool:
        pool.map(_compile_single_file, objects)
    return objects

if "__main__" == __name__:
    try:
        if "Darwin" == platform.system():
            target_env_var_name: str = "MACOSX_DEPLOYMENT_TARGET"
            min_target_version: Tuple[int, int] = (10, 15)
            
            target_str: Optional[str] = os.environ.get(target_env_var_name)
            target: Tuple[int, ...] = ()
            if target_str is not None:
                target = tuple(int(part) for part in target_str.split("."))
            if target < min_target_version:
                target = min_target_version
            os.environ[target_env_var_name] = ".".join(str(part) for part in target)

        distutils.ccompiler.CCompiler.compile = _parallel_compile
        project_name: str = "clp_ffi_py"
        description: str = "CLP FFI Python Interface"
        extension_modules: List[Extension] = [ir_native]
        if (3, 7) > sys.version_info:
            sys.exit("The minimum Python version required is Python3.7")
        else:
            setup(
                name=project_name,
                description=description,
                ext_modules=extension_modules,
                packages=["clp_ffi_py"],
            )

    except Exception as e:
        sys.exit(f"Error: {e}")
