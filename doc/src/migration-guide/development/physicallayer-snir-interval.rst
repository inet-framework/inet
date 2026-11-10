The SNIR of a Time Interval
~~~~~~~~~~~~~~~~~~~~~~~~~~~

:cpp:`ISnir` has three new pure virtual methods. They return the SNIR of a
time interval inside the reception:

.. code-block:: c++

   virtual double getMin(simtime_t startTime, simtime_t endTime) const = 0;
   virtual double getMax(simtime_t startTime, simtime_t endTime) const = 0;
   virtual double getMean(simtime_t startTime, simtime_t endTime) const = 0;

An :cpp:`ISnir` implementation outside INET no longer compiles until it
overrides these methods. If the start and the end of the interval are equal,
the methods return the SNIR at that time. The methods without arguments keep
their meaning: they return the values of the whole reception. In
:cpp:`ScalarSnir` and :cpp:`DimensionalSnir`, the protected functions
:cpp:`computeMin()`, :cpp:`computeMax()`, and :cpp:`computeMean()` now take
the start and the end of the interval.

:cpp:`SnirReceiverBase` has a new overload of
:cpp:`computeIsReceptionSuccessful()` with a time interval. It checks the SNIR
threshold for that interval, and the method without the interval calls it with
the limits of the reception. A receiver that changes the threshold check
overrides the overload, so that the check of a part of the signal follows the
change too. A class that overrides only the method without the interval hides
the overload; such a class needs a ``using`` declaration to call it.
