Classify with named subnets
===========================

**You'll build:** a classifier that tags every address with a
project-specific label — ``"office"``, ``"cloud"``, ``"guest"``,
``"untrusted"`` — by matching it against a list of named CIDR
ranges. This is the primitive behind network-aware metrics,
request-router rules, and audit logs.

**You'll use:**
:cpp:func:`polycpp::ipaddr::subnetMatch`,
:cpp:func:`polycpp::ipaddr::parse`,
:cpp:func:`polycpp::ipaddr::IPv4::parseCIDR`,
:cpp:func:`polycpp::ipaddr::IPv6::parseCIDR`.

**Prerequisites:** you've read :doc:`allow-list`.

Step 1 — build the range list
-----------------------------

:cpp:func:`subnetMatch` takes an
``std::vector<std::pair<std::string, std::vector<CIDRRange>>>`` —
a list of ``(name, [CIDR, CIDR, …])`` pairs. Named groups are
inspected in order and the first match wins.

.. code-block:: cpp

   #include <iostream>
   #include <polycpp/ipaddr/ipaddr.hpp>

   using namespace polycpp::ipaddr;

   std::vector<std::pair<std::string, std::vector<CIDRRange>>> ranges = {
       {"office", {
           IPv4::parseCIDR("10.42.0.0/16"),
           IPv6::parseCIDR("2001:db8:42::/48"),
       }},
       {"cloud", {
           IPv4::parseCIDR("10.100.0.0/16"),
           IPv4::parseCIDR("10.101.0.0/16"),
       }},
       {"guest", {
           IPv4::parseCIDR("192.168.42.0/24"),
       }},
   };

Note the heterogeneous v4+v6 list inside ``office`` — the library
packs the prefix pair as a ``std::variant<IPv4, IPv6>`` so the two
families co-exist in the same bucket.

Step 2 — call the classifier
----------------------------

.. code-block:: cpp

   std::string label(const std::string& addr) {
       auto value = parse(addr);
       return subnetMatch(value, ranges, "untrusted");
   }

   int main() {
       for (const auto& s : {"10.42.5.9", "10.100.0.8",
                             "192.168.42.1", "8.8.8.8",
                             "2001:db8:42::1"}) {
           std::cout << s << " -> " << label(s) << '\n';
       }
   }

Expected output:

.. code-block:: text

   10.42.5.9 -> office
   10.100.0.8 -> cloud
   192.168.42.1 -> guest
   8.8.8.8 -> untrusted
   2001:db8:42::1 -> office

Step 3 — the defaultName parameter
----------------------------------

The third argument to :cpp:func:`subnetMatch` is the label returned
when no named range matches — ``"unicast"`` by default. For a
binary "allow/deny" classifier use ``"deny"``; for a reporting tool,
``"unknown"`` is often clearer than ``"unicast"``.

Step 4 — order matters
----------------------

Named groups are checked top-to-bottom, and the **first** matching
group is returned. If you have overlapping ranges (e.g. an
``internal`` supernet and an ``office`` subnet), list the more
specific group first or you'll never see its name.

.. code-block:: cpp

   // Wrong order — every office address classifies as "internal".
   ranges = {
       {"internal", {IPv4::parseCIDR("10.0.0.0/8")}},   // too broad, first
       {"office",   {IPv4::parseCIDR("10.42.0.0/16")}},
   };

   // Right order — specific first, broad second.
   ranges = {
       {"office",   {IPv4::parseCIDR("10.42.0.0/16")}},
       {"internal", {IPv4::parseCIDR("10.0.0.0/8")}},
   };

What you learned
----------------

- :cpp:func:`subnetMatch` is the one-call idiom for named-subnet
  tagging — the rest is just data.
- v4 and v6 CIDRs share the same ``CIDRRange`` slot, so one named
  bucket can cover both families.
- Order the buckets from most-specific to least-specific.
