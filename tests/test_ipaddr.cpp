/**
 * @file test_ipaddr.cpp
 * @brief Tests for polycpp::ipaddr — ported from ipaddr.js test suite.
 */

#include <polycpp/ipaddr/detail/aggregator.hpp>

#include <gtest/gtest.h>

#include <string>
#include <variant>
#include <vector>

using namespace polycpp::ipaddr;

// ══════════════════════════════════════════════════════════════════════
// IPv4 Construction
// ══════════════════════════════════════════════════════════════════════

TEST(IPv4Test, ConstructFromOctets) {
    EXPECT_NO_THROW(IPv4({192, 168, 1, 2}));
}

TEST(IPv4Test, ConvertsToStringCorrectly) {
    auto addr = IPv4({192, 168, 1, 1});
    EXPECT_EQ(addr.toString(), "192.168.1.1");
    EXPECT_EQ(addr.toNormalizedString(), "192.168.1.1");
}

TEST(IPv4Test, ReturnsCorrectKind) {
    auto addr = IPv4({1, 2, 3, 4});
    EXPECT_EQ(addr.kind(), "ipv4");
}

TEST(IPv4Test, AllowsAccessToOctets) {
    auto addr = IPv4({42, 0, 0, 0});
    EXPECT_EQ(addr.octets()[0], 42);
}

// ══════════════════════════════════════════════════════════════════════
// IPv4 Format Checking
// ══════════════════════════════════════════════════════════════════════

TEST(IPv4Test, ChecksIPv4AddressFormat) {
    EXPECT_TRUE(IPv4::isIPv4("192.168.007.0xa"));
    EXPECT_TRUE(IPv4::isIPv4("1024.0.0.1"));
    EXPECT_FALSE(IPv4::isIPv4("8.0xa.wtf.6"));
}

TEST(IPv4Test, ValidatesIPv4Addresses) {
    EXPECT_TRUE(IPv4::isValid("192.168.007.0xa"));
    EXPECT_FALSE(IPv4::isValid("1024.0.0.1"));
    EXPECT_FALSE(IPv4::isValid("8.0xa.wtf.6"));
}

TEST(IPv4Test, ValidatesIPv4CIDRNotation) {
    EXPECT_TRUE(IPv4::isValidCIDR("192.168.1.1/24"));
    EXPECT_FALSE(IPv4::isValidCIDR("10.5.0.1"));
    EXPECT_FALSE(IPv4::isValidCIDR("0.0.0.0/-1"));
    EXPECT_FALSE(IPv4::isValidCIDR("192.168.1.1/999"));
}

// ══════════════════════════════════════════════════════════════════════
// IPv4 Parsing
// ══════════════════════════════════════════════════════════════════════

TEST(IPv4Test, ParsesWeirdFormats) {
    auto check = [](const std::string& input, uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
        auto addr = IPv4::parse(input);
        EXPECT_EQ(addr.octets()[0], a) << "Failed for: " << input;
        EXPECT_EQ(addr.octets()[1], b) << "Failed for: " << input;
        EXPECT_EQ(addr.octets()[2], c) << "Failed for: " << input;
        EXPECT_EQ(addr.octets()[3], d) << "Failed for: " << input;
    };

    check("192.168.1.1", 192, 168, 1, 1);
    check("0xc0.168.1.1", 192, 168, 1, 1);
    check("192.0250.1.1", 192, 168, 1, 1);
    check("0xc0a80101", 192, 168, 1, 1);
    check("030052000401", 192, 168, 1, 1);
    check("3232235777", 192, 168, 1, 1);
    check("127.42.258", 127, 42, 1, 2);
    check("127.66051", 127, 1, 2, 3);
    check("10.1.1.0xff", 10, 1, 1, 255);
}

TEST(IPv4Test, ThrowsOnInvalidIPv4) {
    EXPECT_THROW(IPv4::parse("10.0.0.wtf"), std::invalid_argument);
    EXPECT_THROW(IPv4::parse("8.0x1ffffff"), std::invalid_argument);
    EXPECT_THROW(IPv4::parse("8.8.0x1ffff"), std::invalid_argument);
    EXPECT_THROW(IPv4::parse("10.048.1.1"), std::invalid_argument);
}

// ══════════════════════════════════════════════════════════════════════
// IPv4 CIDR Matching
// ══════════════════════════════════════════════════════════════════════

TEST(IPv4Test, MatchesCIDRCorrectly) {
    auto addr = IPv4({10, 5, 0, 1});
    EXPECT_TRUE(addr.match(IPv4::parse("0.0.0.0"), 0));
    EXPECT_FALSE(addr.match(IPv4::parse("11.0.0.0"), 8));
    EXPECT_TRUE(addr.match(IPv4::parse("10.0.0.0"), 8));
    EXPECT_TRUE(addr.match(IPv4::parse("10.0.0.1"), 8));
    EXPECT_TRUE(addr.match(IPv4::parse("10.0.0.10"), 8));
    EXPECT_TRUE(addr.match(IPv4::parse("10.5.5.0"), 16));
    EXPECT_FALSE(addr.match(IPv4::parse("10.4.5.0"), 16));
    EXPECT_TRUE(addr.match(IPv4::parse("10.4.5.0"), 15));
    EXPECT_FALSE(addr.match(IPv4::parse("10.5.0.2"), 32));
    EXPECT_TRUE(addr.match(addr, 32));
}

TEST(IPv4Test, ParsesIPv4CIDRCorrectly) {
    auto addr = IPv4({10, 5, 0, 1});
    EXPECT_TRUE(addr.match(IPv4::parseCIDR("0.0.0.0/0")));
    EXPECT_FALSE(addr.match(IPv4::parseCIDR("11.0.0.0/8")));
    EXPECT_TRUE(addr.match(IPv4::parseCIDR("10.0.0.0/8")));
    EXPECT_TRUE(addr.match(IPv4::parseCIDR("10.0.0.1/8")));
    EXPECT_TRUE(addr.match(IPv4::parseCIDR("10.0.0.10/8")));
    EXPECT_TRUE(addr.match(IPv4::parseCIDR("10.5.5.0/16")));
    EXPECT_FALSE(addr.match(IPv4::parseCIDR("10.4.5.0/16")));
    EXPECT_TRUE(addr.match(IPv4::parseCIDR("10.4.5.0/15")));
    EXPECT_FALSE(addr.match(IPv4::parseCIDR("10.5.0.2/32")));
    EXPECT_TRUE(addr.match(IPv4::parseCIDR("10.5.0.1/32")));
    EXPECT_THROW(IPv4::parseCIDR("10.5.0.1"), std::invalid_argument);
    EXPECT_THROW(IPv4::parseCIDR("0.0.0.0/-1"), std::invalid_argument);
    EXPECT_THROW(IPv4::parseCIDR("0.0.0.0/33"), std::invalid_argument);
}

