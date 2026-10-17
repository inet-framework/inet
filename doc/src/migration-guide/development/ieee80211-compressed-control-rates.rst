IEEE 802.11 Compressed control rates
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Custom QoS rate selectors must implement ``computeCompressedBlockAckReqMode(packet, header, startsTxop)``. This operation prepares legal non-HT BAR rate selection from the supplied TXOP position and applicable BSS or peer rates. HCF retains its Compressed-profile rejection in this prerequisite. Runtime activation will supply the request's actual TXOP position. Existing Basic methods retain their contracts.
