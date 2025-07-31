//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef INET_APPLICATIONS_QUIC_QUICPACKET_H_
#define INET_APPLICATIONS_QUIC_QUICPACKET_H_

#include <vector>

#include "PacketHeader_m.h"
#include "QuicFrame.h"
#include "inet/common/packet/Packet.h"
#include "EncryptionKeyTag_m.h"

extern "C" {
#include "picotls.h"
#include "picotls/openssl_opp.h"
}

namespace inet {
namespace quic {

struct EncryptionKey {
    std::vector<uint8_t> key;
    std::vector<uint8_t> iv;
    std::vector<uint8_t> hpkey;

    static EncryptionKey newInitial(ptls_iovec_t initial_random, const char *hkdf_label) {
        ptls_hash_algorithm_t *hash = &ptls_openssl_opp_sha256;

        static const uint8_t quic_v1_salt[] = {
            0x38, 0x76, 0x2c, 0xf7, 0xf5, 0x59, 0x34, 0xb3,
            0x4d, 0x17, 0x9a, 0xe6, 0xa4, 0xc8, 0x0c, 0xad,
            0xcc, 0xbb, 0x7f, 0x0a
        };

        uint8_t initial_secret[32];
        ptls_hkdf_extract(hash, initial_secret, ptls_iovec_init(quic_v1_salt, sizeof(quic_v1_salt)), initial_random);

        ptls_iovec_t null_iovec = ptls_iovec_init(NULL, 0);

        uint8_t secret[32]; // "client_secret"/"server_secret"
        ptls_hkdf_expand_label(hash, secret, 32, ptls_iovec_init(initial_secret, 32), hkdf_label, null_iovec, NULL);
        ptls_iovec_t secret_iovec = ptls_iovec_init(secret, 32);

        std::vector<uint8_t> key(16);
        ptls_hkdf_expand_label(hash, key.data(), key.size(), secret_iovec, "quic key", null_iovec, NULL);

        std::vector<uint8_t> iv(12);
        ptls_hkdf_expand_label(hash, iv.data(), iv.size(), secret_iovec, "quic iv", null_iovec, NULL);

        std::vector<uint8_t> hpkey(16);
        ptls_hkdf_expand_label(hash, hpkey.data(), hpkey.size(), secret_iovec, "quic hp", null_iovec, NULL);

        return {key, iv, hpkey};
    }

    static std::vector<uint8_t> hex2bytes(const std::string& hex) {
        std::vector<uint8_t> bytes;
        bytes.reserve(hex.size() / 2);
        for (size_t i = 0; i < hex.size(); i += 2) {
            std::string byteString = hex.substr(i, 2);
            uint8_t byte = (uint8_t)strtol(byteString.c_str(), nullptr, 16);
            bytes.push_back(byte);
        }
        return bytes;
    }

    static std::string bytes2hex(const std::vector<uint8_t>& bytes) {
        std::string hex;
        hex.reserve(bytes.size() * 2);
        char buffer[3];
        for (uint8_t byte : bytes) {
            snprintf(buffer, sizeof(buffer), "%02x", byte);
            hex.append(buffer);
        }
        return hex;
    }

    static EncryptionKey fromTag(Ptr<const EncryptionKeyTag> tag) {
        EncryptionKey key;
        key.key = hex2bytes(tag->getKey());
        key.iv = hex2bytes(tag->getIv());
        key.hpkey = hex2bytes(tag->getHpkey());
        ASSERT(!key.key.empty());
        ASSERT(!key.iv.empty());
        ASSERT(!key.hpkey.empty());
        return key;
    }

    Ptr<const EncryptionKeyTag> toTag() const {
        auto tag = makeShared<EncryptionKeyTag>();
        tag->setKey(bytes2hex(key).c_str());
        tag->setIv(bytes2hex(iv).c_str());
        tag->setHpkey(bytes2hex(hpkey).c_str());
        return tag;
    }

    void dump() {
        std::cout << "key: " << bytes2hex(key) << std::endl;
        std::cout << "iv: " << bytes2hex(iv) << std::endl;
        std::cout << "hpkey: " << bytes2hex(hpkey) << std::endl;
    }
};

enum PacketNumberSpace {
    Initial,
    Handshake,
    ApplicationData
};

class QuicPacket {
public:
    QuicPacket(std::string name);
    virtual ~QuicPacket();

    uint64_t getPacketNumber();
    bool isCryptoPacket();

    void setHeader(Ptr<PacketHeader> header);
    void addFrame(QuicFrame *frame);
    Packet *createOmnetPacket(const EncryptionKey& key);
    virtual void onPacketLost();
    virtual void onPacketAcked();

    virtual void setIBit(bool iBit);
    virtual bool isDplpmtudProbePacket();

    virtual bool containsFrame(QuicFrame *otherFrame);
    virtual int getMemorySize();

    bool countsAsInFlight() {
        return countsInFlight;
    }
    omnetpp::simtime_t getTimeSent() {
        return timeSent;
    }
    void setTimeSent(omnetpp::simtime_t time) {
        timeSent = time;
    }
    bool isAckEliciting() {
        return ackEliciting;
    }
    size_t getSize() {
        return size;
    }
    size_t getDataSize() {
        return dataSize;
    }
    std::vector<QuicFrame*> *getFrames() {
        return &frames;
    }
    std::string getName() {
        return name;
    }
    Ptr<PacketHeader> getHeader() {
        return header;
    }

private:
    bool ackEliciting = false;
    bool countsInFlight = false;
    omnetpp::simtime_t timeSent;
    Ptr<PacketHeader> header;
    std::vector<QuicFrame*> frames;
    size_t size = 0;
    size_t dataSize = 0;
    std::string name;

};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_APPLICATIONS_QUIC_QUICPACKET_H_ */
