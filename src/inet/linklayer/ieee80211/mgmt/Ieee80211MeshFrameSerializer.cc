// SPDX-License-Identifier: LGPL-3.0-or-later

#include "inet/linklayer/ieee80211/mgmt/Ieee80211MeshFrameSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"

namespace inet {
namespace ieee80211 {

Register_Serializer(Ieee80211MeshControl, Ieee80211MeshControlSerializer);
Register_Serializer(Ieee80211HwmpFrame, Ieee80211MeshFrameSerializer);
Register_Serializer(Ieee80211MeshLinkMetricFrame, Ieee80211MeshFrameSerializer);
Register_Serializer(Ieee80211MeshPeeringOpenFrame, Ieee80211MeshFrameSerializer);
Register_Serializer(Ieee80211MeshPeeringConfirmFrame, Ieee80211MeshFrameSerializer);
Register_Serializer(Ieee80211MeshPeeringCloseFrame, Ieee80211MeshFrameSerializer);

namespace {

// IEEE Std 802.11-2024, Table 9-130 and 9.4.2.5/96--102/110--113.
enum ElementId { TIM = 5, CONFIGURATION = 113, MESH_ID = 114, LINK_METRIC = 115,
    PEERING = 117, AWAKE_WINDOW = 119, RANN = 126, PREQ = 130, PREP = 131, PERR = 132 };
constexpr uint8_t ADDRESS_EXTENSION = 0x40;

void writeElementHeader(MemoryOutputStream& stream, uint8_t id, size_t length)
{
    if (length > 255)
        throw cRuntimeError("Mesh element %u exceeds its one-octet length", id);
    stream.writeByte(id);
    stream.writeByte(length);
}

bool require(MemoryInputStream& stream, B length, Chunk& frame)
{
    if (stream.getRemainingLength() >= length)
        return true;
    frame.markIncorrect();
    frame.markIncomplete();
    stream.seek(stream.getLength());
    return false;
}

bool decodeHwmpElement(MemoryInputStream& s, uint8_t id, Ieee80211HwmpFrame& frame)
{
    // Every read is bounded by this element's Length, never by the containing MPDU.
    if (id == PREQ) {
        if (!require(s, B(26), frame))
            return false;
        HwmpPreq p;
        p.setFlags(s.readByte() & 0x47);
        p.setHopCount(s.readByte());
        p.setTtl(s.readByte());
        p.setDiscoveryId(s.readUint32Le());
        p.setOriginator(s.readMacAddress());
        p.setOriginatorSequenceNumber(s.readUint32Le());
        if (p.getFlags() & ADDRESS_EXTENSION) {
            if (!require(s, B(15), frame))
                return false;
            p.setOriginatorExternalAddress(s.readMacAddress());
        }
        p.setLifetime(s.readUint32Le());
        p.setMetric(s.readUint32Le());
        auto count = s.readByte();
        if (count == 0 || count > 20 || s.getRemainingLength() != B(11 * count))
            return false;
        p.setTargetsArraySize(count);
        for (unsigned int i = 0; i < count; i++) {
            HwmpPreqTarget target;
            target.flags = s.readByte() & 0x05;
            target.address = s.readMacAddress();
            target.sequenceNumber = s.readUint32Le();
            p.setTargets(i, target);
        }
        auto index = frame.getPreqsArraySize();
        frame.setPreqsArraySize(index + 1);
        frame.setPreqs(index, p);
    }
    else if (id == PREP) {
        if (!require(s, B(31), frame))
            return false;
        HwmpPrep p;
        p.flags = s.readByte() & ADDRESS_EXTENSION;
        if (s.getLength() != B(31 + ((p.flags & ADDRESS_EXTENSION) ? 6 : 0)))
            return false;
        p.hopCount = s.readByte();
        p.ttl = s.readByte();
        p.target = s.readMacAddress();
        p.targetSequenceNumber = s.readUint32Le();
        if (p.flags & ADDRESS_EXTENSION)
            p.targetExternalAddress = s.readMacAddress();
        p.lifetime = s.readUint32Le();
        p.metric = s.readUint32Le();
        p.originator = s.readMacAddress();
        p.originatorSequenceNumber = s.readUint32Le();
        auto index = frame.getPrepsArraySize();
        frame.setPrepsArraySize(index + 1);
        frame.setPreps(index, p);
    }
    else if (id == PERR) {
        if (!require(s, B(2), frame))
            return false;
        HwmpPerr p;
        p.setTtl(s.readByte());
        auto count = s.readByte();
        if (count == 0 || count > 19)
            return false;
        p.setDestinationsArraySize(count);
        for (unsigned int i = 0; i < count; i++) {
            if (!require(s, B(13), frame))
                return false;
            HwmpPerrDestination d;
            d.flags = s.readByte() & ADDRESS_EXTENSION;
            d.address = s.readMacAddress();
            d.sequenceNumber = s.readUint32Le();
            if (d.flags & ADDRESS_EXTENSION) {
                if (!require(s, B(8), frame))
                    return false;
                d.externalAddress = s.readMacAddress();
            }
            d.reasonCode = s.readUint16Le();
            p.setDestinations(i, d);
        }
        if (s.getRemainingLength() != b(0))
            return false;
        auto index = frame.getPerrsArraySize();
        frame.setPerrsArraySize(index + 1);
        frame.setPerrs(index, p);
    }
    else if (id == RANN) {
        if (s.getLength() != B(21))
            return false;
        HwmpRann r;
        r.flags = s.readByte() & 1;
        r.hopCount = s.readByte();
        r.ttl = s.readByte();
        r.root = s.readMacAddress();
        r.sequenceNumber = s.readUint32Le();
        r.interval = s.readUint32Le();
        r.metric = s.readUint32Le();
        auto index = frame.getRannsArraySize();
        frame.setRannsArraySize(index + 1);
        frame.setRanns(index, r);
    }
    else
        return false;
    return true;
}

void writeHwmpElements(MemoryOutputStream& s, const Ieee80211HwmpFrame& frame)
{
    for (size_t i = 0; i < frame.getPreqsArraySize(); i++) {
        const auto& p = frame.getPreqs(i);
        size_t count = p.getTargetsArraySize();
        if (count == 0 || count > 20)
            throw cRuntimeError("PREQ must contain 1--20 targets");
        writeElementHeader(s, PREQ, 26 + ((p.getFlags() & ADDRESS_EXTENSION) ? 6 : 0) + 11 * count);
        s.writeByte(p.getFlags() & 0x47);
        s.writeByte(p.getHopCount());
        s.writeByte(p.getTtl());
        s.writeUint32Le(p.getDiscoveryId());
        s.writeMacAddress(p.getOriginator());
        s.writeUint32Le(p.getOriginatorSequenceNumber());
        if (p.getFlags() & ADDRESS_EXTENSION)
            s.writeMacAddress(p.getOriginatorExternalAddress());
        s.writeUint32Le(p.getLifetime());
        s.writeUint32Le(p.getMetric());
        s.writeByte(count);
        for (size_t j = 0; j < count; j++) {
            const auto& t = p.getTargets(j);
            s.writeByte(t.flags & 0x05);
            s.writeMacAddress(t.address);
            s.writeUint32Le(t.sequenceNumber);
        }
    }
    for (size_t i = 0; i < frame.getPrepsArraySize(); i++) {
        const auto& p = frame.getPreps(i);
        writeElementHeader(s, PREP, 31 + ((p.flags & ADDRESS_EXTENSION) ? 6 : 0));
        s.writeByte(p.flags & ADDRESS_EXTENSION);
        s.writeByte(p.hopCount);
        s.writeByte(p.ttl);
        s.writeMacAddress(p.target);
        s.writeUint32Le(p.targetSequenceNumber);
        if (p.flags & ADDRESS_EXTENSION)
            s.writeMacAddress(p.targetExternalAddress);
        s.writeUint32Le(p.lifetime);
        s.writeUint32Le(p.metric);
        s.writeMacAddress(p.originator);
        s.writeUint32Le(p.originatorSequenceNumber);
    }
    for (size_t i = 0; i < frame.getPerrsArraySize(); i++) {
        const auto& p = frame.getPerrs(i);
        size_t count = p.getDestinationsArraySize();
        if (count == 0 || count > 19)
            throw cRuntimeError("PERR must contain 1--19 destinations");
        size_t length = 2;
        for (size_t j = 0; j < count; j++)
            length += 13 + ((p.getDestinations(j).flags & ADDRESS_EXTENSION) ? 6 : 0);
        writeElementHeader(s, PERR, length);
        s.writeByte(p.getTtl());
        s.writeByte(count);
        for (size_t j = 0; j < count; j++) {
            const auto& d = p.getDestinations(j);
            s.writeByte(d.flags & ADDRESS_EXTENSION);
            s.writeMacAddress(d.address);
            s.writeUint32Le(d.sequenceNumber);
            if (d.flags & ADDRESS_EXTENSION)
                s.writeMacAddress(d.externalAddress);
            s.writeUint16Le(d.reasonCode);
        }
    }
    for (size_t i = 0; i < frame.getRannsArraySize(); i++) {
        const auto& r = frame.getRanns(i);
        writeElementHeader(s, RANN, 21);
        s.writeByte(r.flags & 1);
        s.writeByte(r.hopCount);
        s.writeByte(r.ttl);
        s.writeMacAddress(r.root);
        s.writeUint32Le(r.sequenceNumber);
        s.writeUint32Le(r.interval);
        s.writeUint32Le(r.metric);
    }
    for (size_t i = 0; i < frame.getUnmodelledElementsArraySize(); i++)
        s.writeByte(frame.getUnmodelledElements(i));
}

} // namespace

bool Ieee80211MeshFrameSerializer::isMeshElement(uint8_t id)
{
    return id == TIM || id == CONFIGURATION || id == MESH_ID || id == PEERING || id == AWAKE_WINDOW;
}

bool Ieee80211MeshFrameSerializer::readElement(MemoryInputStream& stream, uint8_t id, uint8_t length, Ieee80211MeshFrame& frame, int peeringAction)
{
    // The containing decoder has already checked Length against remaining bytes.
    if (!require(stream, B(length), frame))
        return false;
    if (id == MESH_ID) {
        if (length > 32 || frame.getMeshIdPresent())
            return false;
        frame.setMeshIdPresent(true);
        frame.setMeshIdArraySize(length);
        for (unsigned int i = 0; i < length; i++)
            frame.setMeshId(i, stream.readByte());
    }
    else if (id == CONFIGURATION) {
        if (length != 7 || frame.getMeshConfigurationPresent())
            return false;
        MeshConfiguration c;
        c.pathSelectionProtocolId = stream.readByte();
        c.pathSelectionMetricId = stream.readByte();
        c.congestionControlId = stream.readByte();
        c.synchronizationId = stream.readByte();
        c.authenticationId = stream.readByte();
        c.formationInfo = stream.readByte();
        c.capability = stream.readByte() & 0x7F;
        frame.setMeshConfiguration(c);
        frame.setMeshConfigurationPresent(true);
    }
    else if (id == TIM) {
        if (length < 4 || length > 254 || frame.getTimPresent())
            return false;
        MeshTim t;
        t.dtimCount = stream.readByte();
        t.dtimPeriod = stream.readByte();
        t.bitmapControl = stream.readByte();
        t.bitmapLength = length - 3;
        if (t.dtimPeriod == 0 || t.dtimCount >= t.dtimPeriod || (t.bitmapControl & 0xFE) + t.bitmapLength > 251)
            return false;
        for (unsigned int i = 0; i < t.bitmapLength; i++)
            t.partialVirtualBitmap[i] = stream.readByte();
        frame.setTim(t);
        frame.setTimPresent(true);
    }
    else if (id == AWAKE_WINDOW) {
        if (length != 2 || frame.getMeshAwakeWindowPresent())
            return false;
        frame.setMeshAwakeWindow(stream.readUint16Le());
        frame.setMeshAwakeWindowPresent(true);
    }
    else if (id == PEERING) {
        if (length < 4 || frame.getMeshPeeringPresent())
            return false;
        MeshPeering p;
        p.protocolId = stream.readUint16Le();
        p.localLinkId = stream.readUint16Le();
        int remainder = length - 4;
        p.chosenPmkPresent = p.protocolId == 1;
        if (p.chosenPmkPresent)
            remainder -= 16;
        if (peeringAction == MESH_PEERING_OPEN) {
            if (remainder != 0)
                return false;
        }
        else if (peeringAction == MESH_PEERING_CONFIRM) {
            if (remainder != 2)
                return false;
            p.peerLinkIdPresent = true;
        }
        else if (peeringAction == MESH_PEERING_CLOSE) {
            if (remainder != 2 && remainder != 4)
                return false;
            p.peerLinkIdPresent = remainder == 4;
            p.reasonCodePresent = true;
        }
        else
            return false;
        if (p.peerLinkIdPresent)
            p.peerLinkId = stream.readUint16Le();
        if (p.reasonCodePresent)
            p.reasonCode = stream.readUint16Le();
        if (p.chosenPmkPresent)
            for (auto& byte : p.chosenPmk)
                byte = stream.readByte();
        frame.setMeshPeering(p);
        frame.setMeshPeeringPresent(true);
    }
    else
        return false;
    return true;
}

void Ieee80211MeshFrameSerializer::writeElements(MemoryOutputStream& s, const Ieee80211MeshFrame& frame, const std::function<void()>& beforeElement)
{
    auto header = [&](uint8_t id, size_t length) {
        if (beforeElement)
            beforeElement();
        writeElementHeader(s, id, length);
    };
    if (frame.getTimPresent()) {
        const auto& t = frame.getTim();
        if (t.bitmapLength < 1 || t.bitmapLength > 251 || t.dtimPeriod == 0 || t.dtimCount >= t.dtimPeriod
                || (t.bitmapControl & 0xFE) + t.bitmapLength > 251)
            throw cRuntimeError("Invalid mesh TIM bitmap length or DTIM counters");
        header(TIM, 3 + t.bitmapLength);
        s.writeByte(t.dtimCount);
        s.writeByte(t.dtimPeriod);
        s.writeByte(t.bitmapControl);
        for (unsigned int i = 0; i < t.bitmapLength; i++)
            s.writeByte(t.partialVirtualBitmap[i]);
    }
    if (frame.getMeshIdPresent()) {
        if (frame.getMeshIdArraySize() > 32)
            throw cRuntimeError("Mesh ID exceeds 32 octets");
        header(MESH_ID, frame.getMeshIdArraySize());
        for (size_t i = 0; i < frame.getMeshIdArraySize(); i++)
            s.writeByte(frame.getMeshId(i));
    }
    if (frame.getMeshConfigurationPresent()) {
        const auto& c = frame.getMeshConfiguration();
        header(CONFIGURATION, 7);
        s.writeByte(c.pathSelectionProtocolId);
        s.writeByte(c.pathSelectionMetricId);
        s.writeByte(c.congestionControlId);
        s.writeByte(c.synchronizationId);
        s.writeByte(c.authenticationId);
        s.writeByte(c.formationInfo);
        s.writeByte(c.capability & 0x7F);
    }
    if (frame.getMeshPeeringPresent()) {
        const auto& p = frame.getMeshPeering();
        header(PEERING, 4 + 2 * p.peerLinkIdPresent + 2 * p.reasonCodePresent + 16 * p.chosenPmkPresent);
        s.writeUint16Le(p.protocolId);
        s.writeUint16Le(p.localLinkId);
        if (p.peerLinkIdPresent)
            s.writeUint16Le(p.peerLinkId);
        if (p.reasonCodePresent)
            s.writeUint16Le(p.reasonCode);
        if (p.chosenPmkPresent)
            for (const auto byte : p.chosenPmk)
                s.writeByte(byte);
    }
    if (frame.getMeshAwakeWindowPresent()) {
        header(AWAKE_WINDOW, 2);
        s.writeUint16Le(frame.getMeshAwakeWindow());
    }
}

void Ieee80211MeshFrameSerializer::serializeFields(MemoryOutputStream& s, const Ptr<const Chunk>& chunk) const
{
    // IEEE Std 802.11-2024, 9.6.15.2--4: Capability and AID precede IEs.
    auto peering = dynamicPtrCast<const Ieee80211MeshPeeringOpenFrame>(chunk);
    if (peering) {
        s.writeUint16Le(peering->getCapabilityInformation());
        if (auto confirm = dynamicPtrCast<const Ieee80211MeshPeeringConfirmFrame>(chunk)) {
            if (confirm->getAssociationId() < 1 || confirm->getAssociationId() > 2007)
                throw cRuntimeError("Mesh Confirm AID must be in [1, 2007]");
            // 9.4.1.8/Figure 9-146: non-DMG AID has both high bits set.
            s.writeUint16Le(confirm->getAssociationId() | 0xC000);
        }
        auto count = peering->getSupportedRatesArraySize();
        if (count < 1 || count > 8)
            throw cRuntimeError("Mesh Supported Rates must contain 1--8 entries");
        writeElementHeader(s, 1, count);
        for (size_t i = 0; i < count; i++)
            s.writeByte(peering->getSupportedRates(i));
        if (peering->getExtendedSupportedRatesArraySize()) {
            writeElementHeader(s, 50, peering->getExtendedSupportedRatesArraySize());
            for (size_t i = 0; i < peering->getExtendedSupportedRatesArraySize(); i++)
                s.writeByte(peering->getExtendedSupportedRates(i));
        }
    }
    writeElements(s, *staticPtrCast<const Ieee80211MeshFrame>(chunk));
    if (auto hwmp = dynamicPtrCast<const Ieee80211HwmpFrame>(chunk)) {
        if (hwmp->getPreqsArraySize() + hwmp->getPrepsArraySize() + hwmp->getPerrsArraySize() + hwmp->getRannsArraySize() == 0)
            throw cRuntimeError("HWMP action must contain at least one path-selection element");
        writeHwmpElements(s, *hwmp);
    }
    else if (auto report = dynamicPtrCast<const Ieee80211MeshLinkMetricFrame>(chunk)) {
        writeElementHeader(s, LINK_METRIC, 5);
        s.writeByte(report->getRequest() ? 1 : 0);
        s.writeUint32Le(report->getMetric());
    }
    if (s.getLength() != chunk->getChunkLength())
        throw cRuntimeError("Mesh body chunkLength does not match its serialized length");
}

const Ptr<Chunk> Ieee80211MeshFrameSerializer::deserializeFields(MemoryInputStream& s, const std::type_info& typeInfo) const
{
    Ptr<Ieee80211MeshFrame> frame;
    int action = 0;
    if (typeInfo == typeid(Ieee80211MeshPeeringConfirmFrame)) {
        frame = makeShared<Ieee80211MeshPeeringConfirmFrame>();
        action = MESH_PEERING_CONFIRM;
    }
    else if (typeInfo == typeid(Ieee80211MeshPeeringOpenFrame)) {
        frame = makeShared<Ieee80211MeshPeeringOpenFrame>();
        action = MESH_PEERING_OPEN;
    }
    else if (typeInfo == typeid(Ieee80211MeshPeeringCloseFrame)) {
        frame = makeShared<Ieee80211MeshPeeringCloseFrame>();
        action = MESH_PEERING_CLOSE;
    }
    else if (typeInfo == typeid(Ieee80211MeshLinkMetricFrame))
        frame = makeShared<Ieee80211MeshLinkMetricFrame>();
    else if (typeInfo == typeid(Ieee80211HwmpFrame))
        frame = makeShared<Ieee80211HwmpFrame>();
    else
        throw cRuntimeError("Unsupported mesh body type %s", typeInfo.name());
    if (auto peering = dynamicPtrCast<Ieee80211MeshPeeringOpenFrame>(frame)) {
        if (!require(s, B(action == MESH_PEERING_CONFIRM ? 4 : 2), *frame))
            return frame;
        peering->setCapabilityInformation(s.readUint16Le());
        if (auto confirm = dynamicPtrCast<Ieee80211MeshPeeringConfirmFrame>(frame)) {
            auto aid = s.readUint16Le();
            confirm->setAssociationId(aid & 0x3FFF);
            if ((aid & 0xC000) != 0xC000 || confirm->getAssociationId() < 1 || confirm->getAssociationId() > 2007)
                frame->markIncorrect();
        }
    }
    bool metricPresent = false;
    while (s.getRemainingLength() > b(0)) {
        if (!require(s, B(2), *frame))
            break;
        auto id = s.readByte();
        auto length = s.readByte();
        if (!require(s, B(length), *frame))
            break;
        std::vector<uint8_t> bytes(length);
        s.readBytes(bytes.data(), B(length));
        MemoryInputStream element(bytes);
        bool valid = true;
        if (isMeshElement(id))
            valid = readElement(element, id, length, *frame, action);
        else if (id == PREQ || id == PREP || id == PERR || id == RANN) {
            auto hwmp = dynamicPtrCast<Ieee80211HwmpFrame>(frame);
            valid = hwmp && decodeHwmpElement(element, id, *hwmp);
        }
        else if (id == LINK_METRIC) {
            auto report = dynamicPtrCast<Ieee80211MeshLinkMetricFrame>(frame);
            valid = report && !metricPresent && length == 5;
            if (valid) {
                report->setRequest(element.readByte() & 1);
                report->setMetric(element.readUint32Le());
                metricPresent = true;
            }
        }
        else if ((id == 1 || id == 50) && dynamicPtrCast<Ieee80211MeshPeeringOpenFrame>(frame)) {
            auto p = staticPtrCast<Ieee80211MeshPeeringOpenFrame>(frame);
            valid = length >= 1 && (id != 1 || length <= 8);
            valid &= id == 1 ? p->getSupportedRatesArraySize() == 0 : p->getExtendedSupportedRatesArraySize() == 0;
            if (valid) {
                if (id == 1)
                    p->setSupportedRatesArraySize(length);
                else
                    p->setExtendedSupportedRatesArraySize(length);
                for (unsigned int i = 0; i < length; i++) {
                    auto rate = element.readByte();
                    if ((rate & 0x7F) == 0)
                        valid = false;
                    if (id == 1)
                        p->setSupportedRates(i, rate);
                    else
                        p->setExtendedSupportedRates(i, rate);
                }
            }
        }
        else if (auto hwmp = dynamicPtrCast<Ieee80211HwmpFrame>(frame)) {
            size_t index = hwmp->getUnmodelledElementsArraySize();
            hwmp->setUnmodelledElementsArraySize(index + 2 + length);
            hwmp->setUnmodelledElements(index++, id);
            hwmp->setUnmodelledElements(index++, length);
            for (auto byte : bytes)
                hwmp->setUnmodelledElements(index++, byte);
        }
        // Unknown optional elements and reserved bits are ignored on reception.
        if (!valid)
            frame->markIncorrect();
    }
    if (auto hwmp = dynamicPtrCast<Ieee80211HwmpFrame>(frame)) {
        if (hwmp->getPreqsArraySize() + hwmp->getPrepsArraySize() + hwmp->getPerrsArraySize() + hwmp->getRannsArraySize() == 0)
            frame->markIncorrect();
    }
    if (dynamicPtrCast<Ieee80211MeshLinkMetricFrame>(frame) && !metricPresent)
        frame->markIncorrect();
    if (action) {
        if (!frame->getMeshIdPresent() || !frame->getMeshPeeringPresent())
            frame->markIncorrect();
        if (action != MESH_PEERING_CLOSE && !frame->getMeshConfigurationPresent())
            frame->markIncorrect();
        if (auto p = dynamicPtrCast<Ieee80211MeshPeeringOpenFrame>(frame))
            if (p->getSupportedRatesArraySize() == 0)
                frame->markIncorrect();
    }
    return frame;
}

void Ieee80211MeshControlSerializer::serializeFields(MemoryOutputStream& s, const Ptr<const Chunk>& chunk) const
{
    auto control = staticPtrCast<const Ieee80211MeshControl>(chunk);
    auto ae = control->getAddressExtensionMode();
    if (ae > 2 || control->getTtl() == 0 || control->getChunkLength() != B(6 + 6 * ae))
        throw cRuntimeError("Invalid Mesh Control AE or chunkLength");
    s.writeByte(ae);
    s.writeByte(control->getTtl());
    s.writeUint32Le(control->getSequenceNumber());
    if (ae == 1)
        s.writeMacAddress(control->getAddress4());
    else if (ae == 2) {
        s.writeMacAddress(control->getAddress5());
        s.writeMacAddress(control->getAddress6());
    }
}

const Ptr<Chunk> Ieee80211MeshControlSerializer::deserializeFields(MemoryInputStream& s, const std::type_info&) const
{
    auto control = makeShared<Ieee80211MeshControl>();
    if (!require(s, B(6), *control))
        return control;
    auto ae = s.readByte() & 3;
    control->setAddressExtensionMode(ae);
    control->setTtl(s.readByte());
    if (control->getTtl() == 0)
        control->markIncorrect();
    control->setSequenceNumber(s.readUint32Le());
    if (ae == 3) {
        control->markIncorrect();
        return control;
    }
    if (!require(s, B(6 * ae), *control))
        return control;
    if (ae == 1)
        control->setAddress4(s.readMacAddress());
    else if (ae == 2) {
        control->setAddress5(s.readMacAddress());
        control->setAddress6(s.readMacAddress());
    }
    return control;
}

} // namespace ieee80211
} // namespace inet
