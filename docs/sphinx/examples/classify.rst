Classify addresses by RFC range
===============================

Reads one IP address per line on stdin and prints the detected RFC
range — ``loopback``, ``private``, ``multicast``, ``reserved``,
``ipv4Mapped`` and so on.

.. literalinclude:: ../../../examples/classify.cpp
   :language: cpp
   :linenos:

Build and run:

.. code-block:: bash

   cmake -B build -G Ninja -DPOLYCPP_IPADDR_BUILD_EXAMPLES=ON
   cmake --build build --target classify
   printf '127.0.0.1\n10.0.0.1\n2001:db8::1\n' | ./build/examples/classify
