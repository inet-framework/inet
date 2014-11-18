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

#include "inet/applications/voipstream/VoIPStreamSender.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/transportlayer/contract/udp/UDPControlInfo_m.h"

namespace inet {

extern "C" {
#include <libavutil/opt.h>
}

Define_Module(VoIPStreamSender);

simsignal_t VoIPStreamSender::sentPkSignal = registerSignal("sentPk");

VoIPStreamSender::VoIPStreamSender()
{
    codec = NULL;
    soundFile = NULL;
    pFormatCtx = NULL;
    pCodecCtx = NULL;
    pCodec = NULL;
    pReSampleCtx = NULL;
    pEncoderCtx = NULL;
    pCodecEncoder = NULL;
    timer = NULL;
}

VoIPStreamSender::~VoIPStreamSender()
{
    if (pEncoderCtx) {
        avcodec_close(pEncoderCtx);
        avcodec_free_context(&pEncoderCtx);
    }
    cancelAndDelete(timer);
    av_free_packet(&packet);
}

VoIPStreamSender::Buffer::Buffer() :
    samples(NULL),
    bufferSize(0),
    readOffset(0),
    writeOffset(0)
{
}

VoIPStreamSender::Buffer::~Buffer()
{
    delete[] samples;
}

void VoIPStreamSender::Buffer::clear(int framesize)
{
    int newsize = BUFSIZE + framesize;
    if (bufferSize != newsize) {
        delete [] samples;
        bufferSize = newsize;
        samples = new char[bufferSize];
    }
    readOffset = 0;
    writeOffset = 0;
}

void VoIPStreamSender::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        voipHeaderSize = par("voipHeaderSize");
        voipSilenceThreshold = par("voipSilenceThreshold");
        sampleRate = par("sampleRate");
        codec = par("codec").stringValue();
        compressedBitRate = par("compressedBitRate");
        packetTimeLength = par("packetTimeLength");

        samplesPerPacket = (int)round(sampleRate * SIMTIME_DBL(packetTimeLength));
        if (samplesPerPacket & 1)
            samplesPerPacket++;
        EV_INFO << "The packetTimeLength parameter is " << packetTimeLength * 1000.0 << "ms, ";
        packetTimeLength = ((double)samplesPerPacket) / sampleRate;
        EV_INFO << "adjusted to " << packetTimeLength * 1000.0 << "ms" << endl;

        soundFile = par("soundFile").stringValue();
        repeatCount = par("repeatCount");
        traceFileName = par("traceFileName").stringValue();

        pReSampleCtx = NULL;
        localPort = par("localPort");
        destPort = par("destPort");
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        bool isOperational;
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
        if (!isOperational)
            throw cRuntimeError("This module doesn't support starting in node DOWN state");

        // say HELLO to the world
        EV_TRACE << "VoIPSourceApp -> initialize(" << stage << ")" << endl;

        // Hack for create results folder
        recordScalar("hackForCreateResultsFolder", 0);

        destAddress = L3AddressResolver().resolve(par("destAddress").stringValue());
        socket.setOutputGate(gate("udpOut"));
        socket.bind(localPort);

        simtime_t startTime = par("startTime");

        sampleBuffer.clear(0);

        // initialize avcodec library
        av_register_all();
        avcodec_register_all();

        av_init_packet(&packet);

        openSoundFile(soundFile);

        timer = new cMessage("sendVoIP");
        scheduleAt(startTime, timer);

        voipSilencePacketSize = voipHeaderSize;

        // initialize the sequence number
        pktID = 1;
    }
}