// ══════════════════════════════════════════════════════════════════════
// IPv4 Range Detection
// ══════════════════════════════════════════════════════════════════════

TEST(IPv4Test, DetectsReservedNetworks) {
    EXPECT_EQ(IPv4::parse("0.0.0.0").range(), "unspecified");
    EXPECT_EQ(IPv4::parse("0.1.0.0").range(), "unspecified");
    EXPECT_EQ(IPv4::parse("10.1.0.1").range(), "private");
    EXPECT_EQ(IPv4::parse("100.64.0.0").range(), "carrierGradeNat");
    EXPECT_EQ(IPv4::parse("100.127.255.255").range(), "carrierGradeNat");
    EXPECT_EQ(IPv4::parse("192.52.193.1").range(), "amt");
    EXPECT_EQ(IPv4::parse("192.168.2.1").range(), "private");
    EXPECT_EQ(IPv4::parse("192.175.48.0").range(), "as112");
    EXPECT_EQ(IPv4::parse("224.100.0.1").range(), "multicast");
    EXPECT_EQ(IPv4::parse("169.254.15.0").range(), "linkLocal");
    EXPECT_EQ(IPv4::parse("127.1.1.1").range(), "loopback");
    EXPECT_EQ(IPv4::parse("255.255.255.255").range(), "broadcast");
    EXPECT_EQ(IPv4::parse("240.1.2.3").range(), "reserved");
    EXPECT_EQ(IPv4::parse("8.8.8.8").range(), "unicast");
}

// ══════════════════════════════════════════════════════════════════════
// IPv4 Four-Part Decimal Validation
// ══════════════════════════════════════════════════════════════════════

TEST(IPv4Test, ChecksConventionalFourPartDecimal) {
    EXPECT_TRUE(IPv4::isValidFourPartDecimal("0.0.0.0"));
    EXPECT_TRUE(IPv4::isValidFourPartDecimal("127.0.0.1"));
    EXPECT_TRUE(IPv4::isValidFourPartDecimal("192.168.1.1"));
    EXPECT_FALSE(IPv4::isValidFourPartDecimal("0xc0.168.1.1"));
}

TEST(IPv4Test, ChecksConventionalFourPartDecimalCIDR) {
    EXPECT_TRUE(IPv4::isValidCIDRFourPartDecimal("192.168.1.1/24"));
    EXPECT_TRUE(IPv4::isValidCIDRFourPartDecimal("0.0.0.0/0"));
    EXPECT_FALSE(IPv4::isValidCIDRFourPartDecimal("0xc0.168.1.1/24"));
    EXPECT_FALSE(IPv4::isValidCIDRFourPartDecimal("192.168.1.1/33"));
    EXPECT_FALSE(IPv4::isValidCIDRFourPartDecimal("192.168.1/24"));
}

TEST(IPv4Test, RefusesLeadingTrailingZeros) {
    EXPECT_FALSE(IPv4::isValidFourPartDecimal("000000192.168.100.2"));
    EXPECT_FALSE(IPv4::isValidFourPartDecimal("192.0000168.100.2"));
    EXPECT_FALSE(IPv4::isValidFourPartDecimal("192.168.100.00000002"));
    EXPECT_FALSE(IPv4::isValidFourPartDecimal("192.168.100.20000000"));
}

// ══════════════════════════════════════════════════════════════════════
// IPv4 Byte Array
// ══════════════════════════════════════════════════════════════════════

TEST(IPv4Test, ConvertsByteArray) {
    auto bytes = IPv4::parse("1.2.3.4").toByteArray();
    EXPECT_EQ(bytes.size(), 4u);
    EXPECT_EQ(bytes[0], 0x01);
    EXPECT_EQ(bytes[1], 0x02);
    EXPECT_EQ(bytes[2], 0x03);
    EXPECT_EQ(bytes[3], 0x04);
}

// ══════════════════════════════════════════════════════════════════════
// IPv4 Subnet Mask
// ══════════════════════════════════════════════════════════════════════

