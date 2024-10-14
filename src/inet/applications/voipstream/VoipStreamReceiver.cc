//
// Copyright (C) 2005 M. Bohge (bohge@tkn.tu-berlin.de), M. Renwanz
// Copyright (C) 2010 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include <cstdarg>

#include "inet/applications/voipstream/VoipStreamReceiver.h"

#include "inet/applications/voipstream/VoipStreamPacket_m.h"
#include "inet/common/INETUtils.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"

namespace inet {

//#if defined(__clang__)
//#  pragma clang diagnostic ignored "-Wdeprecated-declarations"
//#elif defined(__GNUC__)
//#  pragma GCC diagnostic ignored "-Wdeprecated-declarations"
//#endif

Define_Module(VoipStreamReceiver);

simsignal_t VoipStreamReceiver::lostSamplesSignal = registerSignal("lostSamples");
simsignal_t VoipStreamReceiver::lostPacketsSignal = registerSignal("lostPackets");
simsignal_t VoipStreamReceiver::packetHasVoiceSignal = registerSignal("packetHasVoice");
simsignal_t VoipStreamReceiver::connStateSignal = registerSignal("connState");
simsignal_t VoipStreamReceiver::delaySignal = registerSignal("delay");

VoipStreamReceiver::~VoipStreamReceiver()
{
    closeConnection();
}

void VoipStreamReceiver::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        // Say Hello to the world
        EV_TRACE << "VoIPSinkApp initialize()" << endl;

        // read parameters
        localPort = par("localPort");
        resultFile = par("resultFile");
        playoutDelay = par("playoutDelay");

        // initialize avcodec library
        av_log_set_callback(&inet_av_log);
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        cModule *node = findContainingNode(this);
        NodeStatus *nodeStatus = node ? check_and_cast_nullable<NodeStatus *>(node->getSubmodule("status")) : nullptr;
        bool isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
        if (!isOperational)
            throw cRuntimeError("This module doesn't support starting in node DOWN state");

        socket.setOutputGate(gate("socketOut"));
        socket.bind(localPort);
        socket.setCallback(this);
    }
}

void VoipStreamReceiver::handleMessage(cMessage *msg)
{
    if (msg->arrivedOn("socketIn")) {
        socket.processMessage(msg);
    }
    else
        throw cRuntimeError("Unknown incoming gate: '%s'", msg->getArrivalGate()->getFullName());
}

void VoipStreamReceiver::socketDataArrived(UdpSocket *socket, Packet *pk)
{
    // process incoming packet

    const auto& vp = pk->peekAtFront<VoipStreamPacket>();
    bool ok = true;
    if (curConn.offline)
        createConnection(pk);
    else {
        checkSourceAndParameters(pk);
        ok = vp->getSeqNo() > curConn.seqNo && vp->getTimeStamp() > curConn.timeStamp;
    }

    if (ok) {
        emit(packetReceivedSignal, pk);
        decodePacket(pk);
    }
    else {
        PacketDropDetails details;
        details.setReason(CONGESTION);
        emit(packetDroppedSignal, pk, &details);
    }

    delete pk;
}

void VoipStreamReceiver::socketErrorArrived(UdpSocket *socket, Indication *indication)
{
    EV_WARN << "Unknown message '" << indication->getName() << "', kind = " << indication->getKind() << ", discarding it." << endl;
    delete indication;
}

void VoipStreamReceiver::Connection::openAudio(const char *fileName)
{
    inet::utils::makePathForFile(fileName);
    outFile.open(fileName, sampleRate, 8 * av_get_bytes_per_sample(decCtx->sample_fmt));
}

void VoipStreamReceiver::Connection::writeLostSamples(int sampleCount)
{
    int pktBytes = sampleCount * av_get_bytes_per_sample(decCtx->sample_fmt);
    if (outFile.isOpen()) {
        uint8_t *decBuf = new uint8_t[pktBytes];
        memset(decBuf, 0, pktBytes);
        outFile.write(decBuf, pktBytes);
        delete [] decBuf;
    }
}