void VoIPStreamSender::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        VoIPStreamPacket *packet;

        if (msg == timer) {
            packet = generatePacket();

            if (!packet) {
                if (repeatCount > 1) {
                    repeatCount--;
                    av_seek_frame(pFormatCtx, streamIndex, 0, 0);
                    packet = generatePacket();
                }
            }

            if (packet) {
                // reschedule trigger message
                scheduleAt(simTime() + packetTimeLength, packet);
                scheduleAt(simTime() + packetTimeLength, msg);
            }
        }
        else {
            packet = check_and_cast<VoIPStreamPacket *>(msg);
            emit(sentPkSignal, packet);
            socket.sendTo(packet, destAddress, destPort);
        }
    }
    else
        delete msg;
}

void VoIPStreamSender::finish()
{
    av_free_packet(&packet);
    outFile.close();

    if (pCodecCtx) {
        avcodec_close(pCodecCtx);
    }
    if (pReSampleCtx) {
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(54, 28, 0)
        avresample_close(pReSampleCtx);
        avresample_free(&pReSampleCtx);
#else // if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(54, 28, 0)
        audio_resample_close(pReSampleCtx);
#endif // if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(54, 28, 0)
        pReSampleCtx = NULL;
    }

    if (pFormatCtx) {
        avformat_close_input(&pFormatCtx);
    }
}

void VoIPStreamSender::openSoundFile(const char *name)
{
    int ret;

    ret = avformat_open_input(&pFormatCtx, name, NULL, NULL);
    if (ret < 0)
        throw cRuntimeError("Audiofile '%s' open error: %d", name, ret);

    ret = avformat_find_stream_info(pFormatCtx, NULL);
    if (ret < 0)
        throw cRuntimeError("Audiofile '%s' avformat_find_stream_info() error: %d", name, ret);

    //get stream number
    streamIndex = -1;
    for (unsigned int j = 0; j < pFormatCtx->nb_streams; j++) {
        if (pFormatCtx->streams[j]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            streamIndex = j;
            break;
        }
    }

    if (streamIndex == -1)
        throw cRuntimeError("The file '%s' not contains any audio stream.", name);

    pCodecCtx = pFormatCtx->streams[streamIndex]->codec;

    //find decoder and open the correct codec
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if (!pCodec)
        throw cRuntimeError("Audiofile '%s' avcodec_find_decoder() error: decoder not found", name);

    ret = avcodec_open2(pCodecCtx, pCodec, NULL);
    if (ret < 0)
        throw cRuntimeError("avcodec_open() error on file '%s': %d", name, ret);

    //allocate encoder
    pEncoderCtx = avcodec_alloc_context3(NULL);
    if (!pEncoderCtx)
        throw cRuntimeError("error occured in avcodec_alloc_context3()");
    //set bitrate:
    pEncoderCtx->bit_rate = compressedBitRate;

    pEncoderCtx->sample_rate = sampleRate;
    pEncoderCtx->channels = 1;

    pCodecEncoder = avcodec_find_encoder_by_name(codec);
    if (!pCodecEncoder)
        throw cRuntimeError("Codec '%s' not found!", codec);

    pEncoderCtx->sample_fmt = pCodecCtx->sample_fmt;    // FIXME hack!

    if (avcodec_open2(pEncoderCtx, pCodecEncoder, NULL) < 0)
        throw cRuntimeError("could not open %s encoding codec!", codec);

    if (pCodecCtx->sample_rate == sampleRate
        && pCodecCtx->sample_fmt == pEncoderCtx->sample_fmt
        && pCodecCtx->channels == 1)
    {
        pReSampleCtx = NULL;
    }
    else {
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(54, 28, 0)
        pReSampleCtx = avresample_alloc_context();
        if (!pReSampleCtx)
            throw cRuntimeError("error in av_audio_resample_init()");

        int inChannelLayout = pCodecCtx->channel_layout == 0 ? av_get_default_channel_layout(pCodecCtx->channels) : pCodecCtx->channel_layout;
        if (av_opt_set_int(pReSampleCtx, "in_channel_layout", inChannelLayout, 0))
            throw cRuntimeError("error in option setting of 'in_channel_layout'");
        if (av_opt_set_int(pReSampleCtx, "in_sample_fmt", pCodecCtx->sample_fmt, 0))
            throw cRuntimeError("error in option setting of 'in_sample_fmt'");
        if (av_opt_set_int(pReSampleCtx, "in_sample_rate", pCodecCtx->sample_rate, 0))
            throw cRuntimeError("error in option setting of 'in_sample_rate'");
        if (av_opt_set_int(pReSampleCtx, "out_channel_layout", AV_CH_LAYOUT_MONO, 0))
            throw cRuntimeError("error in option setting of 'out_channel_layout'");
        if (av_opt_set_int(pReSampleCtx, "out_sample_fmt", pEncoderCtx->sample_fmt, 0))
            throw cRuntimeError("error in option setting of 'out_sample_fmt'");
        if (av_opt_set_int(pReSampleCtx, "out_sample_rate", sampleRate, 0))
            throw cRuntimeError("error in option setting of 'out_sample_rate'");
        if (av_opt_set_int(pReSampleCtx, "internal_sample_fmt", AV_SAMPLE_FMT_FLTP, 0))
            throw cRuntimeError("error in option setting of 'internal_sample_fmt'");

        ret = avresample_open(pReSampleCtx);
        if (ret < 0)
            throw cRuntimeError("Error opening context");
#else // if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(54, 28, 0)
        pReSampleCtx = av_audio_resample_init(1, pCodecCtx->channels, sampleRate, pCodecCtx->sample_rate,
                    pEncoderCtx->sample_fmt, pCodecCtx->sample_fmt, 16, 10, 0, 0.8);
        // parameters copied from the implementation of deprecated audio_resample_init()
        // begin HACK
        long int sec = 2;
        short int *inb = new short int[sec * pCodecCtx->channels * pCodecCtx->sample_rate * av_get_bits_per_sample_format(pCodecCtx->sample_fmt) / (8 * sizeof(short int))];
        short int *outb = new short int[sec * sampleRate * av_get_bits_per_sample_format(pEncoderCtx->sample_fmt) / (8 * sizeof(short int)) + 16];
        int decoded = audio_resample(pReSampleCtx, outb, inb, sec * pCodecCtx->sample_rate);
        EV_DEBUG << "decoded:" << decoded << endl;
        delete[] inb;
        delete[] outb;
        // end HACK
#endif // if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(54, 28, 0)
    }

    if (traceFileName && *traceFileName)
        outFile.open(traceFileName, sampleRate, 8 * av_get_bytes_per_sample(pEncoderCtx->sample_fmt));

    sampleBuffer.clear(samplesPerPacket * av_get_bytes_per_sample(pEncoderCtx->sample_fmt));
}

