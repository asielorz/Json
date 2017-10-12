#Json

[JSON][http://json.org] (JavaScript Object Notation) is a lightweight data-interchange format. It is easy for humans to read and write. It is easy for machines to parse and generate.

This is a library that allows easy manipulation of JSON values, as well as serialization to and from string.

##Documentation

Documentation of the API is generated using Doxygen.
TODO: upload and link documentation

## Dependencies

The code doesn't have any external dependencies appart from the C++ standard library. It is written in standard C++14 with no compiler extensions. It has been tested to compile and link correctly on Visual Studio 15 at warning level 4, Clang 5.0 with -Wall -Wextra -ansi -pedantic and cleanly passes Microsoft's static analyzer.

## Files

The project is self contained and can be directly copied to your project. It's composed by the files

	- config.hh
	- parser.cc
	- parser.hh
	- parser.inl
	- release.hh
	- release.inl
	- string_key.hh
	- string_key.inl
	- value.cc
	- value.hh
	- value.inl
	- writer.cc
	- writer.hh