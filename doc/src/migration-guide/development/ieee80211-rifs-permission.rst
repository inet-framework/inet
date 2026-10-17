IEEE 802.11 RIFS permission
~~~~~~~~~~~~~~~~~~~~~~~~~~~

HT Operation now preserves RIFS Mode through management conversion and equality checks. VHT operation clears that permission. The change preserves the received permission for a consumer that checks RIFS relations; it adds no RIFS transmission scheduler.

Rx reads the local MIB through ``mibModule``. The existing MAC parameter pattern supplies this path. ``recipientAckPolicyModule`` selects the typed recipient ACK policy. Its default selects the HCF policy for QoS operation. An empty path disables RIFS evidence and requires no HCF module.

Rx freezes BSS permission at reception start. An infrastructure station without available operation supplies no RIFS permission. A custom radio must supply actual extension through ``Ieee80211ModeInd`` for a received predecessor to support RIFS. Missing extension evidence does not disable SIFS detection.
