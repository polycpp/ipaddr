IPv6
====

Eight-hextet RFC 2460 address with optional zone ID. Parses
standard and abbreviated colon-hex, plus transitional notation with
an embedded IPv4 tail (``::ffff:192.168.1.1``). Renders in four
forms: RFC 5952 compact (:cpp:func:`IPv6::toString`), no-pad
expanded (:cpp:func:`IPv6::toNormalizedString`), and fully zero-
padded (:cpp:func:`IPv6::toFixedLengthString`).

.. doxygenclass:: polycpp::ipaddr::IPv6
   :members:
   :undoc-members:
