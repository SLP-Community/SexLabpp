#pragma once

#include <cassert>
#include <vector>

namespace Util
{
	template <typename T, size_t N>
	struct RingBuffer
	{
	  public:
		RingBuffer()
		{
			_buffer.reserve(N);
		}
		template <typename... Args>
		RingBuffer(Args&&... args) : _head{ sizeof...(Args) % N }, _size{ sizeof...(Args) }
		{
			_buffer.reserve(N);
			(_buffer.emplace_back(std::forward<Args>(args)), ...);
		}
		~RingBuffer() = default;

	  public:
		T& push(const T& value)
		{
			if (_buffer.size() < N) {
				_buffer.emplace_back(value);
			} else {
				_buffer[_head] = value;
			}
			auto& ret = _buffer[_head];
			_head = (_head + 1) % N;
			if (_size < N) {
				_size++;
			}
			return ret;
		}

		const std::vector<T>& view() const
		{
			return _buffer;
		}

		template <typename K>
		K to() const
		{
			K result;
			if constexpr (requires { result[0]; result.size(); }) {
				if constexpr (result.size() != N) {
					static_assert(sizeof(K) == 0, "Type K must have size() equal to N");
				}
				for (size_t i = 0; i < _size; ++i) {
					result[i] = (*this)[i];
				}
			} else if constexpr (requires { result.push_back(std::declval<T>()); }) {
				for (size_t i = 0; i < _size; ++i) {
					result.push_back((*this)[i]);
				}
			} else {
				static_assert(sizeof(K) == 0, "Type K must support either operator[] with size() or push_back()");
			}
			return result;
		}

	  public:
		const T& front() const
		{
			assert(_size > 0);
			return (*this)[0];
		}
		T& front()
		{
			assert(_size > 0);
			return (*this)[0];
		}
		const T& back() const
		{
			assert(_size > 0);
			return (*this)[_size - 1];
		}
		T& back()
		{
			assert(_size > 0);
			return (*this)[_size - 1];
		}

		size_t head() const { return _head; }
		size_t index() const { return _head; }
		size_t tail() const { return (_head - _size + N) % N; }

		size_t size() const { return _size; }
		size_t length() const { return _size; }

		size_t capacity() const { return N; }

		bool empty() const { return _size == 0; }
		bool full() const { return _size == N; }

		const T& operator[](size_t index) const { return _buffer[(_head - _size + index + N) % N]; }
		T& operator[](size_t index) { return _buffer[(_head - _size + index + N) % N]; }

	  private:
		std::vector<T> _buffer;
		size_t _head{ 0 };
		size_t _size{ 0 };
	};

}  // namespace Util
