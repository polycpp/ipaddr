Compute network and broadcast addresses
=======================================

**When to reach for this:** you've got a CIDR string
(``"192.168.1.50/24"``) and need the network address
(``"192.168.1.0"``), the broadcast address
(``"192.168.1.255"``), or the subnet mask (``"255.255.255.0"``).

All three are one-call static helpers:

.. code-block:: cpp

   #include <polycpp/ipaddr/ipaddr.hpp>
   using namespace polycpp::ipaddr;

   IPv4 net   = IPv4::networkAddressFromCIDR("192.168.1.50/24");   // 192.168.1.0
   IPv4 bcast = IPv4::broadcastAddressFromCIDR("192.168.1.50/24"); // 192.168.1.255
   IPv4 mask  = IPv4::subnetMaskFromPrefixLength(24);               // 255.255.255.0

The IPv6 equivalents work the same way:

.. code-block:: cpp

   IPv6 v6net  = IPv6::networkAddressFromCIDR("2001:db8::1/64");   // 2001:db8::
   IPv6 v6mask = IPv6::subnetMaskFromPrefixLength(64);
   // toString() == "ffff:ffff:ffff:ffff::"

Going the other way — given a subnet mask like ``255.255.255.0``,
what's the prefix length?

.. code-block:: cpp

   IPv4 mask = IPv4::parse("255.255.255.0");
   auto bits = mask.prefixLengthFromSubnetMask();   // std::optional<int>(24)

:cpp:func:`IPv4::prefixLengthFromSubnetMask` returns ``std::nullopt``
when the address isn't a contiguous-bit mask (``255.0.255.0`` and
similar sparse patterns). Always check before dereferencing.
