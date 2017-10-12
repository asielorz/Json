#ifndef VARIANT_HH_INCLUDE_GUARD
#define VARIANT_HH_INCLUDE_GUARD

#include "algorithm.hh"
#include "copy_control_traits.hh"
#include <typeinfo> // std::bad_cast

namespace json
{
	namespace mpl
	{

		template <typename ... Ts>
		class variant
		{
		public:
			//! List of types of the variant
			using value_types = mpl::type_list<Ts...>;
			static_assert(!has_duplicates<value_types>::value, "A variant can't contain duplicated types");

			//! Index of an invalid variant
			static constexpr size_t npos = static_cast<size_t>(-1);

			//! Default constructor constructs an invalid variant
			variant() noexcept = default;
			//! Destructor calls the destructor on the internally managed object, if any
			~variant();

			//! Constructs in place an object of type T
			template <typename T, typename ... Args>
			variant(mpl::type_list<T>, Args && ... args);

			//! Copy or move constructs an object of type T
			template <typename T>
			variant(T && val);

			//! Copy or move assigns an object of type T
			template <typename T>
			variant & operator = (T && val);

			//! Copy constructor from a variant of the same type
			variant(const variant & other);
			//! Move constructor from a variant of the same type
			variant(variant && other);

			//! Copy assignment from a variant of the same type
			variant & operator = (const variant & other);
			//! Move assignment from a variant of the same type
			variant & operator = (variant && other);

			//! Constructs in place an object of type T
			template <typename T, typename ... Args>
			void emplace(Args && ... args);

			//! Destroys the internally managed object and sets the variant to invalid
			void destroy();

			//! Accesses the internal object if it's of type T. Throws std::bad_cast otherwise
			template <typename T>
			T & as();

			//! Accesses the internal object if it's of type T. Throws std::bad_cast otherwise
			template <typename T>
			const T & as() const;

			//! True if T is the type of the internal object, false otherwise
			template <typename T>
			bool is() const noexcept;

			//! Returns the index of the type of the internal object in the list value_types or npos if not valid
			size_t index() const noexcept;

			//! True if there's a managed object
			bool valid() const noexcept;

		private:
			static constexpr size_t buffer_size = mpl::biggest_size<value_types>::value;
			//! Array with the virtual tables of the types of the variant
			static const copy_control_traits_base * vtables[mpl::size<value_types>::value];

			size_t vtable_index = npos;		  //! Index of the current type
			char buffer[buffer_size] = { 0 }; //! Buffer where the internal object is stored
		};

	} // namespace mpl
} // namespace json

#include "variant.inl"

#endif // VARIANT_HH_INCLUDE_GUARD
