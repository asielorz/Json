namespace json
{

	template <typename T>
	bool points_inside(const void * p, const T & obj)
	{
		const auto begin = reinterpret_cast<const char *>(&obj);
		const auto end = begin + sizeof(T);
		const auto ptr = reinterpret_cast<const char *>(p);

		return (ptr >= begin) && (ptr < end);
	}

	template <typename CharT, typename Traits, typename Allocator>
	CharT * release(std::basic_string<CharT, Traits, Allocator> & s)
	{
		return release(std::move(s));
	}

	template <typename CharT, typename Traits, typename Allocator>
	CharT * release(std::basic_string<CharT, Traits, Allocator> && s)
	{
		// If the string is a short string, it doesn't have any dynamic memory so there is no memory to release
		if (points_inside(s.data(), s))
			return nullptr;
		else
		{
			// Brief explanation of the dark magics behind this:
			// std::string is encapsulated such that the only way of making it release the ownership of
			// its dynamic memory without deleting it is moving it into another std::string. However, by 
			// doing this the ownership is passed to another std::string so we still have the same problem
			// of the memory being owned by an RAII object that will delete it in its destructor.
			// The solution used below is moving the string into a buffer of bytes using placement new,
			// in order to make the source string release ownership over its memory without passing this
			// ownership to another RAII object, as an array of characters won't call the destructor of
			// std::string. At this point, the std::string object has released the ownership over its
			// dynamic memory, and the address of this memory can be trivially obtained by calling
			// std::string::data on the buffer.

			using string_t = std::basic_string<CharT, Traits, Allocator>;
			char buffer[sizeof(string_t)];
			::new (buffer) string_t(std::move(s));
			// std::string::data is const in C++11. Non-const version wasn't added until C++14
			return const_cast<CharT *>(reinterpret_cast<string_t *>(buffer)->data());
		}
	}

	template <typename T, typename Allocator>
	T * release(std::vector<T, Allocator> & v)
	{
		return release(std::move(v));
	}

	template <typename T, typename Allocator>
	T * release(std::vector<T, Allocator> && v)
	{
		using vector_t = std::vector<T, Allocator>;
		char buffer[sizeof(vector_t)];
		::new (buffer) vector_t(std::move(v));
		return reinterpret_cast<vector_t *>(buffer)->data();
	}

	template <typename T, typename Deleter>
	typename std::unique_ptr<T, Deleter>::pointer release(std::unique_ptr<T, Deleter> & p)
	{
		return release(std::move(p));
	}

	template <typename T, typename Deleter>
	typename std::unique_ptr<T, Deleter>::pointer release(std::unique_ptr<T, Deleter> && p)
	{
		return p.release();
	}

}