void VoipStreamReceiver::Connection::writeAudioFrame(AVPacket *avpkt)
{
    int err = avcodec_send_packet(decCtx, avpkt);
    if (err < 0)
        throw cRuntimeError("Error in avcodec_send_packet(), error (%d) %s", err, av_err2str(err));

    AVFrame *decodedFrame = av_frame_alloc();
    while (true) {
        // decode audio and save the decoded samples in our buffer
        err = avcodec_receive_frame(decCtx, decodedFrame);
        if (err == AVERROR(EAGAIN) || err == AVERROR_EOF)
            break;
        else if (err < 0)
            throw cRuntimeError("Error in avcodec_receive_frame(), error (%d) %s", err, av_err2str(err));
        simtime_t decodedTime(1.0 * decodedFrame->nb_samples / sampleRate);
        lastPacketFinish += decodedTime;
        if (outFile.isOpen())
            outFile.write(decodedFrame->data[0], decodedFrame->nb_samples * av_get_bytes_per_sample(decCtx->sample_fmt));
    }
    av_frame_free(&decodedFrame);
}

void VoipStreamReceiver::Connection::closeAudio()
{
    outFile.close();
}

void VoipStreamReceiver::createConnection(Packet *pk)
{
    ASSERT(curConn.offline);

    const auto& vp = pk->peekAtFront<VoipStreamPacket>();
    auto l3Addresses = pk->getTag<L3AddressInd>();
    auto ports = pk->getTag<L4PortInd>();

    curConn.srcAddr = l3Addresses->getSrcAddress();
    curConn.srcPort = ports->getSrcPort();
    curConn.destAddr = l3Addresses->getDestAddress();
    curConn.destPort = ports->getDestPort();
    curConn.seqNo = vp->getSeqNo() - 1;
    curConn.timeStamp = vp->getTimeStamp();
    curConn.ssrc = vp->getSsrc();
    curConn.codec = (enum AVCodecID)(vp->getCodec());
    curConn.sampleBits = vp->getSampleBits();
    curConn.sampleRate = vp->getSampleRate();
    curConn.transmitBitrate = vp->getTransmitBitrate();
    curConn.samplesPerPacket = vp->getSamplesPerPacket();
    curConn.lastPacketFinish = simTime() + playoutDelay;

    curConn.pCodecDec = avcodec_find_decoder(curConn.codec);
    if (curConn.pCodecDec == nullptr)
        throw cRuntimeError("Codec %d not found", curConn.codec);

    curConn.decCtx = avcodec_alloc_context3(curConn.pCodecDec);
    curConn.decCtx->bit_rate = curConn.transmitBitrate;
    curConn.decCtx->sample_rate = curConn.sampleRate;
#if LIBAVCODEC_VERSION_MAJOR < 59
    curConn.decCtx->channels = 1;
#else /* LIBAVCODEC_VERSION_MAJOR < 59 */
    curConn.decCtx->ch_layout = AV_CHANNEL_LAYOUT_MONO;
#endif /* LIBAVCODEC_VERSION_MAJOR < 59 */
    curConn.decCtx->bits_per_coded_sample = curConn.sampleBits;

    int err = avcodec_open2(curConn.decCtx, curConn.pCodecDec, nullptr);
    if (err < 0)
        throw cRuntimeError("could not open decoding codec %d (%s): error (%d) %s", curConn.codec, curConn.pCodecDec->name, err, av_err2str(err));

    curConn.openAudio(resultFile);
    curConn.offline = false;
    emit(connStateSignal, 1);
}

