Unknown transit in mesh path selection
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

External ``IMeshPathSelection`` providers must implement the new pure virtual ``reportUnknownTransit(transmitter, destination)`` method. The arguments identify the immediate established transmitter and the native final destination. No ``Packet *`` ownership is transferred: forwarding still drops its received packet.

The production forwarding owner calls this method after supported native DATA/Mesh Control, endpoint and transmitter checks, when an individual transit destination has no usable path. A known usable path with unauthorized precursor admission remains a silent drop. Group traffic, local final delivery, malformed or unsupported data and nonpeer inputs do not generate this notification. Unresolved source packets retain their separate source-discovery contract.

The production ``Hwmp`` response is a paced no-forwarding-information PERR to the immediate transmitter. A provider without a path-error procedure may implement an explicit no-op; it must not move transit packets into a source queue or invent ownership of the dropped packet.
