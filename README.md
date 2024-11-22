# clp-ffi-py

[![PyPI platforms][badge_pypi]][7]
[![Build status][badge_build_status]][clp_ffi_py_gh_actions]
[![Downloads][badge_total_downloads]][pepy/clp_ffi_py]
[![Downloads][badge_monthly_downloads]][pepy/clp_ffi_py]

This module provides Python packages to interface with [CLP Core Features][1]
through CLP's FFI (foreign function interface). At present, this library
supplies built-in functions for serializing/deserializing log messages using [CLP][2].

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
project homepage on PyPI [here][7].

## Compatibility

Tested on Python 3.7, 3.8, 3.11 and 3.12, and it should work on any Python
version >= 3.7.

## API Reference

The API reference for this library can be found on our [docs hub][10].

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
* [Task][9] >= 3.38.0

### Set up
* Initialize and update yscope-dev-utils submodules:
  ```shell
  git submodule update --init --recursive tools/yscope-dev-utils
  ```

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

CLP IR Readers provide a convenient interface for CLP IR deserialization and search
methods.

### ClpIrStreamReader

- Read+deserialize any arbitrary CLP IR stream (as an instance of `IO[bytes]`).
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
# represents a CLP IR stream directly.
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

### Streaming Deserialize/Search Directly from S3 Remote Storage

When working with CLP IR files stored on S3-compatible storage systems,
[smart_open][8] can be used to open and read the IR stream for the following
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
# Using `smart_open.open` to stream the CLP IR byte sequence:
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

The `Query` and `LogEvent` classes can be serialized by [pickle][6]. Therefore,
deserializing and searching can be parallelized across streams/files using libraries
such as [multiprocessing][4] and [tqlm][5].

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

This project utilizes [cibuildwheel][3] configuration. Whenever modifications
are made and committed to GitHub, the cibuildwheel Action will automatically
initiate, building this library for several Python environments across diverse
OS and architectures. You can access the build outcomes (wheel files) via the
GitHub Action page. For instructions on customizing the build targets or running
cibuildwheel locally, please refer to the official documentation of
cibuildwheel.

## Adding files
Certain file types need to be added to our linting rules manually:

- **CMake**. If adding a CMake file, add it (or its parent directory) as an argument to the
  `gersemi` command in [lint-tasks.yaml](lint-tasks.yaml).
  * If adding a directory, the file must be named `CMakeLists.txt` or use the `.cmake` extension.
- **YAML**. If adding a YAML file (regardless of its extension), add it as an argument to the
  `yamllint` command in [lint-tasks.yaml](lint-tasks.yaml).

## Linting
Before submitting a pull request, ensure you’ve run the linting commands below and either fixed any
violations or suppressed the warning.

To run all linting checks:
```shell
task lint:check
```

To run all linting checks AND automatically fix any fixable issues:
```shell
task lint:fix
```

### Running specific linters
The commands above run all linting checks, but for performance you may want to run a subset (e.g.,
if you only changed C++ files, you don't need to run the YAML linting checks) using one of the tasks
in the table below.

| Task                    | Description                                              |
|-------------------------|----------------------------------------------------------|
| `lint:cmake-check`      | Runs the CMake linters.                                  |
| `lint:cmake-fix`        | Runs the CMake linters and fixes any violations.         |
| `lint:cpp-check`        | Runs the C++ linters (formatters and static analyzers).  |
| `lint:cpp-fix`          | Runs the C++ linters and fixes some violations.          |
| `lint:cpp-format-check` | Runs the C++ formatters.                                 |
| `lint:cpp-format-fix`   | Runs the C++ formatters and fixes some violations.       |
| `lint:py-check`         | Runs the Python linters.                                 |
| `lint:py-fix`           | Runs the Python linters and fixes some violations.       |
| `lint:yml-check`        | Runs the YAML linters.                                   |
| `lint:yml-fix`          | Runs the YAML linters and fixes some violations.         |

[1]: https://github.com/y-scope/clp/tree/main/components/core
[2]: https://github.com/y-scope/clp
[3]: https://cibuildwheel.readthedocs.io/en/stable/
[4]: https://docs.python.org/3/library/multiprocessing.html
[5]: https://tqdm.github.io/
[6]: https://docs.python.org/3/library/pickle.html
[7]: https://pypi.org/project/clp-ffi-py/
[8]: https://github.com/RaRe-Technologies/smart_open
[9]: https://taskfile.dev/installation/
[10]: https://docs.yscope.com/clp-ffi-py/main/api/clp_ffi_py.html

[badge_build_status]: https://github.com/y-scope/clp-ffi-py/workflows/Build/badge.svg
[badge_monthly_downloads]: https://static.pepy.tech/badge/clp-ffi-py/month 
[badge_pypi]: https://badge.fury.io/py/clp-ffi-py.svg
[badge_total_downloads]: https://static.pepy.tech/badge/clp-ffi-py
[clp_ffi_py_gh_actions]: https://github.com/y-scope/clp-ffi-py/actions
[pepy/clp_ffi_py]: https://pepy.tech/project/clp-ffi-py