TEST(IPv4Test, PrefixLengthFromSubnetMask) {
    EXPECT_EQ(IPv4::parse("255.255.255.255").prefixLengthFromSubnetMask(), 32);
    EXPECT_EQ(IPv4::parse("255.255.255.254").prefixLengthFromSubnetMask(), 31);
    EXPECT_EQ(IPv4::parse("255.255.255.252").prefixLengthFromSubnetMask(), 30);
    EXPECT_EQ(IPv4::parse("255.255.255.248").prefixLengthFromSubnetMask(), 29);
    EXPECT_EQ(IPv4::parse("255.255.255.240").prefixLengthFromSubnetMask(), 28);
    EXPECT_EQ(IPv4::parse("255.255.255.224").prefixLengthFromSubnetMask(), 27);
    EXPECT_EQ(IPv4::parse("255.255.255.192").prefixLengthFromSubnetMask(), 26);
    EXPECT_EQ(IPv4::parse("255.255.255.128").prefixLengthFromSubnetMask(), 25);
    EXPECT_EQ(IPv4::parse("255.255.255.0").prefixLengthFromSubnetMask(), 24);
    EXPECT_EQ(IPv4::parse("255.255.254.0").prefixLengthFromSubnetMask(), 23);
    EXPECT_EQ(IPv4::parse("255.255.252.0").prefixLengthFromSubnetMask(), 22);
    EXPECT_EQ(IPv4::parse("255.255.248.0").prefixLengthFromSubnetMask(), 21);
    EXPECT_EQ(IPv4::parse("255.255.240.0").prefixLengthFromSubnetMask(), 20);
    EXPECT_EQ(IPv4::parse("255.255.224.0").prefixLengthFromSubnetMask(), 19);
    EXPECT_EQ(IPv4::parse("255.255.192.0").prefixLengthFromSubnetMask(), 18);
    EXPECT_EQ(IPv4::parse("255.255.128.0").prefixLengthFromSubnetMask(), 17);
    EXPECT_EQ(IPv4::parse("255.255.0.0").prefixLengthFromSubnetMask(), 16);
    EXPECT_EQ(IPv4::parse("255.254.0.0").prefixLengthFromSubnetMask(), 15);
    EXPECT_EQ(IPv4::parse("255.252.0.0").prefixLengthFromSubnetMask(), 14);
    EXPECT_EQ(IPv4::parse("255.248.0.0").prefixLengthFromSubnetMask(), 13);
    EXPECT_EQ(IPv4::parse("255.240.0.0").prefixLengthFromSubnetMask(), 12);
    EXPECT_EQ(IPv4::parse("255.224.0.0").prefixLengthFromSubnetMask(), 11);
    EXPECT_EQ(IPv4::parse("255.192.0.0").prefixLengthFromSubnetMask(), 10);
    EXPECT_EQ(IPv4::parse("255.128.0.0").prefixLengthFromSubnetMask(), 9);
    EXPECT_EQ(IPv4::parse("255.0.0.0").prefixLengthFromSubnetMask(), 8);
    EXPECT_EQ(IPv4::parse("254.0.0.0").prefixLengthFromSubnetMask(), 7);
    EXPECT_EQ(IPv4::parse("252.0.0.0").prefixLengthFromSubnetMask(), 6);
    EXPECT_EQ(IPv4::parse("248.0.0.0").prefixLengthFromSubnetMask(), 5);
    EXPECT_EQ(IPv4::parse("240.0.0.0").prefixLengthFromSubnetMask(), 4);
    EXPECT_EQ(IPv4::parse("224.0.0.0").prefixLengthFromSubnetMask(), 3);
    EXPECT_EQ(IPv4::parse("192.0.0.0").prefixLengthFromSubnetMask(), 2);
    EXPECT_EQ(IPv4::parse("128.0.0.0").prefixLengthFromSubnetMask(), 1);
    EXPECT_EQ(IPv4::parse("0.0.0.0").prefixLengthFromSubnetMask(), 0);
    // negative cases
    EXPECT_EQ(IPv4::parse("192.168.255.0").prefixLengthFromSubnetMask(), std::nullopt);
    EXPECT_EQ(IPv4::parse("255.0.255.0").prefixLengthFromSubnetMask(), std::nullopt);
}

TEST(IPv4Test, SubnetMaskFromPrefixLength) {
    EXPECT_EQ(IPv4::subnetMaskFromPrefixLength(0).toString(), "0.0.0.0");
    EXPECT_EQ(IPv4::subnetMaskFromPrefixLength(1).toString(), "128.0.0.0");
    EXPECT_EQ(IPv4::subnetMaskFromPrefixLength(2).toString(), "192.0.0.0");
    EXPECT_EQ(IPv4::subnetMaskFromPrefixLength(3).toString(), "224.0.0.0");
    EXPECT_EQ(IPv4::subnetMaskFromPrefixLength(4).toString(), "240.0.0.0");
    EXPECT_EQ(IPv4::subnetMaskFromPrefixLength(5).toString(), "248.0.0.0");
    EXPECT_EQ(IPv4::subnetMaskFromPrefixLength(6).toString(), "252.0.0.0");
    EXPECT_EQ(IPv4::subnetMaskFromPrefixLength(7).toString(), "254.0.0.0");
    EXPECT_EQ(IPv4::subnetMaskFromPrefixLength(8).toString(), "255.0.0.0");
    EXPECT_EQ(IPv4::subnetMaskFromPrefixLength(24).toString(), "255.255.255.0");
    EXPECT_EQ(IPv4::subnetMaskFromPrefixLength(32).toString(), "255.255.255.255");
}

TEST(IPv4Test, BroadcastAddressFromCIDR) {
    EXPECT_EQ(IPv4::broadcastAddressFromCIDR("172.0.0.1/24").toString(), "172.0.0.255");
    EXPECT_EQ(IPv4::broadcastAddressFromCIDR("172.0.0.1/26").toString(), "172.0.0.63");
}

TEST(IPv4Test, NetworkAddressFromCIDR) {
    EXPECT_EQ(IPv4::networkAddressFromCIDR("172.0.0.1/24").toString(), "172.0.0.0");
    EXPECT_EQ(IPv4::networkAddressFromCIDR("172.0.0.1/5").toString(), "168.0.0.0");
}

// ══════════════════════════════════════════════════════════════════════
// IPv6 Construction
// ══════════════════════════════════════════════════════════════════════

TEST(IPv6Test, ConstructFrom16BitParts) {
    EXPECT_NO_THROW(IPv6(std::array<uint16_t, 8>{0x2001, 0xdb8, 0xf53a, 0, 0, 0, 0, 1}));
}

TEST(IPv6Test, ConstructFrom8BitParts) {
    auto fromBytes = IPv6(std::array<uint8_t, 16>{
        0x20, 0x01, 0x0d, 0xb8, 0xf5, 0x3a, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01});
    auto fromParts = IPv6(std::array<uint16_t, 8>{0x2001, 0xdb8, 0xf53a, 0, 0, 0, 0, 1});
    EXPECT_EQ(fromBytes.parts(), fromParts.parts());
}

TEST(IPv6Test, RefusesInvalidConstruction) {
    // 0xfffff exceeds uint16_t range, so construction should fail.
    // We construct via explicit values that fit in uint16_t but test
    // that values > 0xffff are caught at the parts level.
    // Since std::array<uint16_t, 8> truncates, we test via parse instead.
    // The JS version would throw on values > 0xffff.
    // For C++, we verify the parse-level validation catches this.
    EXPECT_THROW(IPv6::parse("fe80::0::1"), std::invalid_argument);
}

// ══════════════════════════════════════════════════════════════════════
// IPv6 String Conversion
// ══════════════════════════════════════════════════════════════════════

