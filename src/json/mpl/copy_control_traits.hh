#ifndef COPY_CONTROL_TRAITS_HH_INCLUDE_GUARD
#define COPY_CONTROL_TRAITS_HH_INCLUDE_GUARD

namespace json
{
	namespace mpl
	{

		//! Base class for a class that represents the copy control operations of another class.
		//! Used in order to dynamically access the functions as in a virtual table.
		struct copy_control_traits_base
		{
			template <typename T>
			using function_ptr = T *;

			constexpr copy_control_traits_base(
				function_ptr<void(void *, const void *)> copy_constructor,
				function_ptr<void(void *, void *)> move_constructor,
				function_ptr<void(void *, const void *)> copy_assignment_operator,
				function_ptr<void(void *, void *)> move_assignment_operator,
				function_ptr<void(void *)> destructor
			)
				: copy_construct(copy_constructor)
				, move_construct(move_constructor)
				, copy_assign(copy_assignment_operator)
				, move_assign(move_assignment_operator)
				, destroy(destructor)
			{}

			function_ptr<void(void *, const void *)> copy_construct;
			function_ptr<void(void *, void *)>		 move_construct;
			function_ptr<void(void *, const void *)> copy_assign;
			function_ptr<void(void *, void *)>		 move_assign;
			function_ptr<void(void *)>				 destroy;
		};

		//! Represents the copy control operations of another class
		template <typename T>
		struct copy_control_traits : public copy_control_traits_base
		{
			constexpr copy_control_traits()
				: copy_control_traits_base
			{ &copy_control_traits<T>::copy_constructor
			, &copy_control_traits<T>::move_constructor
			, &copy_control_traits<T>::copy_assignment_operator
			, &copy_control_traits<T>::move_assignment_operator
			, &copy_control_traits<T>::destructor
			}
			{}

			static void copy_constructor(void * buffer, const void * other);
			static void move_constructor(void * buffer, void * other);
			static void copy_assignment_operator(void * buffer, const void * other);
			static void move_assignment_operator(void * buffer, void * other);
			static void destructor(void * buffer);

			static const copy_control_traits<T> global_instance;
		};

	} // namespace mpl
} // namespace json

#include "copy_control_traits.inl"

#endif // COPY_CONTROL_TRAITS_HH_INCLUDE_GUARD
