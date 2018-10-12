#ifndef STRING_KEY_HH_INCLUDE_GUARD
#define STRING_KEY_HH_INCLUDE_GUARD

#include "config.hh"
#include "release.hh"
#include <algorithm> // search, find_first_of, lexicographical_compare
#include <string_view>

namespace json
{

	//! Representation of a constant string that will outlive the json::value that is using it (usually a string literal).
	//! A json::value can use static strings as keys for objects in order to avoid allocations if the user knows
	//! for sure that the string will not be destroyed while the value is being used.
	template <
		typename CharT,
		typename Traits = std::char_traits<CharT>
	>
	class basic_static_string
	{
	public:
		//! Constructs a static_string from a pointer to a c string. A static_string is just a view to that string.
		explicit constexpr basic_static_string(const CharT * s) noexcept;

		//! Constructs a static_string from a pointer to a c string and its size. A static_string is just a view to that string.
		constexpr basic_static_string(const CharT * s, size_t l) noexcept;

		//! Returns the pointer to the string passed in the constructor.
		constexpr const CharT * c_str() const noexcept;

		//! Returns the length of the string, without counting the null temrinator.
		constexpr size_t size() const noexcept;

		//! Conversion to string view
		constexpr operator std::basic_string_view<CharT, Traits>() const noexcept;

	private:
		const CharT * str;
		size_t str_size;
	};

	//! Typedef for basic_static_string of chars (the same way std::string is std::basic_string<char>)
	using static_string = basic_static_string<char>;
	//! Typedef for basic_static_string of wchar_t
	using static_wstring = basic_static_string<wchar_t>;
	//! Typedef for basic_static_string of char16_t
	using static_u16string = basic_static_string<char16_t>;
	//! Typedef for basic_static_string of char32_t
	using static_u32string = basic_static_string<char32_t>;

