# Docs

This directory contains the files necessary to generate a Sphinx-based
documentation website for this package:

* `conf` - Configuration files
* `src` - The actual docs

## Requirements

We use the following tools to build the site:

* python3
* python3-venv
* [Task][1]

## Build Commands

* Build the site incrementally:

  ```shell
  task
  ```
  
  * The output of the build will be in `../build/docs/html`.
  * API reference docs will be written to `src/api`.

* Clean-up the build:

  ```shell
  task clean
  ```

## Viewing the Output

You can use [Node.js' npm][2] with [http-server][3] to view the output:

```shell
npx http-server ../build/docs/html -c-1
```

We use `-c-1` to disable caching during development.

# Integration with [yscope-docs][4]

To support hosting on [docs.yscope.com][5], we need:

* `build.sh` which builds a clean version of the site and outputs it to
  `../build/docs/html`.
* `build.sh` must be runnable from any directory.

[1]: https://taskfile.dev/
[2]: https://nodejs.org/en/download/current
[3]: https://www.npmjs.com/package/http-server
[4]: https://github.com/y-scope/yscope-docs
[5]: https://docs.yscope.com
