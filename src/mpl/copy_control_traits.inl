namespace json
{
	namespace mpl
	{

		template <typename T>
		void copy_control_traits<T>::copy_constructor(void * buffer, const void * other)
		{
			::new (buffer) T(*reinterpret_cast<const T *>(other));
		}

		template <typename T>
		void copy_control_traits<T>::move_constructor(void * buffer, void * other)
		{
			::new (buffer) T(std::move(*reinterpret_cast<T *>(other)));
		}

		template <typename T>
		void copy_control_traits<T>::copy_assignment_operator(void * buffer, const void * other)
		{
			*reinterpret_cast<T *>(buffer) = *reinterpret_cast<const T *>(other);
		}

		template <typename T>
		void copy_control_traits<T>::move_assignment_operator(void * buffer, void * other)
		{
			*reinterpret_cast<T *>(buffer) = std::move(*reinterpret_cast<T *>(other));
		}

		template <typename T>
		void copy_control_traits<T>::destructor(void * buffer)
		{
			reinterpret_cast<T *>(buffer)->~T();
		}

		template <typename T>
		const copy_control_traits<T> copy_control_traits<T>::global_instance;

	} // namespace mpl
} // namespace json