VoIPStreamPacket *VoIPStreamSender::generatePacket()
{
    readFrame();

    if (sampleBuffer.empty())
        return NULL;

    short int bytesPerInSample = av_get_bytes_per_sample(pEncoderCtx->sample_fmt);
    int samples = std::min(sampleBuffer.length() / (bytesPerInSample), samplesPerPacket);
    int inBytes = samples * bytesPerInSample;
    bool isSilent = checkSilence(pEncoderCtx->sample_fmt, sampleBuffer.readPtr(), samples);
    VoIPStreamPacket *vp = new VoIPStreamPacket();

    AVPacket opacket;
    av_init_packet(&opacket);
    opacket.data = NULL;
    opacket.size = 0;
    AVFrame *frame = av_frame_alloc();

    frame->nb_samples = samples;

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(54, 28, 0)
    frame->channel_layout = AV_CH_LAYOUT_MONO;
    frame->sample_rate = pEncoderCtx->sample_rate;
#endif // if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(54, 28, 0)

    int ret = avcodec_fill_audio_frame(frame,    /*channels*/ 1, pEncoderCtx->sample_fmt,
                (const uint8_t *)(sampleBuffer.readPtr()), inBytes, 1);
    if (ret < 0)
        throw cRuntimeError("Error in avcodec_fill_audio_frame(): err=%d", ret);

    // The bitsPerOutSample is not 0 when codec is PCM.
    int gotPacket;
    ret = avcodec_encode_audio2(pEncoderCtx, &opacket, frame, &gotPacket);
    if (ret < 0 || gotPacket != 1)
        throw cRuntimeError("avcodec_encode_audio() error: %d gotPacket: %d", ret, gotPacket);

    if (outFile.isOpen())
        outFile.write(sampleBuffer.readPtr(), inBytes);
    sampleBuffer.notifyRead(inBytes);

    vp->setDataFromBuffer(opacket.data, opacket.size);

    if (isSilent) {
        vp->setName("SILENCE");
        vp->setType(SILENCE);
        vp->setByteLength(voipSilencePacketSize);
    }
    else {
        vp->setName("VOICE");
        vp->setType(VOICE);
        vp->setByteLength(voipHeaderSize + opacket.size);
    }

    vp->setTimeStamp(pktID);
    vp->setSeqNo(pktID);
    vp->setCodec(pEncoderCtx->codec_id);
    vp->setSampleRate(sampleRate);
    vp->setSampleBits(pEncoderCtx->bits_per_coded_sample);
    vp->setSamplesPerPacket(samplesPerPacket);
    vp->setTransmitBitrate(compressedBitRate);

    pktID++;

    av_free_packet(&opacket);
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(54, 28, 0)
    av_frame_free(&frame);
#else // if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(54, 28, 0)
    av_freep(&frame);
#endif // if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(54, 28, 0)
    return vp;
}

