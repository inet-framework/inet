//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtFrameSerializer.h"

#include <cmath>

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtFrame_m.h"

namespace inet {

namespace ieee80211 {

Register_Serializer(Ieee80211AssociationRequestFrame, Ieee80211TypedMgmtFrameSerializer<Ieee80211AssociationRequestFrame>);
Register_Serializer(Ieee80211AssociationResponseFrame, Ieee80211TypedMgmtFrameSerializer<Ieee80211AssociationResponseFrame>);
Register_Serializer(Ieee80211AuthenticationFrame, Ieee80211TypedMgmtFrameSerializer<Ieee80211AuthenticationFrame>);
Register_Serializer(Ieee80211BeaconFrame, Ieee80211TypedMgmtFrameSerializer<Ieee80211BeaconFrame>);
Register_Serializer(Ieee80211DeauthenticationFrame, Ieee80211TypedMgmtFrameSerializer<Ieee80211DeauthenticationFrame>);
Register_Serializer(Ieee80211DisassociationFrame, Ieee80211TypedMgmtFrameSerializer<Ieee80211DisassociationFrame>);
Register_Serializer(Ieee80211ProbeRequestFrame, Ieee80211TypedMgmtFrameSerializer<Ieee80211ProbeRequestFrame>);
Register_Serializer(Ieee80211ProbeResponseFrame, Ieee80211TypedMgmtFrameSerializer<Ieee80211ProbeResponseFrame>);
Register_Serializer(Ieee80211ReassociationRequestFrame, Ieee80211TypedMgmtFrameSerializer<Ieee80211ReassociationRequestFrame>);
Register_Serializer(Ieee80211ReassociationResponseFrame, Ieee80211TypedMgmtFrameSerializer<Ieee80211ReassociationResponseFrame>);

static constexpr uint8_t SUPPORTED_RATES_ELEMENT_ID = 1;
static constexpr uint8_t EXTENDED_SUPPORTED_RATES_ELEMENT_ID = 50;
static constexpr uint8_t MAX_SUPPORTED_RATES = 8;
static constexpr uint16_t MAX_EXTENDED_SUPPORTED_RATES = 255;
static constexpr double SUPPORTED_RATE_UNIT = 0.5;
static constexpr double MAX_SUPPORTED_RATE_UNITS = 127;

static void validateSupportedRatesCount(int numRates)
{
    // IEEE Std 802.11-2024, 9.4.2.3: the Supported Rates field contains
    // one to eight octets.
    if (numRates < 1 || numRates > MAX_SUPPORTED_RATES)
        throw cRuntimeError("Malformed Supported Rates element length: %d", numRates);
}

static void validateExtendedSupportedRatesCount(int numRates)
{
    // IEEE Std 802.11-2024, 9.4.2.11: Extended Supported Rates has one to
    // 255 octets when it is present.
    if (numRates < 1 || numRates > MAX_EXTENDED_SUPPORTED_RATES)
        throw cRuntimeError("Malformed Extended Supported Rates element length: %d", numRates);
}

static uint8_t encodeSupportedRate(double rate, bool basicRate)
{
    const double rateUnits = std::ceil(rate / SUPPORTED_RATE_UNIT);
    // IEEE Std 802.11-2024, 9.4.2.3 and 11.1.4.6: a legacy rate is
    // represented in 500 kb/s units, rounded up when necessary, up to
    // 63.5 Mb/s. Bit 7 identifies a basic rate.
    if (!std::isfinite(rate) || rate <= 0 || !std::isfinite(rateUnits) ||
            rateUnits < 1 || rateUnits > MAX_SUPPORTED_RATE_UNITS)
        throw cRuntimeError("Unsupported Supported Rate value: %g Mb/s", rate);
    return static_cast<uint8_t>(rateUnits) | (basicRate ? 0x80 : 0);
}

static void writeSupportedRatesElement(MemoryOutputStream& stream, const Ieee80211SupportedRatesElement& supportedRates)
{
    validateSupportedRatesCount(supportedRates.numRates);
    stream.writeByte(SUPPORTED_RATES_ELEMENT_ID);
    stream.writeByte(supportedRates.numRates);
    for (int i = 0; i < supportedRates.numRates; i++)
        stream.writeByte(encodeSupportedRate(supportedRates.rate[i], supportedRates.basicRate[i]));
}

static void writeExtendedSupportedRatesElement(MemoryOutputStream& stream, const Ieee80211ExtendedSupportedRatesElement& supportedRates)
{
    validateExtendedSupportedRatesCount(supportedRates.numRates);
    stream.writeByte(EXTENDED_SUPPORTED_RATES_ELEMENT_ID);
    stream.writeByte(supportedRates.numRates);
    for (int i = 0; i < supportedRates.numRates; i++)
        stream.writeByte(encodeSupportedRate(supportedRates.rate[i], supportedRates.basicRate[i]));
}

template<typename Frame>
static void writeSupportedRateElements(MemoryOutputStream& stream, const Ptr<const Frame>& frame)
{
    writeSupportedRatesElement(stream, frame->getSupportedRates());
    const auto& extendedSupportedRates = frame->getExtendedSupportedRates();
    if (frame->getExtendedSupportedRatesPresent())
        writeExtendedSupportedRatesElement(stream, extendedSupportedRates);
    else if (extendedSupportedRates.numRates != 0)
        throw cRuntimeError("Extended Supported Rates value is present without its presence flag");
}

static Ieee80211SupportedRatesElement readSupportedRatesElement(MemoryInputStream& stream)
{
    Ieee80211SupportedRatesElement supportedRates;
    const uint8_t elementId = stream.readByte();
    if (elementId != SUPPORTED_RATES_ELEMENT_ID)
        throw cRuntimeError("Expected Supported Rates element, got element ID %d", elementId);
    const uint8_t numRates = stream.readByte();
    validateSupportedRatesCount(numRates);
    if (stream.getRemainingLength() < B(numRates))
        throw cRuntimeError("Malformed Supported Rates element: length=%d remaining=%" PRId64,
                numRates, stream.getRemainingLength().get<B>());
    supportedRates.numRates = numRates;
    for (int i = 0; i < supportedRates.numRates; i++) {
        const uint8_t encodedRate = stream.readByte();
        if ((encodedRate & 0x7F) == 0)
            throw cRuntimeError("Malformed Supported Rates element: zero rate at index %d", i);
        supportedRates.basicRate[i] = (encodedRate & 0x80) != 0;
        supportedRates.rate[i] = (double)(encodedRate & 0x7F) * SUPPORTED_RATE_UNIT;
    }
    return supportedRates;
}

static Ieee80211ExtendedSupportedRatesElement readExtendedSupportedRatesElement(MemoryInputStream& stream, int length, const Ptr<Ieee80211MgmtFrame>& frame)
{
    if (frame->getExtendedSupportedRatesPresent())
        throw cRuntimeError("Duplicate Extended Supported Rates element");
    validateExtendedSupportedRatesCount(length);
    if (stream.getRemainingLength() < B(length))
        throw cRuntimeError("Malformed Extended Supported Rates element: length=%d remaining=%" PRId64,
                length, stream.getRemainingLength().get<B>());
    Ieee80211ExtendedSupportedRatesElement supportedRates;
    supportedRates.numRates = length;
    for (int i = 0; i < length; i++) {
        const uint8_t encodedRate = stream.readByte();
        if ((encodedRate & 0x7F) == 0)
            throw cRuntimeError("Malformed Extended Supported Rates element: zero rate at index %d", i);
        supportedRates.basicRate[i] = (encodedRate & 0x80) != 0;
        supportedRates.rate[i] = (double)(encodedRate & 0x7F) * SUPPORTED_RATE_UNIT;
    }
    frame->setExtendedSupportedRatesPresent(true);
    frame->setExtendedSupportedRates(supportedRates);
    return supportedRates;
}

enum HtElementPresence : unsigned int {
    HT_ELEMENT_NONE = 0,
    EXTENDED_SUPPORTED_RATES_ALLOWED = 4,
};

static void writeHtElements(MemoryOutputStream& stream, const Ptr<const Ieee80211MgmtFrame>& frame, unsigned int allowedElements)
{
    if (!(allowedElements & EXTENDED_SUPPORTED_RATES_ALLOWED) && frame->getExtendedSupportedRatesPresent())
        throw cRuntimeError("Extended Supported Rates element is not allowed in this management frame subtype");
}

static void readHtElements(MemoryInputStream& stream, const Ptr<Ieee80211MgmtFrame>& frame, unsigned int allowedElements)
{
    while (stream.getRemainingLength() != b(0)) {
        if (stream.getRemainingLength() < B(2))
            throw cRuntimeError("Malformed IEEE 802.11 management element header");
        int elementId = stream.readByte();
        int length = stream.readByte();
        if (stream.getRemainingLength() < B(length))
            throw cRuntimeError("Malformed IEEE 802.11 management element: id=%d length=%d remaining=%" PRId64,
                    elementId, length, stream.getRemainingLength().get<B>());
        if (elementId == SUPPORTED_RATES_ELEMENT_ID)
            throw cRuntimeError("Duplicate Supported Rates element");
        else if (elementId == EXTENDED_SUPPORTED_RATES_ELEMENT_ID) {
            if (!(allowedElements & EXTENDED_SUPPORTED_RATES_ALLOWED))
                throw cRuntimeError("Extended Supported Rates element is not allowed in this management frame subtype");
            readExtendedSupportedRatesElement(stream, length, frame);
        }
        else
            for (int i = 0; i < length; i++)
                stream.readByte();
    }
}

void Ieee80211MgmtFrameSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    if (auto authenticationFrame = dynamicPtrCast<const Ieee80211AuthenticationFrame>(chunk)) {
//        type = ST_AUTHENTICATION;
        // 1    Authentication algorithm number
        stream.writeUint16Be(0);
        // 2    Authentication transaction sequence number
        stream.writeUint16Be(authenticationFrame->getSequenceNumber());
        // 3    Status code                                 The status code information is reserved in certain Authentication frames as defined in Table 7-17.
        stream.writeUint16Be(authenticationFrame->getStatusCode());
        writeHtElements(stream, authenticationFrame, HT_ELEMENT_NONE);
        // 4    Challenge text                              The challenge text information is present only in certain Authentication frames as defined in Table 7-17.
        // Last Vendor Specific                             One or more vendor-specific information elements may appear in this frame. This information element follows all other information elements.
    }
    else if (auto deauthenticationFrame = dynamicPtrCast<const Ieee80211DeauthenticationFrame>(chunk)) {
//        type = ST_DEAUTHENTICATION;
        stream.writeUint16Be(deauthenticationFrame->getReasonCode());
        writeHtElements(stream, deauthenticationFrame, HT_ELEMENT_NONE);
    }
    else if (auto disassociationFrame = dynamicPtrCast<const Ieee80211DisassociationFrame>(chunk)) {
//        type = ST_DISASSOCIATION;
        stream.writeUint16Be(disassociationFrame->getReasonCode());
        writeHtElements(stream, disassociationFrame, HT_ELEMENT_NONE);
    }
    else if (auto probeRequestFrame = dynamicPtrCast<const Ieee80211ProbeRequestFrame>(chunk)) {
//        type = ST_PROBEREQUEST;
        // 1    SSID
        const char *SSID = probeRequestFrame->getSSID();
        unsigned int length = strlen(SSID);
        stream.writeByte(0); // FIXME dummy, what is it?
        stream.writeByte(length);
        stream.writeBytes((uint8_t *)SSID, B(length));
        // 2    Supported rates
        writeSupportedRateElements(stream, probeRequestFrame);
        writeHtElements(stream, probeRequestFrame, EXTENDED_SUPPORTED_RATES_ALLOWED);
        // 3    Request information         May be included if dot11MultiDomainCapabilityEnabled is true.
        // 4    Extended Supported Rates    The Extended Supported Rates element is present whenever there are more than eight supported rates, and it is optional otherwise.
        // Last Vendor Specific             One or more vendor-specific information elements may appear in this frame. This information element follows all other information elements.
    }
    else if (auto associationRequestFrame = dynamicPtrCast<const Ieee80211AssociationRequestFrame>(chunk); associationRequestFrame && !dynamicPtrCast<const Ieee80211ReassociationRequestFrame>(chunk)) {
//        type = ST_ASSOCIATIONREQUEST;
        // 1    Capability
        stream.writeUint16Be(0); // FIXME
        // 2    Listen interval
        stream.writeUint16Be(0); // FIXME
        // 3    SSID
        const char *SSID = associationRequestFrame->getSSID();
        unsigned int length = strlen(SSID);
        stream.writeByte(0); // FIXME dummy, what is it?
        stream.writeByte(length);
        stream.writeBytes((uint8_t *)SSID, B(length));
        // 4    Supported rates
        writeSupportedRateElements(stream, associationRequestFrame);
        writeHtElements(stream, associationRequestFrame, EXTENDED_SUPPORTED_RATES_ALLOWED);
        // 5    Extended Supported Rates   The Extended Supported Rates element is present whenever there are more than eight supported rates, and it is optional otherwise.
        // 6    Power Capability           The Power Capability element shall be present if dot11SpectrumManagementRequired is true.
        // 7    Supported Channel          The Supported Channels element shall be present if dot11SpectrumManagementRequired is true.
        // 8    RSN                        The RSN information element is only present within Association Request frames generated by STAs that have dot11RSNAEnabled set to TRUE.
        // 9    QoS Capability             The QoS Capability element is present when dot11QosOption- Implemented is true.
        // Last Vendor Specific            One or more vendor-specific information elements may appear in this frame. This information element follows all other information elements.
    }
    else if (auto reassociationRequestFrame = dynamicPtrCast<const Ieee80211ReassociationRequestFrame>(chunk)) {
//        type = ST_REASSOCIATIONREQUEST;
        // 1    Capability
        stream.writeUint16Be(0); // FIXME
        // 2    Listen interval
        stream.writeUint16Be(0); // FIXME
        // 3    Current AP address
        stream.writeMacAddress(reassociationRequestFrame->getCurrentAP());
        // 4    SSID
        const char *SSID = reassociationRequestFrame->getSSID();
        unsigned int length = strlen(SSID);
        // FIXME buffer.writeByte(buf + packetLength, ???);
        stream.writeByte(0); // FIXME
        stream.writeByte(length);
        stream.writeBytes((uint8_t *)SSID, B(length));
        // 5    Supported rates
        writeSupportedRateElements(stream, reassociationRequestFrame);
        writeHtElements(stream, reassociationRequestFrame, EXTENDED_SUPPORTED_RATES_ALLOWED);
        // 6    Extended Supported Rates   The Extended Supported Rates element is present whenever there are more than eight supported rates, and it is optional otherwise.
        // 7    Power Capability           The Power Capability element shall be present if dot11SpectrumManagementRequired is true.
        // 8    Supported Channels         The Supported Channels element shall be present if dot11SpectrumManagementRequired is true.
        // 9    RSN                        The RSN information element is only present within Reassociation Request frames generated by STAs that have dot11RSNAEnabled set to TRUE.
        // 10   QoS Capability             The QoS Capability element is present when dot11QosOption- Implemented is true.
        // Last Vendor Specific            One or more vendor-specific information elements may appear in this frame. This information element follows all other information elements.
    }
    else if (auto associationResponseFrame = dynamicPtrCast<const Ieee80211AssociationResponseFrame>(chunk); associationResponseFrame && !dynamicPtrCast<const Ieee80211ReassociationResponseFrame>(chunk)) {
//        type = ST_ASSOCIATIONRESPONSE;
        // 1    Capability
        stream.writeUint16Be(0); // FIXME
        // 2    Status code
        stream.writeUint16Be(associationResponseFrame->getStatusCode());
        // 3    AID
        stream.writeUint16Be(associationResponseFrame->getAid());
        // 4    Supported rates
        writeSupportedRateElements(stream, associationResponseFrame);
        writeHtElements(stream, associationResponseFrame, EXTENDED_SUPPORTED_RATES_ALLOWED);
        // 5    Extended Supported Rates   The Extended Supported Rates element is present whenever there are more than eight supported rates, and it is optional otherwise.
        // 6    EDCA Parameter Set
        // Last Vendor Specific            One or more vendor-specific information elements may appear in this frame. This information element follows all other information elements.
    }
    else if (auto reassociationResponseFrame = dynamicPtrCast<const Ieee80211ReassociationResponseFrame>(chunk)) {
//        type = ST_REASSOCIATIONRESPONSE;
        // 1    Capability
        stream.writeUint16Be(0); // FIXME
        // 2    Status code
        stream.writeUint16Be(reassociationResponseFrame->getStatusCode());
        // 3    AID
        stream.writeUint16Be(reassociationResponseFrame->getAid());
        // 4    Supported rates
        writeSupportedRateElements(stream, reassociationResponseFrame);
        writeHtElements(stream, reassociationResponseFrame, EXTENDED_SUPPORTED_RATES_ALLOWED);
        // 5    Extended Supported Rates   The Extended Supported Rates element is present whenever there are more than eight supported rates, and it is optional otherwise.
        // 6    EDCA Parameter Set
        // Last Vendor Specific            One or more vendor-specific information elements may appear in this frame. This information element follows all other information elements.
    }
    else if (auto beaconFrame = dynamicPtrCast<const Ieee80211BeaconFrame>(chunk); beaconFrame && !dynamicPtrCast<const Ieee80211ProbeResponseFrame>(chunk)) {
//        type = ST_BEACON;
        // 1    Timestamp
        stream.writeUint64Be(simTime().raw()); // FIXME
        // 2    Beacon interval
        stream.writeUint16Be((uint16_t)(beaconFrame->getBeaconInterval().inUnit(SIMTIME_US) / 1024));
        // 3    Capability
        stream.writeUint16Be(0); // FIXME set  capability
        // 4    Service Set Identifier (SSID)
        const char *SSID = beaconFrame->getSSID();
        unsigned int length = strlen(SSID);
        stream.writeByte(0); // FIXME
        stream.writeByte(length);
        stream.writeBytes((uint8_t *)SSID, B(length));
        // 5    Supported rates
        writeSupportedRateElements(stream, beaconFrame);
        writeHtElements(stream, beaconFrame, EXTENDED_SUPPORTED_RATES_ALLOWED);
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
    else if (auto probeResponseFrame = dynamicPtrCast<const Ieee80211ProbeResponseFrame>(chunk)) {
//        type = ST_PROBERESPONSE;
        // 1      Timestamp
        stream.writeUint64Be(simTime().raw()); // FIXME
        // 2      Beacon interval
        stream.writeUint16Be((uint16_t)(probeResponseFrame->getBeaconInterval().inUnit(SIMTIME_US) / 1024));
        // 3      Capability
        stream.writeUint16Be(0); // FIXME
        // 4      SSID
        const char *SSID = probeResponseFrame->getSSID();
        unsigned int length = strlen(SSID);
        stream.writeByte(0); // FIXME
        stream.writeByte(length);
        stream.writeBytes((uint8_t *)SSID, B(length));
        // 5      Supported rates
        writeSupportedRateElements(stream, probeResponseFrame);
        writeHtElements(stream, probeResponseFrame, EXTENDED_SUPPORTED_RATES_ALLOWED);
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
    else
        throw cRuntimeError("Cannot serialize frame");
}

const Ptr<Chunk> Ieee80211MgmtFrameSerializer::deserializeFrame(MemoryInputStream& stream, const std::type_info& typeInfo)
{
    int frameType = -1;
    if (typeInfo == typeid(Ieee80211AuthenticationFrame)) frameType = 0xB0;
    else if (typeInfo == typeid(Ieee80211DeauthenticationFrame)) frameType = 0xC0;
    else if (typeInfo == typeid(Ieee80211DisassociationFrame)) frameType = 0xA0;
    else if (typeInfo == typeid(Ieee80211ProbeRequestFrame)) frameType = 0x40;
    else if (typeInfo == typeid(Ieee80211AssociationRequestFrame)) frameType = 0x00;
    else if (typeInfo == typeid(Ieee80211ReassociationRequestFrame)) frameType = 0x02;
    else if (typeInfo == typeid(Ieee80211AssociationResponseFrame)) frameType = 0x01;
    else if (typeInfo == typeid(Ieee80211ReassociationResponseFrame)) frameType = 0x03;
    else if (typeInfo == typeid(Ieee80211BeaconFrame)) frameType = 0x80;
    else if (typeInfo == typeid(Ieee80211ProbeResponseFrame)) frameType = 0x50;
    else throw cRuntimeError("Unsupported IEEE 802.11 management frame type: %s", typeInfo.name());

    switch (frameType) {
        case 0xB0: // ST_AUTHENTICATION
        {
            auto frame = makeShared<Ieee80211AuthenticationFrame>();
            stream.readUint16Be();
            frame->setSequenceNumber(stream.readUint16Be());
            frame->setStatusCode((Ieee80211StatusCode)stream.readUint16Be());
            readHtElements(stream, frame, HT_ELEMENT_NONE);
            return frame;
        }

        case 0xC0: // ST_ST_DEAUTHENTICATION
        {
            auto frame = makeShared<Ieee80211DeauthenticationFrame>();
            frame->setReasonCode((Ieee80211ReasonCode)stream.readUint16Be());
            readHtElements(stream, frame, HT_ELEMENT_NONE);
            return frame;
        }

        case 0xA0: // ST_DISASSOCIATION
        {
            auto frame = makeShared<Ieee80211DisassociationFrame>();
            frame->setReasonCode((Ieee80211ReasonCode)stream.readUint16Be());
            readHtElements(stream, frame, HT_ELEMENT_NONE);
            return frame;
        }

        case 0x40: // ST_PROBEREQUEST
        {
            auto frame = makeShared<Ieee80211ProbeRequestFrame>();

            char SSID[256];
            stream.readByte();
            unsigned int length = stream.readByte();
            stream.readBytes((uint8_t *)SSID, B(length));
            SSID[length] = '\0';
            frame->setSSID(SSID);

            frame->setSupportedRates(readSupportedRatesElement(stream));
            readHtElements(stream, frame, EXTENDED_SUPPORTED_RATES_ALLOWED);
            return frame;
        }

        case 0x00: // ST_ASSOCIATIONREQUEST
        {
            auto frame = makeShared<Ieee80211AssociationRequestFrame>();

            stream.readUint16Be();
            stream.readUint16Be();

            char SSID[256];
            stream.readByte();
            unsigned int length = stream.readByte();
            stream.readBytes((uint8_t *)SSID, B(length));
            SSID[length] = '\0';
            frame->setSSID(SSID);

            frame->setSupportedRates(readSupportedRatesElement(stream));
            readHtElements(stream, frame, EXTENDED_SUPPORTED_RATES_ALLOWED);
            return frame;
        }

        case 0x02: // ST_REASSOCIATIONREQUEST
        {
            auto frame = makeShared<Ieee80211ReassociationRequestFrame>();
            stream.readUint16Be();
            stream.readUint16Be();

            frame->setCurrentAP(stream.readMacAddress());

            char SSID[256];
            stream.readByte();
            unsigned int length = stream.readByte();
            stream.readBytes((uint8_t *)SSID, B(length));
            SSID[length] = '\0';
            frame->setSSID(SSID);

            frame->setSupportedRates(readSupportedRatesElement(stream));
            readHtElements(stream, frame, EXTENDED_SUPPORTED_RATES_ALLOWED);
            return frame;
        }

        case 0x01: // ST_ASSOCIATIONRESPONSE
        {
            auto frame = makeShared<Ieee80211AssociationResponseFrame>();
            stream.readUint16Be();
            frame->setStatusCode((Ieee80211StatusCode)stream.readUint16Be());
            frame->setAid(stream.readUint16Be());

            frame->setSupportedRates(readSupportedRatesElement(stream));
            readHtElements(stream, frame, EXTENDED_SUPPORTED_RATES_ALLOWED);
            return frame;
        }

        case 0x03: // ST_REASSOCIATIONRESPONSE
        {
            auto frame = makeShared<Ieee80211ReassociationResponseFrame>();
            stream.readUint16Be();
            frame->setStatusCode((Ieee80211StatusCode)stream.readUint16Be());
            frame->setAid(stream.readUint16Be());

            frame->setSupportedRates(readSupportedRatesElement(stream));
            readHtElements(stream, frame, EXTENDED_SUPPORTED_RATES_ALLOWED);
            return frame;
        }

        case 0x80: // ST_BEACON
        {
            auto frame = makeShared<Ieee80211BeaconFrame>();

            simtime_t timetstamp;
            timetstamp.setRaw(stream.readUint64Be()); // TODO store timestamp

            frame->setBeaconInterval(SimTime((int64_t)stream.readUint16Be() * 1024, SIMTIME_US));
            stream.readUint16Be(); // Capability

            char SSID[256];
            stream.readByte();
            unsigned int length = stream.readByte();
            stream.readBytes((uint8_t *)SSID, B(length));
            SSID[length] = '\0';
            frame->setSSID(SSID);

            frame->setSupportedRates(readSupportedRatesElement(stream));
            readHtElements(stream, frame, EXTENDED_SUPPORTED_RATES_ALLOWED);
            return frame;
        }

        case 0x50: // ST_PROBERESPONSE
        {
            auto frame = makeShared<Ieee80211ProbeResponseFrame>();

            simtime_t timestamp;
            timestamp.setRaw(stream.readUint64Be()); // TODO store timestamp

            frame->setBeaconInterval(SimTime((int64_t)stream.readUint16Be() * 1024, SIMTIME_US));
            stream.readUint16Be();

            char SSID[256];
            stream.readByte();
            unsigned int length = stream.readByte();
            stream.readBytes((uint8_t *)SSID, B(length));
            SSID[length] = '\0';
            frame->setSSID(SSID);

            frame->setSupportedRates(readSupportedRatesElement(stream));
            readHtElements(stream, frame, EXTENDED_SUPPORTED_RATES_ALLOWED);
            return frame;
        }

        default:
            throw cRuntimeError("Cannot deserialize frame");
    }
}

} // namespace ieee80211

} // namespace inet