void VoipStreamReceiver::checkSourceAndParameters(Packet *pk)
{
    ASSERT(!curConn.offline);

    const auto& vp = pk->peekAtFront<VoipStreamPacket>();
    auto l3Addresses = pk->getTag<L3AddressInd>();
    auto ports = pk->getTag<L4PortInd>();
    L3Address srcAddr = l3Addresses->getSrcAddress();
    L3Address destAddr = l3Addresses->getDestAddress();

    if (curConn.srcAddr != srcAddr
        || curConn.srcPort != ports->getSrcPort()
        || curConn.destAddr != destAddr
        || curConn.destPort != ports->getDestPort()
        || vp->getSsrc() != curConn.ssrc)
        throw cRuntimeError("Voice packet received from third party during a voice session (concurrent voice sessions not supported)");

    if (vp->getCodec() != curConn.codec
        || vp->getSampleBits() != curConn.sampleBits
        || vp->getSampleRate() != curConn.sampleRate
        || vp->getSamplesPerPacket() != curConn.samplesPerPacket
        || vp->getTransmitBitrate() != curConn.transmitBitrate)
        throw cRuntimeError("Cannot change voice encoding parameters a during session");
}

void VoipStreamReceiver::closeConnection()
{
    if (!curConn.offline) {
        curConn.offline = true;
        avcodec_close(curConn.decCtx);
        avcodec_free_context(&curConn.decCtx);
        curConn.outFile.close();
        emit(connStateSignal, -1L); // so that sum() yields the number of active sessions
    }
}

void VoipStreamReceiver::decodePacket(Packet *pk)
{
    const auto& vp = pk->popAtFront<VoipStreamPacket>();
    uint16_t newSeqNo = vp->getSeqNo();
    if (newSeqNo > curConn.seqNo + 1)
        emit(lostPacketsSignal, newSeqNo - (curConn.seqNo + 1));

    if (simTime() > curConn.lastPacketFinish) {
        int lostSamples = ceil(SIMTIME_DBL((simTime() - curConn.lastPacketFinish) * curConn.sampleRate));
        ASSERT(lostSamples > 0);
        EV_INFO << "Lost " << lostSamples << " samples\n";
        emit(lostSamplesSignal, lostSamples);
        curConn.writeLostSamples(lostSamples);
        curConn.lastPacketFinish += lostSamples * (1.0 / curConn.sampleRate);
        FINGERPRINT_ADD_EXTRA_DATA(lostSamples);
    }
    emit(delaySignal, curConn.lastPacketFinish - pk->getCreationTime());
    curConn.seqNo = newSeqNo;

    if (vp->getType() == VOICE) {
        emit(packetHasVoiceSignal, 1);
        uint16_t len = vp->getDataLength();
        auto bb = pk->peekDataAt<BytesChunk>(b(0), B(len));
        //auto buff = bb->getBytes();
        AVPacket *avpkt = av_packet_alloc();
        av_new_packet(avpkt, len + AV_INPUT_BUFFER_PADDING_SIZE); // required extra AV_INPUT_BUFFER_PADDING_SIZE bytes at end of buff for avcodec_decode_audio4()
        bb->copyToBuffer(avpkt->data, len);
        avpkt->size = len;
        FINGERPRINT_ADD_EXTRA_DATA2((const char *)avpkt->data, len);
        curConn.writeAudioFrame(avpkt);
        av_packet_free(&avpkt);
    }
    else if (vp->getType() == SILENCE) {
        emit(packetHasVoiceSignal, 0);
        int silenceSamples = vp->getSamplesPerPacket();
        curConn.writeLostSamples(silenceSamples);
        curConn.lastPacketFinish += silenceSamples * (1.0 / curConn.sampleRate);
        FINGERPRINT_ADD_EXTRA_DATA(silenceSamples);
    }
    else
        throw cRuntimeError("The received VoipStreamPacket has unknown type %d", vp->getType());
}

void VoipStreamReceiver::finish()
{
    EV_TRACE << "Sink finish()" << endl;
    closeConnection();
}

} // namespace inet

