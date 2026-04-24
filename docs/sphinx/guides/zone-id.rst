Handle IPv6 zone identifiers
============================

**When to reach for this:** you've got an IPv6 address like
``fe80::1%eth0`` (a link-local address with a zone identifier) and
need to round-trip the ``%eth0`` suffix or operate on the address
without it.

:cpp:class:`polycpp::ipaddr::IPv6` stores the zone ID separately:

.. code-block:: cpp

   #include <polycpp/ipaddr/ipaddr.hpp>
   using namespace polycpp::ipaddr;

   auto v = IPv6::parse("fe80::1%eth0");
   v.zoneId();     // "eth0"
   v.toString();   // "fe80::1%eth0"  — the zone is re-emitted

   auto v2 = IPv6::parse("fe80::1");
   v2.zoneId();    // ""              — empty when absent

Construct an IPv6 from parts with an explicit zone:

.. code-block:: cpp

   std::array<uint16_t, 8> parts = {0xfe80, 0, 0, 0, 0, 0, 0, 1};
   IPv6 v(parts, "eth0");
   v.toString();   // "fe80::1%eth0"

Zone IDs are just opaque strings to the library — typically the
name of a network interface, but any value is accepted. Equality
(``==``) compares both the 16-byte value **and** the zone ID, so
``fe80::1%eth0`` ≠ ``fe80::1%eth1``. Strip the zone with
``IPv6(v.parts())`` — the single-argument constructor leaves
``zoneId`` empty — if your matching logic shouldn't depend on the
interface.
