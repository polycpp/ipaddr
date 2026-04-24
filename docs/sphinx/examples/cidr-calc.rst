CIDR calculator
===============

Given a CIDR string, prints the network address, broadcast address,
and subnet mask. Handles both IPv4 and IPv6.

.. literalinclude:: ../../../examples/cidr_calc.cpp
   :language: cpp
   :linenos:

Build and run:

.. code-block:: bash

   cmake -B build -G Ninja -DPOLYCPP_IPADDR_BUILD_EXAMPLES=ON
   cmake --build build --target cidr_calc
   ./build/examples/cidr_calc 192.168.1.50/24
   ./build/examples/cidr_calc 2001:db8::1/64
