AODV advertisement lifetimes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

AODV now validates ``myRouteTimeout`` and the Hello duration
``allowedHelloLoss * helloInterval`` during initialization. After truncation to
whole milliseconds, each count must be in ``1..4294967295``. Durations below
one millisecond and durations above the representable range cause an error.
The defaults already satisfy these checks.

Replace zero or submillisecond placeholder values with valid durations. Use
``useHelloMessages=false`` to disable Hello transmission, while keeping its
duration parameters valid. Other AODV timing constraints still apply.
