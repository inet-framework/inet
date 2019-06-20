//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtFrame_m.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtFrameSerializer.h"

namespace inet {

namespace ieee80211 {

Register_Serializer(Ieee80211AuthenticationFrame, Ieee80211AuthenticationFrameSerializer);
Register_Serializer(Ieee80211DeauthenticationFrame, Ieee80211DeauthenticationFrameSerializer);
Register_Serializer(Ieee80211AssociationRequestFrame, Ieee80211AssociationRequestFrameSerializer);
Register_Serializer(Ieee80211AssociationResponseFrame, Ieee80211AssociationResponseFrameSerializer);
Register_Serializer(Ieee80211BeaconFrame, Ieee80211BeaconFrameSerializer);
Register_Serializer(Ieee80211DisassociationFrame, Ieee80211DisassociationFrameSerializer);
Register_Serializer(Ieee80211ProbeRequestFrame, Ieee80211ProbeRequestFrameSerializer);
Register_Serializer(Ieee80211ProbeResponseFrame, Ieee80211ProbeResponseFrameSerializer);
Register_Serializer(Ieee80211ReassociationRequestFrame, Ieee80211ReassociationRequestFrameSerializer);
Register_Serializer(Ieee80211ReassociationResponseFrame, Ieee80211ReassociationResponseFrameSerializer);

/**
 * Authentication Frame
 */
void Ieee80211AuthenticationFrameSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const {
    auto authenticationFrame = dynamicPtrCast<const Ieee80211AuthenticationFrame>(chunk);
    // 1    Authentication algorithm number
    stream.writeUint16Be(0);
    // 2    Authentication transaction sequence number
    stream.writeUint16Be(authenticationFrame->getSequenceNumber());
    // 3    Status code                                 The status code information is reserved in certain Authentication frames as defined in Table 7-17.
    stream.writeUint16Be(authenticationFrame->getStatusCode());
    stream.writeByte(authenticationFrame->isLast() ? 1 : 0);
    // 4    Challenge text                              The challenge text information is present only in certain Authentication frames as defined in Table 7-17.
    stream.writeByte(0);
    // Last Vendor Specific                             One or more vendor-specific information elements may appear in this frame. This information element follows all other information elements.
}

const Ptr<Chunk> Ieee80211AuthenticationFrameSerializer::deserialize(MemoryInputStream& stream) const {
    auto authenticationFrame = makeShared<Ieee80211AuthenticationFrame>();
    stream.readUint16Be();
    authenticationFrame->setSequenceNumber(stream.readUint16Be());
    int16_t statusCode = stream.readUint16Be();
    authenticationFrame->setStatusCode(static_cast<Ieee80211StatusCode>(statusCode));
    authenticationFrame->setIsLast(stream.readByte() == 1);
    stream.readByte();
    return authenticationFrame;
}

/**
 * Deauthentication Frame
 */
void Ieee80211DeauthenticationFrameSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const {
    auto deauthenticationFrame = dynamicPtrCast<const Ieee80211DeauthenticationFrame>(chunk);
    stream.writeUint16Be(deauthenticationFrame->getReasonCode());
}

const Ptr<Chunk> Ieee80211DeauthenticationFrameSerializer::deserialize(MemoryInputStream& stream) const {
    auto dauthenticationFrame = makeShared<Ieee80211DeauthenticationFrame>();
    int16_t reasonCode = stream.readUint16Be();
    dauthenticationFrame->setReasonCode(static_cast<Ieee80211ReasonCode>(reasonCode));
    return dauthenticationFrame;
}

/**
 * Association Request Frame
 */
void Ieee80211AssociationRequestFrameSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const {
    auto associationRequestFrame = dynamicPtrCast<const Ieee80211AssociationRequestFrame>(chunk);
    // 1    Capability
    stream.writeUint16Be(0);    //FIXME
    // 2    Listen interval
    stream.writeUint16Be(0);    //FIXME
    // 3    SSID
    const char *SSID = associationRequestFrame->getSSID();
    unsigned int length = strlen(SSID);
    stream.writeByte(0);    //FIXME dummy, what is it?
    stream.writeByte(length);
    stream.writeBytes((uint8_t *)SSID, B(length));
    // 4    Supported rates
    const Ieee80211SupportedRatesElement& supportedRates = associationRequestFrame->getSupportedRates();
    stream.writeByte(1);
    stream.writeByte(supportedRates.numRates);
    for (int i = 0; i < supportedRates.numRates; i++) {
        uint8_t rate = ceil(supportedRates.rate[i]/0.5);
        // rate |= 0x80 if rate contained in the BSSBasicRateSet parameter
        stream.writeByte(rate);
    }
    // 5    Extended Supported Rates   The Extended Supported Rates element is present whenever there are more than eight supported rates, and it is optional otherwise.
    // 6    Power Capability           The Power Capability element shall be present if dot11SpectrumManagementRequired is true.
    // 7    Supported Channel          The Supported Channels element shall be present if dot11SpectrumManagementRequired is true.
    // 8    RSN                        The RSN information element is only present within Association Request frames generated by STAs that have dot11RSNAEnabled set to TRUE.
    // 9    QoS Capability             The QoS Capability element is present when dot11QosOption- Implemented is true.
    // Last Vendor Specific            One or more vendor-specific information elements may appear in this frame. This information element follows all other information elements.

}

const Ptr<Chunk> Ieee80211AssociationRequestFrameSerializer::deserialize(MemoryInputStream& stream) const {
    auto associationRequestFrame = makeShared<Ieee80211AssociationRequestFrame>();
    associationRequestFrame->setChunkLength(stream.getRemainingLength());
    stream.readUint16Be();
    stream.readUint16Be();
    char SSID[256];
    stream.readByte();
    unsigned int length = stream.readByte();
    stream.readBytes((uint8_t *)SSID, B(length));
    SSID[length] = '\0';
    associationRequestFrame->setSSID(SSID);

    Ieee80211SupportedRatesElement supRat;
    stream.readByte();
    supRat.numRates = stream.readByte();
    for (int i = 0; i < supRat.numRates; i++)
        supRat.rate[i] = (double)(stream.readByte() & 0x7F) * 0.5;
    associationRequestFrame->setSupportedRates(supRat);
    return associationRequestFrame;
}

/**
 * Association Response Frame
 */
void Ieee80211AssociationResponseFrameSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const {
    auto associationResponseFrame = dynamicPtrCast<const Ieee80211AssociationResponseFrame>(chunk);
    // 1    Capability
    stream.writeUint16Be(0);    //FIXME
    // 2    Status code
    stream.writeUint16Be(associationResponseFrame->getStatusCode());
    // 3    AID
    stream.writeUint16Be(associationResponseFrame->getAid());
    // 4    Supported rates
    stream.writeByte(1);
    stream.writeByte(associationResponseFrame->getSupportedRates().numRates);
    for (int i = 0; i < associationResponseFrame->getSupportedRates().numRates; i++)
    {
        uint8_t rate = ceil(associationResponseFrame->getSupportedRates().rate[i]/0.5);
        // rate |= 0x80 if rate contained in the BSSBasicRateSet parameter
        stream.writeByte(rate);
    }
    // 5    Extended Supported Rates   The Extended Supported Rates element is present whenever there are more than eight supported rates, and it is optional otherwise.
    // 6    EDCA Parameter Set
    // Last Vendor Specific            One or more vendor-specific information elements may appear in this frame. This information element follows all other information elements.

}

const Ptr<Chunk> Ieee80211AssociationResponseFrameSerializer::deserialize(MemoryInputStream& stream) const {
    auto associationResponseFrame = makeShared<Ieee80211AssociationResponseFrame>();
    stream.readUint16Be();
    int16_t statusCode = stream.readUint16Be();
    associationResponseFrame->setStatusCode(static_cast<Ieee80211StatusCode>(statusCode));
    associationResponseFrame->setAid(stream.readUint16Be());

    Ieee80211SupportedRatesElement supRat;
    stream.readByte();
    supRat.numRates = stream.readByte();
    for (int i = 0; i < supRat.numRates; i++)
        supRat.rate[i] = (double)(stream.readByte() & 0x7F) * 0.5;
    associationResponseFrame->setSupportedRates(supRat);
    return associationResponseFrame;
}

/**
 * Beacon Frame
 */
void Ieee80211BeaconFrameSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const {
    auto beaconFrame = dynamicPtrCast<const Ieee80211BeaconFrame>(chunk);
    // 1    Timestamp
    stream.writeUint64Be(simTime().raw());   //FIXME
    // 2    Beacon interval
    stream.writeUint16Be((uint16_t)(beaconFrame->getBeaconInterval().inUnit(SIMTIME_MS)));
    // 3    Capability
    stream.writeUint16Be(0);    //FIXME set  capability
    // 4    Service Set Identifier (SSID)
    const char *SSID = beaconFrame->getSSID();
    unsigned int length = strlen(SSID);
    stream.writeByte(beaconFrame->getChannelNumber());    //FIXME
    stream.writeByte(length);
    stream.writeBytes((uint8_t *)SSID, B(length));
    // 5    Supported rates
    stream.writeByte(1);
    stream.writeByte(beaconFrame->getSupportedRates().numRates);
    for (int i = 0; i < beaconFrame->getSupportedRates().numRates; i++)
    {
        uint8_t rate = ceil(beaconFrame->getSupportedRates().rate[i]/0.5);
        // rate |= 0x80 if rate contained in the BSSBasicRateSet parameter
        stream.writeByte(rate);
    }
    // 6    Frequency-Hopping (FH) Parameter Set   The FH Parameter Set information element is present within Beacon frames generated by STAs using FH PHYs.
    // 7    DS Parameter Set                       The DS Parameter Set information element is present within Beacon frames generated by STAs using Clause 15, Clause 18, and Clause 19 PHYs.
    // 8    CF Parameter Set                       The CF Parameter Set information element is present only within Beacon frames generated by APs supporting a PCF.
    // 9    IBSS Parameter Set                     The IBSS Parameter Set information element is present only within Beacon frames generated by STAs in an IBSS.
    // 10   Traffic indication map (TIM)           The TIM information element is present only within Beacon frames generated by APs.
    // 11   Country                                The Country information element shall be present when dot11MultiDomainCapabilityEnabled is true or dot11SpectrumManagementRequired is true.
    // 12   FH Parameters                          FH Parameters as specified in 7.3.2.10 may be included if dot11MultiDomainCapabilityEnabled is true.
    // 13   FH Pattern Table                       FH Pattern Table information as specified in 7.3.2.11 may be included if dot11MultiDomainCapabilityEnabled is true.
    // 14   Power Constraint                       Power Constraint element shall be present if dot11SpectrumManagementRequired is true.
    // 15   Channel Switch Announcement            Channel Switch Announcement element may be present if dot11SpectrumManagementRequired is true.
    // 16   Quiet                                  Quiet element may be present if dot11SpectrumManagementRequired is true.
    // 17   IBSS DFS                               IBSS DFS element shall be present if dot11SpectrumManagementRequired is true in an IBSS.
    // 18   TPC Report                             TPC Report element shall be present if dot11SpectrumManagementRequired is true.
    // 19   ERP Information                        The ERP Information element is present within Beacon frames generated by STAs using extended rate PHYs (ERPs) defined in Clause 19 and is optionally present in other cases.
    // 20   Extended Supported Rates               The Extended Supported Rates element is present whenever there are more than eight supported rates, and it is optional otherwise.
    // 21   RSN                                    The RSN information element shall be present within Beacon frames generated by STAs that have dot11RSNAEnabled set to TRUE.
    // 22   BSS Load                               The BSS Load element is present when dot11QosOption- Implemented and dot11QBSSLoadImplemented are both true.
    // 23   EDCA Parameter Set                     The EDCA Parameter Set element is present when dot11QosOptionImplemented is true and the QoS Capability element is not present.
    // 24   QoS Capability                         The QoS Capability element is present when dot11QosOption- Implemented is true and EDCA Parameter Set element is not present.
    // Last Vendor Specific                        One or more vendor-specific information elements may appear in this frame. This information element follows all other information elements.

}

const Ptr<Chunk> Ieee80211BeaconFrameSerializer::deserialize(MemoryInputStream& stream) const {
    auto beaconFrame = makeShared<Ieee80211BeaconFrame>();

    simtime_t timetstamp;
    timetstamp.setRaw(stream.readUint64Be()); // TODO: store timestamp

    beaconFrame->setBeaconInterval(SimTime((int64_t)stream.readUint16Be(), SIMTIME_MS));
    stream.readUint16Be();     // Capability

    char SSID[256];
    beaconFrame->setChannelNumber(stream.readByte());
    unsigned int length = stream.readByte();
    stream.readBytes((uint8_t *)SSID, B(length));
    SSID[length] = '\0';
    beaconFrame->setSSID(SSID);

    Ieee80211SupportedRatesElement supRat;
    stream.readByte();
    supRat.numRates = stream.readByte();
    for (int i = 0; i < supRat.numRates; i++)
        supRat.rate[i] = (double)(stream.readByte() & 0x7F) * 0.5;
    beaconFrame->setSupportedRates(supRat);
    return beaconFrame;
}

/**
 * Disassociation Frame
 */
void Ieee80211DisassociationFrameSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const {
    auto disassociationFrame = dynamicPtrCast<const Ieee80211DisassociationFrame>(chunk);
    stream.writeUint16Be(disassociationFrame->getReasonCode());
}

const Ptr<Chunk> Ieee80211DisassociationFrameSerializer::deserialize(MemoryInputStream& stream) const {
    auto disassociationFrame = makeShared<Ieee80211DisassociationFrame>();
    int16_t reasonCode = stream.readUint16Be();
    disassociationFrame->setReasonCode(static_cast<Ieee80211ReasonCode>(reasonCode));
    return disassociationFrame;
}

/**
 * Probe Request Frame
 */
void Ieee80211ProbeRequestFrameSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const {
    auto probeRequestFrame = dynamicPtrCast<const Ieee80211ProbeRequestFrame>(chunk);
    // 1    SSID
    const char *SSID = probeRequestFrame->getSSID();
    unsigned int length = strlen(SSID);
    stream.writeByte(0);    //FIXME dummy, what is it?
    stream.writeByte(length);
    stream.writeBytes((uint8_t *)SSID, B(length));
    // 2    Supported rates
    const Ieee80211SupportedRatesElement& supportedRates = probeRequestFrame->getSupportedRates();
    stream.writeByte(1);
    stream.writeByte(supportedRates.numRates);
    for (int i = 0; i < supportedRates.numRates; i++)
    {
        uint8_t rate = ceil(supportedRates.rate[i]/0.5);
        // rate |= 0x80 if rate contained in the BSSBasicRateSet parameter
        stream.writeByte(rate);
    }
    // 3    Request information         May be included if dot11MultiDomainCapabilityEnabled is true.
    // 4    Extended Supported Rates    The Extended Supported Rates element is present whenever there are more than eight supported rates, and it is optional otherwise.
    // Last Vendor Specific             One or more vendor-specific information elements may appear in this frame. This information element follows all other information elements.

}

const Ptr<Chunk> Ieee80211ProbeRequestFrameSerializer::deserialize(MemoryInputStream& stream) const {
    auto probeRequestFrame = makeShared<Ieee80211ProbeRequestFrame>();
    char SSID[256];
    stream.readByte();
    unsigned int length = stream.readByte();
    stream.readBytes((uint8_t *)SSID, B(length));
    SSID[length] = '\0';
    probeRequestFrame->setSSID(SSID);

    Ieee80211SupportedRatesElement supRat;
    stream.readByte();
    supRat.numRates = stream.readByte();
    for (int i = 0; i < supRat.numRates; i++)
        supRat.rate[i] = (double)(stream.readByte() & 0x7F) * 0.5;
    probeRequestFrame->setSupportedRates(supRat);
    return probeRequestFrame;
}

/**
 * Probe Response Frame
 */
void Ieee80211ProbeResponseFrameSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const {
    auto probeResponseFrame = dynamicPtrCast<const Ieee80211ProbeResponseFrame>(chunk);
    // 1      Timestamp
    stream.writeUint64Be(simTime().raw());   //FIXME
    // 2      Beacon interval
    stream.writeUint16Be((uint16_t)(probeResponseFrame->getBeaconInterval().inUnit(SIMTIME_MS)));
    // 3      Capability
    stream.writeUint16Be(0);    //FIXME
    // 4      SSID
    const char *SSID = probeResponseFrame->getSSID();
    unsigned int length = strlen(SSID);
    stream.writeByte(probeResponseFrame->getChannelNumber());    //FIXME
    stream.writeByte(length);
    stream.writeBytes((uint8_t *)SSID, B(length));
    // 5      Supported rates
    stream.writeByte(1);
    stream.writeByte(probeResponseFrame->getSupportedRates().numRates);
    for (int i = 0; i < probeResponseFrame->getSupportedRates().numRates; i++)
    {
        uint8_t rate = ceil(probeResponseFrame->getSupportedRates().rate[i]/0.5);
        // rate |= 0x80 if rate contained in the BSSBasicRateSet parameter
        stream.writeByte(rate);
    }
    // 6      FH Parameter Set                The FH Parameter Set information element is present within Probe Response frames generated by STAs using FH PHYs.
    // 7      DS Parameter Set                The DS Parameter Set information element is present within Probe Response frames generated by STAs using Clause 15, Clause 18, and Clause 19 PHYs.
    // 8      CF Parameter Set                The CF Parameter Set information element is present only within Probe Response frames generated by APs supporting a PCF.
    // 9      IBSS Parameter Set              The IBSS Parameter Set information element is present only within Probe Response frames generated by STAs in an IBSS.
    // 10     Country                         Included if dot11MultiDomainCapabilityEnabled or dot11SpectrumManagementRequired is true.
    // 11     FH Parameters                   FH Parameters, as specified in 7.3.2.10, may be included if dot11MultiDomainCapabilityEnabled is true.
    // 12     FH Pattern Table                FH Pattern Table information, as specified in 7.3.2.11, may be included if dot11MultiDomainCapabilityEnabled is true.
    // 13     Power Constraint                Shall be included if dot11SpectrumManagementRequired is true.
    // 14     Channel Switch Announcement     May be included if dot11SpectrumManagementRequired is true.
    // 15     Quiet                           May be included if dot11SpectrumManagementRequired is true.
    // 16     IBSS DFS                        Shall be included if dot11SpectrumManagementRequired is true in an IBSS.
    // 17     TPC Report                      Shall be included if dot11SpectrumManagementRequired is true.
    // 18     ERP Information                 The ERP Information element is present within Probe Response frames generated by STAs using ERPs and is optionally present in other cases.
    // 19     Extended Supported Rates        The Extended Supported Rates element is present whenever there are more than eight supported rates, and it is optional otherwise.
    // 20     RSN                             The RSN information element is only present within Probe Response frames generated by STAs that have dot11RSNA- Enabled set to TRUE.
    // 21     BSS Load                        The BSS Load element is present when dot11QosOption- Implemented and dot11QBSSLoadImplemented are both true.
    // 22     EDCA Parameter Set              The EDCA Parameter Set element is present when dot11QosOptionImplemented is true.
    // Last�1 Vendor Specific                 One or more vendor-specific information elements may appear in this frame. This information element follows all other information elements, except the Requested Information elements.
    // Last�n Requested information elements  Elements requested by the Request information element of the Probe Request frame.

}

const Ptr<Chunk> Ieee80211ProbeResponseFrameSerializer::deserialize(MemoryInputStream& stream) const {
    auto probeResponseFrame = makeShared<Ieee80211ProbeResponseFrame>();

    simtime_t timestamp;
    timestamp.setRaw(stream.readUint64Be()); // TODO: store timestamp

    probeResponseFrame->setBeaconInterval(SimTime((int64_t)stream.readUint16Be(), SIMTIME_MS));
    stream.readUint16Be();

    char SSID[256];
    probeResponseFrame->setChannelNumber(stream.readByte());
    unsigned int length = stream.readByte();
    stream.readBytes((uint8_t *)SSID, B(length));
    SSID[length] = '\0';
    probeResponseFrame->setSSID(SSID);

    Ieee80211SupportedRatesElement supRat;
    stream.readByte();
    supRat.numRates = stream.readByte();
    for (int i = 0; i < supRat.numRates; i++)
        supRat.rate[i] = (double)(stream.readByte() & 0x7F) * 0.5;
    probeResponseFrame->setSupportedRates(supRat);
    return probeResponseFrame;

}

/**
 * Reassociation Request Frame
 */
void Ieee80211ReassociationRequestFrameSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const {
    auto reassociationRequestFrame = dynamicPtrCast<const Ieee80211ReassociationRequestFrame>(chunk);
    // 1    Capability
    stream.writeUint16Be(0);    //FIXME
    // 2    Listen interval
    stream.writeUint16Be(0);    //FIXME
    // 3    Current AP address
    stream.writeMacAddress(reassociationRequestFrame->getCurrentAP());
    // 4    SSID
    const char *SSID = reassociationRequestFrame->getSSID();
    unsigned int length = strlen(SSID);
    //FIXME buffer.writeByte(buf + packetLength, ???);
    stream.writeByte(0);    //FIXME
    stream.writeByte(length);
    stream.writeBytes((uint8_t *)SSID, B(length));
    // 5    Supported rates
    const Ieee80211SupportedRatesElement& supportedRates = reassociationRequestFrame->getSupportedRates();
    stream.writeByte(1);
    stream.writeByte(supportedRates.numRates);
    for (int i = 0; i < supportedRates.numRates; i++)
    {
        uint8_t rate = ceil(supportedRates.rate[i]/0.5);
        // rate |= 0x80 if rate contained in the BSSBasicRateSet parameter
        stream.writeByte(rate);
    }
    // 6    Extended Supported Rates   The Extended Supported Rates element is present whenever there are more than eight supported rates, and it is optional otherwise.
    // 7    Power Capability           The Power Capability element shall be present if dot11SpectrumManagementRequired is true.
    // 8    Supported Channels         The Supported Channels element shall be present if dot11SpectrumManagementRequired is true.
    // 9    RSN                        The RSN information element is only present within Reassociation Request frames generated by STAs that have dot11RSNAEnabled set to TRUE.
    // 10   QoS Capability             The QoS Capability element is present when dot11QosOption- Implemented is true.
    // Last Vendor Specific            One or more vendor-specific information elements may appear in this frame. This information element follows all other information elements.

}

const Ptr<Chunk> Ieee80211ReassociationRequestFrameSerializer::deserialize(MemoryInputStream& stream) const {
    auto reassociationRequestFrame = makeShared<Ieee80211ReassociationRequestFrame>();
    stream.readUint16Be();
    stream.readUint16Be();

    reassociationRequestFrame->setCurrentAP(stream.readMacAddress());

    char SSID[256];
    stream.readByte();
    unsigned int length = stream.readByte();
    stream.readBytes((uint8_t *)SSID, B(length));
    SSID[length] = '\0';
    reassociationRequestFrame->setSSID(SSID);

    Ieee80211SupportedRatesElement supRat;
    stream.readByte();
    supRat.numRates = stream.readByte();
    for (int i = 0; i < supRat.numRates; i++)
        supRat.rate[i] = (double)(stream.readByte() & 0x7F) * 0.5;
    reassociationRequestFrame->setSupportedRates(supRat);
    return reassociationRequestFrame;
}

/**
 * Reassociation Response Frame
 */
void Ieee80211ReassociationResponseFrameSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const {
    auto reassociationResponseFrame = dynamicPtrCast<const Ieee80211ReassociationResponseFrame>(chunk);
    // 1    Capability
    stream.writeUint16Be(0);    //FIXME
    // 2    Status code
    stream.writeUint16Be(reassociationResponseFrame->getStatusCode());
    // 3    AID
    stream.writeUint16Be(reassociationResponseFrame->getAid());
    // 4    Supported rates
    stream.writeByte(1);
    stream.writeByte(reassociationResponseFrame->getSupportedRates().numRates);
    for (int i = 0; i < reassociationResponseFrame->getSupportedRates().numRates; i++)
    {
        uint8_t rate = ceil(reassociationResponseFrame->getSupportedRates().rate[i]/0.5);
        // rate |= 0x80 if rate contained in the BSSBasicRateSet parameter
        stream.writeByte(rate);
    }
    // 5    Extended Supported Rates   The Extended Supported Rates element is present whenever there are more than eight supported rates, and it is optional otherwise.
    // 6    EDCA Parameter Set
    // Last Vendor Specific            One or more vendor-specific information elements may appear in this frame. This information element follows all other information elements.

}

const Ptr<Chunk> Ieee80211ReassociationResponseFrameSerializer::deserialize(MemoryInputStream& stream) const {
    auto reassociationResponseFrame = makeShared<Ieee80211ReassociationResponseFrame>();
    stream.readUint16Be();
    int16_t statusCode = stream.readUint16Be();
    reassociationResponseFrame->setStatusCode(static_cast<Ieee80211StatusCode>(statusCode));
    reassociationResponseFrame->setAid(stream.readUint16Be());

    Ieee80211SupportedRatesElement supRat;
    stream.readByte();
    supRat.numRates = stream.readByte();
    for (int i = 0; i < supRat.numRates; i++)
        supRat.rate[i] = (double)(stream.readByte() & 0x7F) * 0.5;
    reassociationResponseFrame->setSupportedRates(supRat);
    return reassociationResponseFrame;
}

} // namespace ieee80211

} // namespace inet

