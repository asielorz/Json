# Json

[JSON][json-org] (JavaScript Object Notation) is a lightweight data-interchange format. It is easy for humans to read and write. It is easy for machines to parse and generate.

[json-org]: http://json.org/

This is a library that allows easy manipulation of JSON values, as well as serialization to and from string.

## Documentation

[Documentation of the API][doc-link] is generated using Doxygen.

[doc-link]: https://asielorz.github.io/Json/files.html

## Dependencies

The code doesn't have any external dependencies appart from the C++ standard library. It is written in standard C++17 with no compiler extensions. It has been tested to compile and link correctly on Visual Studio 17 at warning level 4, Clang 5.0 with -Wall -Wextra -ansi -pedantic and cleanly passes Microsoft's static analyzer.

## Files

The project is self contained and can be directly copied to your project. It's composed by the files

  - config.hh

Defines the allocator type used by all the API, and the macro JSON_API which is appended to all functions and classes for DLL_IO.
  - parser.cc
  - parser.hh
  - parser.inl
  
  Functions and classes necessary to build a json::value from a string in JSON format.
  - release.hh
  - release.inl

Some utility functions internally used by string_key.
  - string_key.hh
  - string_key.inl

String class especially designed to be used as a key of a map. Used by json::object as key of it's map. Can be used on it's own.
  - value.cc
  - value.hh
  - value.inl

Class that represents a json::value. This is, a variant type that may be a number, a string, a boolean, an ordered n-tuple of json::values, an unordered set of string-json::value pairs, or null.
  - writer.cc
  - writer.hh

No specific build process is required.

## Other files

The file tests.cc contains a main function that will automatically run several unit tests on the code. These tests cover all the behavour of all the classes and functions of the API.

The file examples.cc contains some code showcasing basic functionality of the API.

The file json.natvis contains native visualizers for the classes of the API in the Visual Studio debugger. It is extremely recommended to include this file in your project if you are using this json in Visual Studio, as it will make it much easier to debug.

## License

See the `LICENSE` file for details. In summary, Json is licensed under the MIT license, or public domain if desired and recognized in your jurisdiction.
