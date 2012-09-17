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


#include "VoIPStreamReceiver.h"

#include "INETEndians.h"


Define_Module(VoIPStreamReceiver);

simsignal_t VoIPStreamReceiver::rcvdPkSignal = SIMSIGNAL_NULL;
simsignal_t VoIPStreamReceiver::lostSamplesSignal = SIMSIGNAL_NULL;
simsignal_t VoIPStreamReceiver::lostPacketsSignal = SIMSIGNAL_NULL;
simsignal_t VoIPStreamReceiver::dropPkSignal = SIMSIGNAL_NULL;
simsignal_t VoIPStreamReceiver::packetHasVoiceSignal = SIMSIGNAL_NULL;
simsignal_t VoIPStreamReceiver::connStateSignal = SIMSIGNAL_NULL;
simsignal_t VoIPStreamReceiver::delaySignal = SIMSIGNAL_NULL;

VoIPStreamReceiver::~VoIPStreamReceiver()
{
    closeConnection();
}

void VoIPStreamReceiver::initSignals()
{
    rcvdPkSignal = registerSignal("rcvdPk");
    lostSamplesSignal = registerSignal("lostSamples");
    lostPacketsSignal = registerSignal("lostPackets");
    dropPkSignal = registerSignal("dropPk");
    packetHasVoiceSignal = registerSignal("packetHasVoice");
    connStateSignal = registerSignal("connState");
    delaySignal = registerSignal("delay");
}

void VoIPStreamReceiver::initialize()
{
    initSignals();

    // Hack for create results folder
    recordScalar("hackForCreateResultsFolder", 0);

    // Say Hello to the world
    ev << "VoIPSinkApp initialize()" << endl;

    // read parameters
    localPort = par("localPort");
    resultFile = par("resultFile");
    playoutDelay = par("playoutDelay");

    // initialize avcodec library
    av_register_all();

    socket.setOutputGate(gate("udpOut"));
    socket.bind(localPort);
}

void VoIPStreamReceiver::handleMessage(cMessage *msg)
{
    if (msg->getKind() == UDP_I_ERROR)
    {
        delete msg;
        return;
    }

    VoIPStreamPacket *vp = check_and_cast<VoIPStreamPacket *>(msg);
    bool ok = true;
    if (curConn.offline)
        createConnection(vp);
    else
    {
        checkSourceAndParameters(vp);
        ok = vp->getSeqNo() > curConn.seqNo && vp->getTimeStamp() > curConn.timeStamp;
    }
    if (ok)
    {
        emit(rcvdPkSignal, vp);
        decodePacket(vp);
    }
    else
        emit(dropPkSignal, msg);

    delete msg;
}

void VoIPStreamReceiver::Connection::openAudio(const char *fileName)
{
/*
    AVCodecContext *c;
    AVCodec *avcodec;

    c = audio_st->codec;

    // find the audio encoder
    avcodec = avcodec_find_encoder(c->codec_id);
    if (!avcodec)
        throw cRuntimeError("Codec %d not found", c->codec_id);

    // open it
    if (avcodec_open(c, avcodec) < 0)
        throw cRuntimeError("Could not open codec %d", c->codec_id);
*/
    outFile.open(fileName, sampleRate, av_get_bits_per_sample_format(decCtx->sample_fmt));
}

void VoIPStreamReceiver::Connection::writeLostSamples(int sampleCount)
{
    int pktBytes = sampleCount * av_get_bits_per_sample_format(decCtx->sample_fmt) / 8;
    if (outFile.isOpen())
    {
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

    int decBufSize = AVCODEC_MAX_AUDIO_FRAME_SIZE;
    int16_t *decBuf = new int16_t[decBufSize]; // output is 16bit
//    int ret = avcodec_decode_audio2(decCtx, decBuf, &decBufSize, inbuf, inbytes);
    int ret = avcodec_decode_audio3(decCtx, decBuf, &decBufSize, &avpkt);
    if (ret < 0)
        throw cRuntimeError("avcodec_decode_audio2(): received packet decoding error: %d", ret);

    lastPacketFinish += simtime_t(1.0) * (decBufSize * 8 / av_get_bits_per_sample_format(decCtx->sample_fmt)) / sampleRate;
    if (outFile.isOpen())
        outFile.write(decBuf, decBufSize);
    delete [] decBuf;
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
    curConn.codec = (enum CodecID)(vp->getCodec());
    curConn.sampleBits = vp->getSampleBits();
    curConn.sampleRate = vp->getSampleRate();
    curConn.transmitBitrate = vp->getTransmitBitrate();
    curConn.samplesPerPacket = vp->getSamplesPerPacket();
    curConn.lastPacketFinish = simTime() + playoutDelay;

    curConn.pCodecDec = avcodec_find_decoder(curConn.codec);
    if (curConn.pCodecDec == NULL)
        error("Codec %d not found", curConn.codec);

#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(53,21,0)
    curConn.decCtx = avcodec_alloc_context();
#else
    curConn.decCtx = avcodec_alloc_context3(curConn.pCodecDec);
#endif

    curConn.decCtx->bit_rate = curConn.transmitBitrate;
    curConn.decCtx->sample_rate = curConn.sampleRate;
    curConn.decCtx->channels = 1;
    curConn.decCtx->bits_per_coded_sample = curConn.sampleBits;

    int ret = avcodec_open(curConn.decCtx, curConn.pCodecDec);
    if (ret < 0)
        error("could not open decoding codec %d (%s): err=%d", curConn.codec, curConn.pCodecDec->name, ret);

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
    if (!curConn.offline)
    {
        curConn.offline = true;
        avcodec_close(curConn.decCtx);
        curConn.outFile.close();
        emit(connStateSignal, -1L); // so that sum() yields the number of active sessions
    }
}

void VoIPStreamReceiver::decodePacket(VoIPStreamPacket *vp)
{
    switch (vp->getType())
    {
        case VOICE:
            emit(packetHasVoiceSignal, 1);
            break;

        case SILENCE:
            emit(packetHasVoiceSignal, 0);
            break;

        default:
            error("The received VoIPStreamPacket has unknown type %d", vp->getType());
            return;
    }
    uint16_t newSeqNo = vp->getSeqNo();
    if (newSeqNo > curConn.seqNo + 1)
        emit(lostPacketsSignal, newSeqNo - (curConn.seqNo + 1));

    // for fingerprint
    cHasher *hasher = simulation.getHasher();

    if (simTime() > curConn.lastPacketFinish)
    {
        int lostSamples = ceil(SIMTIME_DBL((simTime() - curConn.lastPacketFinish) * curConn.sampleRate));
        ASSERT(lostSamples > 0);
        ev << "Lost " << lostSamples << " samples\n";
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
    ev << "Sink finish()" << endl;
    closeConnection();
}

