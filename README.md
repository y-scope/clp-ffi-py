# clp-ffi-py

This module provides Python packages to interface with [CLP Core Features][1]
through CLP's FFI (foreign function interface). At present, this library
supplies built-in functions for encoding/decoding log messages using [CLP][2].

> [!IMPORTANT]
> This project is no longer built for Python3.6.

## Quick Start

### Install with `pip`:

```bash
# Install the latest version
python3 -m pip install --upgrade clp-ffi-py
```

Note:

- Python 3.7 or higher is required.
- Tested on Linux, macOS and Windows.

To install an older version or download the prebuilt `whl` package, check the
project homepage on PyPI [here][16].

## Compatibility

Tested on Python 3.7, 3.8, 3.11 and 3.12, and it should work on any Python
version >= 3.7.

## API Reference

The API reference for this library can be found on our [docs hub][19].

## Building/Packaging

To manually build a package for distribution, follow the steps below.

### Requirements

* A C++ compiler that supports C++20 and `std::span`, e.g:
  * `clang++` >= 7
  * `g++` >= 10
  * `MSVC` >= 1930 (included in Visual Studio 2022)
* python3
* python3-dev
* python3-venv
* [Task][18] >= 3.35.1

### Build commands

* Build a Python wheel incrementally:

  ```bash
  task
  ```
  The command above will generate both a `.tar.gz` and `.whl` package under
  `./build/dist/`.

* Clean up the build:

  ```bash
  task clean
  ```

## CLP IR Readers

CLP IR Readers provide a convenient interface for CLP IR decoding and search
methods.

### ClpIrStreamReader

- Read/decode any arbitrary CLP IR stream (as an instance of `IO[bytes]`).
- Can be used as an iterator that returns each log event as a `LogEvent` object.
- Can search target log events by giving a search query:
  - Searching log events within a certain time range.
  - Searching log messages that match certain wildcard queries.

### ClpIrFileReader

- Simple wrapper around CLPIRStreamHandler that calls `open` with a given local
  path.

### Example Code: Using ClpIrFileReader to iterate and print log events

```python
from pathlib import Path
from clp_ffi_py.ir import ClpIrFileReader

with ClpIrFileReader(Path("example.clp.zst")) as clp_reader:
    for log_event in clp_reader:
        # Print the log message with its timestamp properly formatted.
        print(log_event.get_formatted_message())
```

Each log event is represented by a `LogEvent` object, which offers methods to
retrieve its underlying details, such as the timestamp and the log message. For
more information, use the following code to see all the available methods and
the associated docstring.

```python
from clp_ffi_py.ir import LogEvent
help(LogEvent)
```

### Example Code: Using Query to search log events by specifying a certain time range

```python
from typing import List

from clp_ffi_py.ir import ClpIrStreamReader, LogEvent, Query, QueryBuilder

# Create a QueryBuilder object to build the search query.
query_builder: QueryBuilder = QueryBuilder()

# Create a search query that specifies a time range by UNIX epoch timestamp in
# milliseconds. It will search from 2016.Nov.28 21:00 to 2016.Nov.29 3:00.
time_range_query: Query = (
    query_builder
    .set_search_time_lower_bound(1480366800000) # 2016.11.28 21:00
    .set_search_time_upper_bound(1480388400000) # 2016.11.29 03:00
    .build()
)

# A list to store all the log events within the search time range
log_events: List[LogEvent] = []

# Open IRstream compressed log file as a binary file stream, then pass it to
# CLpIrStreamReader.
with open("example.clp.zst", "rb") as compressed_log_file:
    with ClpIrStreamReader(compressed_log_file) as clp_reader:
        for log_event in clp_reader.search(time_range_query):
            log_events.append(log_event)
```

### Example Code: Using Query to search log messages of certain pattern(s) specified by wildcard queries.

```python
from pathlib import Path
from typing import List, Tuple

from clp_ffi_py.ir import ClpIrFileReader, Query, QueryBuilder
from clp_ffi_py.wildcard_query import FullStringWildcardQuery, SubstringWildcardQuery

# Create a QueryBuilder object to build the search query.
query_builder: QueryBuilder = QueryBuilder()

# Add wildcard patterns to filter log messages:
query_builder.add_wildcard_query(SubstringWildcardQuery("uid=*,status=failed"))
query_builder.add_wildcard_query(
    FullStringWildcardQuery("*UID=*,Status=KILLED*", case_sensitive=True)
)

# Initialize a Query object using the builder:
wildcard_search_query: Query = query_builder.build()
# Store the log events that match the criteria in the format:
# [timestamp, message]
matched_log_messages: List[Tuple[int, str]] = []

# A convenience file reader class is also available to interact with a file that
# represents an encoded CLP IR stream directly.
with ClpIrFileReader(Path("example.clp.zst")) as clp_reader:
    for log_event in clp_reader.search(wildcard_search_query):
        matched_log_messages.append((log_event.get_timestamp(), log_event.get_log_message()))
```

A `Query` object may have both the search time range and the wildcard queries
(`WildcardQuery`) specified to support more complex search scenarios.
`QueryBuilder` can be used to conveniently construct Query objects. For more
details, use the following code to access the related docstring.

