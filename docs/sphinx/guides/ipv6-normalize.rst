Choose the right IPv6 string form
=================================

**When to reach for this:** two IPv6 strings compare unequal in your
code but are actually the same address — ``"2001:0db8::0001"`` vs.
``"2001:db8::1"`` — and you need to agree on one canonical form.

The library offers four string outputs, each deterministic:

.. code-block:: cpp

   #include <polycpp/ipaddr/ipaddr.hpp>
   using namespace polycpp::ipaddr;

   auto v = IPv6::parse("2001:0db8:0000:0000:0000:0000:0000:0001");

   v.toString();             // "2001:db8::1"          (RFC 5952 compact)
   v.toRFC5952String();      //  same as toString
   v.toNormalizedString();   // "2001:db8:0:0:0:0:0:1" (no ::, no zero pad)
   v.toFixedLengthString();  // "2001:0db8:0000:0000:0000:0000:0000:0001"

**When to pick which:**

- :cpp:func:`IPv6::toString` — the usual choice. Compact, lowercase
  hex, ``::`` used for the longest zero run. Safe for log lines,
  URLs (inside brackets), and config files.
- :cpp:func:`IPv6::toNormalizedString` — strip zero padding but
  keep every group visible; good for tabular output where column
  alignment matters.
- :cpp:func:`IPv6::toFixedLengthString` — every group padded to
  four hex digits; use this for lexicographic sort ordering or
  byte-exact comparison.
- :cpp:func:`IPv6::toRFC5952String` — explicit opt-in for the same
  rules as ``toString``; use when you need to spell out that you
  depend on the RFC 5952 rules.

For equality, don't compare strings — compare :cpp:class:`IPv6`
instances directly:

.. code-block:: cpp

   if (IPv6::parse(a) == IPv6::parse(b)) { /* same address */ }

The library compares by byte-level contents, so all the string
variants collapse to the same equivalence class.
