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

## CLP IR Readers

CLP IR Readers provide a convenient interface for CLP IR decoding and search methods.

### CLPIRStreamReader

- Read/decode any arbitrary CLP IR stream (as an instance of `IO[bytes]`).
- Can be used as an iterator that returns each log event as a `LogEvent` object.
- Can search target log events by giving a search query:
  - Searching log events within a certain time range.
  - Searching log messages that match certain wildcard queries.

### CLPIRFileReader

- Simple wrapper around CLPIRStreamHandler that calls open with a given local path.

### Example Code: Using CLPIRFileReader to iterate and print log events
```python
from pathlib import Path
from clp_ffi_py.readers import CLPIRFileReader

with CLPIRFileReader(Path("example.clp.zst")) as clp_reader:
    for log_event in clp_reader:
        # Print the log message with its timestamp properly formatted.
        print(log_event.get_formatted_message())
```
Each log event is represented by an `LogEvent` object, which offers methods to retrieve its
underlying details such as timestamp and log message. For more information, refer to the 
docstring of `LogEvent`.

### Example Code: Using Query to search log events by specifying a certain time range
```python
from pathlib import Path
from typing import List

from clp_ffi_py import LogEvent, Query
from clp_ffi_py.readers import CLPIRFileReader

# Create a search query that specifies a time range by UNIX epoch timestamp.
# It will search from 2016.Nov.28 21:00 to 2016.Nov.29 3:00
time_range_query: Query = Query(
    search_time_lower_bound=1480366800000,  # 2016.11.28 21:00
    search_time_upper_bound=1480388400000,  # 2016.11.29 03:00
)
# A list to store all the log events within the search time range
log_events: List[LogEvent] = []

with CLPIRFileReader(Path("example.clp.zst")) as clp_reader:
    for log_event in clp_reader.search(time_range_query):
        log_events.append(log_event)
```

### Example Code: Using Query to search log messages of certain pattern(s) specified by wildcard queries.
```python
from pathlib import Path
from typing import List, Tuple

from clp_ffi_py import Query, WildcardQuery
from clp_ffi_py.readers import CLPIRFileReader

# Generate a list of wildcard patterns to filter log messages:
wildcard_query_list: List[WildcardQuery] = [
    WildcardQuery("*uid=*,state=failed*"),
    WildcardQuery("*UID=*,Status=KILLED*", case_sensitive=True),
]
# Initialize a Query object with the list of wildcard patterns:
wildcard_search_query: Query = Query(wildcard_queries=wildcard_query_list)
# Store the log events that match the criteria in the format:
# [timestamp, message]
matched_log_messages: List[Tuple[int, str]] = []

with CLPIRFileReader(Path("example.clp.zst")) as clp_reader:
    for log_event in clp_reader.search(wildcard_search_query):
        matched_log_messages.append((log_event.get_timestamp(), log_event.get_log_message()))
```
A `Query` object may have both the search time range and the wildcard queries specified to support
more complex search scenarios. For more details, refer to the docstring of `Query`.

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

Note: If the package is installed from a `whl` file into the site packages,
rather than installed locally (`pip install -e .`),
the tester cannot be launched from the project's root directory.
If `unittest` is ran from the root directory,
the local `clp_ffi_py` directory will shadow the `clp_ffi_py` module installed.
To run the tester with the installed package, try the following:

```bash
cd tests
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