```python
from clp_ffi_py.ir import Query, QueryBuilder
from clp_ffi_py import FullStringWildcardQuery, SubstringWildcardQuery, WildcardQuery
help(Query)
help(QueryBuilder)
help(WildcardQuery)
help(FullStringWildcardQuery)
help(SubstringWildcardQuery)
```

### Streaming Decode/Search Directly from S3 Remote Storage

When working with CLP IR files stored on S3-compatible storage systems,
[smart_open][17] can be used to open and read the IR stream for the following
benefits:

- It only performs stream operation and does not download the file to the disk.
- It only invokes a single `GET` request so that the API access cost is
  minimized.

Here is an example:

```python
from pathlib import Path
from clp_ffi_py.ir import ClpIrStreamReader

import boto3
import os
import smart_open

# Create a boto3 session by reading AWS credentials from environment variables.
session = boto3.Session(
    aws_access_key_id=os.environ['AWS_ACCESS_KEY_ID'],
    aws_secret_access_key=os.environ['AWS_SECRET_ACCESS_KEY'],
)

url = 's3://clp-example-s3-bucket/example.clp.zst'
# Using `smart_open.open` to stream the encoded CLP IR:
with smart_open.open(
    url, mode="rb", compression="disable", transport_params={"client": session.client("s3")}
) as istream:
    with ClpIrStreamReader(istream, allow_incomplete_stream=True) as clp_reader:
        for log_event in clp_reader:
            # Print the log message with its timestamp properly formatted.
            print(log_event.get_formatted_message())
```

Note:
- Setting `compression="disable"` is necessary so that `smart_open` doesn't
undo the IR file's Zstandard compression (based on the file's extension) before
streaming it to `ClpIrStreamReader`; `ClpIrStreamReader` expects the input
stream to be Zstandard-compressed.
- When `allow_incomplete_stream` is set to False (default), the reader will raise
`clp_ffi_py.ir.IncompleteStreamError` if the stream is incomplete (it doesn't end
with the byte sequence indicating the stream's end). In practice, this can occur
if you're reading a stream that is still being written or wasn't properly
closed.

### Parallel Processing

The `Query` and `LogEvent` classes can be serialized by [pickle][15]. Therefore,
decoding and search can be parallelized across streams/files using libraries
such as [multiprocessing][13] and [tqlm][14].

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
rather than installed locally (`pip install -e .`), the tester cannot be
launched from the project's root directory. If `unittest` is ran from the root
directory, the local `clp_ffi_py` directory will shadow the `clp_ffi_py` module
installed. To run the tester with the installed package, try the following:

```bash
cd tests
python -m unittest -bv
```

## Build and Test with cibuildwheel

This project utilizes [cibuildwheel][7] configuration. Whenever modifications
are made and committed to GitHub, the cibuildwheel Action will automatically
initiate, building this library for several Python environments across diverse
OS and architectures. You can access the build outcomes (wheel files) via the
GitHub Action page. For instructions on customizing the build targets or running
cibuildwheel locally, please refer to the official documentation of
cibuildwheel.

## Contributing

Before submitting a pull request, run the following error-checking and
formatting tools (found in [pyproject.toml]):

* [mypy][3]: `mypy clp_ffi_py`
  * mypy checks for typing errors. You should resolve all typing errors or if an
    error cannot be resolved (e.g., it's due to a third-party library), you
    should add a comment `# type: ignore` to [silence][4] the error.
* [docformatter][11]: `docformatter -i clp_ffi_py tests`
  * This formats docstrings. You should review and add any changes to your PR.
* [Black][5]: `black clp_ffi_py`
  * This formats the Python code according to Black's code-style rules. You
    should review and add any changes to your PR.
* [clang-format][6]: `clang-format -i src/clp_ffi_py/**`
  * This formats the C++ code according to the code-style rules specified in
    `.clang-format`. You should review and add any changes to your PR.
* [ruff][10]: `ruff check --fix clp_ffi_py tests`
  * This performs linting according to PEPs. You should review and add any
    changes to your PR.

Note that `docformatter` should be run before `black` to give Black the
[last][12].

Additionally, the following tools can be useful during development. However,
they cannot be installed using `pip`. Developers need to install them using
other package management tools such as `apt-get`:

* [clang-tidy][8]: `clang-tidy --extra-arg=-std=c++17 PATH_TO_THE_FILE`
  * This static analysis tool catches improper coding behaviors based on the
    rules specified in `.clang-tidy`, and sends suggestions corresponding to
    each warning. Developers should manually review all the warnings and try
    with their best effort to fix the reasonable ones.
* [bear][9]: `bear python setup.py build`
  * This tool generates a JSON compilation database on the project's root
    directory named `compile_commands.json`. This file will be used by
    clang-tidy to execute the static analysis. It also helps IDEs to configure
    code completion (such as VSCode).

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
[13]: https://docs.python.org/3/library/multiprocessing.html
[14]: https://tqdm.github.io/
[15]: https://docs.python.org/3/library/pickle.html
[16]: https://pypi.org/project/clp-ffi-py/
[17]: https://github.com/RaRe-Technologies/smart_open
[18]: https://taskfile.dev/installation/
[19]: https://docs.yscope.com/clp-ffi-py/main/api/clp_ffi_py.html
