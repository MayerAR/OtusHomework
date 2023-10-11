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

	struct AllocatorHeader
	{
		AllocatorHeader(){}
		AllocatorHeader(std::size_t extension)
			: extension(extension)
		{
		}

		std::size_t Sizeof() const
		{
			return static_cast<std::size_t>(ceil_int32(extension / static_cast<double>(CHAR_BIT)));
		}

		bool IsAllFree() const
		{
			for (uint8_t* byte = header(); byte != header() + Sizeof(); ++byte)
			{
				if (*byte & 0xFF)
					return false;
			}
			return true;
		}

		[[nodiscard]] bool GetAvailablePos(std::size_t objsNumb, std::size_t& pos) const
		{
			std::optional<std::size_t> counter = std::nullopt;
			auto findPos = [&pos, &objsNumb, &counter](bool bit, std::size_t bitIndex) {
				if (!bit)
				{
					if (counter == std::nullopt)
					{
						counter = 0;
						pos = bitIndex;
					}
					++(*counter);
					if (objsNumb <= counter)
						return false;
				}
				else
				{
					counter = std::nullopt;
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

			if (header()[byteCount] & bitMask)
				return false;

			header()[byteCount] += bitMask;
			return true;
		}

		[[nodiscard]] bool SetPosFree(std::size_t pos)
		{
			const auto byteCount = static_cast<std::size_t>(std::floor(pos / CHAR_BIT));
			const auto bitCount = pos - byteCount * CHAR_BIT;
			const uint8_t bitMask = (0x1 << bitCount);

			if ((header()[byteCount] | ~bitMask) != 0xFF)
				return false;

			header()[byteCount] -= bitMask;
			return true;
		}

		void write(std::ostream& stream) const
		{
			stream << '[';
			auto writeBit = [&stream](bool bit, std::size_t /*bitIndex*/) {
				stream << (bit ? 1 : 0) << ',' ;
				return true;
			};

			foreach_bit(writeBit);
			stream.seekp(-1, std::ios_base::cur);
			stream << "]\n";
		}

		std::size_t GetExtension() const
		{
			return extension;
		}

	
	protected:
		virtual uint8_t* header() const = 0;

		using BitJob = std::function<bool/*continue?*/(bool/*bit*/, std::size_t /*bitIndex*/)>;
		void foreach_bit(const BitJob& bitJob) const
		{
			uint8_t byte = header()[0];
			uint8_t mask = 0x01;
			for (int bitIndex = 0; bitIndex < extension; ++bitIndex)
			{
				if (bitIndex % 8 == 0)
				{
					byte = header()[static_cast<uint8_t>(std::floor(bitIndex / CHAR_BIT))];
					mask = 0x1;
				}

				bool bit = byte & mask;
				if (!bitJob(bit, bitIndex))
					break;
				mask <<= 1;
			}
		}


		std::size_t extension = 0;
	};

	template <std::size_t Extension>
	class StackHeader
		: public AllocatorHeader
	{
	public:
		StackHeader()
			: AllocatorHeader(Extension)
		{
			init();
		}

	private:
		void init()
		{
			std::memset(header(), 0, Sizeof() * sizeof(uint8_t));
		}

		uint8_t* header() const override
		{
			return const_cast<StackHeader*>(this)->_header + 0;
		}

		static constexpr std::size_t Sizeof()
		{
			return static_cast<std::size_t>(ceil_int32(Extension / static_cast<double>(CHAR_BIT)));
		}

		uint8_t _header[Sizeof()];
	};

	class HeapHeader
		: public AllocatorHeader
	{
		using Base = AllocatorHeader;
	public:
		HeapHeader(std::size_t extension)
			: AllocatorHeader(extension)
		{
			createHeader();
			initHeader();
		}
		HeapHeader(const HeapHeader& other)
			: AllocatorHeader(other.GetExtension())
		{
			createHeader();
			std::memcpy(_header, other._header, Sizeof());
		}
		HeapHeader(HeapHeader&& other)
		{
			_header = other._header;
			extension = other.extension;
			other._header = nullptr;
			other.extension = 0;
		}

		HeapHeader& operator=(HeapHeader&& other)
		{
			if (this != &other)
			{
				_header = other._header;
				extension = other.extension;
				other._header = nullptr;
				other.extension = 0;
			}
			return *this;
		}


		~HeapHeader()
		{
			delete _header;
		}

	private:
		void createHeader()
		{
			_header = new uint8_t[Sizeof()];
		}

		void initHeader()
		{
			std::memset(header(), 0, Sizeof() * sizeof(uint8_t));
		}

		uint8_t* header() const override
		{
			return const_cast<HeapHeader*>(this)->_header + 0;
		}
		uint8_t* _header = nullptr;
	};


	namespace stack
	{
		template <typename T, std::size_t Extension = 50>
		class allocator
		{
		public:
			using value_type = T;
		private:
			using Header = StackHeader<Extension>;
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


	namespace heap
	{
		template <typename T>
		class allocator
		{
		public:
			using value_type = T;
			explicit allocator(std::size_t extension = 5) noexcept
				: _header(extension)
			{
			}

			template <typename U>
			allocator(const allocator<U>& other) noexcept 
				: _header(other.GetExtension())
			{
			}

			~allocator()
			{
				operator delete(_data);
			}


			[[nodiscard]] T* allocate(std::size_t n)
			{
				std::cout << this << ": " << __FUNCSIG__ << ' ' << n << std::endl;

				if (!_data)
					_data = reinterpret_cast<T*>(operator new(sizeof(T) * GetExtension()));

				if (_header.GetExtension() < n)
					throw std::bad_array_new_length();
				
				std::size_t pos = 0;
				auto copyHeader = _header;
				if (_header.GetAvailablePos(n, pos))
				{
					for (std::size_t i = pos; i < n + pos; ++i)
					{
						if (!_header.SetPosReserved(i))
						{
							_header = std::move(copyHeader);
							throw std::bad_alloc();
						}
					}
					return _data + pos;
				}
				return nullptr;
			}

			void deallocate(T* p, std::size_t n)
			{
				std::size_t pos = p - _data;
				if (GetExtension() < pos || GetExtension() < pos + n)
					return;

				auto copyHeader = _header;
				for (std::size_t i = pos; i < pos + n; ++i)
				{
					if (!copyHeader.SetPosFree(i))
					{
						_header = std::move(copyHeader);
						return;
					}
				}
			}


			std::size_t GetExtension() const
			{
				return _header.GetExtension();
			}

			allocator select_on_container_copy_construction() const
			{
				return allocator<T>(GetExtension());
			}

			template <typename U> struct rebind {
				typedef allocator<U> other;
			};

			using propagate_on_container_copy_assignment = std::true_type;
			using propagate_on_container_move_assignment = std::true_type;
			using propagate_on_container_swap = std::true_type;

		private:
			HeapHeader _header;
			value_type* _data = nullptr;
		};
	}
}