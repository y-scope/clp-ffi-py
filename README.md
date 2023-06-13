# clp-ffi-py

This module provides Python packages to interface with [CLP Core Features][1]
through CLP's FFI (foreign function interface).
Currently, this library provides native methods to encode/decode log messages with [CLP][2].

## Compatibility

Tested on Python 3.8 and 3.10, and it should work on any Python version >= 3.6.

## Building/Packaging

To manually build a package for distribution, run the following step.
It should generate both .tar.gz package and .whl package under `./dist/`.

```bash
# 1. Create and enter a virtual environment
python -m venv venv && . ./venv/bin/activate

# 2. Install development dependencies:
pip install .[dev]

# 3. Pull all submodules in preparation for building:
git submodule update --init --recursive

# 4. Build:
python -m build
```

## Testing

```bash
# 1. Create and enter a virtual environment
python -m venv venv && . ./venv/bin/activate

# 2. Install development dependencies:
pip install .[dev]

# 3. Pull all submodules in preparation for building:
git submodule update --init --recursive

# 4. Install:
pip install -e .
```

## Build and Test with cibuildwheel

This project utilizes [cibuildwheel][7] configuration.
Whenever modifications are made and committed to Github,
the cibuildwheel Action will automatically initiate,
building this library for several Python environments across diverse OS and architectures.
You can access the build outcomes (wheel files) via the Github Action page.
To customize the build targets, please check cibuildwheel official documents.

## Contributing

Before submitting a pull request, run the following error-checking
and formatting tools (found in [pyproject.toml]):

* [mypy][3]: `mypy clp_ffi_py`
  * mypy checks for typing errors. You should resolve all typing errors or if an
    error cannot be resolved (e.g., it's due to a third-party library), you
    should add a comment `# type: ignore` to [silence][4] the error.
* [Black][5]: `black clp_ffi_py`
  * This formats the Python code according to Black's code-style rules. You should
    review and add any changes to your PR.
* [clang-format][6]: `clang-format -i src/clp_ffi/*`
  * This formats the C++ code according to the code-style rules specified in `.clang-format`.
    You should review and add any changes to your PR.

[1]: https://github.com/y-scope/clp/tree/main/components/core
[2]: https://github.com/y-scope/clp
[3]: https://mypy.readthedocs.io/en/stable/index.html
[4]: https://mypy.readthedocs.io/en/stable/common_issues.html#spurious-errors-and-locally-silencing-the-checker
[5]: https://black.readthedocs.io/en/stable/index.html
[6]: https://clang.llvm.org/docs/ClangFormatStyleOptions.html
[7]: https://cibuildwheel.readthedocs.io/en/stable/
