# libsvg - A simple Library for creating, editing, reading and writing SVG files.

A C language library primarily designed for creating and editing SVG documents programmatically.  This library provides an API in the same spirit as libxml2 or gtk+.

While aiming to provide a full featured API supporting all elements and attributes of SVG, no attempt is made to be fully compliant with W3C standards.

As of 1/7/2025, not all features and elements are fully supported.  Most specifically SVG elements \<defs\> and \<g\> are not yet supported.  Many, if not most, experimental and newer elements may not be supported.

NOTE:  As of 2/20/2025, there is no reference code for creating an SVG document programmatically.
<a id="TOC"></a>
## Table of contents

* [Introduction](#introduction)
* [Installation](#installation)
* [Library](#library)
* [Known issues and limitations](#known-issues-and-limitations)
* [Getting help](#getting-help)
* [License](#license)

<a id="introduction"></a>
## Introduction

@b svg is a C language library that manages SVG documents.  @b svg is reasonably complete and compliant with W3C standards.  @b svg is NOT intended to be totally XML standards compliant.  @b SVG is for use in projects where reading and writing SVG documents in a lightweight and simple manner is desirable.

[Back to Table of Contents](#TOC)

<a id="installation"></a>
## Installation

A reasonably modern version of GNU Autotools should be installed before installing @b xml from the source files.  At the time of writing this document, Autotools version 2.71 were used.

To compile the source code, the standard build tools are required, such as GCC, binutils, make, etc.

If you want to create the Doxygen API documentation, then of course Doxygen and friends need to installed.

If you have obtained the @b svg source code from a GIT repository, then execute the `autoreconf` program while in the directory that the @b svg GIT clone is located.

For both GIT clones and GNU Package tarballs, run the usual `./configure` followed by `make`.

To install the built **libsvg** library, run `sudo make install`.

[Back to Table of Contents](#TOC)

<a id="library"></a>
## Library

A static library named libsvg.a should be installed.
To compile your code using libxml.a:

&nbsp;&nbsp;Run `cc $(pkg-config --cflags --libs svg) -o <yourprogram> <yourprogram>.c`

[Back to Table of Contents](#TOC)

<a id="known-issues-and-limitations"></a>
## Known issues and limitations

At the time of writing of this document, there are no known issues with the @b svg software.

[Back to Table of Contents](#TOC)

<a id="getting-help"></a>
## Getting help

If you encounter any bugs or require additional help, contact [Patrick Head](mailto:patrickhead@gmail.com)

[Back to Table of Contents](#TOC)

<a id="license"></a>
## License

All code and configuration data for the **libsvg** software is licensed under the [LGPLv3](https://www.gnu.org/licenses/lgpl-3.0.en.html) or later.   See **COPYING** for details.

_This_ README file is itself distributed under the terms of the [Creative Commons 1.0 Universal license (CC0)](https://creativecommons.org/publicdomain/zero/1.0/).

[Back to Table of Contents](#TOC)