bool VoIPStreamSender::checkSilence(AVSampleFormat sampleFormat, void *_buf, int samples)
{
    int max = 0;
    int i;

    switch (sampleFormat) {
        case AV_SAMPLE_FMT_U8: {
            uint8_t *buf = (uint8_t *)_buf;
            for (i = 0; i < samples; ++i) {
                int s = abs(int(buf[i]) - 0x80);
                if (s > max)
                    max = s;
            }
        }
        break;

        case AV_SAMPLE_FMT_S16: {
            int16_t *buf = (int16_t *)_buf;
            for (i = 0; i < samples; ++i) {
                int s = abs(buf[i]);
                if (s > max)
                    max = s;
            }
        }
        break;

        case AV_SAMPLE_FMT_S32: {
            int32_t *buf = (int32_t *)_buf;

            for (i = 0; i < samples; ++i) {
                int s = abs(buf[i]);

                if (s > max)
                    max = s;
            }
        }
        break;

        default:
            throw cRuntimeError("Invalid sampleFormat: %d", sampleFormat);
    }

    return max < voipSilenceThreshold;
}

void VoIPStreamSender::Buffer::align()
{
    if (readOffset) {
        if (length())
            memcpy(samples, samples + readOffset, length());
        writeOffset -= readOffset;
        readOffset = 0;
    }
}

