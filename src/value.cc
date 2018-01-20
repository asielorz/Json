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
		: stored_value{ mpl::type_list<int64_t>{}, static_cast<int64_t>(v) }
	{}

	value::value(unsigned v) noexcept
		: stored_value{ mpl::type_list<int64_t>{}, static_cast<int64_t>(v) }
	{}

	value::value(int64_t v) noexcept
		: stored_value{ v }
	{}

	value::value(uint64_t v) noexcept
		: stored_value{ mpl::type_list<int64_t>{}, static_cast<int64_t>(v) }
	{}

	value::value(float v) noexcept
		: stored_value{ mpl::type_list<double>{}, static_cast<double>(v) }
	{}

	value::value(double v) noexcept
		: stored_value{ v }
	{}

	value::value(bool v) noexcept
		: stored_value{ v }
	{}

	value::value(const char * v)
		: stored_value{ mpl::type_list<json::string>{}, v }
	{}

	value::value(const json::string & v)
		: stored_value{ v }
	{}

	value::value(json::string && v)
		: stored_value{ std::move(v) }
	{}

	value::value(const json::array & v)
		: stored_value{ v }
	{}

	value::value(json::array && v)
		: stored_value{ std::move(v) }
	{}

	value::value(const json::object & v)
		: stored_value{ v }
	{}

	value::value(json::object && v)
		: stored_value{ std::move(v) }
	{}

	value::value(std::initializer_list<json::value> list)
		: stored_value{ mpl::type_list<json::array>{}, list }
	{}

	value::value(std::initializer_list<std::pair<const json::string_key, json::value>> list)
		: stored_value{ mpl::type_list<json::object>{}, list }
	{}

	value::value(value_type type)
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
		auto temp = std::move(other.stored_value);
		other.stored_value = std::move(stored_value);
		stored_value = std::move(temp);
	}
	
	//*************************************************************************************************
	// Comparison

	bool value::operator < (const value & other) const
	{
		if (is_numeric() && other.is_numeric())
			return as_double() < other.as_double();
		else if (is_bool() && other.is_bool())
			return stored_value.as<bool>() < other.stored_value.as<bool>();
		else if (is_string() && other.is_string())
			return stored_value.as<json::string>() < other.stored_value.as<json::string>();
		else if (is_array() && other.is_array())
			return stored_value.as<json::array>() < other.stored_value.as<json::array>();
		else if (is_object() && other.is_object())
			return stored_value.as<json::object>() < other.stored_value.as<json::object>();
		else
			throw invalid_operation{ "Value not compatible with operator <" };
	}

	bool value::operator <= (const value & other) const
	{
		return *this < other || *this == other;
	}

	bool value::operator >= (const value & other) const
	{
		return !(*this < other);
	}

	bool value::operator >  (const value & other) const
	{
		return !(*this <= other);
	}

	bool value::operator == (const value & other) const noexcept
	{
		const auto t = type();
		if (t != other.type())
			return false;

		switch (t)
		{
		case value_type::null:
			return true;
		case value_type::integer:
			return stored_value.as<int64_t>() == other.stored_value.as<int64_t>();
		case value_type::real:
			return stored_value.as<double>() == other.stored_value.as<double>();
		case value_type::boolean:
			return stored_value.as<bool>() == other.stored_value.as<bool>();
		case value_type::string:
			return stored_value.as<json::string>() == other.stored_value.as<json::string>();
		case value_type::array:
			return stored_value.as<json::array>() == other.stored_value.as<json::array>();
		case value_type::object:
			return stored_value.as<json::object>() == other.stored_value.as<json::object>();
		default:
			return false;
		}
	}
	
	bool value::operator != (const value & other) const noexcept
	{
		return !(*this == other);
	}

	//*************************************************************************************************
	// Gettors

	int value::as_int() const
	{
		if (is_int())
			return static_cast<int>(stored_value.as<int64_t>());
		else if (is_real())
			return static_cast<int>(stored_value.as<double>());
		else
			throw invalid_operation{ "Value not compatible with as_int" };
	}

	unsigned value::as_uint() const
	{
		if (is_int())
			return static_cast<unsigned>(stored_value.as<int64_t>());
		else if (is_real())
			return static_cast<unsigned>(stored_value.as<double>());
		else
			throw invalid_operation{ "Value not compatible with as_uint" };
	}

	int64_t value::as_int64() const
	{
		if (is_int())
			return stored_value.as<int64_t>();
		else if (is_real())
			return static_cast<int64_t>(stored_value.as<double>());
		else
			throw invalid_operation{ "Value not compatible with as_int64" };
	}

	uint64_t value::as_uint64() const
	{
		if (is_int())
			return static_cast<uint64_t>(stored_value.as<int64_t>());
		else if (is_real())
			return static_cast<uint64_t>(stored_value.as<double>());
		else
			throw invalid_operation{ "Value not compatible with as_uint64" };
	}

	float value::as_float() const
	{
		if (is_real())
			return static_cast<float>(stored_value.as<double>());
		else if (is_int())
			return static_cast<float>(stored_value.as<int64_t>());
		else
			throw invalid_operation{ "Value not compatible with as_double" };
	}

	double value::as_double() const
	{
		if (is_real())
			return stored_value.as<double>();
		else if (is_int())
			return static_cast<double>(stored_value.as<int64_t>());
		else
			throw invalid_operation{ "Value not compatible with as_double" };

	}

	bool value::as_bool() const
	{
		if (is_bool())
			return stored_value.as<bool>();
		else
			throw invalid_operation{ "Value not compatible with as_bool" };
	}

	const char * value::as_c_string() const
	{
		if (is_string())
			return stored_value.as<json::string>().c_str();
		else
			throw invalid_operation{ "Value not compatible with as_c_string" };
	}

	json::string & value::as_string()
	{
		if (is_string())
			return stored_value.as<json::string>();
		else
			throw invalid_operation{ "Value not compatible with as_string" };
	}

	const json::string & value::as_string() const
	{
		if (is_string())
			return stored_value.as<json::string>();
		else
			throw invalid_operation{ "Value not compatible with as_string" };
	}

	json::array & value::as_array()
	{
		if (is_array())
			return stored_value.as<json::array>();
		else
			throw invalid_operation{ "Value not compatible with as_array" };
	}

	const json::array & value::as_array() const
	{
		if (is_array())
			return stored_value.as<json::array>();
		else
			throw invalid_operation{ "Value not compatible with as_array" };
	}

	json::object & value::as_object()
	{
		if (is_object())
			return stored_value.as<json::object>();
		else
			throw invalid_operation{ "Value not compatible with as_object" };
	}

	const json::object & value::as_object() const
	{
		if (is_object())
			return stored_value.as<json::object>();
		else
			throw invalid_operation{ "Value not compatible with as_object" };
	}

	int64_t & value::stored_int()
	{
		if (is_int())
			return stored_value.as<int64_t>();
		else
			throw invalid_operation{ "Value not compatible with stored_int" };
	}

	const int64_t & value::stored_int() const
	{
		if (is_int())
			return stored_value.as<int64_t>();
		else
			throw invalid_operation{ "Value not compatible with stored_int" };
	}

	double & value::stored_double()
	{
		if (is_real())
			return stored_value.as<double>();
		else
			throw invalid_operation{ "Value not compatible with stored_double" };
	}

	const double & value::stored_double() const
	{
		if (is_real())
			return stored_value.as<double>();
		else
			throw invalid_operation{ "Value not compatible with stored_double" };
	}

	bool & value::stored_bool()
	{
		if (is_real())
			return stored_value.as<bool>();
		else
			throw invalid_operation{ "Value not compatible with stored_bool" };
	}

	const bool & value::stored_bool() const
	{
		if (is_real())
			return stored_value.as<bool>();
		else
			throw invalid_operation{ "Value not compatible with stored_bool" };
	}

	//*************************************************************************************************
	// Type checking

	bool value::is_null() const noexcept
	{
		return !stored_value.valid();
	}

	bool value::operator ! () const noexcept
	{
		return is_null();
	}

	bool value::is_int() const noexcept
	{
		return stored_value.is<int64_t>();
	}

	bool value::is_real() const noexcept
	{
		return stored_value.is<double>();
	}

	bool value::is_numeric() const noexcept
	{
		return is_int() || is_real();
	}

	bool value::is_bool() const noexcept
	{
		return stored_value.is<bool>();
	}

	bool value::is_string() const noexcept
	{
		return stored_value.is<json::string>();
	}

	bool value::is_array() const noexcept
	{
		return stored_value.is<json::array>();
	}

	bool value::is_object() const noexcept
	{
		return stored_value.is<json::object>();
	}

	json::value_type value::type() const noexcept
	{
		// stored_value.index() will return
		//	  null    = -1
		//	  integer =  0
		//	  real    =  1
		//	  boolean =  2
		//	  string  =  3
		//	  array   =  4
		//	  object  =  5
		// We have to map this to the enum value_type, where
		//	  null    =  0
		//	  integer =  1
		//	  real    =  2
		//    boolean =  3
		//	  string  =  4
		//	  array   =  5
		//	  object  =  6
		// So just add 1 to the return value of index() to get the current type in the enum

		return static_cast<json::value_type>(stored_value.index() + 1);
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
		// Search for the given key
		const auto it = members.find(key);
		// If found return the value
		if (it != members.end())
			return it->second;
		// Otherwise insert a new value and return it
		else
			return members.emplace(key, json::value{}).first->second;
	}

	value & object::operator [] (const json::string & key)
	{
		// Search for the given key
		const auto it = members.find(key);
		// If found return the value
		if (it != members.end())
			return it->second;
		// Otherwise insert a new value and return it
		else
			return members.emplace(key, json::value{}).first->second;
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

	const value & object::operator [] (const char * key) const noexcept
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

	const value & object::operator [] (const json::string & key) const noexcept
	{
		return (*this)[key.c_str()];
	}

	value & object::at(const char * key)
	{
		// Search for the given key
		const auto it = members.find(key);
		// If found return the value
		if (it != members.end())
			return it->second;
		// Otherwise throw out of range
		else
			throw std::out_of_range{ std::string("Member \"") + key + "\" not found in object" };
	}

	value & object::at(const std::string & key)
	{
		return at(key.c_str());
	}

	const value & object::at(const char * key) const
	{
		// Search for the given key
		const auto it = members.find(key);
		// If found return the value
		if (it != members.end())
			return it->second;
		// Otherwise throw out of range
		else
			throw std::out_of_range{ std::string("Member \"") + key + "\" not found in object" };
	}

	const value & object::at(const std::string & key) const
	{
		return at(key.c_str());
	}

	object::iterator object::find(const char * key) noexcept
	{
		return members.find(key);
	}

	object::iterator object::find(const std::string & key) noexcept
	{
		return find(key.c_str());
	}

	object::const_iterator object::find(const char * key) const noexcept
	{
		return members.find(key);
	}

	object::const_iterator object::find(const std::string & key) const noexcept
	{
		return find(key.c_str());
	}

	bool object::is_member(const char * key) const noexcept
	{
		return members.find(key) != members.end();
	}

	bool object::is_member(const std::string & key) const noexcept
	{
		return is_member(key.c_str());
	}

	bool object::remove_member(const char * key)
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

	bool object::remove_member(const std::string & key)
	{
		return remove_member(key.c_str());
	}

	json::vector<json::string> object::member_names() const
	{
		json::vector<json::string> names;
		names.reserve(members.size());

		for (const auto & pair : members)
			names.push_back(json::string{ pair.first.begin(), pair.first.end(), pair.first.get_allocator() });

		return names;
	}

	json::vector<const char *> object::member_names_as_c_str() const
	{
		json::vector<const char *> names;
		names.reserve(members.size());

		for (const auto & pair : members)
			names.push_back(pair.first.c_str());

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

} // namespace json

namespace std
{

	template<>
	void swap(json::value & a, json::value & b)
	{
		a.swap(b);
	}

} // namespace std

