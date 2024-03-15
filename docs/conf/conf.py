# -- Project information -------------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = "clp-ffi-py"
# NOTE: We don't include a period after "Inc" since the theme adds one already.
copyright = "2023-2024 YScope Inc"

# -- General configuration -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    "myst_parser",
    "sphinx_design",
    "sphinx.ext.viewcode",
    "sphinx.ext.autodoc",
    "sphinx_copybutton",
]

# -- MyST extensions -----------------------------------------------------------
# https://myst-parser.readthedocs.io/en/stable/syntax/optional.html
myst_enable_extensions = [
    "colon_fence",
]

# -- Sphinx autodoc options ----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/extensions/autodoc.html#configuration

autoclass_content = "class"
autodoc_class_signature = "separated"
autodoc_typehints = "description"

# -- HTML output options -------------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_favicon = "https://docs.yscope.com/_static/favicon.ico"
html_title = "clp-ffi-py"
html_show_copyright = True

html_static_path = ["../src/_static"]

html_theme = "pydata_sphinx_theme"

# -- Theme options -------------------------------------------------------------
# https://pydata-sphinx-theme.readthedocs.io/en/stable/user_guide/layout.html

html_theme_options = {
    "footer_start": ["copyright"],
    "footer_center": [],
    "footer_end": ["theme-version"],
    "primary_sidebar_end": [],
    "secondary_sidebar_items": ["page-toc", "edit-this-page"],
    "show_prev_next": False,
    "use_edit_page_button": True,
}

# -- Theme source buttons ------------------------------------------------------
# https://pydata-sphinx-theme.readthedocs.io/en/stable/user_guide/source-buttons.html

html_context = {
    "github_user": "y-scope",
    "github_repo": "clp-ffi-py",
    "github_version": "main",
    "doc_path": "docs/src",
}

# -- Theme custom CSS and JS ---------------------------------------------------
# https://pydata-sphinx-theme.readthedocs.io/en/stable/user_guide/static_assets.html

def setup(app):
    app.add_css_file("custom.css")
