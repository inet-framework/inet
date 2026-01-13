//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "PtlsQuicTls.h"
#include "../packet/VariableLengthInteger.h"

#include <iostream>
#include <cassert>

namespace inet {
namespace quic {

// Constants for packet type detection
#define QUICLY_LONG_HEADER_BIT 0x80
#define QUICLY_QUIC_BIT 0x40
#define QUICLY_PACKET_TYPE_INITIAL (QUICLY_LONG_HEADER_BIT | QUICLY_QUIC_BIT | 0)
#define QUICLY_PACKET_TYPE_0RTT (QUICLY_LONG_HEADER_BIT | QUICLY_QUIC_BIT | 0x10)
#define QUICLY_PACKET_TYPE_HANDSHAKE (QUICLY_LONG_HEADER_BIT | QUICLY_QUIC_BIT | 0x20)
#define QUICLY_PACKET_TYPE_RETRY (QUICLY_LONG_HEADER_BIT | QUICLY_QUIC_BIT | 0x30)
#define QUICLY_PACKET_TYPE_BITMASK 0xf0
#define QUICLY_PACKET_IS_LONG_HEADER(first_byte) (((first_byte) & QUICLY_LONG_HEADER_BIT) != 0)

// Header form constants
#define PACKET_HEADER_FORM_LONG 1
#define PACKET_HEADER_FORM_SHORT 0

PtlsQuicTls::PtlsQuicTls(const EncryptionKey& key, IQuicTlsCallbacks *callbacks)
    : key(key), callbacks(callbacks)
{
}

PtlsQuicTls::~PtlsQuicTls()
{
}

bool PtlsQuicTls::isLongHeader(uint8_t firstByte)
{
    return QUICLY_PACKET_IS_LONG_HEADER(firstByte);
}

size_t PtlsQuicTls::getPnLength(uint8_t firstByte)
{
    return (firstByte & 0x3) + 1;
}

size_t PtlsQuicTls::determinePnOffset(const uint8_t *data, size_t len)
{
    assert(len >= 5 && "Invalid or unsupported type of QUIC packet.");

    size_t pn_offs = 1; // first byte

    if (QUICLY_PACKET_IS_LONG_HEADER(data[0])) {
        // Long header
        pn_offs += 4; // version
        size_t dst_len = data[pn_offs++]; // dst cid len
        pn_offs += dst_len;
        size_t src_len = data[pn_offs++]; // src cid len
        pn_offs += src_len;

        if ((data[0] & QUICLY_PACKET_TYPE_BITMASK) == QUICLY_PACKET_TYPE_RETRY) {
            // Retry packet - all the rest is the retry token, no packet number
            // Return a sentinel value to indicate no packet number
            return len;
        }
        else {
            // Coalescible long header packet
            if ((data[0] & QUICLY_PACKET_TYPE_BITMASK) == QUICLY_PACKET_TYPE_INITIAL) {
                // Initial has a token
                size_t token_len_len;
                size_t token_len = decodeVariableLengthInteger(data + pn_offs, data + len, &token_len_len);
                pn_offs += token_len_len;
                pn_offs += token_len;
            }

            size_t packet_len_len;
            decodeVariableLengthInteger(data + pn_offs, data + len, &packet_len_len);
            pn_offs += packet_len_len;
        }
    }
    else {
        // Short header
        size_t local_cidl = 8; // TODO: This should be configurable
        pn_offs += local_cidl;
    }

    return pn_offs;
}

size_t PtlsQuicTls::determinePnOffset(const std::vector<uint8_t>& packet)
{
    return determinePnOffset(packet.data(), packet.size());
}

bool PtlsQuicTls::protectPacket(std::vector<uint8_t>& packet, size_t pnOffset, uint64_t pn)
{
    size_t packetNumberLength = getPnLength(packet[0]);

    if (packet.size() < pnOffset + packetNumberLength) {
        return false;
    }

    ptls_cipher_suite_t *cs = &ptls_openssl_opp_aes128gcmsha256;
    size_t originalSize = packet.size();

    std::cout << "PtlsQuicTls::protectPacket: original size: " << originalSize << std::endl;
    std::cout << "PtlsQuicTls::protectPacket: packet number: " << pn << std::endl;
    std::cout << "PtlsQuicTls::protectPacket: packet number offset: " << pnOffset << std::endl;
    std::cout << "PtlsQuicTls::protectPacket: packet number length: " << packetNumberLength << std::endl;

    // Ensure enough space for the auth tag (16 bytes for AES-GCM)
    packet.resize(originalSize + 16);

    // Create AEAD context for encryption
    ptls_aead_context_t *packet_protect = ptls_aead_new_direct(
        cs->aead, true, key.key.data(), key.iv.data());

    if (!packet_protect) {
        packet.resize(originalSize);
        return false;
    }

    size_t payload_from = pnOffset + packetNumberLength;

    // Encrypt the payload
    // Note: We don't apply header protection here - that's done separately in protectHeader()
    ptls_aead_encrypt_s(packet_protect,
                        packet.data() + payload_from,
                        packet.data() + payload_from,
                        originalSize - payload_from,
                        pn,
                        packet.data(),
                        payload_from,
                        nullptr);

    ptls_aead_free(packet_protect);

    std::cout << "PtlsQuicTls::protectPacket: protected size: " << packet.size() << std::endl;
    return true;
}

void PtlsQuicTls::protectHeader(std::vector<uint8_t>& packet, size_t pnOffset)
{
    size_t packetNumberLength = getPnLength(packet[0]);

    ptls_cipher_suite_t *cs = &ptls_openssl_opp_aes128gcmsha256;

    // Create header protection cipher
    ptls_cipher_context_t *header_protect = ptls_cipher_new(
        cs->aead->ctr_cipher, true, key.hpkey.data());

    if (!header_protect) {
        return;
    }

    // Generate the header protection mask
    // Sample starts 4 bytes after the packet number
    uint8_t hpmask[5] = {0};
    ptls_cipher_init(header_protect, packet.data() + pnOffset + 4);
    ptls_cipher_encrypt(header_protect, hpmask, hpmask, sizeof(hpmask));
    ptls_cipher_free(header_protect);

    // Apply mask to the first byte
    uint8_t headerForm = (packet[0] >> 7) & 0x01;
    // Mask is applied to the low 4 bits in long headers, and low 5 bits in short headers
    packet[0] ^= hpmask[0] & ((headerForm == PACKET_HEADER_FORM_LONG) ? 0x0f : 0x1f);

    // Apply mask to the packet number bytes
    for (size_t i = 0; i < packetNumberLength; ++i) {
        packet[pnOffset + i] ^= hpmask[i + 1];
    }
}

void PtlsQuicTls::unprotectHeader(std::vector<uint8_t>& packet, size_t pnOffset)
{
    ptls_cipher_suite_t *cs = &ptls_openssl_opp_aes128gcmsha256;

    // Create header protection cipher for decryption
    // Note: AES-CTR uses the same operation for encrypt/decrypt
    ptls_cipher_context_t *header_protect = ptls_cipher_new(
        cs->aead->ctr_cipher, true, key.hpkey.data());

    if (!header_protect) {
        return;
    }

    // Generate the header protection mask
    // Sample starts 4 bytes after the packet number
    uint8_t hpmask[5] = {0};
    ptls_cipher_init(header_protect, packet.data() + pnOffset + 4);
    ptls_cipher_encrypt(header_protect, hpmask, hpmask, sizeof(hpmask));
    ptls_cipher_free(header_protect);

    // Remove mask from the first byte to reveal the packet number length
    packet[0] ^= hpmask[0] & (QUICLY_PACKET_IS_LONG_HEADER(packet[0]) ? 0x0f : 0x1f);

    // Now we can determine the actual packet number length
    size_t packetNumberLength = getPnLength(packet[0]);

    // Remove mask from the packet number bytes
    for (size_t i = 0; i < packetNumberLength; ++i) {
        packet[pnOffset + i] ^= hpmask[i + 1];
    }
}

bool PtlsQuicTls::unprotectPacket(std::vector<uint8_t>& packet, size_t pnOffset, uint64_t pn)
{
    size_t packetNumberLength = getPnLength(packet[0]);

    if (packet.size() < pnOffset + packetNumberLength + 16) {
        // Must have at least header + auth tag
        return false;
    }

    ptls_cipher_suite_t *cs = &ptls_openssl_opp_aes128gcmsha256;
    size_t aead_off = pnOffset + packetNumberLength;

    std::cout << "PtlsQuicTls::unprotectPacket: protected size: " << packet.size() << std::endl;
    std::cout << "PtlsQuicTls::unprotectPacket: packet number: " << pn << std::endl;
    std::cout << "PtlsQuicTls::unprotectPacket: packet number offset: " << pnOffset << std::endl;
    std::cout << "PtlsQuicTls::unprotectPacket: packet number length: " << packetNumberLength << std::endl;

    // Create AEAD context for decryption
    ptls_aead_context_t *packet_protect = ptls_aead_new_direct(
        cs->aead, false, key.key.data(), key.iv.data());

    if (!packet_protect) {
        return false;
    }

    // Decrypt the payload and verify authentication tag
    size_t decrypted_len = ptls_aead_decrypt(
        packet_protect,
        packet.data() + aead_off,
        packet.data() + aead_off,
        packet.size() - aead_off,
        pn,
        packet.data(),
        aead_off);

    ptls_aead_free(packet_protect);

    if (decrypted_len == SIZE_MAX) {
        // Decryption failed (authentication error)
        return false;
    }

    // Resize to remove the auth tag
    packet.resize(aead_off + decrypted_len);

    std::cout << "PtlsQuicTls::unprotectPacket: unprotected size: " << packet.size() << std::endl;
    return true;
}

} // namespace quic
} // namespace inet
