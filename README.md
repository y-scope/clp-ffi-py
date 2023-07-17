# clp-ffi-py

This module provides Python packages to interface with [CLP Core Features][1]
through CLP's FFI (foreign function interface).
At present, this library supplies built-in functions for encoding/decoding log
messages using [CLP][2].

## Compatibility

Tested on Python 3.8, 3.9 and 3.10, and it should work on any Python version >= 3.6.

## Building/Packaging

To manually build a package for distribution, run the following steps.
This process will generate both .tar.gz package and .whl package under `./dist/` directory.

```bash
# 1. Create and enter a virtual environment
python -m venv venv && . ./venv/bin/activate

# 2. Install development dependencies
pip install -r requirements-dev.txt

# 3. Pull all submodules in preparation for building
git submodule update --init --recursive

# 4. Build
python -m build
```

## Testing

```bash
# 1. Create and enter a virtual environment
python -m venv venv && . ./venv/bin/activate

# 2. Install development dependencies
pip install -r requirements-dev.txt

# 3. Pull all submodules in preparation for building
git submodule update --init --recursive

# 4. Install
pip install -e .

# 5. Run unit tests
python -m unittest -bv
```

## Build and Test with cibuildwheel

This project utilizes [cibuildwheel][7] configuration.
Whenever modifications are made and committed to GitHub,
the cibuildwheel Action will automatically initiate,
building this library for several Python environments across diverse OS and architectures.
You can access the build outcomes (wheel files) via the GitHub Action page.
For instructions on customizing the build targets or running cibuildwheel locally,
please refer to the official documentation of cibuildwheel.

## Contributing

Before submitting a pull request, run the following error-checking
and formatting tools (found in [pyproject.toml]):

* [mypy][3]: `mypy clp_ffi_py`
  * mypy checks for typing errors. You should resolve all typing errors or if an
    error cannot be resolved (e.g., it's due to a third-party library), you
    should add a comment `# type: ignore` to [silence][4] the error.
* [docformatter][11]: `docformatter -i clp_ffi_py tests`
  * This formats docstrings. You should review and add any changes to your PR.
* [Black][5]: `black clp_ffi_py`
  * This formats the Python code according to Black's code-style rules. You should
    review and add any changes to your PR.
* [clang-format][6]: `clang-format -i src/clp_ffi_py/**`
  * This formats the C++ code according to the code-style rules specified in `.clang-format`.
    You should review and add any changes to your PR.
* [ruff][10]: `ruff check --fix clp_ffi_py tests`
  * This performs linting according to PEPs. You should review and add any
    changes to your PR.

Note that `docformatter` should be run before `black` to give Black the [last][12].

Additionally, the following tools can be useful during development. However, they cannot be installed
using `pip`. Developers need to install them using other package management tools such as `apt-get`:

* [clang-tidy][8]: `clang-tidy --extra-arg=-std=c++17 PATH_TO_THE_FILE`
  * This static analysis tool catches improper coding behaviors based on the rules specified in
    `.clang-tidy`, and sends suggestions corresponding to each warning. Developers should manually
    review all the warnings and try with their best effort to fix the reasonable ones.
* [bear][9]: `bear python setup.py build`
  * This tool generates a JSON compilation database on the project's root directory named
    `compile_commands.json`. This file will be used by clang-tidy to execute the static analysis.
    It also helps IDEs to configure code completion (such as VSCode).

[1]: https://github.com/y-scope/clp/tree/main/components/core
[2]: https://github.com/y-scope/clp
[3]: https://mypy.readthedocs.io/en/stable/index.html
[4]: https://mypy.readthedocs.io/en/stable/common_issues.html#spurious-errors-and-locally-silencing-the-checker
[5]: https://black.readthedocs.io/en/stable/index.html
[6]: https://clang.llvm.org/docs/ClangFormatStyleOptions.html
[7]: https://cibuildwheel.readthedocs.io/en/stable/
[8]: https://clang.llvm.org/extra/clang-tidy/
[9]: https://github.com/rizsotto/Bear
[10]: https://beta.ruff.rs/docs/
[11]: https://docformatter.readthedocs.io/en/latest/
[12]: https://docformatter.readthedocs.io/en/latest/faq.html#interaction-with-black
