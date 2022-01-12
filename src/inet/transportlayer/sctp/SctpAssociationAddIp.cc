//
// Copyright (C) 2008-2010 Irene Ruengeler
// Copyright (C) 2012 Thomas Dreibholz
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/sctp/SctpAssociation.h"

namespace inet {

namespace sctp {
void SctpAssociation::sendAsconf(const char *type, const bool remote)
{
    SctpAuthenticationChunk *authChunk = nullptr;
    bool nat = false;
    L3Address targetAddr = remoteAddr;
    uint16 chunkLength = 0;

    if (state->asconfOutstanding == false) {
        EV_DEBUG << "sendAsconf\n";
        const auto& sctpAsconf = makeShared<SctpHeader>();
        sctpAsconf->setChunkLength(B(SCTP_COMMON_HEADER));
        sctpAsconf->setSrcPort(localPort);
        sctpAsconf->setDestPort(remotePort);
        SctpAsconfChunk *asconfChunk = new SctpAsconfChunk();
        asconfChunk->setSctpChunkType(ASCONF);
        asconfChunk->setSerialNumber(state->asconfSn);
        chunkLength = SCTP_ADD_IP_CHUNK_LENGTH;
        EV_INFO << "localAddr=" << localAddr << ", remoteAddr=" << remoteAddr << "\n";
        if (getAddressLevel(localAddr) == 3 && getAddressLevel(remoteAddr) == 4 && (bool)sctpMain->par("natFriendly")) {
            asconfChunk->setAddressParam(L3Address("0.0.0.0"));
            asconfChunk->setPeerVTag(peerVTag);
            nat = true;
        }
        else {
            asconfChunk->setAddressParam(localAddr);
        }

        if (localAddr.getType() == L3Address::IPv6) {
            chunkLength += 20;
        }
        else if (localAddr.getType() == L3Address::IPv4) {
            chunkLength += 8;
        }
        else
            throw cRuntimeError("Unknown address type");

        asconfChunk->setByteLength(chunkLength);

        cStringTokenizer tokenizer(type);
        while (tokenizer.hasMoreTokens()) {
            const char *token = tokenizer.nextToken();
            switch (atoi(token)) {
                case ADD_IP_ADDRESS: {
                    SctpAddIPParameter *ipParam;
                    ipParam = new SctpAddIPParameter();
                    chunkLength += SCTP_ADD_IP_PARAMETER_LENGTH;
                    ipParam->setParameterType(ADD_IP_ADDRESS);
                    ipParam->setRequestCorrelationId(++state->corrIdNum);
                    if (nat) {
                        ipParam->setAddressParam(L3Address("0.0.0.0"));
                        sctpMain->addLocalAddressToAllRemoteAddresses(this, L3AddressResolver().resolve(sctpMain->par("addAddress"), 1), remoteAddressList);
                        state->localAddresses.push_back(L3AddressResolver().resolve(sctpMain->par("addAddress"), 1));
                        if (remote)
                            targetAddr = remoteAddr;
                        else
                            targetAddr = getNextAddress(getPath(remoteAddr));
                    }
                    else {
                        ipParam->setAddressParam(L3AddressResolver().resolve(sctpMain->par("addAddress"), 1));
                    }
                    if (ipParam->getAddressParam().getType() == L3Address::IPv6) {
                        chunkLength += 20;
                        ipParam->setByteLength(SCTP_ADD_IP_PARAMETER_LENGTH + 20);
                    }
                    else if (ipParam->getAddressParam().getType() == L3Address::IPv4) {
                        chunkLength += 8;
                        ipParam->setByteLength(SCTP_ADD_IP_PARAMETER_LENGTH + 8);
                    }
                    else
                        throw cRuntimeError("Unknown address type");
                    asconfChunk->addAsconfParam(ipParam);
                    break;
                }

                case DELETE_IP_ADDRESS: {
                    SctpDeleteIPParameter *delParam;
                    delParam = new SctpDeleteIPParameter();
                    chunkLength += SCTP_ADD_IP_PARAMETER_LENGTH;
                    delParam->setParameterType(DELETE_IP_ADDRESS);
                    delParam->setRequestCorrelationId(++state->corrIdNum);
                    delParam->setAddressParam(L3AddressResolver().resolve(sctpMain->par("addAddress"), 1));
                    if (delParam->getAddressParam().getType() == L3Address::IPv6) {
                        chunkLength += 20;
                        delParam->setByteLength(SCTP_ADD_IP_PARAMETER_LENGTH + 20);
                    }
                    else if (delParam->getAddressParam().getType() == L3Address::IPv4) {
                        chunkLength += 8;
                        delParam->setByteLength(SCTP_ADD_IP_PARAMETER_LENGTH + 8);
                    }
                    else
                        throw cRuntimeError("Unknown address type");
                    asconfChunk->addAsconfParam(delParam);
                    break;
                }

                case SET_PRIMARY_ADDRESS: {
                    SctpSetPrimaryIPParameter *priParam;
                    priParam = new SctpSetPrimaryIPParameter();
                    chunkLength += SCTP_ADD_IP_PARAMETER_LENGTH;
                    priParam->setParameterType(SET_PRIMARY_ADDRESS);
                    priParam->setRequestCorrelationId(++state->corrIdNum);
                    priParam->setAddressParam(L3AddressResolver().resolve(sctpMain->par("addAddress"), 1));
                    if (nat) {
                        priParam->setAddressParam(L3Address("0.0.0.0"));
                    }
                    if (priParam->getAddressParam().getType() == L3Address::IPv6) {
                        chunkLength += 20;
                        priParam->setByteLength(SCTP_ADD_IP_PARAMETER_LENGTH + 20);
                    }
                    else if (priParam->getAddressParam().getType() == L3Address::IPv4) {
                        chunkLength += 8;
                        priParam->setByteLength(SCTP_ADD_IP_PARAMETER_LENGTH + 8);
                    }
                    else
                        throw cRuntimeError("Unknown address type");
                    asconfChunk->addAsconfParam(priParam);
                    break;
                }

                default:
                    EV_INFO << "type " <<  atoi(token) << "not known\n";
                    break;
            }
        }
        asconfChunk->setByteLength(chunkLength);

        if (state->auth && state->peerAuth) {
            authChunk = createAuthChunk();
            sctpAsconf->appendSctpChunks(authChunk);
            auto it = sctpMain->assocStatMap.find(assocId);
            it->second.numAuthChunksSent++;
        }
        sctpAsconf->appendSctpChunks(asconfChunk);

        state->asconfChunk = check_and_cast<SctpAsconfChunk *>(asconfChunk->dup());
       // state->asconfChunk->setName("STATE-ASCONF");

        Packet *pkt = new Packet("ASCONF");
        sendToIP(pkt, sctpAsconf, targetAddr);
        state->asconfOutstanding = true;
    }
}

void SctpAssociation::retransmitAsconf()
{
    const auto& sctpmsg = makeShared<SctpHeader>();
    sctpmsg->setChunkLength(B(SCTP_COMMON_HEADER));

    SctpAsconfChunk *sctpasconf = new SctpAsconfChunk();
    sctpasconf = check_and_cast<SctpAsconfChunk *>(state->asconfChunk->dup());
    sctpasconf->setSctpChunkType(ASCONF);
    sctpasconf->setByteLength(state->asconfChunk->getByteLength());

    if (state->auth && state->peerAuth) {
        SctpAuthenticationChunk *authChunk = createAuthChunk();
        sctpmsg->appendSctpChunks(authChunk);
        auto it = sctpMain->assocStatMap.find(assocId);
        it->second.numAuthChunksSent++;
    }
    sctpmsg->appendSctpChunks(sctpasconf);
    Packet *pkt = new Packet("ASCONF");
    sendToIP(pkt, sctpmsg);
}

void SctpAssociation::sendAsconfAck(const uint32 serialNumber)
{
    const auto& sctpAsconfAck = makeShared<SctpHeader>();
    sctpAsconfAck->setChunkLength(B(SCTP_COMMON_HEADER));
    sctpAsconfAck->setSrcPort(localPort);
    sctpAsconfAck->setDestPort(remotePort);

    SctpAsconfAckChunk *asconfAckChunk = new SctpAsconfAckChunk();
    asconfAckChunk->setSctpChunkType(ASCONF_ACK);
    asconfAckChunk->setSerialNumber(serialNumber);
    asconfAckChunk->setByteLength(SCTP_ADD_IP_CHUNK_LENGTH);
    if (state->auth && state->peerAuth) {
        SctpAuthenticationChunk *authChunk = createAuthChunk();
        sctpAsconfAck->appendSctpChunks(authChunk);
        auto it = sctpMain->assocStatMap.find(assocId);
        it->second.numAuthChunksSent++;
    }
    sctpAsconfAck->appendSctpChunks(asconfAckChunk);
    Packet *pkt = new Packet("ASCONF");
    sendToIP(pkt, sctpAsconfAck, remoteAddr);
}

SctpAsconfAckChunk *SctpAssociation::createAsconfAckChunk(const uint32 serialNumber)
{
    SctpAsconfAckChunk *asconfAckChunk = new SctpAsconfAckChunk();
    asconfAckChunk->setSctpChunkType(ASCONF_ACK);
    asconfAckChunk->setSerialNumber(serialNumber);
    asconfAckChunk->setByteLength(SCTP_ADD_IP_CHUNK_LENGTH);
    return asconfAckChunk;
}

SctpAuthenticationChunk *SctpAssociation::createAuthChunk()
{
    SctpAuthenticationChunk *authChunk = new SctpAuthenticationChunk();

    authChunk->setSctpChunkType(AUTH);
    authChunk->setSharedKey(0);
    authChunk->setHMacIdentifier(1);
    authChunk->setHMacOk(true);
    authChunk->setHMACArraySize(SHA_LENGTH);
    for (int32 i = 0; i < SHA_LENGTH; i++) {
        authChunk->setHMAC(i, 0);
    }
    authChunk->setByteLength(SCTP_AUTH_CHUNK_LENGTH + SHA_LENGTH);
    return authChunk;
}

bool SctpAssociation::compareRandom()
{
    int32 i, sizeKeyVector, sizePeerKeyVector, size = 0;

    sizeKeyVector = state->sizeKeyVector;
    sizePeerKeyVector = state->sizePeerKeyVector;

    if (sizeKeyVector != sizePeerKeyVector) {
        if (sizePeerKeyVector > sizeKeyVector) {
            size = sizeKeyVector;
            for (i = sizePeerKeyVector - 1; i > sizeKeyVector; i--) {
                if (state->peerKeyVector[i] != 0)
                    return false;
            }
        }
        else {
            size = sizePeerKeyVector;
            for (i = sizeKeyVector - 1; i > sizePeerKeyVector; i--) {
                if (state->keyVector[i] != 0)
                    return true;
            }
        }
    }
    else
        size = sizeKeyVector;
    for (i = size - 1; i > 0; i--) {
        if (state->keyVector[i] < state->peerKeyVector[i])
            return false;
        if (state->keyVector[i] > state->peerKeyVector[i])
            return true;
    }
    return true;
}

void SctpAssociation::calculateAssocSharedKey()
{
    const bool peerFirst = compareRandom();
    if (peerFirst == true) {
        for (uint32 i = 0; i < state->sizeKeyVector; i++)
            state->sharedKey[i] = state->keyVector[i];
        for (uint32 i = 0; i < state->sizePeerKeyVector; i++)
            state->sharedKey[i + state->sizeKeyVector] = state->peerKeyVector[i];
    }
    else {
        for (uint32 i = 0; i < state->sizePeerKeyVector; i++)
            state->sharedKey[i] = state->peerKeyVector[i];
        for (uint32 i = 0; i < state->sizeKeyVector; i++)
            state->sharedKey[i + state->sizePeerKeyVector] = state->keyVector[i];
    }
}

bool SctpAssociation::typeInChunkList(const uint16 type)
{
    for (auto & elem : state->peerChunkList) {
        if ((elem) == type) {
            return true;
        }
    }
    return false;
}

bool SctpAssociation::typeInOwnChunkList(const uint16 type)
{
    for (auto & elem : state->chunkList) {
        if ((elem) == type) {
            return true;
        }
    }
    return false;
}

SctpSuccessIndication *SctpAssociation::createSuccessIndication(const uint32 correlationId)
{
    SctpSuccessIndication *success = new SctpSuccessIndication();

    success->setParameterType(SUCCESS_INDICATION);
    success->setResponseCorrelationId(correlationId);
    success->setByteLength(SCTP_ADD_IP_PARAMETER_LENGTH);
    return success;
}

} // namespace sctp

} // namespace inet