TEST(IPv6Test, ConvertsToStringCorrectly) {
    auto addr = IPv6(std::array<uint16_t, 8>{0x2001, 0xdb8, 0xf53a, 0, 0, 0, 0, 1});
    EXPECT_EQ(addr.toNormalizedString(), "2001:db8:f53a:0:0:0:0:1");
    EXPECT_EQ(addr.toFixedLengthString(), "2001:0db8:f53a:0000:0000:0000:0000:0001");
    EXPECT_EQ(addr.toString(), "2001:db8:f53a::1");

    EXPECT_EQ(IPv6(std::array<uint16_t, 8>{0, 0, 0, 0, 0, 0, 0, 0}).toString(), "::");
    EXPECT_EQ(IPv6(std::array<uint16_t, 8>{0, 0, 0, 0, 0, 0, 0, 1}).toString(), "::1");
    EXPECT_EQ(IPv6(std::array<uint16_t, 8>{0x2001, 0xdb8, 0, 0, 0, 0, 0, 0}).toString(), "2001:db8::");
    EXPECT_EQ(IPv6(std::array<uint16_t, 8>{0, 0xff, 0, 0, 0, 0, 0, 0}).toString(), "0:ff::");
    EXPECT_EQ(IPv6(std::array<uint16_t, 8>{0, 0, 0, 0, 0, 0, 0xff, 0}).toString(), "::ff:0");
    EXPECT_EQ(IPv6(std::array<uint16_t, 8>{0, 0, 0xff, 0, 0, 0, 0, 0}).toString(), "0:0:ff::");
    EXPECT_EQ(IPv6(std::array<uint16_t, 8>{0, 0, 0, 0, 0, 0xff, 0, 0}).toString(), "::ff:0:0");
    EXPECT_EQ(IPv6(std::array<uint16_t, 8>{0, 0, 0, 0xff, 0xff, 0, 0, 0}).toString(), "::ff:ff:0:0:0");
}

TEST(IPv6Test, RFC5952Compliance) {
    // All groups non-zero, no abbreviation
    EXPECT_EQ(
        IPv6(std::array<uint16_t, 8>{0x2001, 0xdb8, 0xff, 0xabc, 0xdef, 0x123b, 0x456c, 0x78d}).toRFC5952String(),
        "2001:db8:ff:abc:def:123b:456c:78d");

    // Single zero group should NOT be abbreviated
    EXPECT_EQ(
        IPv6(std::array<uint16_t, 8>{0x2001, 0xdb8, 0xff, 0xabc, 0, 0x123b, 0x456c, 0x78d}).toRFC5952String(),
        "2001:db8:ff:abc:0:123b:456c:78d");

    // Two consecutive zero groups: abbreviate
    EXPECT_EQ(
        IPv6(std::array<uint16_t, 8>{0x2001, 0xdb8, 0xff, 0xabc, 0, 0, 0x456c, 0x78d}).toRFC5952String(),
        "2001:db8:ff:abc::456c:78d");
}

// ══════════════════════════════════════════════════════════════════════
// IPv6 Zone ID
// ══════════════════════════════════════════════════════════════════════

TEST(IPv6Test, ReturnsZoneId) {
    auto addr = IPv6(std::array<uint16_t, 8>{0x2001, 0xdb8, 0xf53a, 0, 0, 0, 0, 1}, "utun0");
    EXPECT_EQ(addr.toNormalizedString(), "2001:db8:f53a:0:0:0:0:1%utun0");
    EXPECT_EQ(addr.toString(), "2001:db8:f53a::1%utun0");

    EXPECT_EQ(parse("2001:db8:f53a::1%2").index(), 1u); // IPv6
    auto v6 = std::get<IPv6>(parse("2001:db8:f53a::1%2"));
    EXPECT_EQ(v6.toString(), "2001:db8:f53a::1%2");

    EXPECT_EQ(std::get<IPv6>(parse("2001:db8:f53a::1%WAT")).toString(), "2001:db8:f53a::1%WAT");
    EXPECT_EQ(std::get<IPv6>(parse("2001:db8:f53a::1%sUp")).toString(), "2001:db8:f53a::1%sUp");
}

TEST(IPv6Test, ZoneIdForIPv4MappedAddresses) {
    auto addr = std::get<IPv6>(parse("::ffff:192.168.1.1%eth0"));
    EXPECT_EQ(addr.toNormalizedString(), "0:0:0:0:0:ffff:c0a8:101%eth0");
    EXPECT_EQ(addr.toString(), "::ffff:c0a8:101%eth0");

    EXPECT_EQ(std::get<IPv6>(parse("::ffff:192.168.1.1%2")).toString(), "::ffff:c0a8:101%2");
    EXPECT_EQ(std::get<IPv6>(parse("::ffff:192.168.1.1%WAT")).toString(), "::ffff:c0a8:101%WAT");
    EXPECT_EQ(std::get<IPv6>(parse("::ffff:192.168.1.1%sUp")).toString(), "::ffff:c0a8:101%sUp");
}

TEST(IPv6Test, ReturnsCorrectKind) {
    auto addr = IPv6(std::array<uint16_t, 8>{0x2001, 0xdb8, 0xf53a, 0, 0, 0, 0, 1});
    EXPECT_EQ(addr.kind(), "ipv6");
}

TEST(IPv6Test, AllowsAccessToIPv6Parts) {
    auto addr = IPv6(std::array<uint16_t, 8>{0x2001, 0xdb8, 0xf53a, 0, 0, 42, 0, 1});
    EXPECT_EQ(addr.parts()[5], 42);
}

// ══════════════════════════════════════════════════════════════════════
// IPv6 Format Checking and Validation
// ══════════════════════════════════════════════════════════════════════

TEST(IPv6Test, ChecksIPv6AddressFormat) {
    EXPECT_TRUE(IPv6::isIPv6("2001:db8:F53A::1"));
    EXPECT_TRUE(IPv6::isIPv6("200001::1"));
    EXPECT_TRUE(IPv6::isIPv6("::ffff:192.168.1.1"));
    EXPECT_TRUE(IPv6::isIPv6("::ffff:192.168.1.1%z"));
    EXPECT_TRUE(IPv6::isIPv6("::10.2.3.4"));
    EXPECT_TRUE(IPv6::isIPv6("::12.34.56.78%z"));
    EXPECT_FALSE(IPv6::isIPv6("::ffff:300.168.1.1"));
    EXPECT_FALSE(IPv6::isIPv6("::ffff:300.168.1.1:0"));
    EXPECT_FALSE(IPv6::isIPv6("fe80::wtf"));
    EXPECT_FALSE(IPv6::isIPv6("fe80::%"));
}

