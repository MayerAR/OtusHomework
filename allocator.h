#pragma once
#include <assert.h>
#include <optional>
#include <functional>


namespace my_allocator
{
	constexpr double ceil_int32(double num)
	{
		//assert(num > std::numeric_limits<int32_t>::max());
		return (static_cast<double>(static_cast<int32_t>(num)) == num)
			? static_cast<int32_t>(num)
			: static_cast<int32_t>(num) + ((num > 0) ? 1 : 0);
	}

	template <std::size_t Extension>
	struct AllocatorHeader
	{
		AllocatorHeader()
		{
			std::memset(header, 0, Sizeof() * sizeof(uint8_t));
		}

		static constexpr std::size_t Sizeof()
		{
			return static_cast<std::size_t>(ceil_int32(Extension / static_cast<double>(CHAR_BIT)));
		}

		bool IsAllFree() const
		{
			for (uint8_t* byte = header; byte != header + Sizeof(); ++byte)
			{
				if (*byte & 0xFF)
					return false;
			}
			return true;
		}

		[[nodiscard]] bool GetAvailablePos(std::size_t objsNumb, std::size_t& pos) const
		{
			std::optional<std::size_t> counter = 0;
			auto findPos = [&pos, &objsNumb, &counter](bool bit, std::size_t bitIndex) {
				if (!bit)
				{
					if (counter == 0)
						pos = bitIndex;
					++(*counter);
					if (objsNumb <= counter)
						return false;
				}
				else
				{
					counter = 0;
				}
				return true;
			};

			foreach_bit(findPos);
			return counter.has_value();
		}

		[[nodiscard]] bool SetPosReserved(std::size_t pos)
		{
			const auto byteCount = static_cast<std::size_t>(std::floor(pos / CHAR_BIT));
			const auto bitCount = pos - byteCount * CHAR_BIT;
			const uint8_t bitMask = (0x1 << bitCount);

			if (header[byteCount] & bitMask)
				return false;

			header[byteCount] += bitMask;
			return true;
		}

		[[nodiscard]] bool SetPosFree(std::size_t pos)
		{
			const auto byteCount = static_cast<std::size_t>(std::floor(pos / CHAR_BIT));
			const auto bitCount = pos - byteCount * CHAR_BIT;
			const uint8_t bitMask = (0x1 << bitCount);

			if ((header[byteCount] | ~bitMask) != 0xFF)
				return false;

			header[byteCount] -= bitMask;
			return true;
		}

		void write(std::ostream& stream) const
		{
			stream << '[';
			auto writeBit = [&stream](bool bit, std::size_t bitIndex) {
				stream << bit ? 1 : 0 << , ;
			};

			foreach_bit(writeBit);
			stream.seekp(stream.tellp() - 1);
			stream << "]\n";
		}

	private:
		using BitJob = std::function<bool/*continue?*/(bool/*bit*/, std::size_t /*bitIndex*/)>;
		void foreach_bit(const BitJob& bitJob) const
		{
			uint8_t byte = header[0];
			uint8_t mask = 0x01;
			for (int bitIndex = 0; bitIndex < Extension; ++bitIndex)
			{
				if (bitIndex & 0x08)
				{
					byte = header[static_cast<uint8_t>(std::floor(bitIndex / CHAR_BIT))];
					mask = 0x1;
				}

				bool bit = byte & mask;
				if (!bitJob(bit, bitIndex))
					break;
				mask <<= 1;
			}
		}

		uint8_t header[Sizeof()];
	};

	namespace stack
	{
		template<class T>
		struct Mallocator
		{
			typedef T value_type;

			Mallocator() = default;

			template<class U>
			constexpr Mallocator(const Mallocator <U>&) noexcept {}

			[[nodiscard]] T* allocate(std::size_t n)
			{
				if (n > std::numeric_limits<std::size_t>::max() / sizeof(T))
					throw std::bad_array_new_length();

				if (auto p = static_cast<T*>(std::malloc(n * sizeof(T))))
				{
					report(p, n);
					return p;
				}

				throw std::bad_alloc();
			}

			void deallocate(T* p, std::size_t n) noexcept
			{
				report(p, n, 0);
				std::free(p);
			}
		private:
			void report(T* p, std::size_t n, bool alloc = true) const
			{
				std::cout << (alloc ? "Alloc: " : "Dealloc: ") << sizeof(T) * n
					<< " bytes at " << std::hex << std::showbase
					<< reinterpret_cast<void*>(p) << std::dec << '\n';
			}
		};

		template <typename T, std::size_t Extension = 50>
		class allocator
		{
		public:
			using value_type = T;
		private:
			using Header = AllocatorHeader<Extension>;
		public:
			allocator()
			{
				std::cout << __FUNCSIG__ << std::endl;
				std::memset(_data, 0, sizeof(Extension * sizeof(value_type)));
			}

			template <typename U, std::size_t UExtension>
			allocator(const allocator<U, UExtension>&) noexcept
				: allocator() 
			{
				std::cout << __FUNCSIG__ << std::endl;
			}
			allocator(const allocator<T, Extension>& other)
				: header(other.header)
			{
				std::memcpy(_data, other._data, Extension * sizeof(value_type));
				std::cout << __FUNCSIG__ << std::endl;
			}

			[[nodiscard]] T* allocate(std::size_t n)
			{
				std::cout << this << ": " << __FUNCSIG__ << ' ' << n << std::endl;


				if (Extension < n)
					throw std::bad_array_new_length();

				std::size_t pos = 0;
				auto copyHeader = header;	
				if (copyHeader.GetAvailablePos(n, pos))
				{
					for (std::size_t i = pos; i < n + pos; ++i)
					{
						if (!header.SetPosReserved(i))
						{
							std::swap(copyHeader, header);
							throw std::bad_alloc();
						}
					}
					return reinterpret_cast<T*>(data()) + pos;
				}
				return nullptr;
			}

			void deallocate(T* p, std::size_t n)
			{
				std::size_t pos = p - data();
				if (Extension < pos || Extension < pos + n)
					return;
				
				auto copyHeader = header;
				for (std::size_t i = pos; i < pos + n; ++i)
				{
					if (!copyHeader.SetPosFree(i))
					{
						header = copyHeader;
						return;
					}
				}
			}

			void displayMemory(std::ostream& stream) const
			{
				header.write(stream);
			}

			template <typename U>
			struct rebind
			{
				using other = allocator<U, Extension>;
			};


		private:
			T* data() 
			{
				return reinterpret_cast<T*>(_data + 0);
			}
			
			Header header;
			uint8_t _data[Extension * sizeof(value_type)];
		};
	}


	namespace dynamic
	{
		template <typename T>
		class allocator
		{
		public:
			explicit allocator(std::size_t extension) noexcept;
			template <typename U>
			allocator(const allocator<U>&) noexcept {}
			allocator(const allocator<T>& other) {}

		private:

		};
	}

}