#pragma once

/**
 * @file ipv6_impl.hpp
 * @brief IPv6 class implementation (all functions inline).
 * @since 0.1.0
 */

#include <polycpp/ipaddr/ipaddr.hpp>
#include <polycpp/ipaddr/detail/helpers.hpp>

#include <sstream>
#include <stdexcept>
#include <string>

namespace polycpp {
namespace ipaddr {

// ══════════════════════════════════════════════════════════════════════
// Construction
// ══════════════════════════════════════════════════════════════════════

inline IPv6::IPv6(const std::array<uint16_t, 8>& parts, const std::string& zoneId)
    : parts_(parts), zoneId_(zoneId) {
    for (int i = 0; i < 8; i++) {
        if (parts_[i] > 0xffff) {
            throw std::invalid_argument("ipaddr: ipv6 part should fit in 16 bits");
        }
    }
}

inline IPv6::IPv6(const std::array<uint8_t, 16>& bytes, const std::string& zoneId)
    : zoneId_(zoneId) {
    for (int i = 0; i < 8; i++) {
        parts_[i] = static_cast<uint16_t>((bytes[i * 2] << 8) | bytes[i * 2 + 1]);
    }
}

// ══════════════════════════════════════════════════════════════════════
// Internal parser
// ══════════════════════════════════════════════════════════════════════

inline std::optional<IPv6::ParseResult> IPv6::parser(const std::string& addr) {
    if (addr.empty()) return std::nullopt;

    // Check deprecated transitional form: ::a.b.c.d
    {
        std::smatch match;
        if (std::regex_match(addr, match, detail::ipv6DeprecatedTransitional())) {
            // Reparse as ::ffff:a.b.c.d
            return parser("::ffff:" + match[1].str());
        }
    }

    // Native IPv6 format
    if (detail::regexTest(detail::ipv6Native(), addr)) {
        auto expanded = detail::expandIPv6(addr, 8);
        if (expanded && expanded->parts.size() == 8) {
            std::array<uint32_t, 8> parts;
            for (int i = 0; i < 8; i++) {
                parts[i] = expanded->parts[i];
            }
            return ParseResult{parts, expanded->zoneId};
        }
    }

    // Transitional format: IPv6 prefix with embedded IPv4 at the end
    {
        std::smatch match;
        if (std::regex_match(addr, match, detail::ipv6Transitional())) {
            std::string zoneId;
            if (match[6].matched) {
                zoneId = match[6].str().substr(1); // remove '%'
            }

            std::string prefix = match[1].str();
            if (!prefix.empty() && !prefix.ends_with("::")) {
                // Remove trailing colon before the IPv4 part
                prefix = prefix.substr(0, prefix.size() - 1);
            }

            // Append zone if present (for expandIPv6 to extract)
            std::string toExpand = prefix;
            if (!zoneId.empty()) {
                toExpand += "%" + zoneId;
            }

            auto expanded = detail::expandIPv6(toExpand, 6);
            if (expanded && expanded->parts.size() == 6) {
                // Parse the four IPv4 octets
                int octets[4];
                for (int i = 0; i < 4; i++) {
                    try {
                        octets[i] = std::stoi(match[i + 2].str());
                    } catch (...) {
                        return std::nullopt;
                    }
                    if (octets[i] < 0 || octets[i] > 255) {
                        return std::nullopt;
                    }
                }

                std::array<uint32_t, 8> parts;
                for (int i = 0; i < 6; i++) {
                    parts[i] = expanded->parts[i];
                }
                parts[6] = static_cast<uint32_t>((octets[0] << 8) | octets[1]);
                parts[7] = static_cast<uint32_t>((octets[2] << 8) | octets[3]);

                return ParseResult{parts, expanded->zoneId};
            }
        }
    }

    return std::nullopt;
}

// ══════════════════════════════════════════════════════════════════════
// Static methods
// ══════════════════════════════════════════════════════════════════════

inline IPv6 IPv6::parse(const std::string& addr) {
    auto result = parser(addr);
    if (!result) {
        throw std::invalid_argument("ipaddr: string is not formatted like an IPv6 Address");
    }
    // Validate and convert uint32_t parts to uint16_t
    std::array<uint16_t, 8> parts16;
    for (int i = 0; i < 8; i++) {
        if (result->parts[i] > 0xffff) {
            throw std::invalid_argument("ipaddr: ipv6 part should fit in 16 bits");
        }
        parts16[i] = static_cast<uint16_t>(result->parts[i]);
    }
    return IPv6(parts16, result->zoneId);
}

inline bool IPv6::isValid(const std::string& addr) {
    // Quick check: must contain a colon
    if (addr.find(':') == std::string::npos) {
        return false;
    }

    try {
        auto result = parser(addr);
        if (!result) return false;
        // Check all parts fit in 16 bits
        std::array<uint16_t, 8> parts16;
        for (int i = 0; i < 8; i++) {
            if (result->parts[i] > 0xffff) return false;
            parts16[i] = static_cast<uint16_t>(result->parts[i]);
        }
        IPv6(parts16, result->zoneId);
        return true;
    } catch (...) {
        return false;
    }
}

inline bool IPv6::isIPv6(const std::string& addr) {
    return parser(addr).has_value();
}

inline bool IPv6::isValidCIDR(const std::string& addr) {
    if (addr.find(':') == std::string::npos) {
        return false;
    }
    try {
        parseCIDR(addr);
        return true;
    } catch (...) {
        return false;
    }
}

inline std::pair<IPv6, int> IPv6::parseCIDR(const std::string& addr) {
    std::smatch match;
    if (std::regex_match(addr, match, detail::cidrSplitRegex())) {
        int maskLength = std::stoi(match[2].str());
        if (maskLength >= 0 && maskLength <= 128) {
            return {parse(match[1].str()), maskLength};
        }
    }
    throw std::invalid_argument("ipaddr: string is not formatted like an IPv6 CIDR range");
}

inline IPv6 IPv6::subnetMaskFromPrefixLength(int prefix) {
    if (prefix < 0 || prefix > 128) {
        throw std::invalid_argument("ipaddr: invalid IPv6 prefix length");
    }

    // Build 16 bytes, then convert to 8 x 16-bit parts
    std::array<uint8_t, 16> octets = {};
    int filledOctetCount = prefix / 8;
    for (int j = 0; j < filledOctetCount; j++) {
        octets[j] = 255;
    }
    if (filledOctetCount < 16) {
        int bits = prefix % 8;
        octets[filledOctetCount] = static_cast<uint8_t>(((1 << bits) - 1) << (8 - bits));
    }

    return IPv6(octets);
}

inline IPv6 IPv6::broadcastAddressFromCIDR(const std::string& addr) {
    try {
        auto [ip, prefix] = parseCIDR(addr);
        auto ipBytes = ip.toByteArray();
        auto maskBytes = subnetMaskFromPrefixLength(prefix).toByteArray();

        std::array<uint8_t, 16> octets;
        for (int i = 0; i < 16; i++) {
            octets[i] = static_cast<uint8_t>(ipBytes[i] | (maskBytes[i] ^ 255));
        }
        return IPv6(octets);
    } catch (const std::exception& e) {
        throw std::invalid_argument(
            std::string("ipaddr: the address does not have IPv6 CIDR format (") + e.what() + ")");
    }
}

inline IPv6 IPv6::networkAddressFromCIDR(const std::string& addr) {
    try {
        auto [ip, prefix] = parseCIDR(addr);
        auto ipBytes = ip.toByteArray();
        auto maskBytes = subnetMaskFromPrefixLength(prefix).toByteArray();

        std::array<uint8_t, 16> octets;
        for (int i = 0; i < 16; i++) {
            octets[i] = static_cast<uint8_t>(ipBytes[i] & maskBytes[i]);
        }
        return IPv6(octets);
    } catch (const std::exception& e) {
        throw std::invalid_argument(
            std::string("ipaddr: the address does not have IPv6 CIDR format (") + e.what() + ")");
    }
}

// ══════════════════════════════════════════════════════════════════════
// Instance methods
// ══════════════════════════════════════════════════════════════════════

inline std::string IPv6::kind() const {
    return "ipv6";
}

inline std::string IPv6::toNormalizedString() const {
    std::string result;
    for (int i = 0; i < 8; i++) {
        if (i > 0) result += ":";
        result += detail::toHex(parts_[i]);
    }
    if (!zoneId_.empty()) {
        result += "%" + zoneId_;
    }
    return result;
}

inline std::string IPv6::toFixedLengthString() const {
    std::string result;
    for (int i = 0; i < 8; i++) {
        if (i > 0) result += ":";
        result += detail::padPart(detail::toHex(parts_[i]), 4);
    }
    if (!zoneId_.empty()) {
        result += "%" + zoneId_;
    }
    return result;
}

inline std::string IPv6::toRFC5952String() const {
    // Build the normalized string (without zone)
    std::string normalized;
    for (int i = 0; i < 8; i++) {
        if (i > 0) normalized += ":";
        normalized += detail::toHex(parts_[i]);
    }

    // Find the longest run of consecutive "0" groups (at least 2)
    // to replace with ::
    // Use regex approach like the JS version: find ((^|:)(0(:|$)){2,})
    int bestStart = -1;
    int bestLen = 0;

    int currentStart = -1;
    int currentLen = 0;

    for (int i = 0; i < 8; i++) {
        if (parts_[i] == 0) {
            if (currentStart == -1) {
                currentStart = i;
                currentLen = 1;
            } else {
                currentLen++;
            }
        } else {
            if (currentLen > bestLen && currentLen >= 2) {
                bestStart = currentStart;
                bestLen = currentLen;
            }
            currentStart = -1;
            currentLen = 0;
        }
    }
    // Check at end
    if (currentLen > bestLen && currentLen >= 2) {
        bestStart = currentStart;
        bestLen = currentLen;
    }

    std::string result;
    if (bestLen < 2) {
        // No abbreviation
        result = normalized;
    } else {
        // Build string with :: abbreviation
        // Parts before the run
        for (int i = 0; i < bestStart; i++) {
            if (i > 0) result += ":";
            result += detail::toHex(parts_[i]);
        }
        result += "::";
        // Parts after the run
        bool first = true;
        for (int i = bestStart + bestLen; i < 8; i++) {
            if (!first) result += ":";
            result += detail::toHex(parts_[i]);
            first = false;
        }
    }

    if (!zoneId_.empty()) {
        result += "%" + zoneId_;
    }

    return result;
}

inline std::string IPv6::toString() const {
    return toRFC5952String();
}

inline const std::array<uint16_t, 8>& IPv6::parts() const {
    return parts_;
}

inline const std::string& IPv6::zoneId() const {
    return zoneId_;
}

inline std::vector<uint8_t> IPv6::toByteArray() const {
    std::vector<uint8_t> bytes;
    bytes.reserve(16);
    for (int i = 0; i < 8; i++) {
        bytes.push_back(static_cast<uint8_t>(parts_[i] >> 8));
        bytes.push_back(static_cast<uint8_t>(parts_[i] & 0xff));
    }
    return bytes;
}

inline bool IPv6::match(const IPv6& other, int cidrBits) const {
    return detail::matchCIDR(parts_, other.parts_, 16, cidrBits);
}

inline bool IPv6::match(const std::pair<IPv6, int>& cidr) const {
    return match(cidr.first, cidr.second);
}

inline std::string IPv6::range() const {
    // Special IPv6 ranges
    struct RangeEntry {
        std::string name;
        std::vector<std::pair<std::array<uint16_t, 8>, int>> cidrs;
    };

    static const std::vector<RangeEntry> ranges = {
        {"unspecified", {{{0, 0, 0, 0, 0, 0, 0, 0}, 128}}},
        {"linkLocal",   {{{0xfe80, 0, 0, 0, 0, 0, 0, 0}, 10}}},
        {"multicast",   {{{0xff00, 0, 0, 0, 0, 0, 0, 0}, 8}}},
        {"loopback",    {{{0, 0, 0, 0, 0, 0, 0, 1}, 128}}},
        {"uniqueLocal", {{{0xfc00, 0, 0, 0, 0, 0, 0, 0}, 7}}},
        {"ipv4Mapped",  {{{0, 0, 0, 0, 0, 0xffff, 0, 0}, 96}}},
        {"discard",     {{{0x100, 0, 0, 0, 0, 0, 0, 0}, 64}}},
        {"rfc6145",     {{{0, 0, 0, 0, 0xffff, 0, 0, 0}, 96}}},
        {"rfc6052",     {{{0x64, 0xff9b, 0, 0, 0, 0, 0, 0}, 96}}},
        {"6to4",        {{{0x2002, 0, 0, 0, 0, 0, 0, 0}, 16}}},
        {"teredo",      {{{0x2001, 0, 0, 0, 0, 0, 0, 0}, 32}}},
        {"benchmarking", {{{0x2001, 0x2, 0, 0, 0, 0, 0, 0}, 48}}},
        {"amt",         {{{0x2001, 0x3, 0, 0, 0, 0, 0, 0}, 32}}},
        {"as112v6",     {{{0x2001, 0x4, 0x112, 0, 0, 0, 0, 0}, 48},
                         {{0x2620, 0x4f, 0x8000, 0, 0, 0, 0, 0}, 48}}},
        {"deprecated",  {{{0x2001, 0x10, 0, 0, 0, 0, 0, 0}, 28}}},
        {"orchid2",     {{{0x2001, 0x20, 0, 0, 0, 0, 0, 0}, 28}}},
        {"droneRemoteIdProtocolEntityTags", {{{0x2001, 0x30, 0, 0, 0, 0, 0, 0}, 28}}},
        {"reserved",    {{{0x2001, 0, 0, 0, 0, 0, 0, 0}, 23},
                         {{0x2001, 0xdb8, 0, 0, 0, 0, 0, 0}, 32}}},
    };

    for (const auto& entry : ranges) {
        for (const auto& [network, prefix] : entry.cidrs) {
            if (detail::matchCIDR(parts_, network, 16, prefix)) {
                return entry.name;
            }
        }
    }

    return "unicast";
}

inline bool IPv6::isIPv4MappedAddress() const {
    return range() == "ipv4Mapped";
}

inline IPv4 IPv6::toIPv4Address() const {
    if (!isIPv4MappedAddress()) {
        throw std::invalid_argument("ipaddr: trying to convert a generic ipv6 address to ipv4");
    }
    uint16_t high = parts_[6];
    uint16_t low = parts_[7];
    return IPv4(std::array<uint8_t, 4>{
        static_cast<uint8_t>(high >> 8),
        static_cast<uint8_t>(high & 0xff),
        static_cast<uint8_t>(low >> 8),
        static_cast<uint8_t>(low & 0xff)
    });
}

inline std::optional<int> IPv6::prefixLengthFromSubnetMask() const {
    auto getZeros = [](uint16_t part) -> std::optional<int> {
        switch (part) {
            case 0:     return 16;
            case 0x8000: return 15;
            case 0xc000: return 14;
            case 0xe000: return 13;
            case 0xf000: return 12;
            case 0xf800: return 11;
            case 0xfc00: return 10;
            case 0xfe00: return 9;
            case 0xff00: return 8;
            case 0xff80: return 7;
            case 0xffc0: return 6;
            case 0xffe0: return 5;
            case 0xfff0: return 4;
            case 0xfff8: return 3;
            case 0xfffc: return 2;
            case 0xfffe: return 1;
            case 0xffff: return 0;
            default:     return std::nullopt;
        }
    };

    int cidr = 0;
    bool stop = false;

    for (int i = 7; i >= 0; i--) {
        auto zeros = getZeros(parts_[i]);
        if (!zeros) return std::nullopt;

        if (stop && *zeros != 0) {
            return std::nullopt;
        }
        if (*zeros != 16) {
            stop = true;
        }
        cidr += *zeros;
    }

    return 128 - cidr;
}

inline bool IPv6::operator==(const IPv6& other) const {
    return parts_ == other.parts_ && zoneId_ == other.zoneId_;
}

inline bool IPv6::operator!=(const IPv6& other) const {
    return !(*this == other);
}

} // namespace ipaddr
} // namespace polycpp
