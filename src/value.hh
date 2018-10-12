#ifndef VALUE_HH_INCLUDE_GUARD
#define VALUE_HH_INCLUDE_GUARD

#include "config.hh"
#include "string_key.hh"
#include <stdexcept> // std::logic_error
#include <cstdint> // int64_t, uint64_t
#include <variant> // std::variant

namespace json
{

	//! Enum with the possible types a json::value may hold.
	enum class value_type
	{
		null    = 0,
		integer = 1,
		real    = 2,
		boolean = 3,
		string  = 4,
		array   = 5,
		object  = 6
	};

	//! Thrown when trying to access a value as a type different to its own (eg: using as_int on an array type)
	//! or when comparing for inequality two values of different types
	class invalid_operation : public std::logic_error
	{
	public:
		invalid_operation(const char * msg)
			: logic_error{ msg }
		{}
	};

	class value;

	//! Data structure representing a json array. An array is an ordered collection of values. An array begins with [ (left bracket)
	//! and ends with ] (right bracket). Values are separated by , (comma). http://www.json.org/
	using array = json::vector<json::value>;

	//! Data structure representing json object. An object is an unordered set of name/value pairs. An object begins with
	//! { (left brace) and ends with } (right brace). Each name is followed by : (colon) and the name/value pairs are 
	//! separated by , (comma). http://www.json.org/
	class JSON_API object
	{
	public:
		using map_type = json::map<json::string_key, json::value>;

		using value_type = map_type::value_type;
		using mapped_type = map_type::mapped_type;
		using key_type = map_type::key_type;
		using reference = map_type::reference;
		using const_reference = map_type::const_reference;
		using pointer = map_type::pointer;
		using const_pointer = map_type::const_pointer;

		using size_type = map_type::size_type;
		using difference_type = map_type::difference_type;
		using key_compare = map_type::key_compare;
		using allocator_type = map_type::allocator_type;

		using iterator = map_type::iterator;
		using const_iterator = map_type::const_iterator;
		using reverse_iterator = map_type::reverse_iterator;
		using const_reverse_iterator = map_type::const_reverse_iterator;

		//! Constructs an empty object
		object() = default;
		//! Constructs an object from a map of strings to json::values
		object(const map_type & map);
		//! Constructs an object from a map of strings to json::values by move. map becomes empty.
		object(map_type && map) noexcept(std::is_nothrow_move_constructible<map_type>::value);

		//! Number of members
		size_t size() const noexcept;

		//! True if size == 0
		bool empty() const noexcept;

		//! Removes all members
		void clear();

		//! Returns an iterator to the first member
		iterator begin() noexcept;
		//! Returns an iterator to one past the last member
		iterator end() noexcept;

		//! Returns a const iterator (read only) to the first member
		const_iterator begin() const noexcept;
		//! Returns a const iterator(read only) to one past the last member
		const_iterator end() const noexcept;
		//! Returns a const iterator (read only) to the first member
		const_iterator cbegin() const noexcept;
		//! Returns a const iterator (read only) to the first member
		const_iterator cend() const noexcept;

		//! Returns a reverse iterator to the last member. Incrementing the iterator will advance to the previous member
		reverse_iterator rbegin() noexcept;
		//! Returns a reverse iterator to one before the first member
		reverse_iterator rend() noexcept;

		//! Returns a const reverse iterator (read only) to the last member
		const_reverse_iterator rbegin() const noexcept;
		//! Returns a const reverse iterator (read_only) to one before the first member
		const_reverse_iterator rend() const noexcept;
		//! Returns a const reverse iterator (read only) to the last member
		const_reverse_iterator crbegin() const noexcept;
		//! Returns a const reverse iterator (read_only) to one before the first member
		const_reverse_iterator crend() const noexcept;

		//! True if each member compares equal to each member of other
		bool operator == (const object & other) const;
		//! True if not equal
		bool operator != (const object & other) const;
		//! Lexicographically compares each member with each member of the other object
		bool operator <  (const object & other) const;
		//! Lexicographically compares each member with each member of the other object
		bool operator <= (const object & other) const;
		//! Lexicographically compares each member with each member of the other object
		bool operator >  (const object & other) const;
		//! Lexicographically compares each member with each member of the other object
		bool operator >= (const object & other) const;

		//! Accesses a member with the given key. Inserts a new default constructed element if not found
		value & operator [] (const char * key);
		//! Accesses a member with the given key. Inserts a new default constructed element if not found
		value & operator [] (const json::string & key);
		//! Accesses a member with the given key. Inserts a new default constructed element if not found
		value & operator [] (std::string_view key);
		//! Accesses a member with the given key. Inserts a new default constructed element if not found.
		//! If a new element is inserted, the key will be constructed by moving, so the parameter string will become empty
		value & operator [] (json::string && key);
		//! Accesses a member with the given key. Inserts a new default constructed element if not found
		//! If a value is inserted, the string used as key will not be copied. The map will keep a reference to the original string.
		value & operator [] (static_string key);
		//! Accesses a member with the given key. Returns null reference if not found
		const value & operator [] (std::string_view key) const noexcept;

