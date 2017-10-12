namespace json
{

	template <typename ... Args>
	object::iterator object::emplace(json::string_key key, Args && ... args)
	{
		const auto it_success = members.emplace(
			std::piecewise_construct,
			std::forward_as_tuple(std::move(key)), 
			std::forward_as_tuple(std::forward<Args>(args)...));
		if (it_success.second)
			return { it_success.first };
		else
			return members.end();
	}

} // namespace json

