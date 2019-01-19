#include "value.hh"
#include <cassert>

namespace json
{

	// Constant null value returned by const subscript operator of object on failure
	const value value::null;

	//*************************************************************************************************
	// Constructors

	// Making the constructor = default gives an error in clang
	value::value(){}

	value::value(int v) noexcept
		: stored_value(std::in_place_type<int64_t>, static_cast<int64_t>(v))
	{}

	value::value(unsigned v) noexcept
		: stored_value(std::in_place_type<int64_t>, static_cast<int64_t>(v))
	{}

	value::value(int64_t v) noexcept
		: stored_value(v)
	{}

	value::value(uint64_t v) noexcept
		: stored_value(std::in_place_type<int64_t>, static_cast<int64_t>(v))
	{}

	value::value(float v) noexcept
		: stored_value(std::in_place_type<double>, static_cast<double>(v))
	{}

	value::value(double v) noexcept
		: stored_value(v)
	{}

	value::value(bool v) noexcept
		: stored_value(v)
	{}

	value::value(const char * v)
		: stored_value(std::in_place_type<json::string>, v)
	{}

	value::value(const json::string & v)
		: stored_value(v)
	{}

	value::value(json::string && v)
		: stored_value(std::move(v))
	{}

	value::value(const json::array & v)
		: stored_value(v)
	{}

	value::value(json::array && v)
		: stored_value(std::move(v))
	{}

	value::value(const json::object & v)
		: stored_value(v)
	{}

	value::value(json::object && v)
		: stored_value(std::move(v))
	{}

	value::value(std::initializer_list<json::value> list)
		: stored_value(std::in_place_type<json::array>, list)
	{}

	value::value(std::initializer_list<std::pair<const json::string_key, json::value>> list)
		: stored_value(std::in_place_type<json::object>, list)
	{}

	value::value(value_type type) noexcept
	{
		switch (type)
		{
			case value_type::null:
			{
				break;
			}
			case value_type::integer:
			{
				*this = 0;
				break;
			}
			case value_type::real:
			{
				*this = 0.0;
				break;
			}
			case value_type::boolean:
			{
				*this = false;
				break;
			}
			case value_type::string:
			{
				*this = "";
				break;
			}
			case value_type::array:
			{
				stored_value.emplace<json::array>();
				break;
			}
			case value_type::object:
			{
				stored_value.emplace<json::object>();
				break;
			}
		}
	}

	//*************************************************************************************************
	// Assignment operators

	value & value::operator = (int v) noexcept
	{
		stored_value = static_cast<int64_t>(v);
		return *this;
	}

	value & value::operator = (unsigned v) noexcept
	{
		stored_value = static_cast<int64_t>(v);
		return *this;
	}

	value & value::operator = (int64_t v) noexcept
	{
		stored_value = v;
		return *this;
	}

	value & value::operator = (uint64_t v) noexcept
	{
		stored_value = static_cast<int64_t>(v);
		return *this;
	}

	value & value::operator = (float v) noexcept
	{
		stored_value = static_cast<double>(v);
		return *this;
	}

	value & value::operator = (double v) noexcept
	{
		stored_value = v;
		return *this;
	}

	value & value::operator = (bool v) noexcept
	{
		stored_value = v;
		return *this;
	}

	value & value::operator = (const char * v)
	{
		stored_value = json::string{ v };
		return *this;
	}

	value & value::operator = (const json::string & v)
	{
		stored_value = v;
		return *this;
	}

	value & value::operator = (json::string && v)
	{
		stored_value = std::move(v);
		return *this;
	}

	value & value::operator = (const json::array & v)
	{
		stored_value = v;
		return *this;
	}

	value & value::operator = (json::array && v)
	{
		stored_value = std::move(v);
		return *this;
	}

	value & value::operator = (const json::object & v)
	{
		stored_value = v;
		return *this;
	}

	value & value::operator = (json::object && v)
	{
		stored_value = std::move(v);
		return *this;
	}

