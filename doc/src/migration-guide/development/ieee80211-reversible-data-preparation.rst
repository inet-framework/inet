IEEE 802.11 reversible DATA preparation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Custom QoS data services must implement the reversible preparation operations in ``IOriginatorQosMacDataService``. ``prepareFrame()`` creates an owned preview and retains original MSDUs in their queue. ``isPreparationValid()`` checks current source identity, eligibility, lifetime, sequence state, and aggregation permission. ``commitPreparedFrame()`` transfers the same preview pointer only after successful validation. Failure must preserve the preview and all original queue frames.
