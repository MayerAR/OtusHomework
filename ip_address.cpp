#include "ip_address.h"

namespace ip4
{
	bool HasByte(const StdAddress& address, uint8_t byte)
	{
		for (int i = 0; i < StdAddress::BytesCount; ++i)
			if (byte == address.GetByte(static_cast<StdAddress::ByteIndex>(i)))
				return true;
		return false;
	}

	StdAddress::StdAddress()
	{
		Clear();
	}
	StdAddress::StdAddress(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3)
		: _data{b0, b1, b2 , b3}
	{}


	uint8_t StdAddress::GetByte(ByteIndex index) const
	{
		return _data[index];
	}

	void StdAddress::Clear()
	{
		std::memset(_data, 0, BytesCount * sizeof(decltype(*_data)));
	}

	std::istream& operator>>(std::istream& stream, StdAddress& address)
	{
		auto getByte = [&stream](uint8_t& byte)
		{
			int temp = 0;
			stream >> temp;
			byte = static_cast<uint8_t>(temp);
			return !!stream && byte == temp;
		};
		auto checkDelim = [&stream](char& ch, const char delimeter)
		{
			stream >> ch;
			return stream && ch == delimeter;
		};

		auto delim = StdAddress::delimiter();
		auto bytesCount = StdAddress::BytesCount;
		for (int i = 0; i < bytesCount - 1; ++i)
		{
			char ch;
			if (!getByte(address._data[i]) || !checkDelim(ch, delim))
				return stream;
		}
		if (!getByte(address._data[bytesCount - 1]))
			return stream;

		return stream;
	}
	std::ostream& operator<<(std::ostream& stream, const StdAddress& address)
	{
		auto bytesCount = StdAddress::BytesCount;
		for (int i = 0; i < bytesCount - 1; ++i)
			stream << std::dec << static_cast<int>(address._data[i]) << address.delimiter();
		stream << static_cast<int>(address._data[bytesCount - 1]);
		return stream;
	}

	bool StdAddress::operator<(const StdAddress& right) const
	{
		for (int i = 0; i < BytesCount; ++i)
		{
			if (_data[i] != right._data[i])
				return _data[i] < right._data[i];
		}
		return false;
	}	
	
	bool StdAddress::operator==(const StdAddress& right) const
	{
		for (int i = 0; i < BytesCount; ++i)
		{
			if (_data[i] != right._data[i])
				return false;
		}
		return true;
	}

	bool StdAddress::operator>(const StdAddress& right) const
	{
		return !(*this < right || *this == right);
	}

	constexpr char StdAddress::delimiter()
	{
		return '.';
	}
}
