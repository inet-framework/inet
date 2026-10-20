IEEE 802.11 Beacon and Probe Response Fields
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``Ieee80211BeaconFrame::channelNumber`` (also inherited by Probe Response) now
represents the DSSS Parameter Set's standard Current Channel value. Its default,
``-1``, means that the element is absent. Custom frame producers that previously
stored a radio's internal index must use
``band->getStandardChannelNumber(channelIndex)`` and account for the element's
three bytes in the body chunk length. Consumers convert a present value back
with ``receivedBand->getChannelIndex(currentChannel)``. These mapping functions
throw for unmappable values. Radio configuration and scan-result channel fields
continue to use internal indices.

The built-in AP emits this element for its modeled 2.4 GHz operation and omits
it for other bands or radios without IEEE channel information. Without the
element, discovery uses the receive channel when available. HT discovery still
uses HT Operation as its primary-channel authority.

As a modeling simplification, legacy APs using custom bands without a standard
channel-number mapping also omit the DSSS element. Supply an explicit mapping
to advertise a standard channel. HT operation still requires a valid mapping;
invalid internal channel indices remain errors in all modes.

AP ``beaconInterval`` values are rounded down to whole 1024-us TUs once, during
initialization, and must be between 1 and 65535 TUs. The effective value drives
both target scheduling and advertised content. Use ``102400us`` for exactly
100 TUs; the default ``100ms`` now schedules targets 97 TUs apart. Actual beacon
transmissions can be delayed by channel access. Custom producers should put
the same effective interval in Beacon and Probe Response bodies as they use
for target scheduling.
The serializers require an interval between 1 and 65535 TUs for both frame
types and throw for out-of-range values, including the default zero interval.
Custom producers must set a valid interval before serialization.

``RC_MESH_PATH_ERROR_NO_FORWARDING_INFORMATION`` now has its standard value,
62. Code using the symbolic name needs only recompilation. Update external
numeric mappings that used 60 for this reason. Old stored value 60 cannot be
reinterpreted automatically: it also denoted invalid mesh security capability.
