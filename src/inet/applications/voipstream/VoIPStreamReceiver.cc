//
// Copyright (C) 2005 M. Bohge (bohge@tkn.tu-berlin.de), M. Renwanz
// Copyright (C) 2010 Zoltan Bojthe
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

#include "inet/applications/voipstream/VoIPStreamReceiver.h"

#include "inet/common/INETEndians.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeStatus.h"

namespace inet {

Define_Module(VoIPStreamReceiver);

simsignal_t VoIPStreamReceiver::rcvdPkSignal = registerSignal("rcvdPk");
simsignal_t VoIPStreamReceiver::lostSamplesSignal = registerSignal("lostSamples");
simsignal_t VoIPStreamReceiver::lostPacketsSignal = registerSignal("lostPackets");
simsignal_t VoIPStreamReceiver::dropPkSignal = registerSignal("dropPk");
simsignal_t VoIPStreamReceiver::packetHasVoiceSignal = registerSignal("packetHasVoice");
simsignal_t VoIPStreamReceiver::connStateSignal = registerSignal("connState");
simsignal_t VoIPStreamReceiver::delaySignal = registerSignal("delay");

VoIPStreamReceiver::~VoIPStreamReceiver()
{
    closeConnection();
}

void VoIPStreamReceiver::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        // Hack for create results folder
        recordScalar("hackForCreateResultsFolder", 0);

        // Say Hello to the world
        EV_TRACE << "VoIPSinkApp initialize()" << endl;

        // read parameters
        localPort = par("localPort");
        resultFile = par("resultFile");
        playoutDelay = par("playoutDelay");

        // initialize avcodec library
        av_register_all();
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        bool isOperational;
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
        if (!isOperational)
            throw cRuntimeError("This module doesn't support starting in node DOWN state");

        socket.setOutputGate(gate("udpOut"));
        socket.bind(localPort);
    }
}

void VoIPStreamReceiver::handleMessage(cMessage *msg)
{
    if (msg->getKind() == UDP_I_ERROR) {
        delete msg;
        return;
    }

    VoIPStreamPacket *vp = check_and_cast<VoIPStreamPacket *>(msg);
    bool ok = true;
    if (curConn.offline)
        createConnection(vp);
    else {
        checkSourceAndParameters(vp);
        ok = vp->getSeqNo() > curConn.seqNo && vp->getTimeStamp() > curConn.timeStamp;
    }

    if (ok) {
        emit(rcvdPkSignal, vp);
        decodePacket(vp);
    }
    else
        emit(dropPkSignal, msg);

    delete msg;
}

void VoIPStreamReceiver::Connection::openAudio(const char *fileName)
{
    outFile.open(fileName, sampleRate, 8 * av_get_bytes_per_sample(decCtx->sample_fmt));
}

void VoIPStreamReceiver::Connection::writeLostSamples(int sampleCount)
{
    int pktBytes = sampleCount * av_get_bytes_per_sample(decCtx->sample_fmt);
    if (outFile.isOpen()) {
        uint8_t decBuf[pktBytes];
        memset(decBuf, 0, pktBytes);
        outFile.write(decBuf, pktBytes);
    }
}

void VoIPStreamReceiver::Connection::writeAudioFrame(uint8_t *inbuf, int inbytes)
{
    AVPacket avpkt;
    av_init_packet(&avpkt);
    avpkt.data = inbuf;
    avpkt.size = inbytes;

    int gotFrame;
    AVFrame decodedFrame = {
        { 0 }
    };
    int consumedBytes = avcodec_decode_audio4(decCtx, &decodedFrame, &gotFrame, &avpkt);
    if (consumedBytes < 0 || !gotFrame)
        throw cRuntimeError("Error in avcodec_decode_audio4(): returns: %d, gotFrame: %d", consumedBytes, gotFrame);
    if (consumedBytes != inbytes)
        throw cRuntimeError("Model error: remained bytes after avcodec_decode_audio4(): %d = ( %d - %d )", inbytes - consumedBytes, inbytes, consumedBytes);
    simtime_t decodedTime(1.0 * decodedFrame.nb_samples / sampleRate);
    lastPacketFinish += decodedTime;
    if (outFile.isOpen())
        outFile.write(decodedFrame.data[0], decodedFrame.linesize[0]);
}

void VoIPStreamReceiver::Connection::closeAudio()
{
    outFile.close();
}

