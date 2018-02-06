#ifndef ALGORITHM_HH_INCLUDE_GUARD
#define ALGORITHM_HH_INCLUDE_GUARD

#include <type_traits>
#include <cstddef> // size_t

namespace json
{
	namespace mpl
	{

		//! Represents a list of types
		template <typename ... Ts>
		struct type_list
		{};

		// **************************************************************************

		//! Returns the biggest out of two values
		template <size_t a, size_t b>
		struct max
			: public std::integral_constant<size_t, (a > b) ? a : b>
		{};

		//! Returns the smallest out of two values
		template <size_t a, size_t b>
		struct min
			: public std::integral_constant<size_t, (a < b) ? a : b>
		{};

		// **************************************************************************

		//! Returns the biggest size of a set of types
		template <typename First, typename ... Rest>
		struct biggest_size
			: public std::integral_constant<size_t, max<sizeof(First), biggest_size<Rest...>::value>::value>
		{};

		//! Returns the biggest size of a set of types. Base case for recursion
		template <typename First>
		struct biggest_size<First>
			: public std::integral_constant<size_t, sizeof(First)>
		{};

		//! Returns the biggest size of a set of types. Specialization for type_list
		template <typename ... List>
		struct biggest_size<type_list<List...>>
			: public std::integral_constant<size_t, biggest_size<List...>::value>
		{};

		// **************************************************************************

		//! Returns the biggest alignment of a set of types
		template <typename First, typename ... Rest>
		struct biggest_alignment
			: public std::integral_constant<size_t, max<alignof(First), biggest_alignment<Rest...>::value>::value>
		{};

		//! Returns the biggest alignment of a set of types. Base case for recursion
		template <typename First>
		struct biggest_alignment<First>
			: public std::integral_constant<size_t, alignof(First)>
		{};

		//! Returns the biggest alignment of a set of types. Specialization for type_list
		template <typename ... List>
		struct biggest_alignment<type_list<List...>>
			: public std::integral_constant<size_t, biggest_alignment<List...>::value>
		{};

		// **************************************************************************

		//! True if a type is in a list of types
		template <typename T, typename First, typename ... Rest>
		struct contains
			: public std::integral_constant<bool,
			std::is_same<T, First>::value ||
			contains<T, Rest...>::value>
		{};

		//! True if a type is in a list of types. Base case for recursion
		template <typename T, typename First>
		struct contains<T, First>
			: public std::integral_constant<bool, std::is_same<T, First>::value>
		{};

		//! True if a type is in a list of types. Specialization for type_list
		template <typename T, typename ... List>
		struct contains<T, type_list<List...>>
			: public std::integral_constant<bool, contains<T, List...>::value>
		{};

		// **************************************************************************

		//! True if a type list contains repeated types
		template <typename T, typename ... Rest>
		struct has_duplicates
			// True if T is contained in the rest. Check recursively for every type. Use disjunction to short circuit
			: public std::integral_constant<bool, contains<T, Rest...>::value || has_duplicates<Rest...>::value>
		{};

		//! True if a type list contains repeated types. Base case for recursion
		template <typename T>
		struct has_duplicates<T>
			: public std::false_type
		{};

		//! True if a type list contains repeated types. Specialization for type_list
		template <typename ... List>
		struct has_duplicates<type_list<List...>>
			: public std::integral_constant<bool, has_duplicates<List...>::value>
		{};

		// **************************************************************************

		//! Returns the index of the first instance of a type in a list or the size of
		//! the list (index of end) if not found
		template <typename T, typename First, typename ... Rest>
		struct find
			: public std::conditional<
			std::is_same<T, First>::value,
			std::integral_constant<size_t, 0>,
			std::integral_constant<size_t, 1 + find<T, Rest...>::value>>::type
		{};

		//! Returns the index of the first instance of a type in a list or the size of
		//! the list (index of end) if not found. Base case for recursion
		template <typename T, typename First>
		struct find<T, First>
			: public std::conditional<
			std::is_same<T, First>::value,
			std::integral_constant<size_t, 0>,
			std::integral_constant<size_t, 1>>::type
		{};

		//! Returns the index of the first instance of a type in a list or the size of
		//! the list (index of end) if not found. Specialization for type_list
		template <typename T, typename ... List>
		struct find<T, type_list<List...>>
			: public std::integral_constant<size_t, find<T, List...>::value>
		{};

		// **************************************************************************

		//! Returns the size of a type list
		template <typename ... Ts>
		struct size
			: public std::integral_constant<size_t, sizeof...(Ts)>
		{};

		//! Returns the size of a type list. Specialization for type_list
		template <typename ... Ts>
		struct size<type_list<Ts...>>
			: public std::integral_constant<size_t, sizeof...(Ts)>
		{};

	} // namespace mpl
} // namespace json

#endif // ALGORITHM_HH_INCLUDE_GUARD

