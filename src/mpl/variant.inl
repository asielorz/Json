namespace json
{
	namespace mpl
	{

		template <typename ... Ts>
		const copy_control_traits_base * variant<Ts...>::vtables[mpl::size<value_types>::value] = {
			&copy_control_traits<Ts>::global_instance...
		};

		template <typename ... Ts>
		variant<Ts...>::~variant()
		{
			destroy();
		}

		template <typename ... Ts>
		template <typename T, typename ... Args>
		variant<Ts...>::variant(mpl::type_list<T>, Args && ... args)
		{
			emplace<T>(std::forward<Args>(args)...);
		}

		template <typename ... Ts>
		template <typename T>
		variant<Ts...>::variant(T && val)
		{
			// Decay in order to remove cv qualifiers and references
			emplace<typename std::decay<T>::type>(std::forward<T>(val));
		}

		template <typename ... Ts>
		template <typename T>
		variant<Ts...> & variant<Ts...>::operator = (T && val)
		{
			// Decay in order to remove cv qualifiers and references
			using actual_type = typename std::decay<T>::type;

			try
			{
				// If the type of the managed object is the type of the assigned object, use the assignment operator
				this->as<actual_type>() = std::forward<T>(val);
			}
			catch (const std::bad_cast &)
			{
				// Otherwise emplace
				emplace<typename std::decay<T>::type>(std::forward<T>(val));
			}
			return *this;
		}

		template <typename ... Ts>
		variant<Ts...>::variant(const variant & other)
			: vtable_index(other.vtable_index)
		{
			if (valid())
				vtables[vtable_index]->copy_construct(buffer, other.buffer);
		}

		template <typename ... Ts>
		variant<Ts...>::variant(variant && other)
			: vtable_index(other.vtable_index)
		{
			other.vtable_index = npos;
			if (valid())
				vtables[vtable_index]->move_construct(buffer, other.buffer);
		}

		template <typename ... Ts>
		variant<Ts...> & variant<Ts...>::operator = (const variant & other)
		{
			if (vtable_index == other.vtable_index)
			{
				if (valid())
					vtables[vtable_index]->copy_assign(buffer, other.buffer);
			}
			else
			{
				destroy();
				vtable_index = other.vtable_index;
				if (valid())
					vtables[vtable_index]->copy_construct(buffer, other.buffer);
			}

			return *this;
		}

		template <typename ... Ts>
		variant<Ts...> & variant<Ts...>::operator = (variant && other)
		{
			if (vtable_index == other.vtable_index)
			{
				if (valid())
					vtables[vtable_index]->move_assign(buffer, other.buffer);
			}
			else
			{
				destroy();
				vtable_index = other.vtable_index;
				if (valid())
					vtables[vtable_index]->move_construct(buffer, other.buffer);
			}

			other.vtable_index = npos;
			return *this;
		}

		template <typename ... Ts>
		template <typename T, typename ... Args>
		void variant<Ts...>::emplace(Args && ... args)
		{
			static_assert(contains<T, value_types>::value, "Invalid type for function emplace of variant");

			destroy();
			vtable_index = find<T, value_types>::value;
			::new (buffer) T(std::forward<Args>(args)...);
		}

		template <typename ... Ts>
		void variant<Ts...>::destroy()
		{
			if (valid())
			{
				vtables[vtable_index]->destroy(buffer);
				vtable_index = npos;
			}
		}

		template <typename ... Ts>
		template <typename T>
		T & variant<Ts...>::as()
		{
			static_assert(contains<T, value_types>::value, "Invalid type for function as of variant");

			// Check if the type of this variant is the requested type
			if (vtables[vtable_index] == &copy_control_traits<T>::global_instance)
				return reinterpret_cast<T &>(buffer);
			else
				throw std::bad_cast{};
		}

		template <typename ... Ts>
		template <typename T>
		const T & variant<Ts...>::as() const
		{
			static_assert(contains<T, value_types>::value, "Invalid type for function as of variant");

			// Check if the type of this variant is the requested type
			if (vtables[vtable_index] == &copy_control_traits<T>::global_instance)
				return reinterpret_cast<const T &>(buffer);
			else
				throw std::bad_cast{};
		}

		template <typename ... Ts>
		template <typename T>
		bool variant<Ts...>::is() const noexcept
		{
			static_assert(contains<T, value_types>::value, "Invalid type for function is of variant");

			return vtable_index == mpl::find<T, value_types>::value;
		}

		template <typename ... Ts>
		size_t variant<Ts...>::index() const noexcept
		{
			return vtable_index;
		}

		template <typename ... Ts>
		bool variant<Ts...>::valid() const noexcept
		{
			return vtable_index != npos;
		}

	} // namespace mpl
} // namespace json

