Local source demand in mesh path selection
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

External ``IMeshPathSelection`` providers must implement the new pure virtual ``sourcePathNeeded(destination)`` method. Native forwarding invokes it for valid individual source traffic before it looks up a path, drains older pending packets or encapsulates the current packet. ``findPath()`` remains a pure query.

The call transfers no packet ownership. The current packet remains owned by the forwarding source entry while a provider may synchronously establish readiness and call ``pathAvailable(destination)`` to drain older queued packets. Requery the path after returning, and retain no pending-container iterator across the call.

The production ``Hwmp`` provider uses this notification for a required flag-zero proactive PREP, even when a usable lookup already exists. A provider without proactive roots may implement a documented no-op. Demand that precedes an eligible proactive invitation is detected through forwarding's existing expiry-aware pending-source query; providers do not need a second packet queue or application-demand history.
