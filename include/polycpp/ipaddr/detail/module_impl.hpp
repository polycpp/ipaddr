#pragma once

/**
 * @file module_impl.hpp
 * @brief Module-level function implementations (all inline).
 * @since 0.1.0
 */

#include <polycpp/ipaddr/ipaddr.hpp>
#include <polycpp/ipaddr/detail/ipv4_impl.hpp>
#include <polycpp/ipaddr/detail/ipv6_impl.hpp>

#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

namespace polycpp {
namespace ipaddr {

inline std::variant<IPv4, IPv6> parse(const std::string& addr) {
    if (IPv6::isValid(addr)) {
        return IPv6::parse(addr);
    } else if (IPv4::isValid(addr)) {
        return IPv4::parse(addr);
    }
    throw std::invalid_argument("ipaddr: the address has neither IPv6 nor IPv4 format");
}

inline bool isValid(const std::string& addr) {
    return IPv6::isValid(addr) || IPv4::isValid(addr);
}

inline bool isValidCIDR(const std::string& addr) {
    return IPv6::isValidCIDR(addr) || IPv4::isValidCIDR(addr);
}

inline std::pair<std::variant<IPv4, IPv6>, int> parseCIDR(const std::string& addr) {
    try {
        auto [v6, bits] = IPv6::parseCIDR(addr);
        return {v6, bits};
    } catch (const std::invalid_argument&) {
        try {
            auto [v4, bits] = IPv4::parseCIDR(addr);
            return {v4, bits};
        } catch (const std::invalid_argument&) {
            throw std::invalid_argument(
                "ipaddr: the address has neither IPv6 nor IPv4 CIDR format");
        }
    }
}

inline std::variant<IPv4, IPv6> fromByteArray(const std::vector<uint8_t>& bytes) {
    if (bytes.size() == 4) {
        return IPv4(std::array<uint8_t, 4>{bytes[0], bytes[1], bytes[2], bytes[3]});
    } else if (bytes.size() == 16) {
        std::array<uint8_t, 16> arr;
        for (int i = 0; i < 16; i++) {
            arr[i] = bytes[i];
        }
        return IPv6(arr);
    }
    throw std::invalid_argument(
        "ipaddr: the binary input is neither an IPv6 nor IPv4 address");
}

inline std::variant<IPv4, IPv6> process(const std::string& addr) {
    auto result = parse(addr);
    return std::visit([](auto&& a) -> std::variant<IPv4, IPv6> {
        using T = std::decay_t<decltype(a)>;
        if constexpr (std::is_same_v<T, IPv6>) {
            if (a.isIPv4MappedAddress()) {
                return a.toIPv4Address();
            }
        }
        return a;
    }, result);
}

inline std::string subnetMatch(
    const std::variant<IPv4, IPv6>& address,
    const std::vector<std::pair<std::string, std::vector<CIDRRange>>>& rangeList,
    const std::string& defaultName) {

    for (const auto& [rangeName, subnets] : rangeList) {
        for (const auto& subnet : subnets) {
            const auto& [network, prefix] = subnet;

            // Check if address and network are the same kind
            bool sameKind = std::visit([](const auto& addr, const auto& net) -> bool {
                using AddrT = std::decay_t<decltype(addr)>;
                using NetT = std::decay_t<decltype(net)>;
                return std::is_same_v<AddrT, NetT>;
            }, address, network);

            if (!sameKind) continue;

            bool matched = std::visit([prefix](const auto& addr, const auto& net) -> bool {
                using AddrT = std::decay_t<decltype(addr)>;
                using NetT = std::decay_t<decltype(net)>;
                if constexpr (std::is_same_v<AddrT, NetT>) {
                    return addr.match(net, prefix);
                }
                return false;
            }, address, network);

            if (matched) {
                return rangeName;
            }
        }
    }

    return defaultName;
}

} // namespace ipaddr
} // namespace polycpp
