#ifndef PARSER_HH_INCLUDE_GUARD
#define PARSER_HH_INCLUDE_GUARD

#include "config.hh"
#include "value.hh" // value, value_type
#include <stdexcept> // std::runtime_error
#include <cassert>

namespace json
{

	namespace parser
	{

		//! Possible token types a json source file may have
		enum class token_type
		{
			open_curly_brace,
			close_curly_brace,
			open_square_bracket,
			close_square_bracket,
			colon,
			comma,
			comment,
			string,
			number,
			boolean,
			null
		};

		//! A token is every individual piece of information within a json source file
		template <typename CharForwardIterator>
		struct token
		{
			token_type type;			//!< Type of the token
			CharForwardIterator begin;	//!< Location of the token data in the source string
			CharForwardIterator end;	//!< End of the token data in the source string
		};

		//! Exception class thrown by tokenize and build_value when encountering bad input
		class JSON_API syntax_error : public std::runtime_error
		{
		public:
			syntax_error(const char * msg)
				: runtime_error{ msg }
			{}
		};

		//! Parses the range of characters [begin, end) in json format into a sequence of lexical tokens.
		//! Tokens will contain pointers to the source range so it's important for it to live
		//! while the tokens are being used
		//! CharForwardIterator must satisfy ForwardIterator
		//! CharForwardIterator::value_type must be char
		template <typename CharForwardIterator>
		json::vector<token<CharForwardIterator>> tokenize(CharForwardIterator begin, CharForwardIterator end);
		//! Parses the whole string until a null character is found
		//! Same as tokenize(source, source + strlen(source))
		JSON_API json::vector<token<const char *>> tokenize(const char * source);
		//! Parses the string [source.begin(), source.end())
		//! Same as tokenize(source.c_str(), source.c_str() + source.size())
		JSON_API json::vector<token<const char *>> tokenize(const json::string & source);
		//! Tokens hold references to the source string so tokenizing a temporary string
		//! is explicitly forbidden
		JSON_API json::vector<token<const char *>> tokenize(const json::string && source) = delete;

		//! Builds a value from a sequence of tokens represented by the range [begin, end)
		//! TokenForwardIterator must satisfy ForwardIterator
		//! TokenForwardIterator::value_type must be token<T>
		template <typename TokenForwardIterator>
		json::value build_value(TokenForwardIterator begin, TokenForwardIterator end);
		//! Builds a value from a sequence of tokens represented by the range [begin, end)
		//! The last parameter outputs the position of the iterator, in case the whole range isn't used
		template <typename TokenForwardIterator>
		json::value build_value(TokenForwardIterator begin, TokenForwardIterator end, TokenForwardIterator & new_begin);

		//! Parses the string [begin, end) in json format into a value containing
		//! the data represented in the string
		template <typename CharForwardIterator>
		json::value parse(CharForwardIterator begin, CharForwardIterator end);
		//! Same as parse(source, source + strlen(source))
		JSON_API json::value parse(const char * source);
		//! Same as parse(source.c_str(), source.c_str() + source.size())
		JSON_API json::value parse(const json::string & source);
		//! Reads the whole stream, then parses that
		JSON_API json::value parse(std::istream & is);

		// These are the functions internally used by the tokenization and parsing algorithms.
		// Most of these functions make assumptions on input correctness in order to skip checks that
		// might already have been done in a previous step of the algorithm.
		namespace impl
		{

			//! Determines the token type of a piece of source code from its first character
			JSON_API token_type determine_token_type(char starting_char);
			//! True if a character represents a numeric value
			JSON_API bool is_number(char c);
			//! True if a character represents a whitespace
			JSON_API bool is_whitespace(char c);
			//! True if a character represents one of the six structural characters in json
			JSON_API bool is_structural(char c);
			//! True if a character represents a delimiter character.
			//! A delimiter character is any of the valid characters after a number or keyword literal
			JSON_API bool is_delimiter(char c);

			//! Returns the length of a comment token in the source
			template <typename CharForwardIterator>
			size_t comment_length(CharForwardIterator begin);

			//! Returns the length of a string token in the source
			template <typename CharForwardIterator>
			size_t string_length(CharForwardIterator begin);

			// Returns the length in characters of a number token in the source
			template <typename CharForwardIterator>
			size_t number_length(CharForwardIterator begin);

			//! Returns the length in characters of a keyword token.
			//! The only keywords in json are true, false and null
			template <typename CharForwardIterator>
			size_t keyword_length(CharForwardIterator begin);

			//! Returns the length of a token in the source given the position of the token and its type
			template <typename CharForwardIterator>
			size_t token_length(CharForwardIterator begin, token_type type);

			//! Advances begin until it's not pointing to a comment or until it reaches end
			template <typename TokenForwardIterator>
			TokenForwardIterator skip_comments(TokenForwardIterator begin, TokenForwardIterator end);

			//! Interprets the sequence of tokens [begin, end) as an object and returns it
			template <typename TokenForwardIterator>
			json::value parse_object(TokenForwardIterator begin, TokenForwardIterator end, TokenForwardIterator & new_begin);

			//! Counts the number of elements an array will have by inspecting its tokens so that memory
			//! for those elements can be allocated beforehand
			template <typename TokenForwardIterator>
			size_t count_array_elements(TokenForwardIterator begin, TokenForwardIterator end);

			//! Interprets the sequence of tokens [begin, end) as an array and returns it
			template <typename TokenForwardIterator>
			json::value parse_array(TokenForwardIterator begin, TokenForwardIterator end, TokenForwardIterator & new_begin);

			//! Converts a unicode code-point to UTF-8
			JSON_API json::string code_point_to_utf8(unsigned code_point);

			//! Constructs a string from a token that represents a string in the source
			template <typename CharForwardIterator>
			json::value parse_string(const token<CharForwardIterator> & t);

			//! True if the string represents a floating point number
			template <typename CharForwardIterator>
			bool represents_floating_point(CharForwardIterator begin, CharForwardIterator end);

			//! Converts the range of characters [begin, end) representing an integral number to the value it represents
			template <typename CharForwardIterator>
			int string_to_int(CharForwardIterator begin, CharForwardIterator end);

			//! Converts the range of characters [begin, end) representing a real number to the value it represents
			template <typename CharForwardIterator>
			double string_to_double(CharForwardIterator begin, CharForwardIterator end);

			//! Constructs a numeric value form a token that represents a number in the source
			template <typename CharForwardIterator>
			json::value parse_number(const token<CharForwardIterator> & t);

			//! Constructs a boolean value form a token that represents a boolean in the source
			template <typename CharForwardIterator>
			json::value parse_boolean(const token<CharForwardIterator> & t);

		} // namespace impl
	} // namespace parser
} // namespace json

//! Reads the whole stream, then parses that. Same as val = parse(is)
JSON_API std::istream & operator >> (std::istream & is, json::value & val);

#include "parser.inl"

#endif // PARSER_HH_INCLUDE_GUARD
