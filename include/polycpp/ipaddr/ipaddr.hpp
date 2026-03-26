#pragma once

/**
 * @file ipaddr.hpp
 * @brief C++ port of npm ipaddr.js — IPv4/IPv6 address manipulation.
 *
 * Provides parsing, validation, CIDR matching, range detection, and
 * IPv4/IPv6 conversion. Mirrors the ipaddr.js API surface.
 *
 * @par Example
 * @code{.cpp}
 *   #include <polycpp/ipaddr/ipaddr.hpp>
 *   using namespace polycpp::ipaddr;
 *
 *   auto addr = IPv4::parse("192.168.1.1");
 *   addr.range();      // "private"
 *   addr.toString();   // "192.168.1.1"
 *
 *   auto v6 = IPv6::parse("2001:db8::1");
 *   v6.range();        // "reserved"
 * @endcode
 *
 * @see https://github.com/whitequark/ipaddr.js
 * @since 0.1.0
 */

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace polycpp {
namespace ipaddr {

// Forward declarations
class IPv4;
class IPv6;

// ══════════════════════════════════════════════════════════════════════
// IPv4
// ══════════════════════════════════════════════════════════════════════

/**
 * @brief An IPv4 address (RFC 791).
 *
 * Stores four octets in network order (MSB first). Supports parsing from
 * decimal, hex, and octal notations. Provides CIDR matching, range
 * detection, and conversion to IPv4-mapped IPv6 addresses.
 *
 * @see https://github.com/whitequark/ipaddr.js
 * @since 0.1.0
 */
class IPv4 {
public:
    /**
     * @brief Construct an IPv4 address from four octets.
     * @param octets Array of four octets in network order [0..255].
     * @throws std::invalid_argument If any octet is out of range.
     * @since 0.1.0
     */
    explicit IPv4(const std::array<uint8_t, 4>& octets);

    /**
     * @brief Parse an IPv4 address string.
     *
     * Supports standard dotted-decimal, hex (0x), octal (0), and
     * classful notations (1-4 parts).
     *
     * @param addr The address string to parse.
     * @return The parsed IPv4 address.
     * @throws std::invalid_argument If the string is not a valid IPv4 address.
     *
     * @par Example
     * @code{.cpp}
     *   auto a = IPv4::parse("192.168.1.1");
     *   auto b = IPv4::parse("0xc0a80101");  // same address
     * @endcode
     *
     * @see https://github.com/whitequark/ipaddr.js#ipv4parse
     * @since 0.1.0
     */
    static IPv4 parse(const std::string& addr);

    /**
     * @brief Check if a string is a valid IPv4 address.
     * @param addr The address string to validate.
     * @return true if valid, false otherwise.
     * @since 0.1.0
     */
    static bool isValid(const std::string& addr);

    /**
     * @brief Check if a string looks like an IPv4 address (may not be valid).
     * @param addr The address string to check.
     * @return true if formatted like IPv4.
     * @since 0.1.0
     */
    static bool isIPv4(const std::string& addr);

    /**
     * @brief Check if a string is a valid IPv4 CIDR notation.
     * @param addr The CIDR string to validate.
     * @return true if valid.
     * @since 0.1.0
     */
    static bool isValidCIDR(const std::string& addr);

    /**
     * @brief Check if a string is a valid four-part decimal IPv4 address.
     * @param addr The address string to validate.
     * @return true if valid standard form (e.g. "192.168.1.1").
     * @since 0.1.0
     */
    static bool isValidFourPartDecimal(const std::string& addr);

    /**
     * @brief Check if a string is valid four-part decimal IPv4 CIDR.
     * @param addr The CIDR string to validate.
     * @return true if valid.
     * @since 0.1.0
     */
    static bool isValidCIDRFourPartDecimal(const std::string& addr);

    /**
     * @brief Parse a CIDR notation string.
     * @param addr The CIDR string (e.g. "192.168.1.0/24").
     * @return Pair of IPv4 address and prefix length.
     * @throws std::invalid_argument If the string is not valid CIDR.
     * @since 0.1.0
     */
    static std::pair<IPv4, int> parseCIDR(const std::string& addr);

    /**
     * @brief Create a subnet mask from a prefix length.
     * @param prefix Prefix length [0..32].
     * @return IPv4 subnet mask address.
     * @throws std::invalid_argument If prefix is out of range.
     * @since 0.1.0
     */
    static IPv4 subnetMaskFromPrefixLength(int prefix);

    /**
     * @brief Compute the broadcast address from a CIDR notation.
     * @param addr CIDR string (e.g. "172.0.0.1/24").
     * @return The broadcast address.
     * @throws std::invalid_argument If not valid CIDR.
     * @since 0.1.0
     */
    static IPv4 broadcastAddressFromCIDR(const std::string& addr);

    /**
     * @brief Compute the network address from a CIDR notation.
     * @param addr CIDR string (e.g. "172.0.0.1/24").
     * @return The network address.
     * @throws std::invalid_argument If not valid CIDR.
     * @since 0.1.0
     */
    static IPv4 networkAddressFromCIDR(const std::string& addr);

    /**
     * @brief Return the address kind.
     * @return "ipv4"
     * @since 0.1.0
     */
    std::string kind() const;

    /**
     * @brief Convert to dotted-decimal string.
     * @return String like "192.168.1.1".
     * @since 0.1.0
     */
    std::string toString() const;

    /**
     * @brief Alias for toString() (symmetry with IPv6).
     * @return Same as toString().
     * @since 0.1.0
     */
    std::string toNormalizedString() const;

    /**
     * @brief Get the four octets.
     * @return Array of four octets.
     * @since 0.1.0
     */
    const std::array<uint8_t, 4>& octets() const;

    /**
     * @brief Get the octets as a byte array.
     * @return Vector of 4 bytes in network order.
     * @since 0.1.0
     */
    std::vector<uint8_t> toByteArray() const;

    /**
     * @brief Check if this address matches a CIDR range.
     * @param other Network address.
     * @param cidrBits Prefix length.
     * @return true if this address is within the range.
     * @since 0.1.0
     */
    bool match(const IPv4& other, int cidrBits) const;

    /**
     * @brief Check if this address matches a CIDR range (pair form).
     * @param cidr Pair of network address and prefix length.
     * @return true if this address is within the range.
     * @since 0.1.0
     */
    bool match(const std::pair<IPv4, int>& cidr) const;

    /**
     * @brief Detect the special range this address belongs to.
     * @return Range name: "unspecified", "broadcast", "multicast", "linkLocal",
     *         "loopback", "carrierGradeNat", "private", "reserved", "as112",
     *         "amt", or "unicast".
     * @since 0.1.0
     */
    std::string range() const;

    /**
     * @brief Convert to an IPv4-mapped IPv6 address (::ffff:x.x.x.x).
     * @return The IPv6 address.
     * @since 0.1.0
     */
    IPv6 toIPv4MappedAddress() const;

    /**
     * @brief Compute CIDR prefix length from subnet mask.
     * @return Prefix length, or std::nullopt if not a valid subnet mask.
     * @since 0.1.0
     */
    std::optional<int> prefixLengthFromSubnetMask() const;

    /**
     * @brief Equality comparison.
     * @since 0.1.0
     */
    bool operator==(const IPv4& other) const;
    bool operator!=(const IPv4& other) const;

private:
    std::array<uint8_t, 4> octets_;

    // Internal parser: returns nullopt on failure, octets on success.
    static std::optional<std::array<uint8_t, 4>> parser(const std::string& addr);
};


// ══════════════════════════════════════════════════════════════════════
// IPv6
// ══════════════════════════════════════════════════════════════════════

/**
 * @brief An IPv6 address (RFC 2460).
 *
 * Stores eight 16-bit parts in network order. Supports parsing from
 * standard colon-hex and transitional (with embedded IPv4) notations.
 * Provides CIDR matching, range detection, and conversion to/from
 * IPv4-mapped addresses.
 *
 * @see https://github.com/whitequark/ipaddr.js
 * @since 0.1.0
 */
class IPv6 {
public:
    /**
     * @brief Construct an IPv6 address from eight 16-bit parts.
     * @param parts Array of eight 16-bit values in network order.
     * @param zoneId Optional zone identifier (e.g. "eth0").
     * @throws std::invalid_argument If any part is out of range.
     * @since 0.1.0
     */
    explicit IPv6(const std::array<uint16_t, 8>& parts,
                  const std::string& zoneId = "");

    /**
     * @brief Construct an IPv6 address from sixteen bytes.
     * @param bytes Array of 16 bytes in network order.
     * @param zoneId Optional zone identifier.
     * @throws std::invalid_argument If any derived part is out of range.
     * @since 0.1.0
     */
    explicit IPv6(const std::array<uint8_t, 16>& bytes,
                  const std::string& zoneId = "");

    /**
     * @brief Parse an IPv6 address string.
     *
     * Supports standard colon-hex notation, abbreviated (::), and
     * transitional notation with embedded IPv4 (e.g. ::ffff:192.168.1.1).
     *
     * @param addr The address string to parse.
     * @return The parsed IPv6 address.
     * @throws std::invalid_argument If the string is not a valid IPv6 address.
     * @since 0.1.0
     */
    static IPv6 parse(const std::string& addr);

    /**
     * @brief Check if a string is a valid IPv6 address.
     * @param addr The address string to validate.
     * @return true if valid.
     * @since 0.1.0
     */
    static bool isValid(const std::string& addr);

    /**
     * @brief Check if a string looks like an IPv6 address.
     * @param addr The address string to check.
     * @return true if formatted like IPv6.
     * @since 0.1.0
     */
    static bool isIPv6(const std::string& addr);

    /**
     * @brief Check if a string is a valid IPv6 CIDR notation.
     * @param addr The CIDR string to validate.
     * @return true if valid.
     * @since 0.1.0
     */
    static bool isValidCIDR(const std::string& addr);

    /**
     * @brief Parse a CIDR notation string.
     * @param addr The CIDR string (e.g. "2001:db8::/32").
     * @return Pair of IPv6 address and prefix length.
     * @throws std::invalid_argument If not valid CIDR.
     * @since 0.1.0
     */
    static std::pair<IPv6, int> parseCIDR(const std::string& addr);

    /**
     * @brief Create a subnet mask from a prefix length.
     * @param prefix Prefix length [0..128].
     * @return IPv6 subnet mask address.
     * @throws std::invalid_argument If prefix is out of range.
     * @since 0.1.0
     */
    static IPv6 subnetMaskFromPrefixLength(int prefix);

    /**
     * @brief Compute the broadcast address from a CIDR notation.
     * @param addr CIDR string.
     * @return The broadcast address.
     * @throws std::invalid_argument If not valid CIDR.
     * @since 0.1.0
     */
    static IPv6 broadcastAddressFromCIDR(const std::string& addr);

    /**
     * @brief Compute the network address from a CIDR notation.
     * @param addr CIDR string.
     * @return The network address.
     * @throws std::invalid_argument If not valid CIDR.
     * @since 0.1.0
     */
    static IPv6 networkAddressFromCIDR(const std::string& addr);

    /**
     * @brief Return the address kind.
     * @return "ipv6"
     * @since 0.1.0
     */
    std::string kind() const;

    /**
     * @brief Convert to compact RFC 5952 string with :: abbreviation.
     * @return String like "2001:db8::1".
     * @since 0.1.0
     */
    std::string toString() const;

    /**
     * @brief Convert to expanded string with no zero-padding.
     * @return String like "2001:db8:0:0:0:0:0:1".
     * @since 0.1.0
     */
    std::string toNormalizedString() const;

    /**
     * @brief Convert to fully expanded zero-padded string.
     * @return String like "2001:0db8:0000:0000:0000:0000:0000:0001".
     * @since 0.1.0
     */
    std::string toFixedLengthString() const;

    /**
     * @brief Convert to RFC 5952 compliant string.
     * @return Compact string following RFC 5952 rules.
     * @since 0.1.0
     */
    std::string toRFC5952String() const;

    /**
     * @brief Get the eight 16-bit parts.
     * @return Array of eight 16-bit values.
     * @since 0.1.0
     */
    const std::array<uint16_t, 8>& parts() const;

    /**
     * @brief Get the zone identifier.
     * @return Zone ID string (empty if none).
     * @since 0.1.0
     */
    const std::string& zoneId() const;

    /**
     * @brief Get the address as a 16-byte array.
     * @return Vector of 16 bytes in network order.
     * @since 0.1.0
     */
    std::vector<uint8_t> toByteArray() const;

    /**
     * @brief Check if this address matches a CIDR range.
     * @param other Network address.
     * @param cidrBits Prefix length.
     * @return true if this address is within the range.
     * @since 0.1.0
     */
    bool match(const IPv6& other, int cidrBits) const;

    /**
     * @brief Check if this address matches a CIDR range (pair form).
     * @param cidr Pair of network address and prefix length.
     * @return true if this address is within the range.
     * @since 0.1.0
     */
    bool match(const std::pair<IPv6, int>& cidr) const;

    /**
     * @brief Detect the special range this address belongs to.
     * @return Range name: "unspecified", "linkLocal", "multicast", "loopback",
     *         "uniqueLocal", "ipv4Mapped", "rfc6145", "rfc6052", "6to4",
     *         "teredo", "benchmarking", "amt", "as112v6", "deprecated",
     *         "orchid2", "droneRemoteIdProtocolEntityTags", "discard",
     *         "reserved", or "unicast".
     * @since 0.1.0
     */
    std::string range() const;

    /**
     * @brief Check if this is an IPv4-mapped IPv6 address.
     * @return true if the range is "ipv4Mapped".
     * @since 0.1.0
     */
    bool isIPv4MappedAddress() const;

    /**
     * @brief Convert to IPv4 address (only valid for IPv4-mapped addresses).
     * @return The IPv4 address.
     * @throws std::invalid_argument If not an IPv4-mapped address.
     * @since 0.1.0
     */
    IPv4 toIPv4Address() const;

    /**
     * @brief Compute CIDR prefix length from subnet mask.
     * @return Prefix length, or std::nullopt if not a valid subnet mask.
     * @since 0.1.0
     */
    std::optional<int> prefixLengthFromSubnetMask() const;

    /**
     * @brief Equality comparison.
     * @since 0.1.0
     */
    bool operator==(const IPv6& other) const;
    bool operator!=(const IPv6& other) const;

private:
    std::array<uint16_t, 8> parts_;
    std::string zoneId_;

    // Internal parser result — uses uint32_t to detect overflow before truncation.
    // isIPv6() returns true if parser succeeds (format check only);
    // isValid() additionally checks all parts <= 0xffff.
    struct ParseResult {
        std::array<uint32_t, 8> parts;
        std::string zoneId;
    };

    // Internal parser: returns nullopt on failure
    static std::optional<ParseResult> parser(const std::string& addr);
};


// ══════════════════════════════════════════════════════════════════════
// Module-level functions
// ══════════════════════════════════════════════════════════════════════

/**
 * @brief Parse an IP address string (IPv6 tried first, then IPv4).
 * @param addr The address string.
 * @return Variant containing either IPv4 or IPv6.
 * @throws std::invalid_argument If the string is not a valid IP address.
 * @since 0.1.0
 */
std::variant<IPv4, IPv6> parse(const std::string& addr);

/**
 * @brief Check if a string is a valid IPv4 or IPv6 address.
 * @param addr The address string.
 * @return true if valid.
 * @since 0.1.0
 */
bool isValid(const std::string& addr);

/**
 * @brief Check if a string is a valid IPv4 or IPv6 CIDR notation.
 * @param addr The CIDR string.
 * @return true if valid.
 * @since 0.1.0
 */
bool isValidCIDR(const std::string& addr);

/**
 * @brief Parse a CIDR notation string (IPv6 tried first, then IPv4).
 * @param addr The CIDR string.
 * @return Pair of variant address and prefix length.
 * @throws std::invalid_argument If not valid CIDR.
 * @since 0.1.0
 */
std::pair<std::variant<IPv4, IPv6>, int> parseCIDR(const std::string& addr);

/**
 * @brief Create an IP address from a byte array.
 * @param bytes 4 bytes for IPv4 or 16 bytes for IPv6.
 * @return Variant containing the address.
 * @throws std::invalid_argument If the array length is wrong.
 * @since 0.1.0
 */
std::variant<IPv4, IPv6> fromByteArray(const std::vector<uint8_t>& bytes);

/**
 * @brief Parse and process: if the result is an IPv4-mapped IPv6, return IPv4.
 * @param addr The address string.
 * @return Variant with the processed address.
 * @throws std::invalid_argument If the string is not valid.
 * @since 0.1.0
 */
std::variant<IPv4, IPv6> process(const std::string& addr);

/**
 * @brief Match an address against a named range list.
 *
 * @param address The address to check.
 * @param rangeList Map of range names to CIDR pairs (or vectors of pairs).
 * @param defaultName Default name if no range matches (default "unicast").
 * @return The matching range name.
 *
 * @since 0.1.0
 */
using CIDRRange = std::pair<std::variant<IPv4, IPv6>, int>;

std::string subnetMatch(
    const std::variant<IPv4, IPv6>& address,
    const std::vector<std::pair<std::string, std::vector<CIDRRange>>>& rangeList,
    const std::string& defaultName = "unicast");

} // namespace ipaddr
} // namespace polycpp