		//! Accesses a member with the given key. Throws std::out_of_range if not found
		value & at(std::string_view key);
		//! Accesses a member with the given key. Throws std::out_of_range if not found
		const value & at(std::string_view key) const;

		//! Accesses a member with the given key. Returns end iterator if not found
		iterator find(std::string_view key) noexcept;
		//! Accesses a member with the given key. Returns end iterator if not found
		const_iterator find(std::string_view key) const noexcept;

		//! True if there exists a member with the given key
		bool is_member(std::string_view key) const noexcept;

		//! Removes a member with the given key
		bool remove_member(std::string_view key) noexcept;

		//! Returns a vector with the keys of all members
		json::vector<json::string> member_names() const;
		//! Returns a vector with the keys of all members. The vector doesn't own the strings,
		//! so the pointers will invalidate if the member gets removed or the object gets destroyed.
		json::vector<std::string_view> member_names_as_c_str() const;

		//! Inserts an element with the given key. Returns an iterator to the inserted element, or end on failure
		iterator insert(json::string_key key, const json::value & val);
		//! Inserts an element with the given key. Returns an iterator to the inserted element, or end on failure
		iterator insert(json::string_key key, json::value && val);

		//! Inserts an element with the given key and constructs it in place from variadic arguments.
		//! Returns an iterator to the inserted element, or end on failure
		template <typename ... Args>
		iterator emplace(json::string_key key, Args && ... args);

	private:
		map_type members;
	};

	//! Data structure representing a json value http://www.json.org.
	//!
	//! This class is a variant type which can represent any of the following:
	//!		- 64 bit signed integer (int64_t)
	//!		- double precision floating point value (double)
	//!		- boolean (bool)
	//!		- UTF-8 string (json::string)
	//!		- an ordered sequence of json::value (json::array)
	//!		- an unordered set of string-value pairs (json::object)
	//!		- null
	//!
	//! Type of the value can be queried with type() function and is represented as a json::value_type enum
	//!
	//! A value is of null type by default, and can be converted to other types with the assignment operator.
	//! Trying to access a value with a type different to its own will throw a json::invalid_operation exception. 
	class JSON_API value
	{
	public:
		//! Constant null value returned by const subscript operator of object on failure
		static const value null;

		//! Default constructor constructs a null value
		value();

		//! Constructs an integer value with the value of v
		value(int v) noexcept;
		//! Constructs an integer value with the value of v
		value(unsigned v) noexcept;
		//! Constructs an integer value with the value of v
		value(int64_t v) noexcept;
		//! Constructs an integer value with the value of v
		value(uint64_t v) noexcept;
		//! Constructs a real value with the value of v
		value(float v) noexcept;
		//! Constructs a real value with the value of v
		value(double v) noexcept;
		//! Constructs a boolean value with the value of v
		value(bool v) noexcept;
		//! Constructs a string value with the value of v
		value(const char * v);
		//! Constructs a string value with the value of v
		value(const json::string & v);
		//! Constructs a string value by move. v becomes empty
		value(json::string && v);
		//! Constructs an array value with the value of v
		value(const json::array & v);
		//! Constructs an array value by move. v becomes empty
		value(json::array && v);
		//! Constructs an object value with the value of v
		value(const json::object & v);
		//! Constructs an object value by move. v becomes empty
		value(json::object && v);

		//! Constructs an array value from an initializer list of json values
		value(std::initializer_list<json::value> list);

		//! Constructs an object value from an initializer list of pairs of string-value
		value(std::initializer_list<std::pair<const json::string_key, json::value>> list);

		//! Constructs an empty value of the given type
		//!		- null
		//!		- integer = 0
		//!		- real = 0.0
		//!		- boolean = false
		//!		- string = ""
		//!		- empty array
		//!		- empty object
		explicit value(value_type type);

		//! Convert the value to integer, then assigns v
		value & operator = (int v) noexcept;
		//! Convert the value to integer, then assigns v
		value & operator = (unsigned v) noexcept;
		//! Convert the value to integer, then assigns v
		value & operator = (int64_t v) noexcept;
		//! Convert the value to integer, then assigns v
		value & operator = (uint64_t v) noexcept;
		//! Convert the value to real, then assigns v
		value & operator = (float v) noexcept;
		//! Convert the value to real, then assigns v
		value & operator = (double v) noexcept;
		//! Convert the value to boolean, then assigns v
		value & operator = (bool v) noexcept;
		//! Convert the value to string, then assigns v
		value & operator = (const char * v);
		//! Convert the value to string, then assigns v
		value & operator = (const json::string & v);
		//! Convert the value to string, then move assigns v. v becomes empty
		value & operator = (json::string && v);
		//! Convert the value to array, then assigns v
		value & operator = (const json::array & v);
		//! Convert the value to array, then move assigns v. v becomes empty
		value & operator = (json::array && v);
		//! Convert the value to object, then assigns v
		value & operator = (const json::object & v);
		//! Convert the value to object, then move assigns v. v becomes empty
		value & operator = (json::object && v);

