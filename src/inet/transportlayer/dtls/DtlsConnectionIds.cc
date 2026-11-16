// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/transportlayer/dtls/Dtls.h"

#include "inet/transportlayer/dtls/DtlsPlaintextHeader_m.h"

namespace inet::dtls {

std::vector<uint8_t> Dtls::allocateConnectionId(AssociationId id)
{
    if (nextConnectionId == 0)
        return {};
    auto value = nextConnectionId++;
    std::vector<uint8_t> cid;
    for (int shift = 56; shift >= 0; shift -= 8)
        cid.push_back(value >> shift);
    return cidIndex.emplace(cid, id).second ? cid : std::vector<uint8_t>{};
}

void Dtls::removeConnectionIds(AssociationId id)
{
    for (auto it = cidIndex.begin(); it != cidIndex.end();)
        if (it->second == id)
            it = cidIndex.erase(it);
        else
            ++it;
}

Ptr<DtlsCiphertextHeader> DtlsAssociation::makeRecordHeader(uint64_t epoch, uint64_t number, uint16_t length) const
{
    auto header = makeShared<DtlsCiphertextHeader>();
    header->setEpochBits(epoch & 3);
    header->setSequenceNumber(number & 0xffff);
    header->setLength(length);
    if (cidNegotiated) {
        header->setConnectionIdArraySize(peerCid.size());
        for (size_t i = 0; i < peerCid.size(); ++i)
            header->setConnectionId(i, peerCid[i]);
    }
    header->setChunkLength(B(5 + header->getConnectionIdArraySize()));
    return header;
}

} // namespace inet::dtls
