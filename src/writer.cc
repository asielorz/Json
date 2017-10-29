#include "writer.hh"
#include "value.hh"
#include <algorithm> // count_if
 #include <cassert>

// Visual Studio supports C11 and therefore has sprintf_s. Besides, regular sprintf emits a warning in Visual Studio.
// gcc and clang use regular printf instead
#if _MSC_VER
#define string_printf sprintf_s
#else
#include <cstdio> // sprintf
#define string_printf std::sprintf
#endif

namespace json
{
	namespace writer
	{

		json::string write(const json::value & val, const json::string & indentation, size_t indentation_level);

		// Writes an integer as a string
		json::string write_int(int64_t i)
		{
			char buffer[32] = { 0 };

			// 64 bit integer is long long in Microsoft compiler and long in gcc and clang
			#if _MSC_VER
				const auto chars_written = string_printf(buffer, "%lld", i);
			#else
				const auto chars_written = string_printf(buffer, "%ld", i);
			#endif

			return json::string{ buffer, buffer + chars_written };
		}

		// Writes a double as a string
		json::string write_float(double d)
		{
			char buffer[64] = { 0 };
			const auto chars_written = string_printf(buffer, "%f", d);
			const char * end = buffer + chars_written;
			// Remove the trailing zeros (eg: 2.250000 becomes 2.25)
			while (*(end - 1) == '0')
				end--;

			return json::string{ static_cast<const char *>(buffer), end };
		}

		// Writes a boolean as a string
		json::string write_bool(bool b)
		{
			if (b)
				return "true";
			else
				return "false";
		}

		// True if c represents one of the possible control characters in a json string
		bool is_control_character(char c)
		{
			return 
				c == '"'  || c == '\\' || c == '/' || 
				c == '\b' || c == '\f' || c == '\n' || 
				c == '\r' || c == '\t';
		}

		json::string write_string(const char * begin, const char * end)
		{
			json::string out;
			// Reserve space for the string, the beginning and end quotation marks " and the backslashes \ of the control characters
			out.reserve(end - begin + 2 + std::count_if(begin, end, is_control_character));

			out += '\"';
			out.append(begin, end);

			// Process control characters of the string
			for (auto it = out.begin() + 1; it != out.end(); ++it)
			{
				const char c = *it;
				switch (c)
				{
					// For " and \ just append the control character \ before
				case '"':
				case '\\':
				{
					// Insert returns an iterator to the inserted character, so add 1 in order to skip the next
					// character as well. Otherwise it enters in an infinite loop as it checks the same control
					// character once and again
					it = out.insert(it, '\\') + 1;
					break;
				}
				case '\b':
				{
					// Append the \ before
					it = out.insert(it, '\\') + 1;
					// Change the control character for the printable character that represents it
					*it = 'b';
					break;
				}
				case '\f':
				{
					it = out.insert(it, '\\') + 1;
					*it = 'f';
					break;
				}
				case '\n':
				{
					it = out.insert(it, '\\') + 1;
					*it = 'n';
					break;
				}
				case '\r':
				{
					it = out.insert(it, '\\') + 1;
					*it = 'r';
					break;
				}
				case '\t':
				{
					it = out.insert(it, '\\') + 1;
					*it = 't';
					break;
				}
				}
			}

			out += '\"';

			return out;
		}

		// Writes a string as the representation of a string in a json source file
		json::string write_string(const json::string & str)
		{
			return write_string(str.c_str(), str.c_str() + str.size());
		}

		// Writes a string as the representation of a string in a json source file
		json::string write_string(const json::string_key & str)
		{
			return write_string(str.c_str(), str.c_str() + str.size());
		}

		// Computes the indentation corresponding to a given indentation level
		json::string indent(const json::string & indentation, size_t indentation_level)
		{
			json::string result;
			result.reserve(indentation.size() * indentation_level);

			for (size_t i = 0; i < indentation_level; ++i)
				result += indentation;

			return result;
		}

		// Writes an array in json format
		json::string write_array(const json::array & a, const json::string & indentation, size_t indentation_level)
		{
			json::string out;

			// [ to start array
			out += '[';

			// Write a newline only if the array is not empty so that empty array looks like []
			if (!a.empty())
				out += '\n';

			// Increment the indentation level
			indentation_level++;

			// Precompute body indentation to avoid computing it in a loop
			const auto body_indentation = indent(indentation, indentation_level);

			// Write every value of the array in a different line, separated with comma ,
			for (auto it = a.begin(); it != a.end(); ++it)
			{
				out += body_indentation;
				out += write(*it, indentation, indentation_level);
				// Write a comma only if this is not the last element
				if (it + 1 != a.end())
					out += ',';
				out += '\n';
			}

			indentation_level--;

			// Close the array with ]
			out += indent(indentation, indentation_level);
			out += ']';
			return out;
		}

		// Writes an object in json format
		json::string write_object(const json::object & a, const json::string & indentation, size_t indentation_level)
		{
			json::string out;

			// { to start object
			out += '{';

			// Write a newline only if the object is not empty so that empty object looks like {}
			if (!a.empty())
				out += '\n';

			// Increment the indentation level
			indentation_level++;

			// Precompute body indentation to avoid computing it in a loop
			const auto body_indentation = indent(indentation, indentation_level);

			// Write every value of the array in a different line, separated with comma ,
			for (auto it = a.begin(); it != a.end(); ++it)
			{
				out += body_indentation;
				out += write_string(it->first);
				out += " : ";
				if (it->second.is_array() || it->second.is_object())
				{
					out += '\n';
					out += body_indentation;
				}
				out += write(it->second, indentation, indentation_level);
				// Write a comma only if this is not the last element
				if (std::next(it) != a.end())
					out += ',';
				out += '\n';
			}

			indentation_level--;

			// Close the object with }
			out += indent(indentation, indentation_level);
			out += '}';

			return out;
		}

		json::string write(const json::value & val, const json::string & indentation, size_t indentation_level)
		{
			switch (val.type())
			{
				case value_type::null:
				{
					return "null";
				}
				case value_type::integer:
				{
					return write_int(val.as_int64());
				}
				case value_type::real:
				{
					return write_float(val.as_double());
				}
				case value_type::boolean:
				{
					return write_bool(val.as_bool());
				}
				case value_type::string:
				{
					return write_string(val.as_string());
				}
				case value_type::array:
				{
					return write_array(val.as_array(), indentation, indentation_level);
				}
				case value_type::object:
				{
					return write_object(val.as_object(), indentation, indentation_level);
				}
			}
			assert(false); // Control should never reach this point
			return{};
		}

		json::string write(const json::value & val, const json::string & indentation)
		{
			return write(val, indentation, 0);
		}

		json::string write(const json::object & val, const json::string & indentation)
		{
			return write_object(val, indentation, 0);
		}

		std::ostream & write(std::ostream & os, const json::value & val, const json::string & indentation)
		{
			return os << write(val, indentation);
		}

		std::ostream & write(std::ostream & os, const json::object & val, const json::string & indentation)
		{
			return os << write(val, indentation);
		}

	} // namespace writer
} // namespace json

std::ostream & operator << (std::ostream & os, const json::value & val)
{
	return json::writer::write(os, val);
}

std::ostream & operator << (std::ostream & os, const json::object & val)
{
	return json::writer::write(os, val);
}