TEST(IPv6Test, ValidatesIPv6Addresses) {
    EXPECT_TRUE(IPv6::isValid("2001:db8:F53A::1"));
    EXPECT_FALSE(IPv6::isValid("200001::1"));
    EXPECT_TRUE(IPv6::isValid("::ffff:192.168.1.1"));
    EXPECT_TRUE(IPv6::isValid("::ffff:192.168.1.1%z"));
    EXPECT_TRUE(IPv6::isValid("::1.1.1.1"));
    EXPECT_TRUE(IPv6::isValid("::1.2.3.4%z"));
    EXPECT_FALSE(IPv6::isValid("::ffff:300.168.1.1"));
    EXPECT_FALSE(IPv6::isValid("::ffff:300.168.1.1:0"));
    EXPECT_FALSE(IPv6::isValid("::ffff:222.1.41.9000"));
    EXPECT_FALSE(IPv6::isValid("2001:db8::F53A::1"));
    EXPECT_FALSE(IPv6::isValid("fe80::wtf"));
    EXPECT_FALSE(IPv6::isValid("fe80::%"));
    EXPECT_FALSE(IPv6::isValid("2002::2:"));
    EXPECT_TRUE(IPv6::isValid("::%z"));
}

TEST(IPv6Test, ValidatesIPv6CIDRNotation) {
    EXPECT_TRUE(IPv6::isValidCIDR("::/0"));
    EXPECT_TRUE(IPv6::isValidCIDR("2001:db8:F53A::1%z/64"));
    EXPECT_FALSE(IPv6::isValidCIDR("2001:db8:F53A::1/-1"));
    EXPECT_FALSE(IPv6::isValidCIDR("2001:db8:F53A::1"));
    EXPECT_FALSE(IPv6::isValidCIDR("2001:db8:F53A::1/129"));
    EXPECT_FALSE(IPv6::isValidCIDR("2001:db8:F53A::1%z/129"));
}

// ══════════════════════════════════════════════════════════════════════
// IPv6 Parsing
// ══════════════════════════════════════════════════════════════════════

TEST(IPv6Test, ParsesVariousFormats) {
    auto check = [](const std::string& input, const std::array<uint16_t, 8>& expected) {
        auto addr = IPv6::parse(input);
        EXPECT_EQ(addr.parts(), expected) << "Failed for: " << input;
    };

    check("2001:db8:F53A:0:0:0:0:1", {0x2001, 0xdb8, 0xf53a, 0, 0, 0, 0, 1});
    check("fe80::10", {0xfe80, 0, 0, 0, 0, 0, 0, 0x10});
    check("2001:db8:F53A::", {0x2001, 0xdb8, 0xf53a, 0, 0, 0, 0, 0});
    check("::1", {0, 0, 0, 0, 0, 0, 0, 1});
    check("::", {0, 0, 0, 0, 0, 0, 0, 0});
}

TEST(IPv6Test, ParsesTransitionalFormat) {
    auto addr = IPv6::parse("::ffff:192.168.1.1");
    EXPECT_EQ(addr.parts()[4], 0u);
    EXPECT_EQ(addr.parts()[5], 0xffff);
    EXPECT_EQ(addr.parts()[6], 0xc0a8);
    EXPECT_EQ(addr.parts()[7], 0x0101);
}

TEST(IPv6Test, ParsesZoneIdFromParsed) {
    auto addr = IPv6::parse("::%z");
    EXPECT_EQ(addr.parts(), (std::array<uint16_t, 8>{0, 0, 0, 0, 0, 0, 0, 0}));
    EXPECT_EQ(addr.zoneId(), "z");
}

TEST(IPv6Test, ThrowsOnInvalidIPv6) {
    EXPECT_THROW(IPv6::parse("fe80::0::1"), std::invalid_argument);
}

// ══════════════════════════════════════════════════════════════════════
// IPv6 CIDR Matching
// ══════════════════════════════════════════════════════════════════════

TEST(IPv6Test, MatchesCIDRCorrectly) {
    auto addr = IPv6::parse("2001:db8:f53a::1");
    EXPECT_TRUE(addr.match(IPv6::parse("::"), 0));
    EXPECT_TRUE(addr.match(IPv6::parse("2001:db8:f53a::1:1"), 64));
    EXPECT_FALSE(addr.match(IPv6::parse("2001:db8:f53b::1:1"), 48));
    EXPECT_TRUE(addr.match(IPv6::parse("2001:db8:f531::1:1"), 44));
    EXPECT_TRUE(addr.match(IPv6::parse("2001:db8:f500::1"), 40));
    EXPECT_TRUE(addr.match(IPv6::parse("2001:db8:f500::1%z"), 40));
    EXPECT_FALSE(addr.match(IPv6::parse("2001:db9:f500::1"), 40));
    EXPECT_FALSE(addr.match(IPv6::parse("2001:db9:f500::1%z"), 40));
    EXPECT_TRUE(addr.match(addr, 128));
}

TEST(IPv6Test, ParsesIPv6CIDRCorrectly) {
    auto addr = IPv6::parse("2001:db8:f53a::1");
    EXPECT_TRUE(addr.match(IPv6::parseCIDR("::/0")));
    EXPECT_TRUE(addr.match(IPv6::parseCIDR("2001:db8:f53a::1:1/64")));
    EXPECT_FALSE(addr.match(IPv6::parseCIDR("2001:db8:f53b::1:1/48")));
    EXPECT_TRUE(addr.match(IPv6::parseCIDR("2001:db8:f531::1:1/44")));
    EXPECT_TRUE(addr.match(IPv6::parseCIDR("2001:db8:f500::1/40")));
    EXPECT_TRUE(addr.match(IPv6::parseCIDR("2001:db8:f500::1%z/40")));
    EXPECT_FALSE(addr.match(IPv6::parseCIDR("2001:db9:f500::1/40")));
    EXPECT_FALSE(addr.match(IPv6::parseCIDR("2001:db9:f500::1%z/40")));
    EXPECT_TRUE(addr.match(IPv6::parseCIDR("2001:db8:f53a::1/128")));
    EXPECT_THROW(IPv6::parseCIDR("2001:db8:f53a::1"), std::invalid_argument);
    EXPECT_THROW(IPv6::parseCIDR("2001:db8:f53a::1/-1"), std::invalid_argument);
    EXPECT_THROW(IPv6::parseCIDR("2001:db8:f53a::1/129"), std::invalid_argument);
}

