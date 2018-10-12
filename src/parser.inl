namespace json
{
	namespace parser
	{
		namespace impl
		{

			// Returns the length of a comment
			template <typename CharForwardIterator>
			size_t comment_length(CharForwardIterator begin)
			{
				auto end = std::next(begin);

				// Check if we're dealing with a c comment or a c++ comment
				if (*end == '/')
				{
					while (*end != '\n' && *end != '\0')
						++end;
					return std::distance(begin, end);
				}
				else if (*end == '*')
				{
					while (*end != '\0' && (*end != '*' || *std::next(end) != '/'))
						++end;

					if (*end == '\0')
						throw detailed_syntax_error<CharForwardIterator>{ "Unclosed c style comment.", end };

					return std::distance(begin, end) + 2; // +2 to account the ending "*/". In the c++ comments we don't 
														  // add 1 because the newline is not part of the comment
				}
				else
				{
					throw detailed_syntax_error<CharForwardIterator>{ ("Unexpected character following '/'. Expected '/' or '*', found '" + std::string{ *std::next(end) } +"' instead.").c_str(), end };
				}
			}

			// Returns the length of a string token
			template <typename CharForwardIterator>
			size_t string_length(CharForwardIterator begin)
			{
				auto end = begin;
				// Json considers \" a control character representing " inside the string, so don't end the string if we find \"
				while (*std::next(end) != '\0' && !(*std::next(end) == '"' && !(*end == '\\')))
					++end;

				++end;

				if (*end == '\0')
					throw detailed_syntax_error<CharForwardIterator>{ "Unclosed string.", end };

				return std::distance(begin, end) + 1; // +1  because the ending " is part of the string
			}

			// Returns the length in characters of a number token
			template <typename CharForwardIterator>
			size_t number_length(CharForwardIterator begin)
			{
				auto end = std::next(begin);
				bool dot_read = false;
				bool exp_read = false;

				for (;; ++end)
				{
					if (is_number(*end))
						continue;
					else if (!dot_read && *end == '.')
						dot_read = true;
					else if (!exp_read && (*end == 'e' || *end == 'E'))
					{
						exp_read = true;
						// Account for unary plus and minus in the exponent
						if (*std::next(end) == '+' || *std::next(end) == '-')
							++end;
					}
					else if (is_delimiter(*end))
						break;
					else
						throw detailed_syntax_error<CharForwardIterator>{ "Unknown character found when parsing number literal.", end };
				}

				return std::distance(begin, end);
			}

			// Returns the length in characters of a keyword token.
			// The only keywords in json are true, false and null
			template <typename CharForwardIterator>
			size_t keyword_length(CharForwardIterator begin)
			{
				constexpr size_t length_of_true = 4;
				constexpr size_t length_of_false = 5;
				constexpr size_t length_of_null = 4;
				const char * const true_str = "true";
				const char * const false_str = "false";
				const char * const null_str = "null";

				if (*begin == 't')
				{
					const auto end = std::next(begin, length_of_true);
					if (std::equal(true_str, true_str + length_of_true, begin, end) && is_delimiter(*end))
						return length_of_true;
					else
						throw detailed_syntax_error<CharForwardIterator>{ "Unrecognized keyword.", begin };
				}
				else if (*begin == 'f')
				{
					const auto end = std::next(begin, length_of_false);
					if (std::equal(false_str, false_str + length_of_false, begin, end) && is_delimiter(*end))
						return length_of_false;
					else
						throw detailed_syntax_error<CharForwardIterator>{ "Unrecognized keyword.", begin };
				}
				const auto end = std::next(begin, length_of_null);
				if (std::equal(null_str, null_str + length_of_null, begin, end) && is_delimiter(*end))
					return length_of_null;
				else
					throw detailed_syntax_error<CharForwardIterator>{ "Unrecognized keyword.", begin };
			}

			// Returns the length of a token in the source given the position of the token and its type
			template <typename CharForwardIterator>
			size_t token_length(CharForwardIterator begin, token_type type)
			{
				switch (type)
				{
					case token_type::open_curly_brace:
					case token_type::close_curly_brace:
					case token_type::open_square_bracket:
					case token_type::close_square_bracket:
					case token_type::colon:
					case token_type::comma:
					{
						// Structural character tokens have a single character
						return 1;
					}
					case token_type::comment:
					{
						return comment_length(begin);
					}
					case token_type::string:
					{
						return string_length(begin);
					}
					case token_type::number:
					{
						return number_length(begin);
					}
					case token_type::boolean:
					case token_type::null:
					{
						return keyword_length(begin);
					}
					default:
					{
						// Control should never reach here
						assert(false);
						return 0;
					}
				}
			}

		} // namespace impl

		template <typename CharForwardIterator, typename TokenOutputIterator>
		TokenOutputIterator tokenize(CharForwardIterator begin, CharForwardIterator end, TokenOutputIterator out)
		{
			static_assert(std::is_same<typename
				std::iterator_traits<CharForwardIterator>::value_type,
				char>::value,
				"In function tokenize CharForwardIterator must dereference to char");
			
			using namespace ::json::parser::impl;

			while (begin != end)
			{
				const char & c = *begin;

				// Skip whitespace
				if (is_whitespace(c))
					++begin;
				else
				{
					const token_type type = determine_token_type(c);
					const size_t length = token_length(begin, type);
					const auto token_end = std::next(begin, length);
					*out++ = { type, begin, token_end };
					begin = token_end;
				}
			}

			return out;
		}

		template <typename CharForwardIterator>
		json::vector<token<CharForwardIterator>> tokenize(CharForwardIterator begin, CharForwardIterator end)
		{
			json::vector<token<CharForwardIterator>> tokens;
			// Pre allocate some memory to avoid frequent reallocations. The starting size is temptative,
			// and the function may reallocate depending on data quality, but it should very rarely need
			// more than two allocations.
			constexpr size_t average_token_to_char_ratio = 6; // This was decided by running tests on a set of data
			const size_t source_length = std::distance(begin, end);
			tokens.reserve(source_length / average_token_to_char_ratio);

			tokenize(begin, end, std::back_inserter(tokens));
			return tokens;
		}

		namespace impl
		{

			// Advances begin until it's not pointing to a comment or until it reaches end
			template <typename TokenForwardIterator>
			TokenForwardIterator skip_comments(TokenForwardIterator begin, TokenForwardIterator end)
			{
				while (begin != end && begin->type == token_type::comment)
					++begin;

				return begin;
			}

			// Interprets the sequence of tokens [begin, end) as an object and returns it
			template <typename TokenForwardIterator>
			json::value parse_object(TokenForwardIterator begin, TokenForwardIterator end, TokenForwardIterator & new_begin)
			{
				using token_t = typename std::iterator_traits<TokenForwardIterator>::value_type;
				using CharForwardIterator = typename token_t::char_iterator_type;

				json::value root{ value_type::object };

				// Save where the object starts so that we can use it to give more information if we have to throw
				const auto object_start = begin;

				// Increment begin to skip the '{' token
				++begin;

				// Check for comments and bad input
				begin = skip_comments(begin, end);
				if (begin == end)
					throw detailed_syntax_error<CharForwardIterator>{ "Too few tokens when parsing object", object_start->begin };

				// Check for empty object
				if (begin->type == token_type::close_curly_brace)
				{
					new_begin = std::next(begin, 1);
					return root;
				}

				for (;;)
				{
					// Structure of a member: string - colon - value - comma or }

					// Skip possible comments
					begin = skip_comments(begin, end);

					const auto key_token = begin;
					if (key_token->type != token_type::string)
						throw detailed_syntax_error<CharForwardIterator>{ "Non string value as member key for object.", key_token->begin };

					// Read the key
					json::string key = build_value(begin, end, begin).as_string();

					// Assert the next token is a colon
					begin = skip_comments(begin, end);
					if (begin == end || begin->type != token_type::colon)
						throw detailed_syntax_error<CharForwardIterator>{ "Expected : after member key when parsing object.", begin->begin };

					// Save colon token in order to be able to throw
					const auto colon_token = begin;
					
					// Skip the : as it contains no valuable information. We only need to assert it's there
					++begin;

					begin = skip_comments(begin, end);
					if (begin == end)
						throw detailed_syntax_error<CharForwardIterator>{ "Expected value after : when parsing object.", colon_token->begin };

					// Read the data of the member
					json::value member = build_value(begin, end, begin);

					// Try to insert new value
					if (root.as_object().insert(std::move(key), std::move(member)) == root.as_object().end())
						throw detailed_syntax_error<CharForwardIterator>{ ("Repeated member key \"" + key + "\" when parsing object").c_str(), key_token->begin };

					// Comments
					begin = skip_comments(begin, end);
					// Bad input
					if (begin == end)
						throw detailed_syntax_error<CharForwardIterator>{ "Too few tokens when parsing object.", object_start->begin };
					// Comma
					else if (begin->type == token_type::comma)
						++begin;
					// Last element of the array. Closing '}'
					else if (begin->type == token_type::close_curly_brace)
					{
						++begin;
						break;
					}
					// Bad input
					else
						throw detailed_syntax_error<CharForwardIterator>{ "End } for array not found", begin->begin };
				}

				new_begin = begin;
				return root;
			}

			// Counts the number of elements an array will have by inspecting its tokens so that memory
			// for those elements can be allocated beforehand
			template <typename TokenForwardIterator>
			size_t count_array_elements(TokenForwardIterator begin, TokenForwardIterator end)
			{
				size_t square_bracket_level = 1;
				size_t curly_brace_level = 0;
				size_t commas = 0;

				while (begin != end)
				{
					switch (begin->type)
					{
					case token_type::open_square_bracket:
						++square_bracket_level;
						break;
					case token_type::close_square_bracket:
						--square_bracket_level;
						break;
					case token_type::open_curly_brace:
						++curly_brace_level;
						break;
					case token_type::close_curly_brace:
						--curly_brace_level;
						break;
					// Add an element to the array for every comma at the scope of the array
					case token_type::comma:
						if (square_bracket_level == 1 && curly_brace_level == 0)
							++commas;
						break;
					default:
						break;
					}

					// End of object. Closing ] found
					if (square_bracket_level == 0)
						break;

					++begin;
				}

				return commas + 1; // +1 to account for the last object, which doesn't have a comma
			}

			template <typename TokenForwardIterator>
			json::value parse_array(TokenForwardIterator begin, TokenForwardIterator end, TokenForwardIterator & new_begin)
			{
				using token_t = typename std::iterator_traits<TokenForwardIterator>::value_type;
				using CharForwardIterator = typename token_t::char_iterator_type;

				json::value root{ value_type::array };
				json::array & array = root.as_array();

				// Save start in order we need it to throw
				const auto array_start = begin;

				// Increment begin to skip the '[' token
				++begin;

				// Check for bad input
				begin = skip_comments(begin, end);
				if (begin == end)
					throw detailed_syntax_error<CharForwardIterator>{ "Too few tokens when parsing array", array_start->begin };

				// Check for empty array
				if (begin->type == token_type::close_square_bracket)
				{
					new_begin = std::next(begin, 1);
					return root;
				}

				// Determine from the tokens the amount of elements in the array and preallocate the memory for them
				array.reserve(count_array_elements(begin, end));

				for (;;)
				{
					// Skip comments before the value
					begin = skip_comments(begin, end);

					// Parse the value
					array.push_back(build_value(begin, end, begin));

					// Comments after the value
					begin = skip_comments(begin, end);
					// Bad input
					if (begin == end)
						throw detailed_syntax_error<CharForwardIterator>{ "Too few tokens when parsing array", array_start->begin };
					// Comma
					else if (begin->type == token_type::comma)
						++begin;
					// Last element of the array. Closing ']'
					else if (begin->type == token_type::close_square_bracket)
					{
						++begin;
						break;
					}
					// Bad input
					else
						throw detailed_syntax_error<CharForwardIterator>{ "End ] for array not found", begin->begin };
				}

				new_begin = begin;
				return root;
			}

			// Constructs a string from a token that represents a string in the source
			template <typename CharForwardIterator>
			json::value parse_string(const token<CharForwardIterator> & t)
			{
				// Remove the first and last characters as the "" are not part of the string
				json::string str{ std::next(t.begin), t.end };
				str.pop_back(); // Use pop_back instead std::prev(t.end) in order to allow forward iterators

				// Scan for '\\' to look for control characters
				for (auto it = str.begin(); it != str.end(); ++it)
				{
					if (*it == '\\')
					{
						// Avoid dereferencing the end iterator
						const char next = it + 1 == str.end() ? '\0' : it[1];
						switch (next)
						{
							// For " and \ just erase the control character \ before
							case '"':
							case '\\':
							{
								it = str.erase(it);
								break;
							}
							case 'b':
							{
								it = str.erase(it);
								*it = '\b';
								break;
							}
							case 'f':
							{
								it = str.erase(it);
								*it = '\f';
								break;
							}
							case 'n':
							{
								it = str.erase(it);
								*it = '\n';
								break;
							}
							case 'r':
							{
								it = str.erase(it);
								*it = '\r';
								break;
							}
							case 't':
							{
								it = str.erase(it);
								*it = '\t';
								break;
							}
							case 'u':
							{
								const unsigned code_point = static_cast<unsigned>(std::stoi(std::string{ it + 1, it + 5 }, nullptr, 16));
								it = str.erase(it, it + 5);
								const auto code_point_in_utf8 = code_point_to_utf8(code_point);
								it = str.insert(it, code_point_in_utf8.begin(), code_point_in_utf8.end());
								break;
							}
							default:
							{
								throw detailed_syntax_error<CharForwardIterator>{ "Invalid control character", t.begin };
							}
						}
					}
				}

				return std::move(str);
			}

			// True if the string represents a floating point number
			template <typename CharForwardIterator>
			bool represents_floating_point(CharForwardIterator begin, CharForwardIterator end)
			{
				// A string represents a floating point if it has a fraction part '.' or a exponent ('e' or 'E')
				return
					std::find(begin, end, '.') != end ||
					std::find(begin, end, 'e') != end ||
					std::find(begin, end, 'E') != end;
			}

			// Converts the range of characters [begin, end) representing an integral number to the value it represents
			template <typename CharForwardIterator>
			int64_t string_to_int(CharForwardIterator begin, CharForwardIterator end)
			{
				// Using ptrdiff_t instead of size_t to avoid signed/unsigned mismatch warning on the comparison of the assertion
				constexpr ptrdiff_t buffer_size = 32;
				const auto string_size = std::distance(begin, end);
				assert(string_size < buffer_size);

				char buffer[buffer_size] = { 0 };
				std::copy(begin, end, buffer);
				int64_t value;
				std::from_chars(buffer, buffer + string_size, value);
				return value;
			}

			// Converts the range of characters [begin, end) representing a real number to the value it represents
			template <typename CharForwardIterator>
			double string_to_double(CharForwardIterator begin, CharForwardIterator end)
			{
				constexpr ptrdiff_t buffer_size = 64;
				assert(std::distance(begin, end) < buffer_size);

				char buffer[buffer_size] = { 0 };
				std::copy(begin, end, buffer);
				return atof(buffer);
			}

			// Constructs a numeric value form a token that represents a number in the source
			template <typename CharForwardIterator>
			json::value parse_number(const token<CharForwardIterator> & t)
			{
				if (represents_floating_point(t.begin, t.end))
					return string_to_double(t.begin, t.end);
				else
					return string_to_int(t.begin, t.end);
			}

			// Constructs a boolean value form a token that represents a boolean in the source
			template <typename CharForwardIterator>
			json::value parse_boolean(const token<CharForwardIterator> & t)
			{
				// The string [begin, end) can only be "true" or "false", bad input was discarded
				// in tokenization step, so checking the first letter is enough
				return bool{ *t.begin == 't' };
			}
			
			template <typename T>
			struct is_token : public std::false_type
			{};

			template <typename CharForwardIterator>
			struct is_token<token<CharForwardIterator>> : public std::true_type
			{};

		} // namespace impl
		
		template <typename TokenForwardIterator>
		json::value build_value(TokenForwardIterator begin, TokenForwardIterator end)
		{
			TokenForwardIterator dummy;
			return build_value(begin, end, dummy);
		}

		template <typename TokenForwardIterator>
		json::value build_value(TokenForwardIterator begin, TokenForwardIterator end, TokenForwardIterator & new_begin)
		{
			// Assert that TokenIterator dereferences to a token
			static_assert(
				impl::is_token<typename
					std::iterator_traits<TokenForwardIterator>::value_type>::value,
				"In function build_value TokenForwardIterator must dereference to json::parser::token");

			using token_t = typename std::iterator_traits<TokenForwardIterator>::value_type;
			using CharForwardIterator = typename token_t::char_iterator_type;

			using namespace ::json::parser::impl;

			begin = skip_comments(begin, end);

			if (begin == end)
				throw syntax_error{ "Error at parsing empty string" };

			switch (begin->type)
			{
				case token_type::open_curly_brace:
				{
					return parse_object(begin, end, new_begin);
				}
				case token_type::open_square_bracket:
				{
					return parse_array(begin, end, new_begin);
				}
				case token_type::string:
				{
					new_begin = std::next(begin, 1);
					return parse_string(*begin);
				}
				case token_type::number:
				{
					new_begin = std::next(begin, 1);
					return parse_number(*begin);
				}
				case token_type::boolean:
				{
					new_begin = std::next(begin, 1);
					return parse_boolean(*begin);
				}
				case token_type::null:
				{
					new_begin = std::next(begin, 1);
					return json::value{};
				}
				default:
					throw detailed_syntax_error<CharForwardIterator>{ "Invalid token type at the beginning of a value.", begin->begin };
			}
		}

		template <typename CharForwardIterator>
		json::value parse(CharForwardIterator begin, CharForwardIterator end)
		{
			try
			{
				const auto tokens = tokenize(begin, end);
				typename std::vector<token<CharForwardIterator>>::const_iterator tokens_end;
				json::value root = build_value(tokens.begin(), tokens.end(), tokens_end);

				// A json file may have comments after the value
				tokens_end = impl::skip_comments(tokens_end, tokens.end());

				// This will assert the file only contains a single value
				if (tokens_end != tokens.end())
					throw syntax_error{ "More than one value in the string" };

				return root;
			}
			catch (const impl::detailed_syntax_error<CharForwardIterator> & error)
			{
				throw impl::generate_meaningful_syntax_error(error, begin);
			}
		}

		namespace impl
		{

			template <typename CharForwardIterator>
			detailed_syntax_error<CharForwardIterator> generate_meaningful_syntax_error(const detailed_syntax_error<CharForwardIterator> & error, CharForwardIterator source_begin)
			{
				size_t row = 1;
				size_t column = 1;

				const auto error_location = error.where();
				for (auto it = source_begin; it != error_location; ++it)
				{
					if (*it == '\n')
					{
						++row;
						column = 1;
					}
					else
						++column;
				}

				std::string what_msg = error.what();
				what_msg += "\nRow: ";
				what_msg += std::to_string(row);
				what_msg += "\nColumn: ";
				what_msg += std::to_string(column);

				return detailed_syntax_error<CharForwardIterator>(what_msg.c_str(), error_location);
			}

		} // namespace impl
	} // namespace parser
} // namespace json

