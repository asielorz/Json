namespace json
{

	template <typename CharT, typename Traits>
	constexpr basic_static_string<CharT, Traits>::basic_static_string(const CharT * s) noexcept
		: str{ s }
		, str_size{ Traits::length(s)}
	{}

	template <typename CharT, typename Traits>
	constexpr basic_static_string<CharT, Traits>::basic_static_string(const CharT * s, size_t l) noexcept
		: str{ s }
		, str_size{ l }
	{}

	template <typename CharT, typename Traits>
	constexpr const CharT * basic_static_string<CharT, Traits>::c_str() const noexcept
	{
		return str;
	}

	template <typename CharT, typename Traits>
	constexpr size_t basic_static_string<CharT, Traits>::size() const noexcept
	{
		return str_size;
	}

	//*****************************************************************************************************************

	template <typename CharT, typename Traits, typename Allocator>
	basic_string_key<CharT, Traits, Allocator>::basic_string_key(const CharT * s, const Allocator & a)
		: Allocator{ a }
	{
		copy_other(s, Traits::length(s));
	}

	template <typename CharT, typename Traits, typename Allocator>
	basic_string_key<CharT, Traits, Allocator>::basic_string_key(const std::basic_string<CharT, Traits, Allocator> & s)
		: Allocator{ s.get_allocator() }
	{
		copy_other(s.c_str(), s.size());
	}

	template <typename CharT, typename Traits, typename Allocator>
	template <typename OtherAllocator>
	basic_string_key<CharT, Traits, Allocator>::basic_string_key(const std::basic_string<CharT, Traits, OtherAllocator> & s, const Allocator & a)
		: Allocator{ a }
	{
		copy_other(s.c_str(), s.size());
	}

	template <typename CharT, typename Traits, typename Allocator>
	basic_string_key<CharT, Traits, Allocator>::basic_string_key(std::basic_string<CharT, Traits, Allocator> && s)
		: Allocator{ s.get_allocator() }
	{
		// If s is a short string
		if (points_inside(s.c_str(), s))
			copy_other(s.c_str(), s.size());
		else
		{
			buffer_capacity = s.capacity();
			str_size = s.size();
			str = release(s);
		}
	}

	template <typename CharT, typename Traits, typename Allocator>
	basic_string_key<CharT, Traits, Allocator>::basic_string_key(basic_static_string<CharT, Traits> s, const Allocator & a)
		: Allocator{ a }
		, str_size{ s.size() }
		, str{ s.c_str() }
		, buffer_capacity{ 0 }
	{}

	template <typename CharT, typename Traits, typename Allocator>
	basic_string_key<CharT, Traits, Allocator>::basic_string_key(const basic_string_key & other)
		: Allocator{ other.get_allocator() }
		, str_size{ other.str_size }
		, str{}
	{
		if (other.owns_data())
			copy_other(other.str, other.str_size);
		else
			str = other.str;
	}

	template <typename CharT, typename Traits, typename Allocator>
	basic_string_key<CharT, Traits, Allocator>::basic_string_key(basic_string_key && other)
		: Allocator{ std::move(other.get_stored_allocator()) }
		, str_size{ std::move(other.str_size) }
		, str{}
	{
		if (other.is_short_str())
			copy_other(other.str, other.str_size);
		else
		{
			str = other.str;
			buffer_capacity = other.buffer_capacity;
		}

		other.make_empty();
	}

	template <typename CharT, typename Traits, typename Allocator>
	basic_string_key<CharT, Traits, Allocator>::~basic_string_key()
	{
		free();
	}

	template <typename CharT, typename Traits, typename Allocator>
	Allocator basic_string_key<CharT, Traits, Allocator>::get_allocator() const
	{
		return get_stored_allocator();
	}

	template <typename CharT, typename Traits, typename Allocator>
	typename basic_string_key<CharT, Traits, Allocator>::const_iterator basic_string_key<CharT, Traits, Allocator>::begin() const noexcept
	{
		return cbegin();
	}

	template <typename CharT, typename Traits, typename Allocator>
	typename basic_string_key<CharT, Traits, Allocator>::const_iterator basic_string_key<CharT, Traits, Allocator>::cbegin() const noexcept
	{
		return data();
	}

	template <typename CharT, typename Traits, typename Allocator>
	typename basic_string_key<CharT, Traits, Allocator>::const_iterator basic_string_key<CharT, Traits, Allocator>::end() const noexcept
	{
		return cend();
	}

	template <typename CharT, typename Traits, typename Allocator>
	typename basic_string_key<CharT, Traits, Allocator>::const_iterator basic_string_key<CharT, Traits, Allocator>::cend() const noexcept
	{
		return data() + size();
	}

	template <typename CharT, typename Traits, typename Allocator>
	typename basic_string_key<CharT, Traits, Allocator>::const_reverse_iterator basic_string_key<CharT, Traits, Allocator>::rbegin() const noexcept
	{
		return crbegin();
	}

	template <typename CharT, typename Traits, typename Allocator>
	typename basic_string_key<CharT, Traits, Allocator>::const_reverse_iterator basic_string_key<CharT, Traits, Allocator>::crbegin() const noexcept
	{
		return const_reverse_iterator{ data() + size() };
	}

	template <typename CharT, typename Traits, typename Allocator>
	typename basic_string_key<CharT, Traits, Allocator>::const_reverse_iterator basic_string_key<CharT, Traits, Allocator>::rend() const noexcept
	{
		return crend();
	}

	template <typename CharT, typename Traits, typename Allocator>
	typename basic_string_key<CharT, Traits, Allocator>::const_reverse_iterator basic_string_key<CharT, Traits, Allocator>::crend() const noexcept
	{
		return const_reverse_iterator{ data() };
	}

	template <typename CharT, typename Traits, typename Allocator>
	const CharT & basic_string_key<CharT, Traits, Allocator>::operator [] (size_type index) const noexcept
	{
		return data()[index];
	}

	template <typename CharT, typename Traits, typename Allocator>
	const CharT & basic_string_key<CharT, Traits, Allocator>::at (size_type index) const
	{
		if (index >= size())
			throw std::out_of_range{ "string subscript out of range" };
		return data()[index];
	}

	template <typename CharT, typename Traits, typename Allocator>
	const CharT & basic_string_key<CharT, Traits, Allocator>::front() const noexcept
	{
		return data()[0];
	}

	template <typename CharT, typename Traits, typename Allocator>
	const CharT & basic_string_key<CharT, Traits, Allocator>::back() const noexcept
	{
		return data()[size() - 1];
	}

	template <typename CharT, typename Traits, typename Allocator>
	const CharT * basic_string_key<CharT, Traits, Allocator>::data() const noexcept
	{
		return str;
	}

	template <typename CharT, typename Traits, typename Allocator>
	const CharT * basic_string_key<CharT, Traits, Allocator>::c_str() const noexcept
	{
		return data();
	}

	template <typename CharT, typename Traits, typename Allocator>
	typename basic_string_key<CharT, Traits, Allocator>::size_type basic_string_key<CharT, Traits, Allocator>::size() const noexcept
	{
		return str_size;
	}

	template <typename CharT, typename Traits, typename Allocator>
	typename basic_string_key<CharT, Traits, Allocator>::size_type basic_string_key<CharT, Traits, Allocator>::length() const noexcept
	{
		return size();
	}

	template <typename CharT, typename Traits, typename Allocator>
	typename basic_string_key<CharT, Traits, Allocator>::size_type basic_string_key<CharT, Traits, Allocator>::capacity() const noexcept
	{
		if (is_short_str())
			return short_str_capacity();
		else
			return buffer_capacity;
	}

	template <typename CharT, typename Traits, typename Allocator>
	typename basic_string_key<CharT, Traits, Allocator>::size_type basic_string_key<CharT, Traits, Allocator>::max_size() const noexcept
	{
		return std::allocator_traits<Allocator>::max_size(get_allocator());
	}

	template <typename CharT, typename Traits, typename Allocator>
	bool basic_string_key<CharT, Traits, Allocator>::empty() const noexcept
	{
		return size() == 0;
	}

	template <typename CharT, typename Traits, typename Allocator>
	bool basic_string_key<CharT, Traits, Allocator>::is_view() const noexcept
	{
		return !owns_data();
	}

	template <typename CharT, typename Traits, typename Allocator>
	std::basic_string<CharT, Traits, Allocator> basic_string_key<CharT, Traits, Allocator>::substr(size_type pos, size_type count) const
	{
		const size_type first = std::min(pos, size());
		const size_type last = count == npos 
			? size()
			: std::min(pos + count, size());

		return std::basic_string<CharT, Traits, Allocator>{ begin() + first, begin() + last, get_allocator() };
	}

	template <typename CharT, typename Traits, typename Allocator>
	typename basic_string_key<CharT, Traits, Allocator>::size_type basic_string_key<CharT, Traits, Allocator>::find(const std::basic_string<CharT, Traits, Allocator> & s, size_type pos) const
	{
		return find(s.c_str(), pos, s.size());
	}

	template <typename CharT, typename Traits, typename Allocator>
	typename basic_string_key<CharT, Traits, Allocator>::size_type basic_string_key<CharT, Traits, Allocator>::find(const CharT * s, size_type pos, size_type count) const
	{
		const auto it = std::search(begin() + pos, end(), s, s + count, Traits::eq);
		if (it == end())
			return npos;
		else
			return it - begin();
	}

	template <typename CharT, typename Traits, typename Allocator>
	typename basic_string_key<CharT, Traits, Allocator>::size_type basic_string_key<CharT, Traits, Allocator>::find(const CharT * s, size_type pos) const
	{
		return find(s, pos, Traits::length(s));
	}

	template <typename CharT, typename Traits, typename Allocator>
	typename basic_string_key<CharT, Traits, Allocator>::size_type basic_string_key<CharT, Traits, Allocator>::find(CharT ch, size_type pos) const
	{
		return find(&ch, pos, 1);
	}

	template <typename CharT, typename Traits, typename Allocator>
	typename basic_string_key<CharT, Traits, Allocator>::size_type basic_string_key<CharT, Traits, Allocator>::rfind(const std::basic_string<CharT, Traits, Allocator> & s, size_type pos) const
	{
		return rfind(s.c_str(), pos, s.size());
	}

	template <typename CharT, typename Traits, typename Allocator>
	typename basic_string_key<CharT, Traits, Allocator>::size_type basic_string_key<CharT, Traits, Allocator>::rfind(const CharT * s, size_type pos, size_type count) const
	{
		const reverse_iterator s_begin{ s + count };
		const reverse_iterator s_end{ s };

		const auto it = std::search(rbegin() + pos, rend(), s_begin, s_end, Traits::eq);
		if (it == rend())
			return npos;
		else
			return rend() - it - count;
	}

	template <typename CharT, typename Traits, typename Allocator>
	typename basic_string_key<CharT, Traits, Allocator>::size_type basic_string_key<CharT, Traits, Allocator>::rfind(const CharT * s, size_type pos) const
	{
		return rfind(s, pos, Traits::length(s));
	}

	template <typename CharT, typename Traits, typename Allocator>
	typename basic_string_key<CharT, Traits, Allocator>::size_type basic_string_key<CharT, Traits, Allocator>::rfind(CharT ch, size_type pos) const
	{
		return rfind(&ch, pos, 1);
	}

	template <typename CharT, typename Traits, typename Allocator>
	typename basic_string_key<CharT, Traits, Allocator>::size_type basic_string_key<CharT, Traits, Allocator>::find_first_of(const std::basic_string<CharT, Traits, Allocator> & s, size_type pos) const
	{
		return find_first_of(s.c_str(), pos, s.size());
	}

	template <typename CharT, typename Traits, typename Allocator>
	typename basic_string_key<CharT, Traits, Allocator>::size_type basic_string_key<CharT, Traits, Allocator>::find_first_of(const CharT * s, size_type pos, size_type count) const
	{
		const auto it = std::find_first_of(begin() + pos, end(), s, s + count, Traits::eq);
		if (it == end())
			return npos;
		else
			return it - begin();
	}

	template <typename CharT, typename Traits, typename Allocator>
	typename basic_string_key<CharT, Traits, Allocator>::size_type basic_string_key<CharT, Traits, Allocator>::find_first_of(const CharT * s, size_type pos) const
	{
		return find_first_of(s, pos, Traits::length(s));
	}

	template <typename CharT, typename Traits, typename Allocator>
	typename basic_string_key<CharT, Traits, Allocator>::size_type basic_string_key<CharT, Traits, Allocator>::find_first_of(CharT ch, size_type pos) const
	{
		return find_first_of(&ch, pos, 1);
	}

	template <typename CharT, typename Traits, typename Allocator>
	typename basic_string_key<CharT, Traits, Allocator>::size_type basic_string_key<CharT, Traits, Allocator>::find_first_not_of(const std::basic_string<CharT, Traits, Allocator> & s, size_type pos) const
	{
		return find_first_not_of(s.c_str(), pos, s.size());
	}

	template <typename CharT, typename Traits, typename Allocator>
	typename basic_string_key<CharT, Traits, Allocator>::size_type basic_string_key<CharT, Traits, Allocator>::find_first_not_of(const CharT * s, size_type pos, size_type count) const
	{
		for (size_type i = pos; i < size(); ++i)
			if (std::none_of(s, s + count, [&](CharT ch) { return Traits::eq(ch, data()[i]); }))
				return i;
		return npos;
	}

	template <typename CharT, typename Traits, typename Allocator>
	typename basic_string_key<CharT, Traits, Allocator>::size_type basic_string_key<CharT, Traits, Allocator>::find_first_not_of(const CharT * s, size_type pos) const
	{
		return find_first_not_of(s, pos, Traits::length(s));
	}

	template <typename CharT, typename Traits, typename Allocator>
	typename basic_string_key<CharT, Traits, Allocator>::size_type basic_string_key<CharT, Traits, Allocator>::find_first_not_of(CharT ch, size_type pos) const
	{
		return find_first_not_of(&ch, pos, 1);
	}

	template <typename CharT, typename Traits, typename Allocator>
	typename basic_string_key<CharT, Traits, Allocator>::size_type basic_string_key<CharT, Traits, Allocator>::find_last_of(const std::basic_string<CharT, Traits, Allocator> & s, size_type pos) const
	{
		return find_last_of(s.c_str(), pos, s.size());
	}

	template <typename CharT, typename Traits, typename Allocator>
	typename basic_string_key<CharT, Traits, Allocator>::size_type basic_string_key<CharT, Traits, Allocator>::find_last_of(const CharT * s, size_type pos, size_type count) const
	{
		const reverse_iterator s_begin{ s + count };
		const reverse_iterator s_end{ s };

		const auto it = std::find_first_of(rbegin() + pos, rend(), s_begin, s_end, Traits::eq);
		if (it == rend())
			return npos;
		else
			return rend() - it - 1;
	}

	template <typename CharT, typename Traits, typename Allocator>
	typename basic_string_key<CharT, Traits, Allocator>::size_type basic_string_key<CharT, Traits, Allocator>::find_last_of(const CharT * s, size_type pos) const
	{
		return find_last_of(s, pos, Traits::length(s));
	}

	template <typename CharT, typename Traits, typename Allocator>
	typename basic_string_key<CharT, Traits, Allocator>::size_type basic_string_key<CharT, Traits, Allocator>::find_last_of(CharT ch, size_type pos) const
	{
		return find_last_of(&ch, pos, 1);
	}

	template <typename CharT, typename Traits, typename Allocator>
	typename basic_string_key<CharT, Traits, Allocator>::size_type basic_string_key<CharT, Traits, Allocator>::find_last_not_of(const std::basic_string<CharT, Traits, Allocator> & s, size_type pos) const
	{
		return find_last_not_of(s.c_str(), pos, s.size());
	}

	template <typename CharT, typename Traits, typename Allocator>
	typename basic_string_key<CharT, Traits, Allocator>::size_type basic_string_key<CharT, Traits, Allocator>::find_last_not_of(const CharT * s, size_type pos, size_type count) const
	{
		for (size_type i = size() - 1 - pos; i != npos; --i)
			if (std::none_of(s, s + count, [&](CharT ch) { return Traits::eq(ch, data()[i]); }))
				return i;
		return npos;
	}

	template <typename CharT, typename Traits, typename Allocator>
	typename basic_string_key<CharT, Traits, Allocator>::size_type basic_string_key<CharT, Traits, Allocator>::find_last_not_of(const CharT * s, size_type pos) const
	{
		return find_last_not_of(s, pos, Traits::length(s));
	}

	template <typename CharT, typename Traits, typename Allocator>
	typename basic_string_key<CharT, Traits, Allocator>::size_type basic_string_key<CharT, Traits, Allocator>::find_last_not_of(CharT ch, size_type pos) const
	{
		return find_last_not_of(&ch, pos, 1);
	}

	template <typename CharT, typename Traits, typename Allocator>
	basic_string_key<CharT, Traits, Allocator>::operator std::basic_string<CharT, Traits, Allocator>() const
	{
		return std::basic_string<CharT, Traits, Allocator>{ begin(), end(), get_allocator() };
	}

	template <typename CharT, typename Traits, typename Allocator>
	Allocator & basic_string_key<CharT, Traits, Allocator>::get_stored_allocator() noexcept
	{
		return *this;
	}

	template <typename CharT, typename Traits, typename Allocator>
	const Allocator & basic_string_key<CharT, Traits, Allocator>::get_stored_allocator() const noexcept
	{
		return *this;
	}

	//*****************************************************************************************************************

	template <typename CharT, typename Traits, typename Allocator>
	bool basic_string_key<CharT, Traits, Allocator>::owns_data() const noexcept
	{
		return is_short_str() || buffer_capacity > 0;
	}

	template <typename CharT, typename Traits, typename Allocator>
	void basic_string_key<CharT, Traits, Allocator>::free()
	{
		// Free memory only if this is not a view and if this is not a short string
		if (owns_data() && !is_short_str())
			std::allocator_traits<Allocator>::deallocate(get_stored_allocator(), mutable_str(), buffer_capacity + 1);
	}

	template <typename CharT, typename Traits, typename Allocator>
	void basic_string_key<CharT, Traits, Allocator>::copy_other(const_pointer src, size_type size)
	{
		CharT * buffer = allocate_buffer(size);
		Traits::copy(buffer, src, size + 1);
		str = buffer;
		str_size = size;
	}

	template <typename CharT, typename Traits, typename Allocator>
	CharT * basic_string_key<CharT, Traits, Allocator>::allocate_buffer(size_type size)
	{
		// If the string fits in the short string buffer, use that
		if (size <= short_str_capacity())
			return short_str();
		// Otherwise allocate a new buffer big enough to fit the string and the null character
		else
		{
			buffer_capacity = size;
			return std::allocator_traits<Allocator>::allocate(get_stored_allocator(), size + 1);
		}
	}

	template <typename CharT, typename Traits, typename Allocator>
	CharT * basic_string_key<CharT, Traits, Allocator>::short_str() noexcept
	{
		return reinterpret_cast<CharT *>(short_str_buffer);
	}

	template <typename CharT, typename Traits, typename Allocator>
	const CharT * basic_string_key<CharT, Traits, Allocator>::short_str() const noexcept
	{
		return reinterpret_cast<const CharT *>(short_str_buffer);
	}

	template <typename CharT, typename Traits, typename Allocator>
	CharT * basic_string_key<CharT, Traits, Allocator>::mutable_str() noexcept
	{
		return const_cast<CharT *>(str);
	}

	template <typename CharT, typename Traits, typename Allocator>
	bool basic_string_key<CharT, Traits, Allocator>::is_short_str() const noexcept
	{
		return str == short_str();
	}

	template <typename CharT, typename Traits, typename Allocator>
	constexpr typename basic_string_key<CharT, Traits, Allocator>::size_type basic_string_key<CharT, Traits, Allocator>::short_str_capacity() const noexcept
	{
		return (short_string_buffer_size / sizeof(CharT)) - 1;
	}

	template <typename CharT, typename Traits, typename Allocator>
	void basic_string_key<CharT, Traits, Allocator>::make_empty() noexcept
	{
		const CharT c = CharT{}; // Null character
		str = short_str();
		str_size = 0;
		Traits::copy(short_str(), &c, 1);
	}

	//*****************************************************************************************************************

	template <typename CharT, typename Traits, typename Allocator1, typename Allocator2>
	inline bool operator == (const basic_string_key<CharT, Traits, Allocator1> & a, const basic_string_key<CharT, Traits, Allocator2> & b) noexcept
	{
		return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin(), b.end(), Traits::eq);
	}

	template <typename CharT, typename Traits, typename Allocator1, typename Allocator2>
	inline bool operator != (const basic_string_key<CharT, Traits, Allocator1> & a, const basic_string_key<CharT, Traits, Allocator2> & b) noexcept
	{
		return !(a == b);
	}

	template <typename CharT, typename Traits, typename Allocator1, typename Allocator2>
	inline bool operator < (const basic_string_key<CharT, Traits, Allocator1> & a, const basic_string_key<CharT, Traits, Allocator2> & b) noexcept
	{
		return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(), Traits::lt);
	}

	template <typename CharT, typename Traits, typename Allocator1, typename Allocator2>
	inline bool operator <= (const basic_string_key<CharT, Traits, Allocator1> & a, const basic_string_key<CharT, Traits, Allocator2> & b) noexcept
	{
		return (a == b) || (a < b);
	}

	template <typename CharT, typename Traits, typename Allocator1, typename Allocator2>
	inline bool operator >  (const basic_string_key<CharT, Traits, Allocator1> & a, const basic_string_key<CharT, Traits, Allocator2> & b) noexcept
	{
		return !(a <= b);
	}

	template <typename CharT, typename Traits, typename Allocator1, typename Allocator2>
	inline bool operator >= (const basic_string_key<CharT, Traits, Allocator1> & a, const basic_string_key<CharT, Traits, Allocator2> & b) noexcept
	{
		return !(a < b);
	}

	template <typename CharT, typename Traits, typename Allocator>
	inline bool operator == (const basic_string_key<CharT, Traits, Allocator> & a, const CharT * b) noexcept
	{
		return Traits::compare(a.c_str(), b, a.size() + 1) == 0;
	}

	template <typename CharT, typename Traits, typename Allocator>
	inline bool operator != (const basic_string_key<CharT, Traits, Allocator> & a, const CharT * b) noexcept
	{
		return !(a == b);
	}

	template <typename CharT, typename Traits, typename Allocator>
	inline bool operator <  (const basic_string_key<CharT, Traits, Allocator> & a, const CharT * b) noexcept
	{
		return Traits::compare(a.c_str(), b, a.size() + 1) < 0;
	}

	template <typename CharT, typename Traits, typename Allocator>
	inline bool operator <= (const basic_string_key<CharT, Traits, Allocator> & a, const CharT * b) noexcept
	{
		return Traits::compare(a.c_str(), b, a.size() + 1) <= 0;
	}

	template <typename CharT, typename Traits, typename Allocator>
	inline bool operator >  (const basic_string_key<CharT, Traits, Allocator> & a, const CharT * b) noexcept
	{
		return Traits::compare(a.c_str(), b, a.size() + 1) > 0;
	}

	template <typename CharT, typename Traits, typename Allocator>
	inline bool operator >= (const basic_string_key<CharT, Traits, Allocator> & a, const CharT * b) noexcept
	{
		return Traits::compare(a.c_str(), b, a.size() + 1) >= 0;
	}

	template <typename CharT, typename Traits, typename Allocator>
	inline bool operator == (const CharT * a, const basic_string_key<CharT, Traits, Allocator> & b) noexcept
	{
		return b == a;
	}

	template <typename CharT, typename Traits, typename Allocator>
	inline bool operator != (const CharT * a, const basic_string_key<CharT, Traits, Allocator> & b) noexcept
	{
		return b != a;
	}

	template <typename CharT, typename Traits, typename Allocator>
	inline bool operator <  (const CharT * a, const basic_string_key<CharT, Traits, Allocator> & b) noexcept
	{
		return Traits::compare(a, b.c_str(), b.size() + 1) < 0;
	}

	template <typename CharT, typename Traits, typename Allocator>
	inline bool operator <= (const CharT * a, const basic_string_key<CharT, Traits, Allocator> & b) noexcept
	{
		return Traits::compare(a, b.c_str(), b.size() + 1) <= 0;
	}

	template <typename CharT, typename Traits, typename Allocator>
	inline bool operator >  (const CharT * a, const basic_string_key<CharT, Traits, Allocator> & b) noexcept
	{
		return Traits::compare(a, b.c_str(), b.size() + 1) > 0;
	}

	template <typename CharT, typename Traits, typename Allocator>
	inline bool operator >= (const CharT * a, const basic_string_key<CharT, Traits, Allocator> & b) noexcept
	{
		return Traits::compare(a, b.c_str(), b.size() + 1) >= 0;
	}

	template <typename CharT, typename Traits, typename Allocator1, typename Allocator2>
	inline bool operator == (const basic_string_key<CharT, Traits, Allocator1> & a, const std::basic_string<CharT, Traits, Allocator2> & b) noexcept
	{
		return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin(), b.end());
	}

	template <typename CharT, typename Traits, typename Allocator1, typename Allocator2>
	inline bool operator != (const basic_string_key<CharT, Traits, Allocator1> & a, const std::basic_string<CharT, Traits, Allocator2> & b) noexcept
	{
		return !(a == b);
	}

	template <typename CharT, typename Traits, typename Allocator1, typename Allocator2>
	inline bool operator <  (const basic_string_key<CharT, Traits, Allocator1> & a, const std::basic_string<CharT, Traits, Allocator2> & b) noexcept
	{
		return a < b.c_str();
	}

	template <typename CharT, typename Traits, typename Allocator1, typename Allocator2>
	inline bool operator <= (const basic_string_key<CharT, Traits, Allocator1> & a, const std::basic_string<CharT, Traits, Allocator2> & b) noexcept
	{
		return a <= b.c_str();
	}

	template <typename CharT, typename Traits, typename Allocator1, typename Allocator2>
	inline bool operator >  (const basic_string_key<CharT, Traits, Allocator1> & a, const std::basic_string<CharT, Traits, Allocator2> & b) noexcept
	{
		return a > b.c_str();
	}

	template <typename CharT, typename Traits, typename Allocator1, typename Allocator2>
	inline bool operator >= (const basic_string_key<CharT, Traits, Allocator1> & a, const std::basic_string<CharT, Traits, Allocator2> & b) noexcept
	{
		return a >= b.c_str();
	}

	template <typename CharT, typename Traits, typename Allocator1, typename Allocator2>
	inline bool operator == (const std::basic_string<CharT, Traits, Allocator1> & a, const basic_string_key<CharT, Traits, Allocator2> & b) noexcept
	{
		return b == a;
	}

	template <typename CharT, typename Traits, typename Allocator1, typename Allocator2>
	inline bool operator != (const std::basic_string<CharT, Traits, Allocator1> & a, const basic_string_key<CharT, Traits, Allocator2> & b) noexcept
	{
		return !(a == b);
	}

	template <typename CharT, typename Traits, typename Allocator1, typename Allocator2>
	inline bool operator <  (const std::basic_string<CharT, Traits, Allocator1> & a, const basic_string_key<CharT, Traits, Allocator2> & b) noexcept
	{
		return a.c_str() < b;
	}

	template <typename CharT, typename Traits, typename Allocator1, typename Allocator2>
	inline bool operator <= (const std::basic_string<CharT, Traits, Allocator1> & a, const basic_string_key<CharT, Traits, Allocator2> & b) noexcept
	{
		return a.c_str() <= b;
	}

	template <typename CharT, typename Traits, typename Allocator1, typename Allocator2>
	inline bool operator >  (const std::basic_string<CharT, Traits, Allocator1> & a, const basic_string_key<CharT, Traits, Allocator2> & b) noexcept
	{
		return a.c_str() > b;
	}

	template <typename CharT, typename Traits, typename Allocator1, typename Allocator2>
	inline bool operator >= (const std::basic_string<CharT, Traits, Allocator1> & a, const basic_string_key<CharT, Traits, Allocator2> & b) noexcept
	{
		return a.c_str() >= b;
	}

	inline namespace string_key_literals
	{

		//! Construct static_string from [str, str + length)
		constexpr static_string operator "" _ss(const char * str, size_t length) noexcept
		{
			return static_string{ str, length };
		}

		//! Construct static_wstring from [str, str + length)
		constexpr static_wstring operator "" _ss(const wchar_t * str, size_t length) noexcept
		{
			return static_wstring{ str, length };
		}

		//! Construct static_u16string from [str, str + length)
		constexpr static_u16string operator "" _ss(const char16_t * str, size_t length) noexcept
		{
			return static_u16string{ str, length };
		}

		//! Construct static_u32string from [str, str + length)
		constexpr static_u32string operator "" _ss(const char32_t * str, size_t length) noexcept
		{
			return static_u32string{ str, length };
		}

		//! Construct string_key from [str, str + length). The object is a view of the string literal, and allocates no memory
		inline string_key operator "" _sk(const char * str, size_t length) noexcept
		{
			return string_key{ static_string{ str, length } };
		}

		//! Construct wstring_key from [str, str + length). The object is a view of the string literal, and allocates no memory
		inline wstring_key operator "" _sk(const wchar_t * str, size_t length) noexcept
		{
			return wstring_key{ static_wstring{ str, length } };
		}

		//! Construct u16string_key from [str, str + length). The object is a view of the string literal, and allocates no memory
		inline u16string_key operator "" _sk(const char16_t * str, size_t length) noexcept
		{
			return u16string_key{ static_u16string{ str, length } };
		}

		//! Construct u16string_key from [str, str + length). The object is a view of the string literal, and allocates no memory
		inline u32string_key operator "" _sk(const char32_t * str, size_t length) noexcept
		{
			return u32string_key{ static_u32string{ str, length } };
		}

	} // namespace string_key_literals

} // namespace json

