#!/usr/bin/env python3
"""Generate bounded M1 wire-vector captures without importing the INET model."""

import hashlib
from pathlib import Path
import struct


def fcs(data):
    """Direct polynomial division over LSB-first input bits (IEEE 802.15.4-2024, 7.2.11)."""
    bits = [((octet >> bit) & 1) for octet in data for bit in range(8)]
    dividend = 0
    for bit in bits:
        dividend = (dividend << 1) | bit
    dividend <<= 16
    polynomial = 0x11021
    while dividend.bit_length() >= polynomial.bit_length():
        dividend ^= polynomial << (dividend.bit_length() - polynomial.bit_length())
    return int(f"{dividend:016b}"[::-1], 2)


def main():
    assert fcs(b"123456789") == 0x2189
    ack = bytes.fromhex("02006a")
    assert ack + struct.pack("<H", fcs(ack)) == bytes.fromhex("02006ae479")
    # 7.2 legacy MHR: version 1, AR, same-PAN extended addressing, DSN 7, opaque MSDU.
    data = bytes.fromhex("61dc073412bc9a785634120104bc9a785634120102deadbeef")
    output = Path(__file__).resolve().parents[1] / "pcap" / "generated"
    output.mkdir(exist_ok=True)
    for has_fcs, link_type in [(True, 195), (False, 230)]:
        result = bytearray(struct.pack("<IHHIIII", 0xa1b2c3d4, 2, 4, 0, 0, 65535, link_type))
        for index, frame in enumerate([ack, data]):
            wire = frame + struct.pack("<H", fcs(frame)) if has_fcs else frame
            result += struct.pack("<IIII", 1, index * 1000, len(wire), len(wire)) + wire
        path = output / ("ieee802154-m1-with-fcs.pcap" if has_fcs else "ieee802154-m1-no-fcs.pcap")
        path.write_bytes(result)
        print(path.name, hashlib.sha256(result).hexdigest())


if __name__ == "__main__":
    main()
