Validate user-supplied addresses
================================

**When to reach for this:** you're taking an IP address from an
HTTP form, a config file, or a CLI flag and need to reject bad
input before it reaches logic that assumes a parsed address.

Use the non-throwing predicates — they cover both families and all
the notations you care about:

.. code-block:: cpp

   #include <polycpp/ipaddr/ipaddr.hpp>
   using namespace polycpp::ipaddr;

   if (!isValid(addr)) {
       // reject — not a valid IPv4 or IPv6 address
       return 400;
   }

If you need family-specific validation:

.. code-block:: cpp

   IPv4::isValid("192.168.1.1");              // true
   IPv4::isValidFourPartDecimal("192.168.1.1"); // true (no hex/octal)
   IPv6::isValid("2001:db8::1");              // true
   IPv6::isValid("::ffff:10.0.0.5");          // true — v4-mapped form

For CIDR strings (address + ``/bits``):

.. code-block:: cpp

   isValidCIDR("10.0.0.0/8");        // true, family-agnostic
   IPv4::isValidCIDR("10.0.0.0/8");  // true
   IPv4::isValidCIDR("::1/128");     // false — not IPv4
   IPv6::isValidCIDR("::1/128");     // true

:cpp:func:`IPv4::isIPv4` and :cpp:func:`IPv6::isIPv6` are **format
sniffers** that answer "does this look like the right family?" with
less rigour than ``isValid`` — useful for pre-dispatching to the
right parser, not for input validation.

Prefer ``isValid`` + ``parse`` over a ``try`` / ``catch`` around
``parse`` — it's clearer and slightly faster (no exception path).