void VoIPStreamSender::readFrame()
{
    short int inBytesPerSample = av_get_bytes_per_sample(pEncoderCtx->sample_fmt);
    short int outBytesPerSample = av_get_bytes_per_sample(pCodecCtx->sample_fmt);
    if (sampleBuffer.length() >= samplesPerPacket * inBytesPerSample)
        return;

    sampleBuffer.align();

    while (sampleBuffer.length() < samplesPerPacket * inBytesPerSample) {
        //read one frame
        int err = av_read_frame(pFormatCtx, &packet);
        if (err < 0)
            break;

        // if the frame doesn't belong to our audiostream, continue... is not supposed to happen,
        // since .wav contain only one media stream
        if (packet.stream_index != streamIndex)
            continue;

        // packet length == 0 ? read next packet
        if (packet.size == 0)
            continue;

        AVPacket avpkt;
        avpkt.data = NULL;
        avpkt.size = 0;
        av_init_packet(&avpkt);
        ASSERT(avpkt.data == NULL && avpkt.size == 0);
        avpkt.data = packet.data;
        avpkt.size = packet.size;

        while (avpkt.size > 0) {
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(54, 28, 0)
            // decode audio and save the decoded samples in our buffer
            AVFrame *frame = av_frame_alloc();
            int gotFrame;
            int decoded = avcodec_decode_audio4(pCodecCtx, frame, &gotFrame, &avpkt);
            if (decoded < 0 || !gotFrame)
                throw cRuntimeError("Error in avcodec_decode_audio4(), err=%d, gotFrame=%d", decoded, gotFrame);

            avpkt.data += decoded;
            avpkt.size -= decoded;

            if (!pReSampleCtx) {
                // copy frame to sampleBuffer
                int dataSize = av_samples_get_buffer_size(NULL, pCodecCtx->channels, frame->nb_samples, pCodecCtx->sample_fmt, 1);
                memcpy(sampleBuffer.writePtr(), frame->data[0], dataSize);
                sampleBuffer.notifyWrote(dataSize);
            }
            else {
                uint8_t *tmpSamples = new uint8_t[Buffer::BUFSIZE];

                uint8_t **in_data = frame->extended_data;
                int in_linesize = frame->linesize[0];
                int in_nb_samples = frame->nb_samples;

                uint8_t *out_data[AVRESAMPLE_MAX_CHANNELS] = {
                    0
                };
                int maxOutSamples = sampleBuffer.availableSpace() / outBytesPerSample;
                int out_linesize;
                int ret;
                ret = av_samples_fill_arrays(out_data, &out_linesize, tmpSamples,
                            1, maxOutSamples,
                            pEncoderCtx->sample_fmt, 0);
                if (ret < 0)
                    throw cRuntimeError("failed out_data fill arrays");

                decoded = avresample_convert(pReSampleCtx, out_data, out_linesize, decoded,
                            in_data, in_linesize, in_nb_samples);
                if (!decoded)
                    throw cRuntimeError("audio_resample() returns error");
//                if (avresample_get_delay(pReSampleCtx) > 0)
//                    throw cRuntimeError("%d delay samples not converted\n", avresample_get_delay(pReSampleCtx));
//                if (avresample_available(pReSampleCtx) > 0)
//                    throw cRuntimeError("%d samples available for output\n", avresample_available(pReSampleCtx));
                memcpy(sampleBuffer.writePtr(), out_data[0], decoded * outBytesPerSample);
                sampleBuffer.notifyWrote(decoded * outBytesPerSample);
                delete[] tmpSamples;
            }
            av_frame_free(&frame);
#else // if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(54, 28, 0)
            uint8_t *tmpSamples = new uint8_t[Buffer::BUFSIZE];

            int16_t *rbuf, *nbuf;
            nbuf = (int16_t *)(sampleBuffer.writePtr());
            rbuf = (pReSampleCtx) ? (int16_t *)tmpSamples : nbuf;

            int frame_size = (pReSampleCtx) ? Buffer::BUFSIZE : sampleBuffer.availableSpace();
            memset(rbuf, 0, frame_size);
            int decoded = avcodec_decode_audio3(pCodecCtx, rbuf, &frame_size, &avpkt);

            if (decoded < 0)
                throw cRuntimeError("Error decoding frame, err=%d", decoded);

            avpkt.data += decoded;
            avpkt.size -= decoded;

            if (frame_size == 0)
                continue;

            decoded = frame_size / (inBytesPerSample * pCodecCtx->channels);
            ASSERT(frame_size == decoded * inBytesPerSample * pCodecCtx->channels);

            if (pReSampleCtx)
                decoded = audio_resample(pReSampleCtx, nbuf, rbuf, decoded);

            sampleBuffer.notifyWrote(decoded * inBytesPerSample);
            delete[] tmpSamples;
#endif // if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(54, 28, 0)
            av_free_packet(&avpkt);
        }
        av_free_packet(&packet);
    }
}

} // namespace inet

