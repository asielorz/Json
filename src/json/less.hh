#ifndef LESS_HH_INCLUDE_GUARD
#define LESS_HH_INCLUDE_GUARD

namespace json
{

	//! Implementation of std::less<void> so that it can be used in C++11,
	//! as it was added to the standard in C++14
	class less
	{
	public:
		// Allow comparing objects of different types
		using is_transparent = int;

		template<typename T, typename U>
		bool operator() (T && a, U && b) const
		{
			return a < b;
		}

	};

}

#endif // LESS_HH_INCLUDE_GUARD
