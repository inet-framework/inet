//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtFrameSerializer.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211CapabilityElements.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtFrame_m.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211BeaconInterval.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MeshFrameSerializer.h"

namespace inet {

namespace ieee80211 {

// IEEE Std 802.11-2024, 9.2.2: fixed-width numeric management fields use least-significant octet first.

Register_Serializer(Ieee80211AssociationRequestFrame, Ieee80211MgmtFrameSerializer);
Register_Serializer(Ieee80211AssociationResponseFrame, Ieee80211MgmtFrameSerializer);
Register_Serializer(Ieee80211AuthenticationFrame, Ieee80211MgmtFrameSerializer);
Register_Serializer(Ieee80211BeaconFrame, Ieee80211MgmtFrameSerializer);
Register_Serializer(Ieee80211DeauthenticationFrame, Ieee80211MgmtFrameSerializer);
Register_Serializer(Ieee80211DisassociationFrame, Ieee80211MgmtFrameSerializer);
Register_Serializer(Ieee80211ProbeRequestFrame, Ieee80211MgmtFrameSerializer);
Register_Serializer(Ieee80211ProbeResponseFrame, Ieee80211MgmtFrameSerializer);
Register_Serializer(Ieee80211ReassociationRequestFrame, Ieee80211MgmtFrameSerializer);
Register_Serializer(Ieee80211ReassociationResponseFrame, Ieee80211MgmtFrameSerializer);

static constexpr uint8_t HT_CAPABILITIES_ELEMENT_ID = 45;
static constexpr uint8_t DSSS_PARAMETER_SET_ELEMENT_ID = 3;
static constexpr uint8_t HT_OPERATION_ELEMENT_ID = 61;
static constexpr uint8_t SUPPORTED_RATES_ELEMENT_ID = 1;
static constexpr uint8_t EXTENDED_SUPPORTED_RATES_ELEMENT_ID = 50;
static constexpr uint8_t MAX_SUPPORTED_RATES = 8;
static constexpr uint16_t MAX_EXTENDED_SUPPORTED_RATES = 255;
static constexpr double SUPPORTED_RATE_UNIT = 0.5;
static constexpr double MAX_SUPPORTED_RATE_UNITS = 127;
static constexpr uint16_t ASSOCIATION_ID_MARKER = 0xC000;
static constexpr uint16_t ASSOCIATION_ID_MASK = 0x3FFF;
static constexpr int MAX_LOGICAL_ASSOCIATION_ID = 2007;

// Writes the information elements of a management frame body in wire order: the
// modelled elements in the order this serializer emits them, and the unmodelled
// elements a deserialized frame carries back in the places they had among them.
class ElementWriter
{
  private:
    MemoryOutputStream& stream;
    const Ieee80211MgmtFrame& frame;
    size_t modelledElementCount = 0;
    size_t unmodelledElementIndex = 0;
    size_t unmodelledElementOffset = 0;

    void writeUnmodelledElements(size_t precedingModelledElementCount)
    {
        size_t numOctets = frame.getUnmodelledElementsArraySize();
        while (unmodelledElementIndex < frame.getUnmodelledElementPositionsArraySize() &&
                frame.getUnmodelledElementPositions(unmodelledElementIndex) <= precedingModelledElementCount) {
            if (unmodelledElementOffset + 2 > numOctets ||
                    unmodelledElementOffset + 2 + frame.getUnmodelledElements(unmodelledElementOffset + 1) > numOctets)
                throw cRuntimeError("Cannot serialize %s: unmodelledElements holds fewer than the %d elements unmodelledElementPositions describes",
                        frame.getClassName(), (int)frame.getUnmodelledElementPositionsArraySize());
            size_t end = unmodelledElementOffset + 2 + frame.getUnmodelledElements(unmodelledElementOffset + 1);
            for (; unmodelledElementOffset < end; unmodelledElementOffset++)
                stream.writeByte(frame.getUnmodelledElements(unmodelledElementOffset));
            unmodelledElementIndex++;
        }
    }

  public:
    ElementWriter(MemoryOutputStream& stream, const Ieee80211MgmtFrame& frame) : stream(stream), frame(frame) {}

    // to be called right before writing each modelled element
    void beginModelledElement() { writeUnmodelledElements(modelledElementCount++); }

