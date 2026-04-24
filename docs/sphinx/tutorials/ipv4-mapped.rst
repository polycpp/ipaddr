Work with IPv4-mapped IPv6 addresses
====================================

**You'll build:** a request-logging helper that canonicalises the
remote address a dual-stack listener hands you — turning
``::ffff:10.0.0.5`` into ``10.0.0.5`` before your downstream code
ever sees it. This is the trip-wire behind countless "why doesn't my
IPv4 allow-list work on this dual-stack server?" bugs.

**You'll use:**
:cpp:func:`polycpp::ipaddr::process`,
:cpp:func:`polycpp::ipaddr::IPv6::isIPv4MappedAddress`,
:cpp:func:`polycpp::ipaddr::IPv6::toIPv4Address`,
:cpp:func:`polycpp::ipaddr::IPv4::toIPv4MappedAddress`.

**Prerequisites:** you've got a grip on the
``std::variant<IPv4, IPv6>`` that :cpp:func:`parse` returns.

Step 1 — the one-liner
----------------------

:cpp:func:`process` is a convenience wrapper: it parses the address
and, if the result is an IPv4-mapped IPv6, converts it to a plain
``IPv4`` automatically. For the common case, you do nothing else.

.. code-block:: cpp

   #include <polycpp/ipaddr/ipaddr.hpp>
   using namespace polycpp::ipaddr;

   auto v = process("::ffff:10.0.0.5");
   // std::holds_alternative<IPv4>(v) == true
   // std::get<IPv4>(v).toString() == "10.0.0.5"

   auto v2 = process("2001:db8::1");
   // std::holds_alternative<IPv6>(v2) == true   (not IPv4-mapped; preserved)

   auto v3 = process("192.168.1.1");
   // Plain IPv4; preserved as-is.

Step 2 — detect without converting
----------------------------------

If you want to keep the IPv6 typed form but know whether it came
from a v4 source (e.g. for logging), check the predicate directly:

.. code-block:: cpp

   auto v = IPv6::parse("::ffff:10.0.0.5");
   if (v.isIPv4MappedAddress()) {
       IPv4 native = v.toIPv4Address();
       log("client " + native.toString() + " (via v4-mapped v6)");
   }

:cpp:func:`IPv6::toIPv4Address` throws
``std::invalid_argument`` if the IPv6 is not actually v4-mapped, so
gate the call with
:cpp:func:`IPv6::isIPv4MappedAddress` — or use
:cpp:func:`IPv6::range` and compare to ``"ipv4Mapped"``.

Step 3 — go the other direction
-------------------------------

To produce the v4-mapped form for a socket API that wants IPv6 only,
call :cpp:func:`IPv4::toIPv4MappedAddress`:

.. code-block:: cpp

   auto v4 = IPv4::parse("10.0.0.5");
   auto v6 = v4.toIPv4MappedAddress();
   // v6.toString() == "::ffff:10.0.0.5"

Step 4 — plug into a logging pipeline
-------------------------------------

.. code-block:: cpp

   std::string canonical(const std::string& remote) {
       auto v = process(remote);
       return std::visit([](const auto& a) { return a.toString(); }, v);
   }

   // remote == "::ffff:10.0.0.5" on a dual-stack listener
   log_line("request from " + canonical(remote));
   // → "request from 10.0.0.5"

Step 5 — what *not* to do
-------------------------

Don't substring-match for ``"::ffff:"`` and strip the prefix
yourself. The IPv4-mapped form can legally be written in several
equivalent ways (``::ffff:0a00:0005``,
``::ffff:10.0.0.5``, and the fully expanded
``0000:0000:0000:0000:0000:ffff:0a00:0005``). Only the typed
conversion path handles them all.

What you learned
----------------

- :cpp:func:`process` is the single-call form: parse + auto-convert
  v4-mapped addresses to plain IPv4.
- :cpp:func:`IPv6::isIPv4MappedAddress` is the predicate;
  :cpp:func:`IPv6::toIPv4Address` does the conversion and
  throws on misuse.
- :cpp:func:`IPv4::toIPv4MappedAddress` goes the other direction
  when your socket layer wants IPv6-only.