// ══════════════════════════════════════════════════════════════════════
// IPv6 Range Detection
// ══════════════════════════════════════════════════════════════════════

TEST(IPv6Test, DetectsReservedNetworks) {
    EXPECT_EQ(IPv6::parse("::").range(), "unspecified");
    EXPECT_EQ(IPv6::parse("fe80::1234:5678:abcd:0123").range(), "linkLocal");
    EXPECT_EQ(IPv6::parse("ff00::1234").range(), "multicast");
    EXPECT_EQ(IPv6::parse("::1").range(), "loopback");
    EXPECT_EQ(IPv6::parse("100::42").range(), "discard");
    EXPECT_EQ(IPv6::parse("fc00::").range(), "uniqueLocal");
    EXPECT_EQ(IPv6::parse("::ffff:192.168.1.10").range(), "ipv4Mapped");
    EXPECT_EQ(IPv6::parse("::ffff:0:192.168.1.10").range(), "rfc6145");
    EXPECT_EQ(IPv6::parse("64:ff9b::1234").range(), "rfc6052");
    EXPECT_EQ(IPv6::parse("2002:1f63:45e8::1").range(), "6to4");
    EXPECT_EQ(IPv6::parse("2001::4242").range(), "teredo");
    EXPECT_EQ(IPv6::parse("2001:2::").range(), "benchmarking");
    EXPECT_EQ(IPv6::parse("2001:3::").range(), "amt");
    EXPECT_EQ(IPv6::parse("2001:4:112::").range(), "as112v6");
    EXPECT_EQ(IPv6::parse("2620:4f:8000::").range(), "as112v6");
    EXPECT_EQ(IPv6::parse("2001:10::").range(), "deprecated");
    EXPECT_EQ(IPv6::parse("2001:20::").range(), "orchid2");
    EXPECT_EQ(IPv6::parse("2001:30::").range(), "droneRemoteIdProtocolEntityTags");
    EXPECT_EQ(IPv6::parse("2001:db8::3210").range(), "reserved");
    EXPECT_EQ(IPv6::parse("2001:470:8:66::1").range(), "unicast");
    EXPECT_EQ(IPv6::parse("2001:470:8:66::1%z").range(), "unicast");
}

// ══════════════════════════════════════════════════════════════════════
// IPv4/IPv6 Conversion
// ══════════════════════════════════════════════════════════════════════

TEST(ConversionTest, ConvertsBetweenIPv4MappedAndIPv4) {
    auto v4 = IPv4::parse("77.88.21.11");
    auto mapped = v4.toIPv4MappedAddress();
    EXPECT_EQ(mapped.parts()[0], 0u);
    EXPECT_EQ(mapped.parts()[4], 0u);
    EXPECT_EQ(mapped.parts()[5], 0xffff);
    EXPECT_EQ(mapped.parts()[6], 0x4d58);
    EXPECT_EQ(mapped.parts()[7], 0x150b);
    EXPECT_EQ(mapped.toIPv4Address().octets(), v4.octets());
}

TEST(ConversionTest, RefusesNonMappedToIPv4) {
    EXPECT_THROW(IPv6::parse("2001:db8::1").toIPv4Address(), std::invalid_argument);
}

// ══════════════════════════════════════════════════════════════════════
// IPv6 Subnet Mask
// ══════════════════════════════════════════════════════════════════════

TEST(IPv6Test, PrefixLengthFromSubnetMask) {
    EXPECT_EQ(IPv6::parse("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff").prefixLengthFromSubnetMask(), 128);
    EXPECT_EQ(IPv6::parse("ffff:ffff:ffff:ffff::").prefixLengthFromSubnetMask(), 64);
    EXPECT_EQ(IPv6::parse("ffff:ffff:ffff:ff80::").prefixLengthFromSubnetMask(), 57);
    EXPECT_EQ(IPv6::parse("ffff:ffff:ffff::").prefixLengthFromSubnetMask(), 48);
    EXPECT_EQ(IPv6::parse("ffff:ffff:ffff::%z").prefixLengthFromSubnetMask(), 48);
    EXPECT_EQ(IPv6::parse("::").prefixLengthFromSubnetMask(), 0);
    EXPECT_EQ(IPv6::parse("::%z").prefixLengthFromSubnetMask(), 0);
    // negative cases
    EXPECT_EQ(IPv6::parse("2001:db8::").prefixLengthFromSubnetMask(), std::nullopt);
    EXPECT_EQ(IPv6::parse("ffff:0:0:ffff::").prefixLengthFromSubnetMask(), std::nullopt);
    EXPECT_EQ(IPv6::parse("ffff:0:0:ffff::%z").prefixLengthFromSubnetMask(), std::nullopt);
}

TEST(IPv6Test, SubnetMaskFromPrefixLength) {
    EXPECT_EQ(IPv6::subnetMaskFromPrefixLength(128).toString(), "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff");
    EXPECT_EQ(IPv6::subnetMaskFromPrefixLength(112).toString(), "ffff:ffff:ffff:ffff:ffff:ffff:ffff:0");
    EXPECT_EQ(IPv6::subnetMaskFromPrefixLength(96).toString(),  "ffff:ffff:ffff:ffff:ffff:ffff::");
    EXPECT_EQ(IPv6::subnetMaskFromPrefixLength(64).toString(),  "ffff:ffff:ffff:ffff::");
    EXPECT_EQ(IPv6::subnetMaskFromPrefixLength(48).toString(),  "ffff:ffff:ffff::");
    EXPECT_EQ(IPv6::subnetMaskFromPrefixLength(32).toString(),  "ffff:ffff::");
    EXPECT_EQ(IPv6::subnetMaskFromPrefixLength(16).toString(),  "ffff::");
    EXPECT_EQ(IPv6::subnetMaskFromPrefixLength(8).toString(),   "ff00::");
    EXPECT_EQ(IPv6::subnetMaskFromPrefixLength(4).toString(),   "f000::");
    EXPECT_EQ(IPv6::subnetMaskFromPrefixLength(2).toString(),   "c000::");
    EXPECT_EQ(IPv6::subnetMaskFromPrefixLength(1).toString(),   "8000::");
    EXPECT_EQ(IPv6::subnetMaskFromPrefixLength(0).toString(),   "::");
}