	//! Representation of a string key for an associative container. It's used as the key of the map
	//! in json::object. Supports standard traits and allocators, as well as transparent comparison
	//! with c strings and std::basic_strings. If constructed from a static_string, the class will keep
	//! a reference to the original string instead of allocating a copy, acting as an string_view, thus
	//! saving a heap operation if the user is sure that the static string will outlive the string_key.
	//! The string is immutable. After being constructed, it can't be mutated in any way.
	template <
		typename CharT, 
		typename Traits = std::char_traits<CharT>, 
		typename Allocator = json::allocator<CharT>
	> class basic_string_key : private Allocator
	{
	public:
		using traits_type = Traits;
		using allocator_type = Allocator;
		using value_type = CharT;
		using pointer = typename std::allocator_traits<Allocator>::pointer;
		using const_pointer = typename std::allocator_traits<Allocator>::const_pointer;
		using reference = value_type &;
		using const_reference = const value_type &;
		using iterator = const_pointer;
		using const_iterator = const_pointer;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;
		using size_type = typename std::allocator_traits<Allocator>::size_type;
		using difference_type = typename std::allocator_traits<Allocator>::difference_type;

		static constexpr size_type npos = static_cast<size_type>(-1);

		//! Constructs a string_key from a character array. The array is copied. Size is computed by Traits::size(s)
		basic_string_key(const CharT * s, const Allocator & a = Allocator());
		//! Constructs a string_key from a std::basic_string
		basic_string_key(const std::basic_string<CharT, Traits, Allocator> & s);
		//! Constructs a string_key from a std::basic_string_view
		basic_string_key(std::basic_string_view<CharT, Traits> s, const Allocator & a = Allocator());
		//! Constructs a string_key from a std::basic_string and a custom allocator
		template <typename OtherAllocator>
		basic_string_key(const std::basic_string<CharT, Traits, OtherAllocator> & s, const Allocator & a);
		//! Constructs a string_key from a std::basic_string by move. If s is not a short string, it becomes empty
		basic_string_key(std::basic_string<CharT, Traits, Allocator> && s);
		//! Constructs a string_key from a static_string. The string is not copied. string_key will act as a view of the string
		basic_string_key(basic_static_string<CharT, Traits> s, const Allocator & a = Allocator{});
		//! Copy constructor
		basic_string_key(const basic_string_key & other);
		//! Move constructor. Other becomes empty afterwards
		basic_string_key(basic_string_key && other);
		//! Destructor frees dynamic memory if the string is not a static string
		~basic_string_key();

		//! Returns the allocator by copy
		Allocator get_allocator() const;
		//! Returns the allocator by reference
		Allocator & get_stored_allocator() noexcept;
		const Allocator & get_stored_allocator() const noexcept;

		//! Returns an iterator to the beginning of the string
		const_iterator begin() const noexcept;
		//! Returns an iterator to the beginning of the string
		const_iterator cbegin() const noexcept;
		//! Returns an iterator to the end of the string
		const_iterator end() const noexcept;
		//! Returns an iterator to the end of the string
		const_iterator cend() const noexcept;

		//! Returns a reverse iterator to the beginning of the reversed string (the end of the string)
		const_reverse_iterator rbegin() const noexcept;
		//! Returns a reverse iterator to the beginning of the reversed string (the end of the string)
		const_reverse_iterator crbegin() const noexcept;
		//! Returns a reverse iterator to the end of the reversed string (the beginning of the string)
		const_reverse_iterator rend() const noexcept;
		//! Returns a reverse iterator to the end of the reversed string (the beginning of the string)
		const_reverse_iterator crend() const noexcept;

		//! Accesses the character at index
		const CharT & operator [] (size_type index) const noexcept;
		//! Accesses the character at index. Throws if index > size()
		const CharT & at(size_type index) const;
		//! Accesses the first character
		const CharT & front() const noexcept;
		//! Accesses the last character
		const CharT & back() const noexcept;
		//! Returns a pointer to the character array
		const CharT * data() const noexcept;
		//! Accesses the character array
		const CharT * c_str() const noexcept;

		//! Returns the size of the string. Doesn't take into account the null character
		size_type size() const noexcept;
		//! Accesses the length of the string. Same as size()
		size_type length() const noexcept;
		//! Returns the capacity of the allocated buffer
		size_type capacity() const noexcept;
		//! Accesses the maximum length the string may have
		size_type max_size() const noexcept;
		//! True if the string is empty
		bool empty() const noexcept;
		//! True if the string is a static string and therefore is not owned by the string_key object
		bool is_view() const noexcept;

		//! Returns a substring [pos, pos + count) or [pos, size()) if the requested substring extends past the end of the string
		std::basic_string_view<CharT, Traits> substr(size_type pos = 0, size_type count = npos) const noexcept;

		//! Finds the first substring equal to the given character sequence. Search begins at pos
		size_type find(std::basic_string_view<CharT, Traits> str, size_type pos = 0) const noexcept;
		//! Finds the first substring equal to the given character sequence. Search begins at pos
		size_type find(const CharT * s, size_type pos, size_type count) const noexcept;
		//! Finds the first substring equal to the given character sequence. Search begins at pos
		size_type find(CharT ch, size_type pos = 0) const noexcept;

		//! Finds the last substring equal to the given character sequence. Search begins at pos
		size_type rfind(std::basic_string_view<CharT, Traits> str, size_type pos = npos) const noexcept;
		//! Finds the last substring equal to the given character sequence. Search begins at pos
		size_type rfind(const CharT * s, size_type pos, size_type count) const noexcept;
		//! Finds the last substring equal to the given character sequence. Search begins at pos
		size_type rfind(CharT ch, size_type pos = npos) const noexcept;

		//! Finds the first character equal to any of the characters in the given sequence. Search begins at pos
		size_type find_first_of(std::basic_string_view<CharT, Traits> str, size_type pos = 0) const noexcept;
		//! Finds the first character equal to any of the characters in the given sequence. Search begins at pos
		size_type find_first_of(const CharT * s, size_type pos, size_type count) const noexcept;
		//! Finds the first character equal to any of the characters in the given sequence. Search begins at pos
		size_type find_first_of(CharT ch, size_type pos = 0) const noexcept;

		//! Finds the first character equal to none of the characters in the given sequence. Search begins at pos
		size_type find_first_not_of(std::basic_string_view<CharT, Traits> str, size_type pos = 0) const noexcept;
		//! Finds the first character equal to none of the characters in the given sequence. Search begins at pos
		size_type find_first_not_of(const CharT * s, size_type pos, size_type count) const noexcept;
		//! Finds the first character equal to none of the characters in the given sequence. Search begins at pos
		size_type find_first_not_of(CharT ch, size_type pos = 0) const noexcept;

		//! Finds the last character equal to any of the characters in the given sequence. Search begins at pos
		size_type find_last_of(std::basic_string_view<CharT, Traits> str, size_type pos = npos) const noexcept;
		//! Finds the last character equal to any of the characters in the given sequence. Search begins at pos
		size_type find_last_of(const CharT * s, size_type pos, size_type count) const noexcept;
		//! Finds the last character equal to any of the characters in the given sequence. Search begins at pos
		size_type find_last_of(CharT ch, size_type pos = npos) const noexcept;

		//! Finds the last character equal to none of the characters in the given sequence. Search begins at pos
		size_type find_last_not_of(std::basic_string_view<CharT, Traits> str, size_type pos = npos) const noexcept;
		//! Finds the last character equal to none of the characters in the given sequence. Search begins at pos
		size_type find_last_not_of(const CharT * s, size_type pos, size_type count) const noexcept;
		//! Finds the last character equal to none of the characters in the given sequence. Search begins at pos
		size_type find_last_not_of(CharT ch, size_type pos = npos) const noexcept;

		//! Conversion to std::basic_string
		operator std::basic_string_view<CharT, Traits>() const noexcept;

	private:
		//! True if the string is owned by the object, this is, if it's not a static string
		bool owns_data() const noexcept;
		//! Frees the dynamic memory allocated by the object
		void free();
		//! Makes this string a copy of another. If possible, performs short string optimization. If not, allocates a buffer
		void copy_other(const_pointer src, size_type size);
		//! Allocates a buffer for size chars plus the null terminator
		CharT * allocate_buffer(size_type size);
		//! Accesses the short string buffer casted to a pointer of the character type of the string
		CharT * short_str() noexcept;
		//! Accesses the short string buffer casted to a pointer of the character type of the string. Const version
		const CharT * short_str() const noexcept;
		//! Gives write access to the otherwise const buffer pointed to by str
		CharT * mutable_str() noexcept;
		//! True if the string is currently stored in place in the string_key object
		bool is_short_str() const noexcept;
		//! Returns number of characters the short string buffer may hold at most
		constexpr size_type short_str_capacity() const noexcept;
		//! Makes this string a copy of ""
		void make_empty() noexcept;

		static constexpr size_type short_string_buffer_size = 16; //!< Size of the short string buffer;

		size_type str_size;		//!< Size of the string
		const CharT * str;		//!< Pointer to the array of characters
		union
		{
			char short_str_buffer[short_string_buffer_size];	//!< Buffer of characters used to store the string in place if it fits in the buffer
			size_t buffer_capacity = 0;							//!< Number of characters that fit in the currently allocated buffer, ignoring the null character
		};
	};