		//! Swaps the content of two values
		void swap(value & other) noexcept;

		//! True if both are the same type and this is less than other. Invalid operation if different type
		bool operator <  (const value & other) const;
		//! True if both are the same type and this is less than or equal to other. Invalid operation if different type
		bool operator <= (const value & other) const;
		//! True if both are the same type and this is greater than or equal to other. Invalid operation if different type
		bool operator >= (const value & other) const;
		//! True if both are the same type and this is greater than other. Invalid operation if different type
		bool operator >  (const value & other) const;
		//! True if both are the same type and values compare equal. False otherwise
		bool operator == (const value & other) const noexcept;
		//! True if both are of different types or if their values don't compare equal
		bool operator != (const value & other) const noexcept;

		//! If numeric, returns the value casted to int. Invalid operation otherwise
		int as_int() const;
		//! If numeric, returns the value casted to unsigned. Invalid operation otherwise
		unsigned as_uint() const;
		//! If numeric, returns the value casted to int64_t. Invalid operation otherwise
		int64_t as_int64() const;
		//! If numeric, returns the value casted to uint64_t. Invalid operation otherwise
		uint64_t as_uint64() const;
		//! If numeric, returns the value casted to float. Invalid operation otherwise
		float as_float() const;
		//! If numeric, returns the value casted to double. Invalid operation otherwise
		double as_double() const;
		//! If boolean, returns the value. Invalid operation otherwise
		bool as_bool() const;
		//! If string, returns a pointer to the stored character array. Invalid operation otherwise
		const char * as_c_string() const;

		//! If string, returns a reference to the stored string. Invalid operation otherwise
		json::string & as_string();
		//! If string, returns a reference to the stored string. Invalid operation otherwise
		const json::string & as_string() const;

		//! If array, returns a reference to the stored array. Invalid operation otherwise
		json::array & as_array();
		//! If array, returns a reference to the stored array. Invalid operation otherwise
		const json::array & as_array() const;

		//! If object, returns a reference to the stored object. Invalid operation otherwise
		json::object & as_object();
		//! If object, returns a reference to the stored object. Invalid operation otherwise
		const json::object & as_object() const;

		//! If integer, returns a reference to the stored int64_t. Invalid operation otherwise
		int64_t & stored_int();
		//! If integer, returns a reference to the stored int64_t. Invalid operation otherwise
		const int64_t & stored_int() const;

		//! If real, returns a reference to the stored double. Invalid operation otherwise
		double & stored_double();
		//! If real, returns a reference to the stored double. Invalid operation otherwise
		const double & stored_double() const;

		//! If boolean, returns a reference to the stored bool. Invalid operation otherwise
		bool & stored_bool();
		//! If boolean, returns a reference to the stored bool. Invalid operation otherwise
		const bool & stored_bool() const;

		//! True if type is null
		bool is_null() const noexcept;
		//! True if type is null
		bool operator ! () const noexcept;
		//! True if type is integer
		bool is_int() const noexcept;
		//! True if type is real
		bool is_real() const noexcept;
		//! True if type is int or real
		bool is_numeric() const noexcept;
		//! True if type is boolean
		bool is_bool() const noexcept;
		//! True if type is string
		bool is_string() const noexcept;
		//! True if type is array
		bool is_array() const noexcept;
		//! True if type is object
		bool is_object() const noexcept;
		//! Returns the type of the value
		json::value_type type() const noexcept;

	private:
		//! Variant with the possible value types the class may have
		std::variant<
			std::monostate,	// This represents null
			int64_t,
			double,
			bool,
			json::string,
			json::array,
			json::object
		> stored_value;
	};

	inline namespace literals
	{
		inline namespace value_literals
		{

			value operator "" _jv(unsigned long long i) noexcept;
			value operator "" _jv(long double d) noexcept;
			value operator "" _jv(const char * str, size_t length) noexcept;

		} // namespace value_literals
	} // namespace literals

} // namespace json

namespace std
{

	//! Specialization of std::swap for json::value
	template<>
	JSON_API void swap(json::value & a, json::value & b);

} // namespace std

#include "value.inl"

#endif // VALUE_HH_INCLUDE_GUARD