TEST(IPv6Test, NetworkAddressFromCIDR) {
    EXPECT_EQ(IPv6::networkAddressFromCIDR("::/0").toString(),                  "::");
    EXPECT_EQ(IPv6::networkAddressFromCIDR("2001:db8:f53a::1:1/64").toString(), "2001:db8:f53a::");
    EXPECT_EQ(IPv6::networkAddressFromCIDR("2001:db8:f53b::1:1/48").toString(), "2001:db8:f53b::");
    EXPECT_EQ(IPv6::networkAddressFromCIDR("2001:db8:f531::1:1/44").toString(), "2001:db8:f530::");
    EXPECT_EQ(IPv6::networkAddressFromCIDR("2001:db8:f500::1/40").toString(),   "2001:db8:f500::");
    EXPECT_EQ(IPv6::networkAddressFromCIDR("2001:db8:f500::1%z/40").toString(), "2001:db8:f500::");
    EXPECT_EQ(IPv6::networkAddressFromCIDR("2001:db9:f500::1/40").toString(),   "2001:db9:f500::");
    EXPECT_EQ(IPv6::networkAddressFromCIDR("2001:db9:f500::1%z/40").toString(), "2001:db9:f500::");
    EXPECT_EQ(IPv6::networkAddressFromCIDR("2001:db8:f53a::1/128").toString(),  "2001:db8:f53a::1");
}

TEST(IPv6Test, BroadcastAddressFromCIDR) {
    EXPECT_EQ(IPv6::broadcastAddressFromCIDR("::/0").toString(),                  "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff");
    EXPECT_EQ(IPv6::broadcastAddressFromCIDR("2001:db8:f53a::1:1/64").toString(), "2001:db8:f53a:0:ffff:ffff:ffff:ffff");
    EXPECT_EQ(IPv6::broadcastAddressFromCIDR("2001:db8:f53b::1:1/48").toString(), "2001:db8:f53b:ffff:ffff:ffff:ffff:ffff");
    EXPECT_EQ(IPv6::broadcastAddressFromCIDR("2001:db8:f531::1:1/44").toString(), "2001:db8:f53f:ffff:ffff:ffff:ffff:ffff");
    EXPECT_EQ(IPv6::broadcastAddressFromCIDR("2001:db8:f500::1/40").toString(),   "2001:db8:f5ff:ffff:ffff:ffff:ffff:ffff");
    EXPECT_EQ(IPv6::broadcastAddressFromCIDR("2001:db8:f500::1%z/40").toString(), "2001:db8:f5ff:ffff:ffff:ffff:ffff:ffff");
    EXPECT_EQ(IPv6::broadcastAddressFromCIDR("2001:db9:f500::1/40").toString(),   "2001:db9:f5ff:ffff:ffff:ffff:ffff:ffff");
    EXPECT_EQ(IPv6::broadcastAddressFromCIDR("2001:db9:f500::1%z/40").toString(), "2001:db9:f5ff:ffff:ffff:ffff:ffff:ffff");
    EXPECT_EQ(IPv6::broadcastAddressFromCIDR("2001:db8:f53a::1/128").toString(),  "2001:db8:f53a::1");
}

// ══════════════════════════════════════════════════════════════════════
// Module-level functions
// ══════════════════════════════════════════════════════════════════════

TEST(ModuleTest, DeterminesIPAddressType) {
    auto v4 = parse("8.8.8.8");
    EXPECT_TRUE(std::holds_alternative<IPv4>(v4));
    EXPECT_EQ(std::get<IPv4>(v4).kind(), "ipv4");

    auto v6 = parse("2001:db8:3312::1");
    EXPECT_TRUE(std::holds_alternative<IPv6>(v6));
    EXPECT_EQ(std::get<IPv6>(v6).kind(), "ipv6");

    auto v6z = parse("2001:db8:3312::1%z");
    EXPECT_TRUE(std::holds_alternative<IPv6>(v6z));
}

TEST(ModuleTest, ThrowsOnInvalidAddress) {
    EXPECT_THROW(parse("::some.nonsense"), std::invalid_argument);
}

TEST(ModuleTest, ProcessesIPv4MappedAddresses) {
    auto r1 = process("8.8.8.8");
    EXPECT_TRUE(std::holds_alternative<IPv4>(r1));

    auto r2 = process("2001:db8:3312::1");
    EXPECT_TRUE(std::holds_alternative<IPv6>(r2));

    // IPv4-mapped should return IPv4
    auto r3 = process("::ffff:192.168.1.1");
    EXPECT_TRUE(std::holds_alternative<IPv4>(r3));
    EXPECT_EQ(std::get<IPv4>(r3).kind(), "ipv4");

    auto r4 = process("::ffff:192.168.1.1%z");
    EXPECT_TRUE(std::holds_alternative<IPv4>(r4));
}

TEST(ModuleTest, ConvertsIPv6AndIPv4ToByteArrays) {
    auto v4bytes = std::get<IPv4>(parse("1.2.3.4")).toByteArray();
    EXPECT_EQ(v4bytes, (std::vector<uint8_t>{0x01, 0x02, 0x03, 0x04}));

    auto v6bytes = std::get<IPv6>(parse("2a00:1450:8007::68")).toByteArray();
    EXPECT_EQ(v6bytes.size(), 16u);
    EXPECT_EQ(v6bytes[0], 42u);  // 0x2a
    EXPECT_EQ(v6bytes[15], 0x68u);

    // With zone ID
    auto v6zbytes = std::get<IPv6>(parse("2a00:1450:8007::68%z")).toByteArray();
    EXPECT_EQ(v6zbytes.size(), 16u);
    EXPECT_EQ(v6zbytes[0], 42u);
}

