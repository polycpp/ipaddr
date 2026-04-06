#pragma once

/**
 * @file helpers.hpp
 * @brief Internal helper functions for ipaddr parsing and matching.
 * @since 0.1.0
 */

#include <array>
#include <cctype>
#include <climits>
#include <cstdint>
#include <optional>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace polycpp {
namespace ipaddr {
namespace detail {

// ══════════════════════════════════════════════════════════════════════
// String utilities
// ══════════════════════════════════════════════════════════════════════

/**
 * @brief Pad a hex string with leading zeros to given length.
 */
inline std::string padPart(const std::string& part, size_t length) {
    std::string result = part;
    while (result.size() < length) {
        result = "0" + result;
    }
    return result;
}

/**
 * @brief Convert an integer to a hex string (lowercase).
 */
inline std::string toHex(uint16_t value) {
    std::ostringstream oss;
    oss << std::hex << value;
    return oss.str();
}

/**
 * @brief Check if a string matches a regex.
 */
inline bool regexTest(const std::regex& re, const std::string& str) {
    return std::regex_search(str, re);
}

/**
 * @brief Get the first regex match.
 */
inline std::optional<std::smatch> regexMatch(const std::regex& re, const std::string& str) {
    std::smatch m;
    if (std::regex_search(str, m, re)) {
        return m;
    }
    return std::nullopt;
}

// ══════════════════════════════════════════════════════════════════════
// IPv4 regex patterns
// ══════════════════════════════════════════════════════════════════════

inline const std::string ipv4Part = "(0?\\d+|0x[a-f0-9]+)";

inline const std::regex& ipv4FourOctet() {
    static const std::regex re(
        "^" + ipv4Part + "\\." + ipv4Part + "\\." + ipv4Part + "\\." + ipv4Part + "$",
        std::regex::icase);
    return re;
}

inline const std::regex& ipv4ThreeOctet() {
    static const std::regex re(
        "^" + ipv4Part + "\\." + ipv4Part + "\\." + ipv4Part + "$",
        std::regex::icase);
    return re;
}

inline const std::regex& ipv4TwoOctet() {
    static const std::regex re(
        "^" + ipv4Part + "\\." + ipv4Part + "$",
        std::regex::icase);
    return re;
}

inline const std::regex& ipv4LongValue() {
    static const std::regex re(
        "^" + ipv4Part + "$",
        std::regex::icase);
    return re;
}

inline const std::regex& octalRegex() {
    static const std::regex re("^0[0-7]+$", std::regex::icase);
    return re;
}

inline const std::regex& hexRegex() {
    static const std::regex re("^0x[a-f0-9]+$", std::regex::icase);
    return re;
}

// ══════════════════════════════════════════════════════════════════════
// IPv6 regex patterns
// ══════════════════════════════════════════════════════════════════════

inline const std::string zoneIndexPattern = "%[0-9a-z]{1,}";
inline const std::string ipv6Part = "(?:[0-9a-f]+::?)+";

inline const std::regex& ipv6ZoneIndex() {
    static const std::regex re(zoneIndexPattern, std::regex::icase);
    return re;
}

inline const std::regex& ipv6Native() {
    static const std::regex re(
        "^(::)?(" + ipv6Part + ")?([0-9a-f]+)?(::)?(" + zoneIndexPattern + ")?$",
        std::regex::icase);
    return re;
}

inline const std::regex& ipv6DeprecatedTransitional() {
    static const std::regex re(
        "^(?:::)(" + ipv4Part + "\\." + ipv4Part + "\\." + ipv4Part + "\\." + ipv4Part +
        "(" + zoneIndexPattern + ")?)$",
        std::regex::icase);
    return re;
}

inline const std::regex& ipv6Transitional() {
    static const std::regex re(
        "^((?:" + ipv6Part + ")|(?:::)(?:" + ipv6Part + ")?)" +
        ipv4Part + "\\." + ipv4Part + "\\." + ipv4Part + "\\." + ipv4Part +
        "(" + zoneIndexPattern + ")?$",
        std::regex::icase);
    return re;
}

inline const std::regex& fourPartDecimalRegex() {
    static const std::regex re("^(0|[1-9]\\d*)(\\.(0|[1-9]\\d*)){3}$");
    return re;
}

inline const std::regex& cidrSplitRegex() {
    static const std::regex re("^(.+)/(\\d+)$");
    return re;
}

// ══════════════════════════════════════════════════════════════════════
// Numeric parsing
// ══════════════════════════════════════════════════════════════════════

/**
 * @brief Parse an integer with auto-detection of base (hex 0x, octal 0, decimal).
 * @param str The string to parse.
 * @return The parsed integer.
 * @throws std::invalid_argument On invalid input.
 */
inline long long parseIntAuto(const std::string& str) {
    if (str.empty()) {
        throw std::invalid_argument("ipaddr: empty string in parseIntAuto");
    }

    // Hex: 0x prefix
    if (regexTest(hexRegex(), str)) {
        return std::stoll(str, nullptr, 16);
    }

    // Octal: starts with 0 followed by digits
    if (str[0] == '0' && str.size() > 1 && std::isdigit(static_cast<unsigned char>(str[1]))) {
        if (regexTest(octalRegex(), str)) {
            return std::stoll(str, nullptr, 8);
        }
        throw std::invalid_argument("ipaddr: cannot parse " + str + " as octal");
    }

    // Decimal
    return std::stoll(str, nullptr, 10);
}

// ══════════════════════════════════════════════════════════════════════
// CIDR matching
// ══════════════════════════════════════════════════════════════════════

/**
 * @brief Generic CIDR range matcher.
 *
 * Compares two arrays of parts, checking that the first `cidrBits` bits match.
 *
 * @tparam T Part type (uint8_t for IPv4, uint16_t for IPv6).
 * @tparam N Number of parts.
 * @param first First address parts.
 * @param second Second address parts.
 * @param partSize Bit size of each part (8 for IPv4, 16 for IPv6).
 * @param cidrBits Number of bits to compare.
 * @return true if the first cidrBits match.
 */
template <typename T, size_t N>
inline bool matchCIDR(const std::array<T, N>& first,
                      const std::array<T, N>& second,
                      int partSize, int cidrBits) {
    if (cidrBits < 0 || cidrBits > static_cast<int>(N) * partSize) {
        return false;
    }
    int part = 0;
    while (cidrBits > 0) {
        int shift = partSize - cidrBits;
        if (shift < 0) {
            shift = 0;
        }
        if ((first[part] >> shift) != (second[part] >> shift)) {
            return false;
        }
        cidrBits -= partSize;
        part += 1;
    }
    return true;
}

// ══════════════════════════════════════════════════════════════════════
// expandIPv6
// ══════════════════════════════════════════════════════════════════════

/**
 * @brief Expand :: in an IPv6 address string.
 *
 * @param input The IPv6 string (possibly abbreviated with ::).
 * @param numParts Expected number of parts (usually 8, or 6 for transitional).
 * @return Parsed parts and zone ID, or nullopt on failure.
 */
struct ExpandResult {
    std::vector<uint32_t> parts;  // uint32_t to detect overflow before truncation
    std::string zoneId;
};

inline std::optional<ExpandResult> expandIPv6(const std::string& input, int numParts) {
    std::string str = input;

    // More than one '::' means invalid
    auto firstDC = str.find("::");
    if (firstDC != std::string::npos) {
        auto secondDC = str.find("::", firstDC + 2);
        if (secondDC != std::string::npos) {
            return std::nullopt;
        }
    }

    // Extract zone ID
    std::string zoneId;
    auto zm = regexMatch(ipv6ZoneIndex(), str);
    if (zm) {
        zoneId = (*zm)[0].str().substr(1); // remove the '%'
        // Remove zone index from string
        auto pos = str.find('%');
        if (pos != std::string::npos) {
            str = str.substr(0, pos);
        }
    }

    // Count colons
    int colonCount = 0;
    for (char c : str) {
        if (c == ':') colonCount++;
    }

    // Adjust for leading/trailing ::
    if (str.size() >= 2 && str.substr(0, 2) == "::") {
        colonCount--;
    }
    if (str.size() >= 2 && str.substr(str.size() - 2) == "::") {
        colonCount--;
    }

    if (colonCount > numParts) {
        return std::nullopt;
    }

    // Build replacement
    int replacementCount = numParts - colonCount;

    // Replace :: with the expanded zeros
    auto dcPos = str.find("::");
    if (dcPos != std::string::npos) {
        // :: must expand to at least one zero group
        if (replacementCount < 1) {
            return std::nullopt;
        }
        std::string replacement = ":";
        for (int i = 0; i < replacementCount; i++) {
            replacement += "0:";
        }
        str = str.substr(0, dcPos) + replacement + str.substr(dcPos + 2);
    }

    // Trim leading/trailing colons
    if (!str.empty() && str[0] == ':') {
        str = str.substr(1);
    }
    if (!str.empty() && str[str.size() - 1] == ':') {
        str = str.substr(0, str.size() - 1);
    }

    // Split and parse
    std::vector<uint32_t> parts;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, ':')) {
        if (token.empty()) {
            // This can happen with malformed input
            parts.push_back(0);
        } else {
            try {
                unsigned long val = std::stoul(token, nullptr, 16);
                if (val > static_cast<unsigned long>(UINT32_MAX)) {
                    return std::nullopt;
                }
                parts.push_back(static_cast<uint32_t>(val));
            } catch (...) {
                return std::nullopt;
            }
        }
    }

    return ExpandResult{parts, zoneId};
}

} // namespace detail
} // namespace ipaddr
} // namespace polycpp
