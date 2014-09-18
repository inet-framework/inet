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
#include "inet/transportlayer/sctp/SCTPAssociation.h"

namespace inet {

namespace sctp {
void SCTPAssociation::sendAsconf(const char *type, const bool remote)
{
    SCTPAuthenticationChunk *authChunk;
    bool nat = false;
    L3Address targetAddr = remoteAddr;
    uint16 chunkLength = 0;

    if (state->asconfOutstanding == false) {
        EV_DEBUG << "sendAsconf\n";
        SCTPMessage *sctpAsconf = new SCTPMessage("ASCONF-MSG");
        sctpAsconf->setByteLength(SCTP_COMMON_HEADER);
        sctpAsconf->setSrcPort(localPort);
        sctpAsconf->setDestPort(remotePort);
        SCTPAsconfChunk *asconfChunk = new SCTPAsconfChunk("ASCONF-CHUNK");
        asconfChunk->setChunkType(ASCONF);
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

        char typecopy[128];
        strcpy(typecopy, type);
        char *token;
        token = strtok((char *)typecopy, ",");
        while (token != NULL) {
            switch (atoi(token)) {
                case ADD_IP_ADDRESS: {
                    SCTPAddIPParameter *ipParam;
                    ipParam = new SCTPAddIPParameter("AddIP");
                    chunkLength += SCTP_ADD_IP_PARAMETER_LENGTH;
                    ipParam->setParameterType(ADD_IP_ADDRESS);
                    ipParam->setRequestCorrelationId(++state->corrIdNum);
                    if (nat) {
                        ipParam->setAddressParam(L3Address("0.0.0.0"));
                        sctpMain->addLocalAddressToAllRemoteAddresses(this, L3AddressResolver().resolve(sctpMain->par("addAddress"), 1), (std::vector<L3Address>)remoteAddressList);
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
                        ipParam->setBitLength((SCTP_ADD_IP_PARAMETER_LENGTH + 20) * 8);
                    }
                    else if (ipParam->getAddressParam().getType() == L3Address::IPv4) {
                        chunkLength += 8;
                        ipParam->setBitLength((SCTP_ADD_IP_PARAMETER_LENGTH + 8) * 8);
                    }
                    else
                        throw cRuntimeError("Unknown address type");
                    asconfChunk->addAsconfParam(ipParam);
                    break;
                }

                case DELETE_IP_ADDRESS: {
                    SCTPDeleteIPParameter *delParam;
                    delParam = new SCTPDeleteIPParameter("DeleteIP");
                    chunkLength += SCTP_ADD_IP_PARAMETER_LENGTH;
                    delParam->setParameterType(DELETE_IP_ADDRESS);
                    delParam->setRequestCorrelationId(++state->corrIdNum);
                    delParam->setAddressParam(L3AddressResolver().resolve(sctpMain->par("addAddress"), 1));
                    if (delParam->getAddressParam().getType() == L3Address::IPv6) {
                        chunkLength += 20;
                        delParam->setBitLength((SCTP_ADD_IP_PARAMETER_LENGTH + 20) * 8);
                    }
                    else if (delParam->getAddressParam().getType() == L3Address::IPv4) {
                        chunkLength += 8;
                        delParam->setBitLength((SCTP_ADD_IP_PARAMETER_LENGTH + 8) * 8);
                    }
                    else
                        throw cRuntimeError("Unknown address type");
                    asconfChunk->addAsconfParam(delParam);
                    break;
                }

                case SET_PRIMARY_ADDRESS: {
                    SCTPSetPrimaryIPParameter *priParam;
                    priParam = new SCTPSetPrimaryIPParameter("SetPrimary");
                    chunkLength += SCTP_ADD_IP_PARAMETER_LENGTH;
                    priParam->setParameterType(SET_PRIMARY_ADDRESS);
                    priParam->setRequestCorrelationId(++state->corrIdNum);
                    priParam->setAddressParam(L3AddressResolver().resolve(sctpMain->par("addAddress"), 1));
                    if (nat) {
                        priParam->setAddressParam(L3Address("0.0.0.0"));
                    }
                    if (priParam->getAddressParam().getType() == L3Address::IPv6) {
                        chunkLength += 20;
                        priParam->setByteLength((SCTP_ADD_IP_PARAMETER_LENGTH + 20));
                    }
                    else if (priParam->getAddressParam().getType() == L3Address::IPv4) {
                        chunkLength += 8;
                        priParam->setByteLength((SCTP_ADD_IP_PARAMETER_LENGTH + 8));
                    }
                    else
                        throw cRuntimeError("Unknown address type");
                    asconfChunk->addAsconfParam(priParam);
                    break;
                }

                default:
                    printf("type %d not known\n", atoi(token));
                    break;
            }
            token = strtok(NULL, ",");
        }
        asconfChunk->setByteLength(chunkLength);

        if (state->auth && state->peerAuth) {
            authChunk = createAuthChunk();
            sctpAsconf->addChunk(authChunk);
            SCTP::AssocStatMap::iterator it = sctpMain->assocStatMap.find(assocId);
            it->second.numAuthChunksSent++;
        }
        sctpAsconf->addChunk(asconfChunk);

        state->asconfChunk = check_and_cast<SCTPAsconfChunk *>(asconfChunk->dup());
        state->asconfChunk->setName("STATE-ASCONF");

        sendToIP(sctpAsconf, targetAddr);
        state->asconfOutstanding = true;
    }
}

void SCTPAssociation::retransmitAsconf()
{
    SCTPMessage *sctpmsg = new SCTPMessage();
    sctpmsg->setByteLength(SCTP_COMMON_HEADER);

    SCTPAsconfChunk *sctpasconf = new SCTPAsconfChunk("ASCONF-RTX");
    sctpasconf = check_and_cast<SCTPAsconfChunk *>(state->asconfChunk->dup());
    sctpasconf->setChunkType(ASCONF);
    sctpasconf->setByteLength(state->asconfChunk->getByteLength());

    if (state->auth && state->peerAuth) {
        SCTPAuthenticationChunk *authChunk = createAuthChunk();
        sctpmsg->addChunk(authChunk);
        SCTP::AssocStatMap::iterator it = sctpMain->assocStatMap.find(assocId);
        it->second.numAuthChunksSent++;
    }
    sctpmsg->addChunk(sctpasconf);

    sendToIP(sctpmsg);
}

void SCTPAssociation::sendAsconfAck(const uint32 serialNumber)
{
    SCTPMessage *sctpAsconfAck = new SCTPMessage("ASCONF_ACK");
    sctpAsconfAck->setByteLength(SCTP_COMMON_HEADER);
    sctpAsconfAck->setSrcPort(localPort);
    sctpAsconfAck->setDestPort(remotePort);

    SCTPAsconfAckChunk *asconfAckChunk = new SCTPAsconfAckChunk("ASCONF_ACK");
    asconfAckChunk->setChunkType(ASCONF_ACK);
    asconfAckChunk->setSerialNumber(serialNumber);
    asconfAckChunk->setByteLength(SCTP_ADD_IP_CHUNK_LENGTH);
    if (state->auth && state->peerAuth) {
        SCTPAuthenticationChunk *authChunk = createAuthChunk();
        sctpAsconfAck->addChunk(authChunk);
        SCTP::AssocStatMap::iterator it = sctpMain->assocStatMap.find(assocId);
        it->second.numAuthChunksSent++;
    }
    sctpAsconfAck->addChunk(asconfAckChunk);
    sendToIP(sctpAsconfAck, remoteAddr);
}

SCTPAsconfAckChunk *SCTPAssociation::createAsconfAckChunk(const uint32 serialNumber)
{
    SCTPAsconfAckChunk *asconfAckChunk = new SCTPAsconfAckChunk("ASCONF_ACK");
    asconfAckChunk->setChunkType(ASCONF_ACK);
    asconfAckChunk->setSerialNumber(serialNumber);
    asconfAckChunk->setByteLength(SCTP_ADD_IP_CHUNK_LENGTH);
    return asconfAckChunk;
}

SCTPAuthenticationChunk *SCTPAssociation::createAuthChunk()
{
    SCTPAuthenticationChunk *authChunk = new SCTPAuthenticationChunk("AUTH");

    authChunk->setChunkType(AUTH);
    authChunk->setSharedKey(0);
    authChunk->setHMacIdentifier(1);
    authChunk->setHMacOk(true);
    authChunk->setHMACArraySize(SHA_LENGTH);
    for (int32 i = 0; i < SHA_LENGTH; i++) {
        authChunk->setHMAC(i, 0);
    }
    authChunk->setBitLength((SCTP_AUTH_CHUNK_LENGTH + SHA_LENGTH) * 8);
    return authChunk;
}

bool SCTPAssociation::compareRandom()
{
    int32 i, sizeKeyVector, sizePeerKeyVector, size = 0;

    sizeKeyVector = sizeof(state->keyVector);
    sizePeerKeyVector = sizeof(state->peerKeyVector);

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

void SCTPAssociation::calculateAssocSharedKey()
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

bool SCTPAssociation::typeInChunkList(const uint16 type)
{
    for (std::vector<uint16>::iterator i = state->peerChunkList.begin(); i != state->peerChunkList.end(); i++) {
        if ((*i) == type) {
            return true;
        }
    }
    return false;
}

bool SCTPAssociation::typeInOwnChunkList(const uint16 type)
{
    for (std::vector<uint16>::iterator i = state->chunkList.begin(); i != state->chunkList.end(); i++) {
        if ((*i) == type) {
            return true;
        }
    }
    return false;
}

SCTPSuccessIndication *SCTPAssociation::createSuccessIndication(const uint32 correlationId)
{
    SCTPSuccessIndication *success = new SCTPSuccessIndication("Success");

    success->setParameterType(SUCCESS_INDICATION);
    success->setResponseCorrelationId(correlationId);
    success->setBitLength(SCTP_ADD_IP_PARAMETER_LENGTH * 8);
    return success;
}

} // namespace sctp

} // namespace inet

