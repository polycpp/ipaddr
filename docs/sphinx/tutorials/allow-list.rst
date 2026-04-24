Build a CIDR allow-list
=======================

**You'll build:** an ``is_allowed(addr)`` predicate that takes a
client IP (IPv4 or IPv6 as a string) and returns true iff it falls
inside any entry of a configured allow-list. The exact shape is what
most firewall / reverse-proxy code needs.

**You'll use:**
:cpp:func:`polycpp::ipaddr::parse`,
:cpp:func:`polycpp::ipaddr::IPv4::parseCIDR`,
:cpp:func:`polycpp::ipaddr::IPv6::parseCIDR`,
:cpp:func:`polycpp::ipaddr::IPv4::match`,
:cpp:func:`polycpp::ipaddr::IPv6::match`,
:cpp:func:`polycpp::ipaddr::isValid`.

**Prerequisites:** you've read the :doc:`../getting-started/quickstart`
and know that ``parse()`` returns a ``std::variant<IPv4, IPv6>``.

Step 1 — sketch the data structure
----------------------------------

A single allow-list entry is a pair of ``(network, prefix-length)``,
and it lives in one of two buckets depending on address family.
Using the typed classes (not the variant) for the *rules* means
matching is family-homogeneous and fast.

.. code-block:: cpp

   #include <string>
   #include <vector>
   #include <variant>
   #include <polycpp/ipaddr/ipaddr.hpp>

   using namespace polycpp::ipaddr;

   struct AllowList {
       std::vector<std::pair<IPv4, int>> v4;
       std::vector<std::pair<IPv6, int>> v6;
   };

Step 2 — parse the configuration
--------------------------------

Build the list from an array of CIDR strings. ``parseCIDR`` on the
wrong class throws, so detect the family with ``isIPv4`` /
``isIPv6`` first — both are pure string checks, not full parses.

.. code-block:: cpp

   AllowList compile(const std::vector<std::string>& cidrs) {
       AllowList out;
       for (const auto& cidr : cidrs) {
           // parseCIDR needs the prefix; strip it before the family check.
           auto slash = cidr.find('/');
           std::string host = slash == std::string::npos
               ? cidr : cidr.substr(0, slash);
           if (IPv6::isIPv6(host))
               out.v6.push_back(IPv6::parseCIDR(cidr));
           else
               out.v4.push_back(IPv4::parseCIDR(cidr));
       }
       return out;
   }

Step 3 — implement the predicate
--------------------------------

Parse the caller-supplied address, dispatch on the variant, and scan
the appropriate bucket.

.. code-block:: cpp

   bool is_allowed(const AllowList& list, const std::string& addr) {
       if (!isValid(addr)) return false;
       auto v = parse(addr);
       return std::visit([&](const auto& a) -> bool {
           using T = std::decay_t<decltype(a)>;
           if constexpr (std::is_same_v<T, IPv4>) {
               for (const auto& rule : list.v4)
                   if (a.match(rule)) return true;
           } else {
               for (const auto& rule : list.v6)
                   if (a.match(rule)) return true;
           }
           return false;
       }, v);
   }

Each :cpp:func:`IPv4::match` / :cpp:func:`IPv6::match` call takes
the ``(network, bits)`` pair directly — exactly what ``parseCIDR``
returns — so there's no glue between the parsing and matching
stages.

Step 4 — smoke test
-------------------

.. code-block:: cpp

   int main() {
       auto list = compile({
           "10.0.0.0/8",
           "192.168.0.0/16",
           "2001:db8::/32",
       });
       for (const auto& addr : {
                "10.1.2.3", "172.16.0.1", "2001:db8::42", "2001::1"}) {
           std::cout << addr << ": "
                     << (is_allowed(list, addr) ? "allow" : "deny")
                     << '\n';
       }
   }

Expected output:

.. code-block:: text

   10.1.2.3: allow
   172.16.0.1: deny
   2001:db8::42: allow
   2001::1: deny

Step 5 — performance note
-------------------------

:cpp:func:`IPv4::match` does a constant-time bitmask comparison,
so a linear scan over O(hundreds) of rules is fine. For tens of
thousands of rules, sort the list by prefix length descending and
add an early-exit on the first match — or reach for a radix tree.

What you learned
----------------

- Keep IPv4 and IPv6 rules in separate buckets so matching is
  family-homogeneous.
- :cpp:func:`isIPv4` / :cpp:func:`isIPv6` are **format sniffers**,
  not full validators — perfect for family detection.
- :cpp:func:`parse` gives you a variant; ``std::visit`` dispatches
  cleanly to the family-specific matcher.
