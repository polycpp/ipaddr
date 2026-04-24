Allow-list gateway
==================

Checks a single client address against a compiled allow-list of
CIDR blocks. The ruleset is baked into the example — in a real
service it would come from config.

.. literalinclude:: ../../../examples/allow_check.cpp
   :language: cpp
   :linenos:

Build and run:

.. code-block:: bash

   cmake -B build -G Ninja -DPOLYCPP_IPADDR_BUILD_EXAMPLES=ON
   cmake --build build --target allow_check
   ./build/examples/allow_check 10.1.2.3
   ./build/examples/allow_check 8.8.8.8
