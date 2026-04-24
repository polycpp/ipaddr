Quickstart
==========

This page walks through a minimal ipaddr program end-to-end. Copy the
snippet, run it, then jump to :doc:`../tutorials/index` for task-oriented
walkthroughs or :doc:`../api/index` for the full reference.

The program below takes a handful of addresses — IPv4, IPv6, and an
IPv4-mapped IPv6 — and prints, for each one: its normalised string
form, its RFC range (``private``, ``loopback``, ``multicast``, …),
and whether it falls inside the ``10.0.0.0/8`` private block. It
exercises :cpp:func:`polycpp::ipaddr::parse` (the variant version),
:cpp:func:`polycpp::ipaddr::IPv4::match`, and the range classifier
on both v4 and v6.

Full example
------------

.. code-block:: cpp

   #include <iostream>
   #include <string>
   #include <variant>
   #include <vector>

   #include <polycpp/ipaddr/ipaddr.hpp>
   using namespace polycpp::ipaddr;

   static std::string canonical(const std::variant<IPv4, IPv6>& v) {
       return std::visit([](const auto& a) { return a.toString(); }, v);
   }

   static std::string rangeOf(const std::variant<IPv4, IPv6>& v) {
       return std::visit([](const auto& a) { return a.range(); }, v);
   }

   int main() {
       const auto [privNet, privBits] = IPv4::parseCIDR("10.0.0.0/8");

       const std::vector<std::string> inputs = {
           "127.0.0.1",
           "10.2.3.4",
           "192.168.1.1",
           "2001:db8::1",
           "::ffff:10.0.0.1",     // IPv4-mapped IPv6
       };

       for (const auto& text : inputs) {
           if (!isValid(text)) {
               std::cout << text << " -> invalid\n";
               continue;
           }
           auto value = parse(text);
           std::string note;
           if (auto* v4 = std::get_if<IPv4>(&value)) {
               note = v4->match(privNet, privBits) ? "  [10/8 match]" : "";
           }
           std::cout << text << " -> " << canonical(value)
                     << "  (" << rangeOf(value) << ")" << note << '\n';
       }
       return 0;
   }

Compile it with the same CMake wiring from :doc:`installation`:

.. code-block:: bash

   cmake -B build -G Ninja
   cmake --build build
   ./build/my_app

Expected output:

.. code-block:: text

   127.0.0.1 -> 127.0.0.1  (loopback)
   10.2.3.4 -> 10.2.3.4  (private)  [10/8 match]
   192.168.1.1 -> 192.168.1.1  (private)
   2001:db8::1 -> 2001:db8::1  (reserved)
   ::ffff:10.0.0.1 -> ::ffff:10.0.0.1  (ipv4Mapped)

What just happened
------------------

:cpp:func:`polycpp::ipaddr::parse` returns a
``std::variant<IPv4, IPv6>`` — it tries IPv6 first (because
``::ffff:…`` and ``2001:…`` are unambiguous), then falls back to
IPv4. If neither recognises the string it throws
``std::invalid_argument``; use :cpp:func:`isValid` first when the
caller might feed bogus input.

:cpp:func:`IPv4::parseCIDR` and :cpp:func:`IPv6::parseCIDR` each
return a ``std::pair`` of address and prefix length — treat the
pair as a "network". :cpp:func:`IPv4::match` takes the pair (or
``(addr, bits)`` positionally) and answers "is my address in this
subnet?" with a bool.

:cpp:func:`IPv4::range` classifies the address against the RFC-
listed special ranges — ``"private"``, ``"loopback"``,
``"multicast"``, ``"linkLocal"``, ``"broadcast"``, and more. The
IPv6 variant does the same against 19 separate RFC ranges; an
IPv4-mapped address (``::ffff:…``) comes back as
``"ipv4Mapped"``, which is your cue to call
:cpp:func:`IPv6::toIPv4Address` if you want the underlying v4
address instead.

``toString`` on an IPv6 emits the **canonical RFC 5952 form** —
double-colon run of zeros, lowercase hex, no leading zeros. Use
:cpp:func:`IPv6::toNormalizedString` or
:cpp:func:`IPv6::toFixedLengthString` when you need the expanded
form (e.g., for exact string comparison or pretty-printing).

Next steps
----------

- :doc:`../tutorials/index` — step-by-step walkthroughs of common tasks.
- :doc:`../guides/index` — short how-tos for specific problems.
- :doc:`../api/index` — every public type, function, and option.
- :doc:`../examples/index` — runnable programs you can drop into a sandbox.