	void value::swap(value & other) noexcept
	{
		std::swap(stored_value, other.stored_value);
	}
	
	//*************************************************************************************************
	// Comparison

	bool value::operator < (const value & other) const noexcept
	{
		if (is_numeric() && other.is_numeric())
			return as_double() < other.as_double();
		else if (is_bool() && other.is_bool())
			return as_bool() < other.as_bool();
		else if (is_string() && other.is_string())
			return as_string() < other.as_string();
		else if (is_array() && other.is_array())
			return as_array() < other.as_array();
		else if (is_object() && other.is_object())
			return as_object() < other.as_object();
		else
		{
			assert(false);
			return false;
		}
	}

	bool value::operator <= (const value & other) const noexcept
	{
		return other >= *this;
	}

	bool value::operator >= (const value & other) const noexcept
	{
		return !(*this < other);
	}

	bool value::operator >  (const value & other) const noexcept
	{
		return !(*this <= other);
	}

	bool value::operator == (const value & other) const noexcept
	{
		return stored_value == other.stored_value;
	}
	
	bool value::operator != (const value & other) const noexcept
	{
		return stored_value != other.stored_value;
	}

	//*************************************************************************************************
	// Gettors

	int value::as_int() const noexcept
	{
		assert(is_numeric());
		if (is_int())
			return static_cast<int>(std::get<int64_t>(stored_value));
		else if (is_real())
			return static_cast<int>(std::get<double>(stored_value));
		else
		{
			assert(false);
			return 0;
		}
	}

	unsigned value::as_uint() const noexcept
	{
		assert(is_numeric());
		if (is_int())
			return static_cast<unsigned>(std::get<int64_t>(stored_value));
		else
			return static_cast<unsigned>(std::get<double>(stored_value));
	}

	int64_t value::as_int64() const noexcept
	{
		assert(is_numeric());
		if (is_int())
			return std::get<int64_t>(stored_value);
		else
			return static_cast<int64_t>(std::get<double>(stored_value));
	}

	uint64_t value::as_uint64() const noexcept
	{
		assert(is_numeric());
		if (is_int())
			return static_cast<uint64_t>(std::get<int64_t>(stored_value));
		else
			return static_cast<uint64_t>(std::get<double>(stored_value));
	}

	float value::as_float() const noexcept
	{
		assert(is_numeric());
		if (is_real())
			return static_cast<float>(std::get<double>(stored_value));
		else
			return static_cast<float>(std::get<int64_t>(stored_value));
	}

	double value::as_double() const noexcept
	{
		assert(is_numeric());
		if (is_real())
			return std::get<double>(stored_value);
		else
			return static_cast<double>(std::get<int64_t>(stored_value));
	}

	bool value::as_bool() const noexcept
	{
		assert(is_bool());
		return std::get<bool>(stored_value);
	}

	const char * value::as_c_string() const noexcept
	{
		assert(is_string());
		return std::get<json::string>(stored_value).c_str();
	}

	json::string & value::as_string() noexcept
	{
		assert(is_string());
		return std::get<json::string>(stored_value);
	}

	const json::string & value::as_string() const noexcept
	{
		assert(is_string());
		return std::get<json::string>(stored_value);
	}

	json::array & value::as_array() noexcept
	{
		assert(is_array());
		return std::get<json::array>(stored_value);
	}

	const json::array & value::as_array() const noexcept
	{
		assert(is_array());
		return std::get<json::array>(stored_value);
	}

	json::object & value::as_object() noexcept
	{
		assert(is_object());
		return std::get<json::object>(stored_value);
	}

	const json::object & value::as_object() const noexcept
	{
		assert(is_object());
		return std::get<json::object>(stored_value);
	}

	int64_t & value::stored_int() noexcept
	{
		assert(is_int());
		return std::get<int64_t>(stored_value);
	}

	const int64_t & value::stored_int() const noexcept
	{
		assert(is_int());
		return std::get<int64_t>(stored_value);
	}

