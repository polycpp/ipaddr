// Check a client address against a compiled allow-list. The list is
// baked into the example so it's self-contained — in a real service
// you would load it from config.
//
//   $ ./allow_check 10.1.2.3
//   10.1.2.3: allow
//   $ ./allow_check 8.8.8.8
//   8.8.8.8: deny

#include <iostream>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include <polycpp/ipaddr/ipaddr.hpp>

using namespace polycpp::ipaddr;

struct AllowList {
    std::vector<std::pair<IPv4, int>> v4;
    std::vector<std::pair<IPv6, int>> v6;
};

static AllowList compile(const std::vector<std::string>& cidrs) {
    AllowList out;
    for (const auto& cidr : cidrs) {
        auto slash = cidr.find('/');
        std::string host = slash == std::string::npos
            ? cidr : cidr.substr(0, slash);
        if (IPv6::isIPv6(host))
            out.v6.push_back(IPv6::parseCIDR(cidr));
        else
            out.v4.push_back(IPv4::parseCIDR(cidr));
    }
    return out;
}

static bool is_allowed(const AllowList& list, const std::string& addr) {
    if (!isValid(addr)) return false;
    auto v = parse(addr);
    return std::visit([&](const auto& a) -> bool {
        using T = std::decay_t<decltype(a)>;
        if constexpr (std::is_same_v<T, IPv4>) {
            for (const auto& r : list.v4)
                if (a.match(r)) return true;
        } else {
            for (const auto& r : list.v6)
                if (a.match(r)) return true;
        }
        return false;
    }, v);
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: allow_check <address>\n";
        return 2;
    }

    const AllowList list = compile({
        "10.0.0.0/8",
        "192.168.0.0/16",
        "2001:db8::/32",
    });

    std::cout << argv[1] << ": "
              << (is_allowed(list, argv[1]) ? "allow" : "deny") << '\n';
    return 0;
}