	//! True if both strings are equal
	template <typename CharT, typename Traits, typename Allocator1, typename Allocator2>
	inline bool operator == (const basic_string_key<CharT, Traits, Allocator1> & a, const basic_string_key<CharT, Traits, Allocator2> & b) noexcept;
	//! True if both strings are not equal
	template <typename CharT, typename Traits, typename Allocator1, typename Allocator2>
	inline bool operator != (const basic_string_key<CharT, Traits, Allocator1> & a, const basic_string_key<CharT, Traits, Allocator2> & b) noexcept;
	//! Lexicographically compares both strings
	template <typename CharT, typename Traits, typename Allocator1, typename Allocator2>
	inline bool operator <  (const basic_string_key<CharT, Traits, Allocator1> & a, const basic_string_key<CharT, Traits, Allocator2> & b) noexcept;
	//! Lexicographically compares both strings
	template <typename CharT, typename Traits, typename Allocator1, typename Allocator2>
	inline bool operator <= (const basic_string_key<CharT, Traits, Allocator1> & a, const basic_string_key<CharT, Traits, Allocator2> & b) noexcept;
	//! Lexicographically compares both strings
	template <typename CharT, typename Traits, typename Allocator1, typename Allocator2>
	inline bool operator >  (const basic_string_key<CharT, Traits, Allocator1> & a, const basic_string_key<CharT, Traits, Allocator2> & b) noexcept;
	//! Lexicographically compares both strings
	template <typename CharT, typename Traits, typename Allocator1, typename Allocator2>
	inline bool operator >= (const basic_string_key<CharT, Traits, Allocator1> & a, const basic_string_key<CharT, Traits, Allocator2> & b) noexcept;

