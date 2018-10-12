#include "parser.hh"
#include <algorithm>
#include <sstream>

namespace json
{
	namespace parser
	{
		namespace impl
		{

			// Determines the token type of a piece of source code from its first character
			token_type determine_token_type(char starting_char)
			{
				// Lookup table mapping each char with the token type that starts with that char
				static const std::pair<char, token_type> map[] = {
					{ '"', token_type::string }, { ',', token_type::comma }, { '-', token_type::number },
					{ '/', token_type::comment }, { '0', token_type::number }, { '1', token_type::number },
					{ '2', token_type::number }, { '3', token_type::number }, { '4', token_type::number },
					{ '5', token_type::number }, { '6', token_type::number }, { '7', token_type::number },
					{ '8', token_type::number }, { '9', token_type::number }, { ':', token_type::colon },
					{ '[', token_type::open_square_bracket }, { ']', token_type::close_square_bracket },
					{ 'f', token_type::boolean }, { 'n', token_type::null }, { 't', token_type::boolean },
					{ '{', token_type::open_curly_brace }, { '}', token_type::close_curly_brace }
				};

				const auto compare_keys = [](const std::pair<char, token_type> & a, char key)
				{
					return a.first < key;
				};

				// Look if there's a possible token type for the given char
				auto it = std::lower_bound(std::begin(map), std::end(map), starting_char, compare_keys);
				// If there is, return it
				if (it->first == starting_char)
					return it->second;
				// If not, there is a syntax error
				else
					throw syntax_error{ ("Character " + std::string{ starting_char } + " could not be recognized.").c_str() };
			}

			// True if a character represents a numeric value
			bool is_number(char c)
			{
				return c >= '0' && c <= '9';
			}

			// True if a character represents a whitespace
			bool is_whitespace(char c)
			{
				return c == '\t' || c == '\n' || c == '\r' || c == ' ';
			}

			// True if a character represents one of the six structural characters in json
			bool is_structural(char c)
			{
				return c == '{' || c == '}' || c == '[' || c == ']' || c == ':' || c == ',';
			}

			// True if a character represents a delimiter character.
			// A delimiter character is a valid character after a number or keyword literal
			bool is_delimiter(char c)
			{
				return is_whitespace(c) || is_structural(c) || c == '/' || c == '\0';
			}

		}

		json::vector<token<const char *>> tokenize(std::string_view source)
		{
			return tokenize(source.data(), source.data() + source.size());
		}

		namespace impl
		{
			// Converts a unicode code-point to UTF-8
			json::string code_point_to_utf8(unsigned code_point)
			{
				json::string result;

				// Based on description from http://en.wikipedia.org/wiki/UTF-8

				if (code_point <= 0x7f)
				{
					result.resize(1);
					result[0] = static_cast<char>(code_point);
				}
				else if (code_point <= 0x7FF)
				{
					result.resize(2);
					result[1] = static_cast<char>(0x80 | (0x3f & code_point));
					result[0] = static_cast<char>(0xC0 | (0x1f & (code_point >> 6)));
				}
				else if (code_point <= 0xFFFF)
				{
					result.resize(3);
					result[2] = static_cast<char>(0x80 | (0x3f & code_point));
					result[1] = static_cast<char>(0x80 | (0x3f & (code_point >> 6)));
					result[0] = static_cast<char>(0xE0 | (0xf & (code_point >> 12)));
				}
				else if (code_point <= 0x10FFFF)
				{
					result.resize(4);
					result[3] = static_cast<char>(0x80 | (0x3f & code_point));
					result[2] = static_cast<char>(0x80 | (0x3f & (code_point >> 6)));
					result[1] = static_cast<char>(0x80 | (0x3f & (code_point >> 12)));
					result[0] = static_cast<char>(0xF0 | (0x7 & (code_point >> 18)));
				}

				return result;
			}

		} // namespace detail

		json::value parse(std::string_view source)
		{
			return parse(source.data(), source.data() + source.size());
		}

		json::value parse(std::istream & is)
		{
			std::basic_ostringstream<char, std::char_traits<char>, json::allocator<char>> oss;
			oss << is.rdbuf();
			return parse(oss.str());
		}

	} // namespace parser
} // namespace json

std::istream & operator >> (std::istream & is, json::value & val)
{
	val = json::parser::parse(is);
	return is;
}

