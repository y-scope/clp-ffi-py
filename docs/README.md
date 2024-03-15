# Docs

This directory contains the files necessary to generate a Sphinx-based
documentation website for this package:

* `conf` - Configuration files
* `src` - The actual docs

## Requirements

* All the [requirements](../README.md#requirements) for building the Python
  package.
* [Node.js] >= 16 to be able to [view the output](#viewing-the-output)

## Build Commands

* Build the site incrementally:

  ```shell
  task docs:site
  ```
  
  * The output of the build will be in `../build/docs/html`.
  * API reference docs will be written to `src/api`.

* Clean up the build:

  ```shell
  task docs:clean
  ```

## Viewing the Output

```shell
task docs:serve
```

The command above will install [http-server] and serve the built docs site.

# Integration with [yscope-docs]

To support hosting on [docs.yscope.com], we need:

* `build.sh` which builds a clean version of the site and outputs it to
  `../build/docs/html`.
* `build.sh` must be runnable from any directory.

[docs.yscope.com]: https://docs.yscope.com
[http-server]: https://www.npmjs.com/package/http-server
[Node.js]: https://nodejs.org/en/download/current
[yscope-docs]: https://github.com/y-scope/yscope-docs
