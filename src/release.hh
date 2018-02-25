#ifndef RELEASE_HH_INCLUDE_GUARD
#define RELEASE_HH_INCLUDE_GUARD

#include <string>
#include <vector>
#include <memory>

namespace json
{
	namespace utils
	{

		//! True if p points to obj or a member of obj
		//! True if p points to an address in memory in the range [&obj, (char *)&obj + sizeof(obj))
		template <typename T>
		bool points_inside(const void * p, const T & obj);

		//! Makes a string release the ownership over its dynamic memory and return a pointer to it
		template <typename CharT, typename Traits, typename Allocator>
		CharT * release(std::basic_string<CharT, Traits, Allocator> & s);

		//! Makes a string release the ownership over its dynamic memory and return a pointer to it
		template <typename CharT, typename Traits, typename Allocator>
		CharT * release(std::basic_string<CharT, Traits, Allocator> && s);

		//! Makes a vector release the ownership over its dynamic memory and return a pointer to it
		template <typename T, typename Allocator>
		T * release(std::vector<T, Allocator> & v);

		//! Makes a vector release the ownership over its dynamic memory and return a pointer to it
		template <typename T, typename Allocator>
		T * release(std::vector<T, Allocator> && v);

		//! Makes a unique_ptr release the ownership over its dynamic memory. Same as p.release()
		template <typename T, typename Deleter>
		typename std::unique_ptr<T, Deleter>::pointer release(std::unique_ptr<T, Deleter> & p);

		//! Makes a unique_ptr release the ownership over its dynamic memory. Same as p.release()
		template <typename T, typename Deleter>
		typename std::unique_ptr<T, Deleter>::pointer release(std::unique_ptr<T, Deleter> && p);

	} // namespace utils
} // namespace json

#include "release.inl"

#endif // !RELEASE_HH_INCLUDE_GUARD

