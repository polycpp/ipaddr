Module-level functions
======================

Family-agnostic entry points that dispatch between
:cpp:class:`polycpp::ipaddr::IPv4` and
:cpp:class:`polycpp::ipaddr::IPv6` based on the input string. The
return type is a ``std::variant<IPv4, IPv6>``; use ``std::visit``
or ``std::holds_alternative`` / ``std::get`` to branch.

.. doxygenfunction:: polycpp::ipaddr::parse
.. doxygenfunction:: polycpp::ipaddr::isValid
.. doxygenfunction:: polycpp::ipaddr::isValidCIDR
.. doxygenfunction:: polycpp::ipaddr::parseCIDR
.. doxygenfunction:: polycpp::ipaddr::fromByteArray
.. doxygenfunction:: polycpp::ipaddr::process
.. doxygenfunction:: polycpp::ipaddr::subnetMatch

CIDRRange
---------

.. doxygentypedef:: polycpp::ipaddr::CIDRRange
