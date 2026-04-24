ipaddr
======

**IPv4 and IPv6 address manipulation**

Parse, validate, match, and classify IPv4 and IPv6 addresses. Port of the npm ``ipaddr.js`` package — all 24 IPv6 RFC ranges, CIDR parsing, IPv4-mapped address conversion, and named-range ``subnetMatch`` out of the box.

.. code-block:: cpp

   #include <polycpp/ipaddr/ipaddr.hpp>
   using namespace polycpp::ipaddr;

   auto addr = IPv4::parse("192.168.1.1");
   // addr.range()     == "private"
   // addr.toString()  == "192.168.1.1"

   auto [net, bits] = IPv4::parseCIDR("10.0.0.0/8");
   bool inside = addr.match(net, bits);   // false — addr is 192.168.x.x

   auto v6 = IPv6::parse("2001:db8::1");
   // v6.range()    == "reserved"
   // v6.toString() == "2001:db8::1"   (canonical RFC 5952 form)

.. grid:: 2

   .. grid-item-card:: Drop-in familiarity
      :margin: 1

      Mirrors the npm ``ipaddr.js`` API — ``parse``, ``isValid``, ``parseCIDR``, ``subnetMatch``, plus typed ``IPv4`` / ``IPv6`` classes with ``match``, ``range``, and canonical string conversion.

   .. grid-item-card:: C++20 native
      :margin: 1

      Header-only where possible, zero-overhead abstractions, ``constexpr``
      and ``std::string_view`` throughout.

   .. grid-item-card:: Tested
      :margin: 1

      65 GoogleTest cases port the ipaddr.js JS suite: dotted-decimal, hex, octal, IPv4-mapped transitional syntax, CIDR parsing, every RFC-listed range, and round-trip normalisation.

   .. grid-item-card:: Plays well with polycpp
      :margin: 1

      Uses the same JSON value, error, and typed-event types as the rest of
      the polycpp ecosystem — no impedance mismatch.

Getting started
---------------

.. code-block:: bash

   # With FetchContent (recommended)
   FetchContent_Declare(
       polycpp_ipaddr
       GIT_REPOSITORY https://github.com/polycpp/ipaddr.git
       GIT_TAG        master
   )
   FetchContent_MakeAvailable(polycpp_ipaddr)
   target_link_libraries(my_app PRIVATE polycpp::ipaddr)

:doc:`Installation <getting-started/installation>` · :doc:`Quickstart <getting-started/quickstart>` · :doc:`Tutorials <tutorials/index>` · :doc:`API reference <api/index>`

.. toctree::
   :hidden:
   :caption: Getting started

   getting-started/installation
   getting-started/quickstart

.. toctree::
   :hidden:
   :caption: Tutorials

   tutorials/index

.. toctree::
   :hidden:
   :caption: How-to guides

   guides/index

.. toctree::
   :hidden:
   :caption: API reference

   api/index

.. toctree::
   :hidden:
   :caption: Examples

   examples/index
