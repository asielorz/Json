#ifndef WRITER_HH_INCLUDE_GUARD
#define WRITER_HH_INCLUDE_GUARD

#include "config.hh"

namespace json
{
	class value;
	class object;

	namespace writer
	{

		//! Writes a json::value in json format to a string
		JSON_API json::string write(const json::value & val, const json::string & indentation = "\t");

		//! Writes a json::object in json format to a string
		JSON_API json::string write(const json::object & val, const json::string & indentation = "\t");

		//! Writes a json::value in json format to a standard stream
		//! Same as os << write(val, indentation)
		JSON_API std::ostream & write(std::ostream & os, const json::value & val, const json::string & indentation = "\t");
		//! Writes a json::object in json format to a standard stream
		//! Same as os << write(val, indentation)
		JSON_API std::ostream & write(std::ostream & os, const json::object & val, const json::string & indentation = "\t");

	} // namespace writer
} // namespace json

//! Writes a json::value in json format to a standard stream. Same as write(os, val)
JSON_API std::ostream & operator << (std::ostream & os, const json::value & val);
//! Writes a json::object in json format to a standard stream. Same as write(os, val)
JSON_API std::ostream & operator << (std::ostream & os, const json::object & val);

#endif // WRITER_HH_INCLUDE_GUARD

