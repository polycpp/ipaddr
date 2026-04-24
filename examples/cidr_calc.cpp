// Print the network address, broadcast address, and subnet mask for
// a CIDR given on the command line.
//
//   $ ./cidr_calc 192.168.1.50/24
//   input:     192.168.1.50/24
//   network:   192.168.1.0
//   broadcast: 192.168.1.255
//   netmask:   255.255.255.0

#include <iostream>
#include <string>

#include <polycpp/ipaddr/ipaddr.hpp>

int main(int argc, char** argv) {
    using namespace polycpp::ipaddr;

    if (argc != 2) {
        std::cerr << "usage: cidr_calc <cidr>\n";
        return 2;
    }
    const std::string cidr = argv[1];

    if (IPv4::isValidCIDR(cidr)) {
        auto [addr, bits] = IPv4::parseCIDR(cidr);
        IPv4 net   = IPv4::networkAddressFromCIDR(cidr);
        IPv4 bcast = IPv4::broadcastAddressFromCIDR(cidr);
        IPv4 mask  = IPv4::subnetMaskFromPrefixLength(bits);
        std::cout << "input:     " << cidr << '\n';
        std::cout << "network:   " << net.toString()   << '\n';
        std::cout << "broadcast: " << bcast.toString() << '\n';
        std::cout << "netmask:   " << mask.toString()  << '\n';
    } else if (IPv6::isValidCIDR(cidr)) {
        auto [addr, bits] = IPv6::parseCIDR(cidr);
        IPv6 net   = IPv6::networkAddressFromCIDR(cidr);
        IPv6 bcast = IPv6::broadcastAddressFromCIDR(cidr);
        IPv6 mask  = IPv6::subnetMaskFromPrefixLength(bits);
        std::cout << "input:     " << cidr << '\n';
        std::cout << "network:   " << net.toString()   << '\n';
        std::cout << "broadcast: " << bcast.toString() << '\n';
        std::cout << "netmask:   " << mask.toString()  << '\n';
    } else {
        std::cerr << "not a valid CIDR: " << cidr << '\n';
        return 1;
    }
    return 0;
}