	double & value::stored_double() noexcept
	{
		assert(is_real());
		return std::get<double>(stored_value);
	}

	const double & value::stored_double() const noexcept
	{
		assert(is_real());
		return std::get<double>(stored_value);
	}

	bool & value::stored_bool() noexcept
	{
		assert(is_bool());
		return std::get<bool>(stored_value);
	}

	const bool & value::stored_bool() const noexcept
	{
		assert(is_bool());
		return std::get<bool>(stored_value);
	}

	//*************************************************************************************************
	// Type checking

	bool value::is_null() const noexcept
	{
		return stored_value.index() == 0;
	}

	bool value::operator ! () const noexcept
	{
		return is_null();
	}

	bool value::is_int() const noexcept
	{
		return std::holds_alternative<int64_t>(stored_value);
	}

	bool value::is_real() const noexcept
	{
		return std::holds_alternative<double>(stored_value);
	}

	bool value::is_numeric() const noexcept
	{
		return is_int() || is_real();
	}

	bool value::is_bool() const noexcept
	{
		return std::holds_alternative<bool>(stored_value);
	}

	bool value::is_string() const noexcept
	{
		return std::holds_alternative<json::string>(stored_value);
	}

	bool value::is_array() const noexcept
	{
		return std::holds_alternative<json::array>(stored_value);
	}

	bool value::is_object() const noexcept
	{
		return std::holds_alternative<json::object>(stored_value);
	}

	json::value_type value::type() const noexcept
	{
		return static_cast<json::value_type>(stored_value.index());
	}

	//*************************************************************************************************
	// Object

	object::object(const map_type & map)
		: members(map)
	{}

	object::object(map_type && map) noexcept(std::is_nothrow_move_constructible<map_type>::value)
		: members(std::move(map))
	{}

	size_t object::size() const noexcept
	{
		return members.size();
	}

	bool object::empty() const noexcept
	{
		return members.empty();
	}

	void object::clear()
	{
		members.clear();
	}

	object::iterator object::begin() noexcept
	{
		return members.begin();
	}

	object::iterator object::end() noexcept
	{
		return members.end();
	}

	object::const_iterator object::begin() const noexcept
	{
		return cbegin();
	}

	object::const_iterator object::end() const noexcept
	{
		return cend();
	}

	object::const_iterator object::cbegin() const noexcept
	{
		return members.cbegin();
	}

	object::const_iterator object::cend() const noexcept
	{
		return members.cend();
	}

	object::reverse_iterator object::rbegin() noexcept
	{
		return members.rbegin();
	}

	object::reverse_iterator object::rend() noexcept
	{
		return members.rend();
	}

	object::const_reverse_iterator object::rbegin() const noexcept
	{
		return crbegin();
	}

	object::const_reverse_iterator object::rend() const noexcept
	{
		return crend();
	}

	object::const_reverse_iterator object::crbegin() const noexcept
	{
		return members.crbegin();
	}

	object::const_reverse_iterator object::crend() const noexcept
	{
		return members.crend();
	}

	bool object::operator == (const object & other) const
	{
		return members == other.members;
	}

	bool object::operator != (const object & other) const
	{
		return members != other.members;
	}

	bool object::operator <  (const object & other) const
	{
		return members < other.members;
	}

	bool object::operator <= (const object & other) const
	{
		return members <= other.members;
	}

	bool object::operator >  (const object & other) const
	{
		return members > other.members;
	}

	bool object::operator >= (const object & other) const
	{
		return members >= other.members;
	}

	// All searching in the map is implemented in terms of map::find because from C++14 onwards map::find
	// has an overload that can take any type that can be transparently compared with key. Therefore, we
	// can search the map with a const char * without constructing an std::string and thus saving ourselves
	// a heap operation (malloc/free)
	value & object::operator [] (const char * key)
	{
		return this->operator[](std::string_view(key));
	}

	value & object::operator [] (const json::string & key)
	{
		return this->operator[](std::string_view(key));
	}

