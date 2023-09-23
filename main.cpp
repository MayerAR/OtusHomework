#include <iostream>
#include <sstream>
#include <set>
#include <fstream>
#include <algorithm>
#include <iterator>
#include "ip_address.h"

int main (int, char **) 
{
    std::multiset<ip4::StdAddress, std::greater<ip4::StdAddress>> addresses;
   
    std::istream& in = std::cin;
    while (in)
    {
        ip4::StdAddress address;
        if (in >> address)
            addresses.insert(address);        
        std::string str;
        std::getline(in, str);
    }

    //out
    std::ostream& out = std::cout;
    std::copy(addresses.begin(), addresses.end(), std::ostream_iterator<ip4::StdAddress>(out, "\n"));
	
	std::copy(addresses.lower_bound(ip4::StdAddress(2, 0, 0, 0))
    		, addresses.upper_bound(ip4::StdAddress(1, 0, 0, 0))
    		, std::ostream_iterator<ip4::StdAddress>(out, "\n"));

    std::copy(addresses.lower_bound(ip4::StdAddress(46, 71, 0, 0))
    		, addresses.upper_bound(ip4::StdAddress(46, 70, 0, 0))
    		, std::ostream_iterator<ip4::StdAddress>(out, "\n"));
    
    constexpr uint8_t byte = 46;
	std::copy_if(addresses.begin(), addresses.end(), std::ostream_iterator<ip4::StdAddress>(out, "\n"), 
        [byte](const auto& address) {
			return ip4::HasByte(address, byte);
		});
    
    //std::cin.get();
    return 0; 
}
 