void VoIPStreamReceiver::createConnection(VoIPStreamPacket *vp)
{
    ASSERT(curConn.offline);

    UDPDataIndication *udpCtrl = check_and_cast<UDPDataIndication *>(vp->getControlInfo());

    curConn.srcAddr = udpCtrl->getSrcAddr();
    curConn.srcPort = udpCtrl->getSrcPort();
    curConn.destAddr = udpCtrl->getDestAddr();
    curConn.destPort = udpCtrl->getDestPort();
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
    if (curConn.pCodecDec == NULL)
        throw cRuntimeError("Codec %d not found", curConn.codec);

    curConn.decCtx = avcodec_alloc_context3(curConn.pCodecDec);
    curConn.decCtx->bit_rate = curConn.transmitBitrate;
    curConn.decCtx->sample_rate = curConn.sampleRate;
    curConn.decCtx->channels = 1;
    curConn.decCtx->bits_per_coded_sample = curConn.sampleBits;

    int ret = avcodec_open2(curConn.decCtx, curConn.pCodecDec, NULL);
    if (ret < 0)
        throw cRuntimeError("could not open decoding codec %d (%s): err=%d", curConn.codec, curConn.pCodecDec->name, ret);

    curConn.openAudio(resultFile);
    curConn.offline = false;
    emit(connStateSignal, 1);
}

void VoIPStreamReceiver::checkSourceAndParameters(VoIPStreamPacket *vp)
{
    ASSERT(!curConn.offline);

    UDPDataIndication *udpCtrl = check_and_cast<UDPDataIndication *>(vp->getControlInfo());
    if (curConn.srcAddr != udpCtrl->getSrcAddr()
        || curConn.srcPort != udpCtrl->getSrcPort()
        || curConn.destAddr != udpCtrl->getDestAddr()
        || curConn.destPort != udpCtrl->getDestPort()
        || vp->getSsrc() != curConn.ssrc)
        throw cRuntimeError("Voice packet received from third party during a voice session (concurrent voice sessions not supported)");

    if (vp->getCodec() != curConn.codec
        || vp->getSampleBits() != curConn.sampleBits
        || vp->getSampleRate() != curConn.sampleRate
        || vp->getSamplesPerPacket() != curConn.samplesPerPacket
        || vp->getTransmitBitrate() != curConn.transmitBitrate
        )
        throw cRuntimeError("Cannot change voice encoding parameters a during session");
}

void VoIPStreamReceiver::closeConnection()
{
    if (!curConn.offline) {
        curConn.offline = true;
        avcodec_close(curConn.decCtx);
        avcodec_free_context(&curConn.decCtx);
        curConn.outFile.close();
        emit(connStateSignal, -1L);    // so that sum() yields the number of active sessions
    }
}

void VoIPStreamReceiver::decodePacket(VoIPStreamPacket *vp)
{
    switch (vp->getType()) {
        case VOICE:
            emit(packetHasVoiceSignal, 1);
            break;

        case SILENCE:
            emit(packetHasVoiceSignal, 0);
            break;

        default:
            throw cRuntimeError("The received VoIPStreamPacket has unknown type %d", vp->getType());
    }
    uint16_t newSeqNo = vp->getSeqNo();
    if (newSeqNo > curConn.seqNo + 1)
        emit(lostPacketsSignal, newSeqNo - (curConn.seqNo + 1));

    // for fingerprint
    cHasher *hasher = simulation.getHasher();

    if (simTime() > curConn.lastPacketFinish) {
        int lostSamples = ceil(SIMTIME_DBL((simTime() - curConn.lastPacketFinish) * curConn.sampleRate));
        ASSERT(lostSamples > 0);
        EV_INFO << "Lost " << lostSamples << " samples\n";
        emit(lostSamplesSignal, lostSamples);
        curConn.writeLostSamples(lostSamples);
        curConn.lastPacketFinish += lostSamples * (1.0 / curConn.sampleRate);
        if (hasher)
            hasher->add(lostSamples);
    }
    emit(delaySignal, curConn.lastPacketFinish - vp->getCreationTime());
    curConn.seqNo = newSeqNo;

    int len = vp->getByteArray().getDataArraySize();
    uint8_t buff[len];
    vp->copyDataToBuffer(buff, len);
    curConn.writeAudioFrame(buff, len);
    if (hasher)
        hasher->add((const char *)buff, len);
}

void VoIPStreamReceiver::finish()
{
    EV_TRACE << "Sink finish()" << endl;
    closeConnection();
}

} // namespace inet