	value & object::operator [] (std::string_view key)
	{
		// Search for the given key
		const auto it = members.find(key);
		// If found return the value
		if (it != members.end())
			return it->second;
		// Otherwise insert a new value and return it
		else
			return members.emplace(json::string(key), json::value{}).first->second;
	}

	value & object::operator [] (json::string && key)
	{
		// Search for the given key
		const auto it = members.find(key);
		// If found return the value
		if (it != members.end())
			return it->second;
		// Otherwise insert a new value and return it
		else
			return members.emplace(std::move(key), json::value{}).first->second;
	}

	value & object::operator [] (static_string key)
	{
		// Search for the given key
		const auto it = members.find(key.c_str());
		// If found return the value
		if (it != members.end())
			return it->second;
		// Otherwise insert a new value and return it
		else
			return members.emplace(key, json::value{}).first->second;
	}

	const value & object::operator [] (std::string_view key) const noexcept
	{
		// Search for the given key
		const auto it = members.find(key);
		// If found return the value
		if (it != members.end())
			return it->second;
		// Otherwise return null
		else
			return json::value::null;
	}

	value & object::at(std::string_view key)
	{
		// Search for the given key
		const auto it = members.find(key);
		// If found return the value
		if (it != members.end())
			return it->second;
		// Otherwise throw out of range
		else
			throw std::out_of_range(std::string("Member \"") + std::string(key.begin(), key.end()) + "\" not found in object");
	}

	const value & object::at(std::string_view key) const
	{
		// Search for the given key
		const auto it = members.find(key);
		// If found return the value
		if (it != members.end())
			return it->second;
		// Otherwise throw out of range
		else
			throw std::out_of_range(std::string("Member \"") + std::string(key.begin(), key.end()) + "\" not found in object");
	}

	object::iterator object::find(std::string_view key) noexcept
	{
		return members.find(key);
	}

	object::const_iterator object::find(std::string_view key) const noexcept
	{
		return members.find(key);
	}

	bool object::is_member(std::string_view key) const noexcept
	{
		return members.find(key) != members.end();
	}

	bool object::remove_member(std::string_view key) noexcept
	{
		// map::erase(iterator) has constant time complexity, so erase(find(key)) is equivalent in complexity
		// to erase(key), with the extra advantage of find being aware of transparent comparison

		const auto it = members.find(key);
		if (it != members.end())
		{
			members.erase(it);
			return true;
		}
		else
			return false;
	}

	json::vector<json::string> object::member_names() const
	{
		json::vector<json::string> names;
		names.reserve(members.size());

		for (const auto & pair : members)
			names.push_back(json::string{ pair.first.begin(), pair.first.end(), pair.first.get_allocator() });

		return names;
	}

	json::vector<std::string_view> object::member_names_as_c_str() const
	{
		json::vector<std::string_view> names;
		names.reserve(members.size());

		for (const auto & pair : members)
			names.push_back(pair.first);

		return names;
	}

	object::iterator object::insert(json::string_key key, const json::value & val)
	{
		const auto it_success = members.emplace(std::move(key), val);
		if (it_success.second)
			return { it_success.first };
		else
			return { members.end() };
	}

	object::iterator object::insert(json::string_key key, json::value && val)
	{
		const auto it_success = members.emplace(std::move(key), std::move(val));
		if (it_success.second)
			return { it_success.first };
		else
			return { members.end() };
	}

	inline namespace literals
	{
		inline namespace value_literals
		{

			value operator "" _jv(unsigned long long i) noexcept
			{
				return value(static_cast<int64_t>(i));
			}

			value operator "" _jv(long double d) noexcept
			{
				return value(static_cast<double>(d));
			}

			value operator "" _jv(const char * str, size_t length) noexcept
			{
				return value(string(str, str + length));
			}

		}
	}

} // namespace json

namespace std
{

	template<>
	void swap(json::value & a, json::value & b)
	{
		a.swap(b);
	}

} // namespace std

