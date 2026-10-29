IEEE 802.11 HT-SIG representation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The modeled HT-SIG header preserves all 48 bits through byte conversion. ``lengthField`` supplies HT Length. ``aggregation`` distinguishes an A-MPDU from a standalone MPDU, including singleton aggregates. The radio derives mode fields from the selected mode and calculates the CRC after field assignment. Decode preserves the received CRC. HEADER validation rejects invalid CRC and unsupported fields before delimiter or MPDU outcomes.

Separate-part reception retains its current abort; whole-signal reception supplies a failed container. DATA separately refuses modeled content shorter than a valid HT Length. This refusal retains the accepted header property but supplies no range outcomes. It represents malformed model input, not physical carrier loss. Decapsulation never uses an invalid header or insufficient content for length-based trailer removal. Ordinary valid HT duration inputs remain unchanged.