TEST(ModuleTest, ParsesSingleNumberAsIPv4) {
    EXPECT_FALSE(IPv6::isValid("1"));
    EXPECT_TRUE(IPv4::isValid("1"));
    auto result = parse("1");
    EXPECT_TRUE(std::holds_alternative<IPv4>(result));
    EXPECT_EQ(std::get<IPv4>(result).octets(), (std::array<uint8_t, 4>{0, 0, 0, 1}));
}

TEST(ModuleTest, DetectsIPv4AndIPv6CIDR) {
    auto [addr6, bits6] = parseCIDR("fc00::/64");
    EXPECT_TRUE(std::holds_alternative<IPv6>(addr6));
    EXPECT_EQ(bits6, 64);

    auto [addr4, bits4] = parseCIDR("1.2.3.4/5");
    EXPECT_TRUE(std::holds_alternative<IPv4>(addr4));
    EXPECT_EQ(bits4, 5);
}

TEST(ModuleTest, LargeAndNegativeNumbersNotValid) {
    EXPECT_FALSE(isValid("4999999999"));
    EXPECT_FALSE(isValid("-1"));
}

TEST(ModuleTest, DoesNotHangOnLongInput) {
    EXPECT_FALSE(IPv6::isValid("::8:8:8:8:8:8:8:8:8"));
    EXPECT_FALSE(IPv6::isValid("::8:8:8:8:8:8:8:8:8%z"));
}

TEST(ModuleTest, IsValid) {
    EXPECT_TRUE(isValid("192.168.1.1"));
    EXPECT_TRUE(isValid("::1"));
    EXPECT_FALSE(isValid("not-an-ip"));
}

TEST(ModuleTest, IsValidCIDR) {
    EXPECT_TRUE(isValidCIDR("192.168.1.0/24"));
    EXPECT_TRUE(isValidCIDR("fc00::/64"));
    EXPECT_FALSE(isValidCIDR("not-a-cidr"));
}

TEST(ModuleTest, FromByteArray) {
    auto v4 = fromByteArray({0x7f, 0, 0, 1});
    EXPECT_TRUE(std::holds_alternative<IPv4>(v4));
    EXPECT_EQ(std::get<IPv4>(v4).kind(), "ipv4");

    auto v6 = fromByteArray({0x20, 0x01, 0x0d, 0xb8, 0xf5, 0x3a, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1});
    EXPECT_TRUE(std::holds_alternative<IPv6>(v6));
    EXPECT_EQ(std::get<IPv6>(v6).kind(), "ipv6");

    EXPECT_THROW(fromByteArray({1}), std::invalid_argument);
}

TEST(ModuleTest, SubnetMatchDoesNotFailOnEmptyRange) {
    EXPECT_NO_THROW(
        subnetMatch(
            IPv4({1, 2, 3, 4}),
            {},
            "fallback"
        )
    );
}

TEST(ModuleTest, SubnetMatchReturnsDefaultOnEmptyRange) {
    auto result = subnetMatch(
        IPv4({1, 2, 3, 4}),
        {},
        "fallback"
    );
    EXPECT_EQ(result, "fallback");
}

TEST(ModuleTest, Equality) {
    EXPECT_EQ(IPv4::parse("10.0.0.1"), IPv4::parse("10.0.0.1"));
    EXPECT_NE(IPv4::parse("10.0.0.1"), IPv4::parse("10.0.0.2"));
    EXPECT_EQ(IPv6::parse("::1"), IPv6::parse("::1"));
    EXPECT_NE(IPv6::parse("::1"), IPv6::parse("::2"));
}

// ══════════════════════════════════════════════════════════════════════
// IPv6 CIDR string representation
// ══════════════════════════════════════════════════════════════════════

TEST(IPv6Test, CIDRToStringCorrectly) {
    auto [addr1, bits1] = IPv6::parseCIDR("0:0:0:0:0:0:0:0/64");
    EXPECT_EQ(addr1.toString() + "/" + std::to_string(bits1), "::/64");

    auto [addr2, bits2] = IPv6::parseCIDR("0:0:0:ff:ff:0:0:0/64");
    EXPECT_EQ(addr2.toString() + "/" + std::to_string(bits2), "::ff:ff:0:0:0/64");

    auto [addr3, bits3] = IPv6::parseCIDR("2001:db8:ff:abc:def:123b:456c:78d/64");
    EXPECT_EQ(addr3.toString() + "/" + std::to_string(bits3), "2001:db8:ff:abc:def:123b:456c:78d/64");
}

TEST(IPv4Test, CIDRToStringCorrectly) {
    auto [addr, bits] = IPv4::parseCIDR("192.168.1.1/24");
    EXPECT_EQ(addr.toString() + "/" + std::to_string(bits), "192.168.1.1/24");
}

TEST(ModuleTest, ParseCIDRReversible) {
    auto [addr4, bits4] = parseCIDR("1.2.3.4/24");
    EXPECT_EQ(
        std::visit([](const auto& a) { return a.toString(); }, addr4) + "/" + std::to_string(bits4),
        "1.2.3.4/24"
    );

    auto [addr6, bits6] = parseCIDR("::1%zone/24");
    EXPECT_EQ(
        std::visit([](const auto& a) { return a.toString(); }, addr6) + "/" + std::to_string(bits6),
        "::1%zone/24"
    );
}

// Additional edge case: transitional IPv6 parsing (::8.8.8.8 form)
TEST(IPv6Test, ParsesDeprecatedTransitional) {
    auto addr = IPv6::parse("::8.8.8.8");
    // This should produce ::ffff:8.8.8.8 internally
    EXPECT_EQ(addr.parts()[5], 0xffff);
    EXPECT_EQ(addr.parts()[6], (8 << 8) | 8);  // 0x0808
    EXPECT_EQ(addr.parts()[7], (8 << 8) | 8);  // 0x0808
}

TEST(IPv6Test, ParsesTransitionalWithPrefix) {
    auto addr = IPv6::parse("FFFF::255.255.255.255");
    EXPECT_EQ(addr.parts()[0], 0xffff);
    EXPECT_EQ(addr.parts()[6], 0xffff);
    EXPECT_EQ(addr.parts()[7], 0xffff);
}