    // to be called after the last modelled element
    void finish()
    {
        writeUnmodelledElements(SIZE_MAX);
        if (unmodelledElementOffset != frame.getUnmodelledElementsArraySize())
            throw cRuntimeError("Cannot serialize %s: unmodelledElements holds more octets than the %d elements unmodelledElementPositions describes",
                    frame.getClassName(), (int)frame.getUnmodelledElementPositionsArraySize());
    }
};

// Keeps an element this model does not represent, verbatim, together with the number of
// modelled elements that preceded it on the wire.
static void readUnmodelledElement(MemoryInputStream& stream, const Ptr<Ieee80211MgmtFrame>& frame, uint8_t elementId, uint8_t length, int precedingModelledElementCount)
{
    size_t offset = frame->getUnmodelledElementsArraySize();
    frame->setUnmodelledElementsArraySize(offset + 2 + length);
    frame->setUnmodelledElements(offset, elementId);
    frame->setUnmodelledElements(offset + 1, length);
    for (int i = 0; i < length; i++)
        frame->setUnmodelledElements(offset + 2 + i, stream.readByte());
    size_t index = frame->getUnmodelledElementPositionsArraySize();
    frame->setUnmodelledElementPositionsArraySize(index + 1);
    frame->setUnmodelledElementPositions(index, precedingModelledElementCount);
}

static void validateSupportedRatesCount(int numRates)
{
    // IEEE Std 802.11-2024, 9.4.2.3: the Supported Rates field contains
    // one to eight octets.
    if (numRates < 1 || numRates > MAX_SUPPORTED_RATES)
        throw cRuntimeError("Malformed Supported Rates element length: %d", numRates);
}

static void writeDsssParameterSet(MemoryOutputStream& stream, ElementWriter& elements, const Ptr<const Ieee80211BeaconFrame>& frame)
{
    // IEEE Std 802.11-2024, 9.4.2.4: one-octet Current Channel.
    int channel = frame->getChannelNumber();
    if (channel == -1)
        return;
    if (channel < 1 || channel > 255)
        throw cRuntimeError("Invalid DSSS Parameter Set Current Channel: %d", channel);
    elements.beginModelledElement();
    stream.writeByte(DSSS_PARAMETER_SET_ELEMENT_ID);
    stream.writeByte(1);
    stream.writeByte(channel);
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
static void writeSupportedRateElements(MemoryOutputStream& stream, ElementWriter& elements, const Ptr<const Frame>& frame)
{
    elements.beginModelledElement();
    writeSupportedRatesElement(stream, frame->getSupportedRates());
    if (auto beacon = dynamicPtrCast<const Ieee80211BeaconFrame>(frame))
        writeDsssParameterSet(stream, elements, beacon);
    const auto& extendedSupportedRates = frame->getExtendedSupportedRates();
    if (frame->getExtendedSupportedRatesPresent()) {
        elements.beginModelledElement();
        writeExtendedSupportedRatesElement(stream, extendedSupportedRates);
    }
    else if (extendedSupportedRates.numRates != 0)
        throw cRuntimeError("Extended Supported Rates value is present without its presence flag");
}

static void readExtendedSupportedRatesElement(MemoryInputStream& stream, int length, const Ptr<Ieee80211MgmtFrame>& frame)
{
    if (frame->getExtendedSupportedRatesPresent()) {
        frame->markIncorrect();
        stream.seek(stream.getPosition() + B(length));
        return;
    }
    if (length < 1 || length > MAX_EXTENDED_SUPPORTED_RATES) {
        frame->markIncorrect();
        stream.seek(stream.getPosition() + B(length));
        return;
    }

    Ieee80211ExtendedSupportedRatesElement supportedRates;
    supportedRates.numRates = length;
    for (int i = 0; i < length; i++) {
        const uint8_t encodedRate = stream.readByte();
        if ((encodedRate & 0x7F) == 0)
            frame->markIncorrect();
        supportedRates.basicRate[i] = (encodedRate & 0x80) != 0;
        supportedRates.rate[i] = (double)(encodedRate & 0x7F) * SUPPORTED_RATE_UNIT;
    }
    frame->setExtendedSupportedRatesPresent(true);
    frame->setExtendedSupportedRates(supportedRates);
}


enum HtElementPresence : unsigned int {
    HT_ELEMENT_NONE = 0,
    HT_CAPABILITIES_ALLOWED = 1,
    HT_OPERATION_ALLOWED = 2,
    EXTENDED_SUPPORTED_RATES_ALLOWED = 4,
    BASIC_HT_MCS_SET_PRESENT = 8,
};

static void writeHtElements(MemoryOutputStream& stream, ElementWriter& elements, const Ptr<const Ieee80211MgmtFrame>& frame, unsigned int allowedElements)
{
    if (!(allowedElements & EXTENDED_SUPPORTED_RATES_ALLOWED) && frame->getExtendedSupportedRatesPresent())
        throw cRuntimeError("Extended Supported Rates element is not allowed in this management frame subtype");
    if (!(allowedElements & HT_CAPABILITIES_ALLOWED) && frame->getHtCapabilitiesPresent())
        throw cRuntimeError("HT Capabilities element is not allowed in this management frame subtype");
    if (!(allowedElements & HT_OPERATION_ALLOWED) && frame->getHtOperationPresent())
        throw cRuntimeError("HT Operation element is not allowed in this management frame subtype");
    if (frame->getHtCapabilitiesPresent()) {
        elements.beginModelledElement();
        writeHtCapabilitiesElement(stream, frame->getHtCapabilities());
    }
    if (frame->getHtOperationPresent()) {
        elements.beginModelledElement();
        writeHtOperationElement(stream, frame->getHtOperation(), allowedElements & BASIC_HT_MCS_SET_PRESENT);
    }
}


static void readHtElements(MemoryInputStream& stream, const Ptr<Ieee80211MgmtFrame>& frame, unsigned int allowedElements, int modelledElementCount)
{
    // TIM remains opaque for legacy management. Mesh beacons can place it before
    // Mesh ID/Configuration, so identify the mesh profile before decoding it.
    bool meshElementsAllowed = false;
    MemoryInputStream probe(stream.getData(), stream.getLength(), stream.getPosition());
    while (probe.getRemainingLength() >= B(2)) {
        auto id = probe.readByte();
        auto length = B(probe.readByte());
        if (probe.getRemainingLength() < length)
            break;
        if (id == 113 || id == 114)
            meshElementsAllowed = true;
        probe.seek(probe.getPosition() + length);
    }
    while (stream.getRemainingLength() != b(0)) {
        if (stream.getRemainingLength() < B(2)) {
            frame->markIncorrect();
            frame->markIncomplete();
            stream.seek(stream.getLength());
            return;
        }

        const int elementId = stream.readByte();
        const int length = stream.readByte();
        const b declaredBodyLength = B(length);
        if (stream.getRemainingLength() < declaredBodyLength) {
            frame->markIncorrect();
            frame->markIncomplete();
            stream.seek(stream.getLength());
            return;
        }

        if (elementId == SUPPORTED_RATES_ELEMENT_ID) {
            frame->markIncorrect();
            stream.seek(stream.getPosition() + declaredBodyLength);
        }
        else if (elementId == DSSS_PARAMETER_SET_ELEMENT_ID) {
            auto beacon = dynamicPtrCast<Ieee80211BeaconFrame>(frame);
            if (beacon == nullptr)
                readUnmodelledElement(stream, frame, elementId, length, modelledElementCount);
            else if (length != 1 || beacon->getChannelNumber() != -1) {
                frame->markIncorrect();
                stream.seek(stream.getPosition() + declaredBodyLength);
            }
            else {
                int channel = stream.readByte();
                beacon->setChannelNumber(channel);
                if (channel == 0)
                    frame->markIncorrect();
                modelledElementCount++;
            }
        }
        else if (elementId == EXTENDED_SUPPORTED_RATES_ELEMENT_ID) {
            if (!(allowedElements & EXTENDED_SUPPORTED_RATES_ALLOWED)) {
                frame->markIncorrect();
                stream.seek(stream.getPosition() + declaredBodyLength);
            }
            else {
                bool wasPresent = frame->getExtendedSupportedRatesPresent();
                readExtendedSupportedRatesElement(stream, length, frame);
                if (!wasPresent && frame->getExtendedSupportedRatesPresent())
                    modelledElementCount++;
            }
        }
        else if (elementId == HT_CAPABILITIES_ELEMENT_ID) {
            if (!(allowedElements & HT_CAPABILITIES_ALLOWED)) {
                frame->markIncorrect();
                stream.seek(stream.getPosition() + declaredBodyLength);
            }
            else {
                bool wasPresent = frame->getHtCapabilitiesPresent();
                readHtCapabilitiesElement(stream, length, *frame);
                if (!wasPresent && frame->getHtCapabilitiesPresent())
                    modelledElementCount++;
            }
        }
        else if (elementId == HT_OPERATION_ELEMENT_ID) {
            if (!(allowedElements & HT_OPERATION_ALLOWED)) {
                frame->markIncorrect();
                stream.seek(stream.getPosition() + declaredBodyLength);
            }
            else {
                bool wasPresent = frame->getHtOperationPresent();
                readHtOperationElement(stream, length, *frame, allowedElements & BASIC_HT_MCS_SET_PRESENT);
                if (!wasPresent && frame->getHtOperationPresent())
                    modelledElementCount++;
            }
        }
        else if (meshElementsAllowed && Ieee80211MeshFrameSerializer::isMeshElement(elementId)) {
            auto end = stream.getPosition() + declaredBodyLength;
            if (!Ieee80211MeshFrameSerializer::readElement(stream, elementId, length, *frame))
                frame->markIncorrect();
            stream.seek(end);
            modelledElementCount++;
        }
        else
            readUnmodelledElement(stream, frame, elementId, length, modelledElementCount);
    }
}

namespace {

// IEEE Std 802.11-2024, 9.4.2.1, 9.4.2.2, Table 9-130, and Figure 9-209:
// SSID has EID 0, a one-octet length, and a 0-32 octet body.
static std::string deserializeSsid(MemoryInputStream& stream, Ieee80211MgmtFrame& frame)
{
    constexpr uint8_t ssidElementId = 0;
    constexpr uint8_t maxSsidLength = 32;
    const b elementHeaderLength = B(2);

    if (stream.getRemainingLength() < elementHeaderLength) {
        stream.seek(stream.getLength());
        frame.markIncorrect();
        frame.markIncomplete();
        return {};
    }

    const uint8_t elementId = stream.readByte();
    const uint8_t length = stream.readByte();
    const b declaredBodyLength = B(length);

    if (elementId != ssidElementId || length > maxSsidLength) {
        frame.markIncorrect();
        if (stream.getRemainingLength() < declaredBodyLength) {
            frame.markIncomplete();
            stream.seek(stream.getLength());
        }
        else
            stream.seek(stream.getPosition() + declaredBodyLength);
        return {};
    }

    if (stream.getRemainingLength() < declaredBodyLength) {
        frame.markIncorrect();
        frame.markIncomplete();
        stream.seek(stream.getLength());
        return {};
    }

    char ssid[maxSsidLength + 1] = {};
    stream.readBytes(reinterpret_cast<uint8_t *>(ssid), declaredBodyLength);
    return std::string(ssid, length);
}

// IEEE Std 802.11-2024, 9.4.2.1, 9.4.2.3, Table 9-130, Figures 9-208 and 9-210:
// Supported Rates has EID 1, a one-octet length, and one octet per rate, with length 1-8.
static void deserializeSupportedRates(MemoryInputStream& stream, Ieee80211MgmtFrame& frame, Ieee80211SupportedRatesElement& supportedRates)
{
    constexpr uint8_t supportedRatesElementId = 1;
    constexpr uint8_t maxSupportedRates = 8;
    const b elementHeaderLength = B(2);

    if (stream.getRemainingLength() < elementHeaderLength) {
        stream.seek(stream.getLength());
        frame.markIncorrect();
        frame.markIncomplete();
        return;
    }

    const uint8_t elementId = stream.readByte();
    const uint8_t count = stream.readByte();
    const b declaredBodyLength = B(count);

    if (elementId != supportedRatesElementId || count == 0 || count > maxSupportedRates) {
        frame.markIncorrect();
        if (stream.getRemainingLength() < declaredBodyLength) {
            frame.markIncomplete();
            stream.seek(stream.getLength());
        }
        else
            stream.seek(stream.getPosition() + declaredBodyLength);
        return;
    }

    if (stream.getRemainingLength() < declaredBodyLength) {
        frame.markIncorrect();
        frame.markIncomplete();
        stream.seek(stream.getLength());
        return;
    }

    supportedRates.numRates = count;
    for (uint8_t i = 0; i < count; i++) {
        const uint8_t encodedRate = stream.readByte();
        if ((encodedRate & 0x7F) == 0)
            frame.markIncorrect();
        supportedRates.basicRate[i] = (encodedRate & 0x80) != 0;
        supportedRates.rate[i] = (double)(encodedRate & 0x7F) * SUPPORTED_RATE_UNIT;
    }
}

static uint16_t encodeAssociationId(Ieee80211StatusCode statusCode, int aid)
{
    if (statusCode == SC_SUCCESSFUL) {
        if (aid < 1 || aid > MAX_LOGICAL_ASSOCIATION_ID)
            throw cRuntimeError("Malformed successful Association Response AID: %d", aid);
        return ASSOCIATION_ID_MARKER | static_cast<uint16_t>(aid);
    }
    // This model uses zero for unsuccessful response AIDs. Keep that
    // policy explicit at the wire boundary instead of accepting a stale ID.
    if (aid != 0)
        throw cRuntimeError("Malformed unsuccessful Association Response AID: %d", aid);
    return 0;
}

static int decodeAssociationId(Ieee80211StatusCode statusCode, uint16_t wireAid)
{
    if (statusCode == SC_SUCCESSFUL) {
        if ((wireAid & ASSOCIATION_ID_MARKER) != ASSOCIATION_ID_MARKER)
            throw cRuntimeError("Malformed successful Association Response AID: missing marker 0xC000");
        const int aid = wireAid & ASSOCIATION_ID_MASK;
        if (aid < 1 || aid > MAX_LOGICAL_ASSOCIATION_ID)
            throw cRuntimeError("Malformed successful Association Response AID: %d", aid);
        return aid;
    }
    if (wireAid != 0)
        throw cRuntimeError("Malformed unsuccessful Association Response AID: expected zero, got 0x%04x", wireAid);
    return 0;
}

} // namespace

// IEEE Std 802.11-2024, 9.2.2: octets in multi-octet numeric fields are transmitted
// from the least significant octet to the most significant octet.
void Ieee80211MgmtFrameSerializer::serializeFields(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    ElementWriter elements(stream, *staticPtrCast<const Ieee80211MgmtFrame>(chunk));
    if (auto authenticationFrame = dynamicPtrCast<const Ieee80211AuthenticationFrame>(chunk)) {
//        type = ST_AUTHENTICATION;
        // 1    Authentication algorithm number
        stream.writeUint16Le(0);
        // 2    Authentication transaction sequence number
        stream.writeUint16Le(authenticationFrame->getSequenceNumber());
        // 3    Status code                                 The status code information is reserved in certain Authentication frames as defined in Table 7-17.
        stream.writeUint16Le(authenticationFrame->getStatusCode());
        // 4    Challenge text                              The challenge text information is present only in certain Authentication frames as defined in Table 7-17.
        // Last Vendor Specific                             One or more vendor-specific information elements may appear in this frame. This information element follows all other information elements.
        writeHtElements(stream, elements, authenticationFrame, HT_ELEMENT_NONE);
    }
    else if (auto deauthenticationFrame = dynamicPtrCast<const Ieee80211DeauthenticationFrame>(chunk)) {
//        type = ST_DEAUTHENTICATION;
        stream.writeUint16Le(deauthenticationFrame->getReasonCode());
        writeHtElements(stream, elements, deauthenticationFrame, HT_ELEMENT_NONE);
    }
    else if (auto disassociationFrame = dynamicPtrCast<const Ieee80211DisassociationFrame>(chunk)) {
//        type = ST_DISASSOCIATION;
        stream.writeUint16Le(disassociationFrame->getReasonCode());
        writeHtElements(stream, elements, disassociationFrame, HT_ELEMENT_NONE);
    }
    else if (auto probeRequestFrame = dynamicPtrCast<const Ieee80211ProbeRequestFrame>(chunk)) {
//        type = ST_PROBEREQUEST;
        // 1    SSID
        const char *SSID = probeRequestFrame->getSSID();
        unsigned int length = strlen(SSID);
        elements.beginModelledElement();
        stream.writeByte(0); // FIXME dummy, what is it?
        stream.writeByte(length);
        stream.writeBytes((uint8_t *)SSID, B(length));
        // 2    Supported rates
        writeSupportedRateElements(stream, elements, probeRequestFrame);
        writeHtElements(stream, elements, probeRequestFrame, HT_CAPABILITIES_ALLOWED | EXTENDED_SUPPORTED_RATES_ALLOWED);
        // 3    Request information         May be included if dot11MultiDomainCapabilityEnabled is true.
        // 4    Extended Supported Rates    The Extended Supported Rates element is present whenever there are more than eight supported rates, and it is optional otherwise.
        // Last Vendor Specific             One or more vendor-specific information elements may appear in this frame. This information element follows all other information elements.
    }
    else if (auto reassociationRequestFrame = dynamicPtrCast<const Ieee80211ReassociationRequestFrame>(chunk)) {
//        type = ST_REASSOCIATIONREQUEST;
        // 1    Capability
        stream.writeUint16Le(reassociationRequestFrame->getCapabilityInformation());
        // 2    Listen interval
        stream.writeUint16Le(reassociationRequestFrame->getListenInterval());
        // 3    Current AP address
        stream.writeMacAddress(reassociationRequestFrame->getCurrentAP());
        // 4    SSID
        const char *SSID = reassociationRequestFrame->getSSID();
        unsigned int length = strlen(SSID);
        // FIXME buffer.writeByte(buf + packetLength, ???);
        elements.beginModelledElement();
        stream.writeByte(0); // FIXME
        stream.writeByte(length);
        stream.writeBytes((uint8_t *)SSID, B(length));
        // 5    Supported rates
        writeSupportedRateElements(stream, elements, reassociationRequestFrame);
        writeHtElements(stream, elements, reassociationRequestFrame, HT_CAPABILITIES_ALLOWED | EXTENDED_SUPPORTED_RATES_ALLOWED);
        // 6    Extended Supported Rates   The Extended Supported Rates element is present whenever there are more than eight supported rates, and it is optional otherwise.
        // 7    Power Capability           The Power Capability element shall be present if dot11SpectrumManagementRequired is true.
        // 8    Supported Channels         The Supported Channels element shall be present if dot11SpectrumManagementRequired is true.
        // 9    RSN                        The RSN information element is only present within Reassociation Request frames generated by STAs that have dot11RSNAEnabled set to TRUE.
        // 10   QoS Capability             The QoS Capability element is present when dot11QosOption- Implemented is true.
        // Last Vendor Specific            One or more vendor-specific information elements may appear in this frame. This information element follows all other information elements.
    }
    else if (auto associationRequestFrame = dynamicPtrCast<const Ieee80211AssociationRequestFrame>(chunk)) {
//        type = ST_ASSOCIATIONREQUEST;
        // 1    Capability
        stream.writeUint16Le(associationRequestFrame->getCapabilityInformation());
        // 2    Listen interval
        stream.writeUint16Le(associationRequestFrame->getListenInterval());
        // 3    SSID
        const char *SSID = associationRequestFrame->getSSID();
        unsigned int length = strlen(SSID);
        elements.beginModelledElement();
        stream.writeByte(0); // FIXME dummy, what is it?
        stream.writeByte(length);
        stream.writeBytes((uint8_t *)SSID, B(length));
        // 4    Supported rates
        writeSupportedRateElements(stream, elements, associationRequestFrame);
        writeHtElements(stream, elements, associationRequestFrame, HT_CAPABILITIES_ALLOWED | EXTENDED_SUPPORTED_RATES_ALLOWED);
        // 5    Extended Supported Rates   The Extended Supported Rates element is present whenever there are more than eight supported rates, and it is optional otherwise.
        // 6    Power Capability           The Power Capability element shall be present if dot11SpectrumManagementRequired is true.
        // 7    Supported Channel          The Supported Channels element shall be present if dot11SpectrumManagementRequired is true.
        // 8    RSN                        The RSN information element is only present within Association Request frames generated by STAs that have dot11RSNAEnabled set to TRUE.
        // 9    QoS Capability             The QoS Capability element is present when dot11QosOption- Implemented is true.
        // Last Vendor Specific            One or more vendor-specific information elements may appear in this frame. This information element follows all other information elements.
    }
    else if (auto associationResponseFrame = dynamicPtrCast<const Ieee80211AssociationResponseFrame>(chunk); associationResponseFrame && !dynamicPtrCast<const Ieee80211ReassociationResponseFrame>(chunk)) {
//        type = ST_ASSOCIATIONRESPONSE;
        // 1    Capability
        stream.writeUint16Le(associationResponseFrame->getCapabilityInformation());
        // 2    Status code
        stream.writeUint16Le(associationResponseFrame->getStatusCode());
        // 3    AID
        stream.writeUint16Le(encodeAssociationId(associationResponseFrame->getStatusCode(), associationResponseFrame->getAid()));
        // 4    Supported rates
        writeSupportedRateElements(stream, elements, associationResponseFrame);
        writeHtElements(stream, elements, associationResponseFrame, HT_CAPABILITIES_ALLOWED | HT_OPERATION_ALLOWED | EXTENDED_SUPPORTED_RATES_ALLOWED);
        // 5    Extended Supported Rates   The Extended Supported Rates element is present whenever there are more than eight supported rates, and it is optional otherwise.
        // 6    EDCA Parameter Set
        // Last Vendor Specific            One or more vendor-specific information elements may appear in this frame. This information element follows all other information elements.
    }
    else if (auto reassociationResponseFrame = dynamicPtrCast<const Ieee80211ReassociationResponseFrame>(chunk)) {
//        type = ST_REASSOCIATIONRESPONSE;
        // 1    Capability
        stream.writeUint16Le(reassociationResponseFrame->getCapabilityInformation());
        // 2    Status code
        stream.writeUint16Le(reassociationResponseFrame->getStatusCode());
        // 3    AID
        stream.writeUint16Le(encodeAssociationId(reassociationResponseFrame->getStatusCode(), reassociationResponseFrame->getAid()));
        // 4    Supported rates
        writeSupportedRateElements(stream, elements, reassociationResponseFrame);
        writeHtElements(stream, elements, reassociationResponseFrame, HT_CAPABILITIES_ALLOWED | HT_OPERATION_ALLOWED | EXTENDED_SUPPORTED_RATES_ALLOWED);
        // 5    Extended Supported Rates   The Extended Supported Rates element is present whenever there are more than eight supported rates, and it is optional otherwise.
        // 6    EDCA Parameter Set
        // Last Vendor Specific            One or more vendor-specific information elements may appear in this frame. This information element follows all other information elements.
    }
    else if (auto beaconFrame = dynamicPtrCast<const Ieee80211BeaconFrame>(chunk); beaconFrame && !dynamicPtrCast<const Ieee80211ProbeResponseFrame>(chunk)) {
//        type = ST_BEACON;
        // 1    Timestamp
        // IEEE Std 802.11-2024, 9.4.1.10 and 11.1.3.1: the TSF timer counts in microseconds.
        stream.writeUint64Le(beaconFrame->getHasTimestamp() ? beaconFrame->getTimestamp() : simTime().inUnit(SIMTIME_US));
        // 2    Beacon interval
        stream.writeUint16Le(normalizeIeee80211BeaconInterval(beaconFrame->getBeaconInterval()).inUnit(SIMTIME_US) / 1024);
        // 3    Capability
        stream.writeUint16Le(beaconFrame->getCapabilityInformation());
        // 4    Service Set Identifier (SSID)
        const char *SSID = beaconFrame->getSSID();
        unsigned int length = strlen(SSID);
        elements.beginModelledElement();
        stream.writeByte(0); // FIXME
        stream.writeByte(length);
        stream.writeBytes((uint8_t *)SSID, B(length));
        // 5    Supported rates
        writeSupportedRateElements(stream, elements, beaconFrame);
        writeHtElements(stream, elements, beaconFrame, HT_CAPABILITIES_ALLOWED | HT_OPERATION_ALLOWED | EXTENDED_SUPPORTED_RATES_ALLOWED | BASIC_HT_MCS_SET_PRESENT);
        // 6    Frequency-Hopping (FH) Parameter Set   The FH Parameter Set information element is present within Beacon frames generated by STAs using FH PHYs.
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
        // IEEE Std 802.11-2024, 9.4.1.10 and 11.1.3.1: the TSF timer counts in microseconds.
        stream.writeUint64Le(probeResponseFrame->getHasTimestamp() ? probeResponseFrame->getTimestamp() : simTime().inUnit(SIMTIME_US));
        // 2      Beacon interval
        stream.writeUint16Le(normalizeIeee80211BeaconInterval(probeResponseFrame->getBeaconInterval()).inUnit(SIMTIME_US) / 1024);
        // 3      Capability
        stream.writeUint16Le(probeResponseFrame->getCapabilityInformation());
        // 4      SSID
        const char *SSID = probeResponseFrame->getSSID();
        unsigned int length = strlen(SSID);
        elements.beginModelledElement();
        stream.writeByte(0); // FIXME
        stream.writeByte(length);
        stream.writeBytes((uint8_t *)SSID, B(length));
        // 5      Supported rates
        writeSupportedRateElements(stream, elements, probeResponseFrame);
        writeHtElements(stream, elements, probeResponseFrame, HT_CAPABILITIES_ALLOWED | HT_OPERATION_ALLOWED | EXTENDED_SUPPORTED_RATES_ALLOWED | BASIC_HT_MCS_SET_PRESENT);
        // 6      FH Parameter Set                The FH Parameter Set information element is present within Probe Response frames generated by STAs using FH PHYs.
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
    Ieee80211MeshFrameSerializer::writeElements(stream, *staticPtrCast<const Ieee80211MgmtFrame>(chunk), [&]() { elements.beginModelledElement(); }, false);
    elements.finish();
}

// IEEE Std 802.11-2024, 9.2.2, 9.3.3.2 and 9.3.3.5-9.3.3.10, Tables 9-62 and 9-64-9-69:
// Numeric fields are decoded least-significant octet first and management-body fields follow each frame layout.
const Ptr<Chunk> Ieee80211MgmtFrameSerializer::deserializeFields(MemoryInputStream& stream, const std::type_info& typeInfo) const
{
    if (typeInfo == typeid(Ieee80211AuthenticationFrame)) {
        auto frame = makeShared<Ieee80211AuthenticationFrame>();
        stream.readUint16Le();
        frame->setSequenceNumber(stream.readUint16Le());
        frame->setStatusCode((Ieee80211StatusCode)stream.readUint16Le());
        readHtElements(stream, frame, HT_ELEMENT_NONE, 0);
        return frame;
    }
    else if (typeInfo == typeid(Ieee80211DeauthenticationFrame)) {
        auto frame = makeShared<Ieee80211DeauthenticationFrame>();
        frame->setReasonCode((Ieee80211ReasonCode)stream.readUint16Le());
        readHtElements(stream, frame, HT_ELEMENT_NONE, 0);
        return frame;
    }
    else if (typeInfo == typeid(Ieee80211DisassociationFrame)) {
        auto frame = makeShared<Ieee80211DisassociationFrame>();
        frame->setReasonCode((Ieee80211ReasonCode)stream.readUint16Le());
        readHtElements(stream, frame, HT_ELEMENT_NONE, 0);
        return frame;
    }
    else if (typeInfo == typeid(Ieee80211ProbeRequestFrame)) {
        auto frame = makeShared<Ieee80211ProbeRequestFrame>();

        frame->setSSID(deserializeSsid(stream, *frame).c_str());

        Ieee80211SupportedRatesElement supRat;
        deserializeSupportedRates(stream, *frame, supRat);
        frame->setSupportedRates(supRat);
        readHtElements(stream, frame, HT_CAPABILITIES_ALLOWED | EXTENDED_SUPPORTED_RATES_ALLOWED, 2);
        return frame;
    }
    else if (typeInfo == typeid(Ieee80211AssociationRequestFrame)) {
        auto frame = makeShared<Ieee80211AssociationRequestFrame>();
        frame->setCapabilityInformation(stream.readUint16Le());
        frame->setListenInterval(stream.readUint16Le());

        frame->setSSID(deserializeSsid(stream, *frame).c_str());

        Ieee80211SupportedRatesElement supRat;
        deserializeSupportedRates(stream, *frame, supRat);
        frame->setSupportedRates(supRat);
        readHtElements(stream, frame, HT_CAPABILITIES_ALLOWED | EXTENDED_SUPPORTED_RATES_ALLOWED, 2);
        return frame;
    }
    else if (typeInfo == typeid(Ieee80211ReassociationRequestFrame)) {
        auto frame = makeShared<Ieee80211ReassociationRequestFrame>();
        frame->setCapabilityInformation(stream.readUint16Le());
        frame->setListenInterval(stream.readUint16Le());

        frame->setCurrentAP(stream.readMacAddress());

        frame->setSSID(deserializeSsid(stream, *frame).c_str());

        Ieee80211SupportedRatesElement supRat;
        deserializeSupportedRates(stream, *frame, supRat);
        frame->setSupportedRates(supRat);
        readHtElements(stream, frame, HT_CAPABILITIES_ALLOWED | EXTENDED_SUPPORTED_RATES_ALLOWED, 2);
        return frame;
    }
    else if (typeInfo == typeid(Ieee80211AssociationResponseFrame)) {
        auto frame = makeShared<Ieee80211AssociationResponseFrame>();
        frame->setCapabilityInformation(stream.readUint16Le());
        frame->setStatusCode((Ieee80211StatusCode)stream.readUint16Le());
        frame->setAid(decodeAssociationId(frame->getStatusCode(), stream.readUint16Le()));

        Ieee80211SupportedRatesElement supRat;
        deserializeSupportedRates(stream, *frame, supRat);
        frame->setSupportedRates(supRat);
        readHtElements(stream, frame, HT_CAPABILITIES_ALLOWED | HT_OPERATION_ALLOWED | EXTENDED_SUPPORTED_RATES_ALLOWED, 1);
        return frame;
    }
    else if (typeInfo == typeid(Ieee80211ReassociationResponseFrame)) {
        auto frame = makeShared<Ieee80211ReassociationResponseFrame>();
        frame->setCapabilityInformation(stream.readUint16Le());
        frame->setStatusCode((Ieee80211StatusCode)stream.readUint16Le());
        frame->setAid(decodeAssociationId(frame->getStatusCode(), stream.readUint16Le()));

        Ieee80211SupportedRatesElement supRat;
        deserializeSupportedRates(stream, *frame, supRat);
        frame->setSupportedRates(supRat);
        readHtElements(stream, frame, HT_CAPABILITIES_ALLOWED | HT_OPERATION_ALLOWED | EXTENDED_SUPPORTED_RATES_ALLOWED, 1);
        return frame;
    }
    else if (typeInfo == typeid(Ieee80211BeaconFrame)) {
        auto frame = makeShared<Ieee80211BeaconFrame>();

        // IEEE Std 802.11-2024, 9.4.1.10 and 11.1.3.1: unsigned TSF counter in microseconds,
        // kept as is; no conversion to simulation-resolution ticks is needed.
        frame->setHasTimestamp(true);
        frame->setTimestamp(stream.readUint64Le());

        frame->setBeaconInterval(SimTime((int64_t)stream.readUint16Le() * 1024, SIMTIME_US));
        frame->setCapabilityInformation(stream.readUint16Le());

        frame->setSSID(deserializeSsid(stream, *frame).c_str());

        Ieee80211SupportedRatesElement supRat;
        deserializeSupportedRates(stream, *frame, supRat);
        frame->setSupportedRates(supRat);
        readHtElements(stream, frame, HT_CAPABILITIES_ALLOWED | HT_OPERATION_ALLOWED | EXTENDED_SUPPORTED_RATES_ALLOWED | BASIC_HT_MCS_SET_PRESENT, 2);
        return frame;
    }
    else if (typeInfo == typeid(Ieee80211ProbeResponseFrame)) {
        auto frame = makeShared<Ieee80211ProbeResponseFrame>();

        // IEEE Std 802.11-2024, 9.4.1.10 and 11.1.3.1: unsigned TSF counter in microseconds,
        // kept as is; no conversion to simulation-resolution ticks is needed.
        frame->setHasTimestamp(true);
        frame->setTimestamp(stream.readUint64Le());

        frame->setBeaconInterval(SimTime((int64_t)stream.readUint16Le() * 1024, SIMTIME_US));
        frame->setCapabilityInformation(stream.readUint16Le());

        frame->setSSID(deserializeSsid(stream, *frame).c_str());

        Ieee80211SupportedRatesElement supRat;
        deserializeSupportedRates(stream, *frame, supRat);
        frame->setSupportedRates(supRat);
        readHtElements(stream, frame, HT_CAPABILITIES_ALLOWED | HT_OPERATION_ALLOWED | EXTENDED_SUPPORTED_RATES_ALLOWED | BASIC_HT_MCS_SET_PRESENT, 2);
        return frame;
    }
    else
        throw cRuntimeError("Cannot deserialize IEEE 802.11 management frame body of type %s", opp_typename(typeInfo));
}

} // namespace ieee80211

} // namespace inet