	//! Overload of comparison operators for string view
	template <typename CharT, typename Traits, typename Allocator>
	inline bool operator == (const basic_string_key<CharT, Traits, Allocator> & a, std::basic_string_view<CharT, Traits> b) noexcept;
	template <typename CharT, typename Traits, typename Allocator>
	inline bool operator != (const basic_string_key<CharT, Traits, Allocator> & a, std::basic_string_view<CharT, Traits> b) noexcept;
	template <typename CharT, typename Traits, typename Allocator>
	inline bool operator <  (const basic_string_key<CharT, Traits, Allocator> & a, std::basic_string_view<CharT, Traits> b) noexcept;
	template <typename CharT, typename Traits, typename Allocator>
	inline bool operator <= (const basic_string_key<CharT, Traits, Allocator> & a, std::basic_string_view<CharT, Traits> b) noexcept;
	template <typename CharT, typename Traits, typename Allocator>
	inline bool operator >  (const basic_string_key<CharT, Traits, Allocator> & a, std::basic_string_view<CharT, Traits> b) noexcept;
	template <typename CharT, typename Traits, typename Allocator>
	inline bool operator >= (const basic_string_key<CharT, Traits, Allocator> & a, std::basic_string_view<CharT, Traits> b) noexcept;

	template <typename CharT, typename Traits, typename Allocator>
	inline bool operator == (std::basic_string_view<CharT, Traits> a, const basic_string_key<CharT, Traits, Allocator> & b) noexcept;
	template <typename CharT, typename Traits, typename Allocator>
	inline bool operator != (std::basic_string_view<CharT, Traits> a, const basic_string_key<CharT, Traits, Allocator> & b) noexcept;
	template <typename CharT, typename Traits, typename Allocator>
	inline bool operator <  (std::basic_string_view<CharT, Traits> a, const basic_string_key<CharT, Traits, Allocator> & b) noexcept;
	template <typename CharT, typename Traits, typename Allocator>
	inline bool operator <= (std::basic_string_view<CharT, Traits> a, const basic_string_key<CharT, Traits, Allocator> & b) noexcept;
	template <typename CharT, typename Traits, typename Allocator>
	inline bool operator >  (std::basic_string_view<CharT, Traits> a, const basic_string_key<CharT, Traits, Allocator> & b) noexcept;
	template <typename CharT, typename Traits, typename Allocator>
	inline bool operator >= (std::basic_string_view<CharT, Traits> a, const basic_string_key<CharT, Traits, Allocator> & b) noexcept;

	//! Typedef for basic_string_key of chars (the same way std::string is std::basic_string<char>)
	using string_key = basic_string_key<char>;
	//! Typedef for basic_string_key of wchar_t
	using wstring_key = basic_string_key<wchar_t>;
	//! Typedef for basic_string_key of char16_t
	using u16string_key = basic_string_key<char16_t>;
	//! Typedef for basic_string_key of char32_t
	using u32string_key = basic_string_key<char32_t>;

	inline namespace literals
	{
		inline namespace string_key_literals
		{

			//! Construct static_string from [str, str + length)
			constexpr static_string operator "" _ss(const char * str, size_t length) noexcept;

			//! Construct static_wstring from [str, str + length)
			constexpr static_wstring operator "" _ss(const wchar_t * str, size_t length) noexcept;

			//! Construct static_u16string from [str, str + length)
			constexpr static_u16string operator "" _ss(const char16_t * str, size_t length) noexcept;

			//! Construct static_u32string from [str, str + length)
			constexpr static_u32string operator "" _ss(const char32_t * str, size_t length) noexcept;

			//! Construct string_key from [str, str + length). The object is a view of the string literal, and allocates no memory
			inline string_key operator "" _sk(const char * str, size_t length) noexcept;

			//! Construct wstring_key from [str, str + length). The object is a view of the string literal, and allocates no memory
			inline wstring_key operator "" _sk(const wchar_t * str, size_t length) noexcept;

			//! Construct u16string_key from [str, str + length). The object is a view of the string literal, and allocates no memory
			inline u16string_key operator "" _sk(const char16_t * str, size_t length) noexcept;

			//! Construct u16string_key from [str, str + length). The object is a view of the string literal, and allocates no memory
			inline u32string_key operator "" _sk(const char32_t * str, size_t length) noexcept;

		} // namespace string_key_literals
	} // namespace literals

} // namespace json

#include "string_key.inl"

#endif // STRING_KEY_HH_INCLUDE_GUARD

