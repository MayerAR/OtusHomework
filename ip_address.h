#pragma once
#include <cstdint>
#include <istream>


namespace ip4
{
	class StdAddress
	{
	public:
		enum ByteIndex : uint8_t
		{
			Byte_0 = 0,
			Byte_1,
			Byte_2,
			Byte_3,
			BytesCount
		};
	public:

		StdAddress();
		StdAddress(uint8_t, uint8_t, uint8_t, uint8_t);
		StdAddress(const StdAddress&) = default;

		uint8_t GetByte(ByteIndex) const;
		void Clear();

		friend std::istream& operator>>(std::istream&, StdAddress&);
		friend std::ostream& operator<<(std::ostream&, const StdAddress&);
		
		bool operator<(const StdAddress&) const;
		bool operator==(const StdAddress&) const;
		bool operator>(const StdAddress&) const;

	private:
		static constexpr char delimiter();
		uint8_t _data[BytesCount];
	};

	bool HasByte(const StdAddress&, uint8_t byte);
}