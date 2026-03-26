# polycpp/ipaddr

C++ port of [ipaddr.js](https://github.com/whitequark/ipaddr.js) for [polycpp](https://github.com/enricohuang/polycpp).

A library for manipulating IPv4 and IPv6 addresses in C++. Provides parsing, validation, CIDR matching, range detection, and IPv4/IPv6 conversion.

## Features

- **IPv4 and IPv6 parsing** with support for various notations (hex, octal, classful)
- **CIDR matching** for subnet checks
- **Range detection** — identify `loopback`, `private`, `linkLocal`, `multicast`, `unicast`, etc.
- **IPv4-mapped IPv6** address conversion
- **Subnet mask** utilities (prefix length, broadcast/network address from CIDR)
- **RFC 5952** compliant IPv6 string formatting

## Quick Start

```cpp
#include <polycpp/ipaddr/ipaddr.hpp>

using namespace polycpp::ipaddr;

// Parse and inspect
auto addr = IPv4::parse("192.168.1.1");
addr.kind();       // "ipv4"
addr.range();      // "private"
addr.toString();   // "192.168.1.1"

// CIDR matching
addr.match(IPv4::parse("192.168.0.0"), 16);  // true

// IPv6
auto v6 = IPv6::parse("2001:db8::1");
v6.range();  // "reserved"

// Polymorphic parsing
auto any = parse("::1");  // returns variant<IPv4, IPv6>

// IPv4-mapped IPv6 conversion
auto mapped = addr.toIPv4MappedAddress();
mapped.toIPv4Address();  // back to IPv4
```

## Building

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)
cd build && ctest --output-on-failure
```

## License

MIT. See [LICENSE](LICENSE).
