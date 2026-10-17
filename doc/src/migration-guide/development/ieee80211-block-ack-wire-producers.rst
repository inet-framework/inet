IEEE 802.11 Block Ack wire producers
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The serializer decodes the complete four-bit Type field. Type 0 retains Basic compatibility; Type 2 selects Compressed. Other Types produce invalid input. Basic producers must keep the two low bits of their reserved field zero because those bits overlap Type. The serializer rejects inconsistent Basic objects. Compressed reserved fields write zero; received reserved values do not affect the decoded result.

Compressed producers must supply Fragment Number zero and a 64-bit bitmap. A Compressed Block Ack header chunk contains 28 bytes before FCS. Both BAR profiles occupy 24 bytes with FCS. Keep the corresponding header chunk and trailer lengths consistent. The serializer rejects malformed wire fields before runtime processing.
