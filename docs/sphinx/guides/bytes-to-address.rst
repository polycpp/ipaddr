Convert between bytes and addresses
===================================

**When to reach for this:** you've got a 4- or 16-byte buffer
straight from a socket, a database, or wire protocol, and need
either the typed address or the canonical string form.

:cpp:func:`polycpp::ipaddr::fromByteArray` dispatches on length — 4
bytes give you :cpp:class:`IPv4`, 16 bytes give you
:cpp:class:`IPv6`, anything else throws
``std::invalid_argument``.

.. code-block:: cpp

   #include <polycpp/ipaddr/ipaddr.hpp>
   using namespace polycpp::ipaddr;

   std::vector<uint8_t> v4_bytes = {10, 0, 0, 1};
   auto v4 = std::get<IPv4>(fromByteArray(v4_bytes));
   // v4.toString() == "10.0.0.1"

   std::vector<uint8_t> v6_bytes(16, 0);
   v6_bytes[15] = 1;    // ::1
   auto v6 = std::get<IPv6>(fromByteArray(v6_bytes));
   // v6.toString() == "::1"

Going the other direction — given a typed address, emit the byte
array — use the member method:

.. code-block:: cpp

   auto v4 = IPv4::parse("192.168.1.1");
   std::vector<uint8_t> bytes = v4.toByteArray();
   // bytes == {192, 168, 1, 1}

   auto v6 = IPv6::parse("2001:db8::1");
   std::vector<uint8_t> bytes6 = v6.toByteArray();
   // bytes6.size() == 16; network-order MSB-first.

Byte order is always network order (big-endian). On a little-endian
host, don't ``memcpy`` an IPv4 into a ``uint32_t`` and expect the
numeric value to match — use :cpp:func:`IPv4::octets` for the typed
four-byte array.
