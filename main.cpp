#include <iostream>
#include <vector>
#include <list>
#include <string>
#include <tuple>


namespace ip_format
{
	template <typename TCont
		, typename = std::decay_t<decltype(std::declval<TCont>().begin())>
		, typename = std::decay_t<decltype(std::declval<TCont>().end())>>
		void print(const TCont& cont)
	{
		for (const auto& elem : cont)
			std::cout << elem << ".";
		std::cout << std::endl;
	}

	template <>
	void print<std::string>(const std::string& str)
	{
		std::cout << str << std::endl;
	}

	template <typename TInt>
	std::enable_if_t<std::is_integral_v<TInt>, void>
	print(TInt var)
	{
		constexpr uint8_t bytesCount = sizeof(std::declval<TInt>());
		uint8_t* arr = reinterpret_cast<uint8_t*>(&var);
		std::string str;
		for (int i = bytesCount - 1; i >= 0; --i)
			str += std::to_string(arr[i]) + ".";
		str.pop_back();
		print(str);
	}

	template <size_t N>
	struct PrintElem
	{
		template < typename TTuple>
		void operator()(const TTuple & tuple)
		{
			PrintElem<N - 1>()(tuple);
			std::cout << std::get<N>(tuple) << ".";
		}
	};

	template<>
	struct PrintElem<0>
	{
		template <typename TTuple>
		void operator()(const TTuple& tuple)
		{
			std::cout << std::get<0>(tuple) << ".";
		}
	};

	template <typename T, typename... Args>
	struct IsSameTypes
	{
		using type = std::enable_if_t<std::is_same<T, typename IsSameTypes<Args...>::type>::value, T>;
	};

	template <typename T>
	struct IsSameTypes<T>
	{
		using type = T;
	};

	template <typename... Args, typename = IsSameTypes<Args...>::type>
	void print(const std::tuple<Args...>& tuple)
	{
		PrintElem<sizeof...(Args) - 1>()(tuple);
	}

}


int main()
{
	ip_format::print(int8_t{ -1 }); // 255
	ip_format::print(int16_t{ 0 }); // 0.0
	ip_format::print(int32_t{ 2130706433 }); // 127.0.0.1
	ip_format::print(int64_t{ 8875824491850138409 });// 123.45.67.89.101.112.131.41
	ip_format::print(std::string{ "Hello, World!"}); // Hello, World!
	ip_format::print(std::vector<int>{100, 200, 300, 400}); // 100.200.300.400
	ip_format::print(std::list<short>{400, 300, 200, 100}); // 400.300.200.100
	ip_format::print(std::make_tuple(123, 456, 789, 0)); // 123.456.789.0

	std::cin.get();
}
