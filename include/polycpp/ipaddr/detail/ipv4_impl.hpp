#pragma once

/**
 * @file ipv4_impl.hpp
 * @brief IPv4 class implementation (all functions inline).
 * @since 0.1.0
 */

#include <polycpp/ipaddr/ipaddr.hpp>
#include <polycpp/ipaddr/detail/helpers.hpp>

#include <cmath>
#include <sstream>
#include <stdexcept>
#include <string>

namespace polycpp {
namespace ipaddr {

// ══════════════════════════════════════════════════════════════════════
// Construction
// ══════════════════════════════════════════════════════════════════════

inline IPv4::IPv4(const std::array<uint8_t, 4>& octets) : octets_(octets) {
    // All uint8_t values are inherently in [0..255], no validation needed.
}

// ══════════════════════════════════════════════════════════════════════
// Internal parser
// ══════════════════════════════════════════════════════════════════════

inline std::optional<std::array<uint8_t, 4>> IPv4::parser(const std::string& addr) {
    std::smatch match;

    // Four-octet notation: a.b.c.d
    if (std::regex_match(addr, match, detail::ipv4FourOctet())) {
        try {
            std::array<uint8_t, 4> result;
            for (int i = 0; i < 4; i++) {
                long long val = detail::parseIntAuto(match[i + 1].str());
                if (val < 0 || val > 255) return std::nullopt;
                result[i] = static_cast<uint8_t>(val);
            }
            return result;
        } catch (...) {
            return std::nullopt;
        }
    }

    // Single long value: 0xc0a80101 or 3232235777 etc.
    if (std::regex_match(addr, match, detail::ipv4LongValue())) {
        try {
            long long value = detail::parseIntAuto(match[1].str());
            if (value > 0xffffffffLL || value < 0) {
                return std::nullopt;
            }
            auto v = static_cast<uint32_t>(value);
            return std::array<uint8_t, 4>{
                static_cast<uint8_t>((v >> 24) & 0xff),
                static_cast<uint8_t>((v >> 16) & 0xff),
                static_cast<uint8_t>((v >> 8) & 0xff),
                static_cast<uint8_t>(v & 0xff)
            };
        } catch (...) {
            return std::nullopt;
        }
    }

    // Two-octet notation: a.b (b is 24 bits)
    if (std::regex_match(addr, match, detail::ipv4TwoOctet())) {
        try {
            long long a = detail::parseIntAuto(match[1].str());
            long long b = detail::parseIntAuto(match[2].str());
            if (a < 0 || a > 255) return std::nullopt;
            if (b < 0 || b > 0xffffff) return std::nullopt;
            return std::array<uint8_t, 4>{
                static_cast<uint8_t>(a),
                static_cast<uint8_t>((b >> 16) & 0xff),
                static_cast<uint8_t>((b >> 8) & 0xff),
                static_cast<uint8_t>(b & 0xff)
            };
        } catch (...) {
            return std::nullopt;
        }
    }

    // Three-octet notation: a.b.c (c is 16 bits)
    if (std::regex_match(addr, match, detail::ipv4ThreeOctet())) {
        try {
            long long a = detail::parseIntAuto(match[1].str());
            long long b = detail::parseIntAuto(match[2].str());
            long long c = detail::parseIntAuto(match[3].str());
            if (a < 0 || a > 255) return std::nullopt;
            if (b < 0 || b > 255) return std::nullopt;
            if (c < 0 || c > 0xffff) return std::nullopt;
            return std::array<uint8_t, 4>{
                static_cast<uint8_t>(a),
                static_cast<uint8_t>(b),
                static_cast<uint8_t>((c >> 8) & 0xff),
                static_cast<uint8_t>(c & 0xff)
            };
        } catch (...) {
            return std::nullopt;
        }
    }

    return std::nullopt;
}

// ══════════════════════════════════════════════════════════════════════
// Static methods
// ══════════════════════════════════════════════════════════════════════

inline IPv4 IPv4::parse(const std::string& addr) {
    auto parts = parser(addr);
    if (!parts) {
        throw std::invalid_argument("ipaddr: string is not formatted like an IPv4 Address");
    }
    return IPv4(*parts);
}

inline bool IPv4::isValid(const std::string& addr) {
    try {
        auto result = parser(addr);
        if (!result) return false;
        // Verify construction succeeds (octets in range)
        IPv4(result.value());
        return true;
    } catch (...) {
        return false;
    }
}

inline bool IPv4::isIPv4(const std::string& addr) {
    // isIPv4 checks format only (regex match), not value validity.
    // "1024.0.0.1" is formatted like IPv4 (true), but isValid is false.
    std::smatch match;
    return std::regex_match(addr, match, detail::ipv4FourOctet()) ||
           std::regex_match(addr, match, detail::ipv4LongValue()) ||
           std::regex_match(addr, match, detail::ipv4TwoOctet()) ||
           std::regex_match(addr, match, detail::ipv4ThreeOctet());
}

inline bool IPv4::isValidCIDR(const std::string& addr) {
    try {
        parseCIDR(addr);
        return true;
    } catch (...) {
        return false;
    }
}

inline bool IPv4::isValidFourPartDecimal(const std::string& addr) {
    if (!isValid(addr)) return false;
    return std::regex_match(addr, detail::fourPartDecimalRegex());
}

inline bool IPv4::isValidCIDRFourPartDecimal(const std::string& addr) {
    std::smatch match;
    if (!std::regex_match(addr, match, detail::cidrSplitRegex())) {
        return false;
    }
    if (!isValidCIDR(addr)) return false;
    return isValidFourPartDecimal(match[1].str());
}

inline std::pair<IPv4, int> IPv4::parseCIDR(const std::string& addr) {
    std::smatch match;
    if (std::regex_match(addr, match, detail::cidrSplitRegex())) {
        int maskLength = std::stoi(match[2].str());
        if (maskLength >= 0 && maskLength <= 32) {
            return {parse(match[1].str()), maskLength};
        }
    }
    throw std::invalid_argument("ipaddr: string is not formatted like an IPv4 CIDR range");
}

inline IPv4 IPv4::subnetMaskFromPrefixLength(int prefix) {
    if (prefix < 0 || prefix > 32) {
        throw std::invalid_argument("ipaddr: invalid IPv4 prefix length");
    }

    std::array<uint8_t, 4> octets = {0, 0, 0, 0};
    int filledOctetCount = prefix / 8;
    for (int j = 0; j < filledOctetCount; j++) {
        octets[j] = 255;
    }
    if (filledOctetCount < 4) {
        int bits = prefix % 8;
        octets[filledOctetCount] = static_cast<uint8_t>(((1 << bits) - 1) << (8 - bits));
    }
    return IPv4(octets);
}

inline IPv4 IPv4::broadcastAddressFromCIDR(const std::string& addr) {
    try {
        auto [ip, prefix] = parseCIDR(addr);
        auto ipBytes = ip.toByteArray();
        auto maskBytes = subnetMaskFromPrefixLength(prefix).toByteArray();
        std::array<uint8_t, 4> octets;
        for (int i = 0; i < 4; i++) {
            octets[i] = static_cast<uint8_t>(ipBytes[i] | (maskBytes[i] ^ 255));
        }
        return IPv4(octets);
    } catch (...) {
        throw std::invalid_argument("ipaddr: the address does not have IPv4 CIDR format");
    }
}

inline IPv4 IPv4::networkAddressFromCIDR(const std::string& addr) {
    try {
        auto [ip, prefix] = parseCIDR(addr);
        auto ipBytes = ip.toByteArray();
        auto maskBytes = subnetMaskFromPrefixLength(prefix).toByteArray();
        std::array<uint8_t, 4> octets;
        for (int i = 0; i < 4; i++) {
            octets[i] = static_cast<uint8_t>(ipBytes[i] & maskBytes[i]);
        }
        return IPv4(octets);
    } catch (...) {
        throw std::invalid_argument("ipaddr: the address does not have IPv4 CIDR format");
    }
}

// ══════════════════════════════════════════════════════════════════════
// Instance methods
// ══════════════════════════════════════════════════════════════════════

inline std::string IPv4::kind() const {
    return "ipv4";
}

inline std::string IPv4::toString() const {
    return std::to_string(octets_[0]) + "." +
           std::to_string(octets_[1]) + "." +
           std::to_string(octets_[2]) + "." +
           std::to_string(octets_[3]);
}

inline std::string IPv4::toNormalizedString() const {
    return toString();
}

inline const std::array<uint8_t, 4>& IPv4::octets() const {
    return octets_;
}

inline std::vector<uint8_t> IPv4::toByteArray() const {
    return {octets_[0], octets_[1], octets_[2], octets_[3]};
}

inline bool IPv4::match(const IPv4& other, int cidrBits) const {
    return detail::matchCIDR(octets_, other.octets_, 8, cidrBits);
}

inline bool IPv4::match(const std::pair<IPv4, int>& cidr) const {
    return match(cidr.first, cidr.second);
}

inline std::string IPv4::range() const {
    // Special IPv4 ranges
    // Order matters: more specific ranges should be checked within their parent ranges
    struct RangeEntry {
        std::string name;
        std::vector<std::pair<std::array<uint8_t, 4>, int>> cidrs;
    };

    static const std::vector<RangeEntry> ranges = {
        {"unspecified",    {{{0, 0, 0, 0}, 8}}},
        {"broadcast",     {{{255, 255, 255, 255}, 32}}},
        {"multicast",     {{{224, 0, 0, 0}, 4}}},
        {"linkLocal",     {{{169, 254, 0, 0}, 16}}},
        {"loopback",      {{{127, 0, 0, 0}, 8}}},
        {"carrierGradeNat", {{{100, 64, 0, 0}, 10}}},
        {"private",       {{{10, 0, 0, 0}, 8},
                           {{172, 16, 0, 0}, 12},
                           {{192, 168, 0, 0}, 16}}},
        {"reserved",      {{{192, 0, 0, 0}, 24},
                           {{192, 0, 2, 0}, 24},
                           {{192, 88, 99, 0}, 24},
                           {{198, 18, 0, 0}, 15},
                           {{198, 51, 100, 0}, 24},
                           {{203, 0, 113, 0}, 24},
                           {{240, 0, 0, 0}, 4}}},
        {"as112",         {{{192, 175, 48, 0}, 24},
                           {{192, 31, 196, 0}, 24}}},
        {"amt",           {{{192, 52, 193, 0}, 24}}},
    };

    for (const auto& entry : ranges) {
        for (const auto& [network, prefix] : entry.cidrs) {
            if (detail::matchCIDR(octets_, network, 8, prefix)) {
                return entry.name;
            }
        }
    }

    return "unicast";
}

inline IPv6 IPv4::toIPv4MappedAddress() const {
    // ::ffff:a.b.c.d => parts [0,0,0,0,0,0xffff, (a<<8)|b, (c<<8)|d]
    std::array<uint16_t, 8> parts = {
        0, 0, 0, 0, 0, 0xffff,
        static_cast<uint16_t>((octets_[0] << 8) | octets_[1]),
        static_cast<uint16_t>((octets_[2] << 8) | octets_[3])
    };
    return IPv6(parts);
}

inline std::optional<int> IPv4::prefixLengthFromSubnetMask() const {
    // Table: octet value -> number of trailing zeros
    static const int zerotable[] = {
        // index is octet value, value is number of zero bits
        // Only valid subnet mask octets are: 0,128,192,224,240,248,252,254,255
    };

    // Use a map approach like the JS version
    auto getZeros = [](uint8_t octet) -> std::optional<int> {
        switch (octet) {
            case 0:   return 8;
            case 128: return 7;
            case 192: return 6;
            case 224: return 5;
            case 240: return 4;
            case 248: return 3;
            case 252: return 2;
            case 254: return 1;
            case 255: return 0;
            default:  return std::nullopt;
        }
    };

    int cidr = 0;
    bool stop = false;

    for (int i = 3; i >= 0; i--) {
        auto zeros = getZeros(octets_[i]);
        if (!zeros) return std::nullopt;

        if (stop && *zeros != 0) {
            return std::nullopt;
        }
        if (*zeros != 8) {
            stop = true;
        }
        cidr += *zeros;
    }

    return 32 - cidr;
}

inline bool IPv4::operator==(const IPv4& other) const {
    return octets_ == other.octets_;
}

inline bool IPv4::operator!=(const IPv4& other) const {
    return !(*this == other);
}

} // namespace ipaddr
} // namespace polycpp
