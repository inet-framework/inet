IEEE 802.11 HT A-MPDU PHY reception
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Error models use the generic ``IErrorModel::computeReceivedData()`` operation for partial DATA reception. The caller supplies bounded chunk content. The model returns content of the same length with incorrect chunks marked. The operation permits an entirely correct result. It does not parse delimiters or identify MPDUs.

Custom models that derive from ``ErrorModelBase`` inherit this operation. Its default calculation uses DATA BER and rejects an unavailable BER. ``Ieee80211ErrorModelBase`` overrides the chunk probability calculation through its existing DATA success function. NIST and Yans need no separate aggregation interface. Direct implementations of ``IErrorModel`` must implement the new operation.

The receiver checks PHY acquisition, header validity, and available PSDU length before it requests DATA corruption. Header failure prevents DATA draws. A valid header whose Length exceeds the modeled PSDU causes structural refusal. This check establishes bounds safety, with no claim about physical carrier loss.

The DATA decision retains the received chunks. WHOLE and result queries reuse those chunks without more random draws. The receiver replaces only PSDU content in its local packet. The transmitted chunks remain unchanged. Region tags retain their positions.

``Ieee80211AmpduInd`` contains header success, aggregation identity, and content completeness. Error outcomes reside in chunk correctness flags. The indication contains no parallel range table. The explicit packet bit-error flag denotes failed prerequisites or missing content. ``Packet::hasBitError()`` also reports incorrect chunks, so a partial DATA error makes that query true.

For example, a three-member aggregate contains an incorrect payload chunk in its middle MPDU. Extraction preserves that chunk, so the middle MPDU reports incorrect content. The other MPDUs can remain correct. The default ``Rx`` still rejects all aggregate containers before standalone header or FCS checks. Member acceptance, upper delivery, and implicit Block Ack remain separate work.

Chunk structure controls error granularity. Typed input can preserve separate delimiters, headers, payloads, and trailers. ``sendRawBytes`` converts the transmission to one byte chunk. Its DATA view therefore receives one outcome. The receiver does not reconstruct hidden member boundaries for the error model.

HT-SIG uses all 48 bits for Length, Aggregation, coding fields, and CRC. The receiver validates the received CRC before DATA corruption. Ordinary reception retains its existing whole-packet path.

With a present model, aggregate ``ErrorRateInd`` fields contain ``NaN`` for unknown probabilities. An absent model preserves the common metadata path and performs no error draws. BerTable retains its current HT configuration limit.

The tested profile covers HT20 mixed format, either guard interval, and scalar or dimensional radios. Each chunk uses an independent draw for a probability strictly between zero and one. Padding chunks also receive outcomes. The model does not represent correlated FEC errors. It establishes no new error-accuracy or goodput claim.
