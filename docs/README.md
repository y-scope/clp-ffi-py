# Docs

This directory contains the files necessary to generate a Sphinx-based
documentation website for this package:

* `conf` - Configuration files
* `src` - The actual docs

## Requirements

* All the [requirements](../README.md#requirements) for building the Python
  package.
* [Node.js] >= 16 to be able to [view the output](#viewing-the-output)
* Python 3.10 or later

## Build Commands

* Build the site incrementally:

  ```shell
  task docs:site
  ```
  
  * The output of the build will be in `../build/docs/html`.

* Clean up the build:

  ```shell
  task docs:clean
  ```

* Generate API docs:

  ```shell
  task docs:api-docs
  ```
  
  * This will regenerate the API docs in `src/api`. You should run this whenever you change the
    modules or package that exist in `clp-ffi-py`, and then commit the changes.
  * This will overwrite any existing API doc files. This is only a problem if any of the previously
    generated files were manually customized (e.g., to reformat some docs). You should
    review the changes carefully to make sure any customizations have not been mistakenly deleted.
  * This will *not* remove docs for packages/modules that no longer exist. You should make sure to
    delete those docs from `src/api`. In addition, ensure that:
    * in the case of a rename, any previous customizations are retained.
    * no unrelated files (e.g., indexes) are deleted.

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
