//
// Copyright (C) 2005 M. Bohge (bohge@tkn.tu-berlin.de), M. Renwanz
// Copyright (C) 2010 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include <cstdarg>

#include "inet/applications/voipstream/VoipStreamSender.h"

#include "inet/applications/voipstream/VoipStreamPacket_m.h"
#include "inet/common/INETUtils.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"

namespace inet {

//#if defined(__clang__)
//#  pragma clang diagnostic ignored "-Wdeprecated-declarations"
//#elif defined(__GNUC__)
//#  pragma GCC diagnostic ignored "-Wdeprecated-declarations"
//#endif

extern "C" {
#include <libavutil/opt.h>
}

Define_Module(VoipStreamSender);

VoipStreamSender::VoipStreamSender()
{
}

VoipStreamSender::~VoipStreamSender()
{
    if (pEncoderCtx) {
        avcodec_close(pEncoderCtx);
        avcodec_free_context(&pEncoderCtx);
    }
    cancelAndDelete(timer);
}

VoipStreamSender::Buffer::Buffer() :
    samples(nullptr),
    bufferSize(0),
    readOffset(0),
    writeOffset(0)
{
}

VoipStreamSender::Buffer::~Buffer()
{
    delete[] samples;
}

void VoipStreamSender::Buffer::clear(int framesize)
{
    int newsize = BUFSIZE + framesize;
    if (bufferSize != newsize) {
        delete[] samples;
        bufferSize = newsize;
        samples = new char[bufferSize];
    }
    readOffset = 0;
    writeOffset = 0;
}

void VoipStreamSender::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        voipHeaderSize = par("voipHeaderSize");
        voipSilenceThreshold = par("voipSilenceThreshold");
        sampleRate = par("sampleRate");
        codec = par("codec");
        compressedBitRate = par("compressedBitRate");
        packetTimeLength = par("packetTimeLength");

        samplesPerPacket = (int)round(sampleRate * SIMTIME_DBL(packetTimeLength));
        if (samplesPerPacket & 1)
            samplesPerPacket++;
        EV_INFO << "The packetTimeLength parameter is " << packetTimeLength * 1000.0 << "ms, ";
        packetTimeLength = ((double)samplesPerPacket) / sampleRate;
        EV_INFO << "adjusted to " << packetTimeLength * 1000.0 << "ms" << endl;

        soundFile = par("soundFile");
        repeatCount = par("repeatCount");
        traceFileName = par("traceFileName");

        pReSampleCtx = nullptr;
        localPort = par("localPort");
        destPort = par("destPort");
        EV_DEBUG << "libavcodec: " << LIBAVCODEC_VERSION_MAJOR << "." << LIBAVCODEC_VERSION_MINOR << "." << LIBAVCODEC_VERSION_MICRO << endl;
        EV_DEBUG << "libavformat: " << LIBAVFORMAT_VERSION_MAJOR << "." << LIBAVFORMAT_VERSION_MINOR << "." << LIBAVFORMAT_VERSION_MICRO << endl;
        EV_DEBUG << "libavutil: " << LIBAVUTIL_VERSION_MAJOR << "." << LIBAVUTIL_VERSION_MINOR << "." << LIBAVUTIL_VERSION_MICRO << endl;
        EV_DEBUG << "libswresample: " << LIBSWRESAMPLE_VERSION_MAJOR << "." << LIBSWRESAMPLE_VERSION_MINOR << "." << LIBSWRESAMPLE_VERSION_MICRO << endl;
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        cModule *node = findContainingNode(this);
        NodeStatus *nodeStatus = node ? check_and_cast_nullable<NodeStatus *>(node->getSubmodule("status")) : nullptr;
        bool isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
        if (!isOperational)
            throw cRuntimeError("This module doesn't support starting in node DOWN state");

        // say HELLO to the world
        EV_TRACE << "VoIPSourceApp -> initialize(" << stage << ")" << endl;

        destAddress = L3AddressResolver().resolve(par("destAddress"));
        socket.setOutputGate(gate("socketOut"));

        socket.bind(localPort);

        int timeToLive = par("timeToLive");
        if (timeToLive != -1)
            socket.setTimeToLive(timeToLive);

        int dscp = par("dscp");
        if (dscp != -1)
            socket.setDscp(dscp);

        int tos = par("tos");
        if (tos != -1)
            socket.setTos(tos);

        simtime_t startTime = par("startTime");

        sampleBuffer.clear(0);

        // initialize avcodec library
        av_log_set_callback(&inet_av_log);

        openSoundFile(soundFile);

        timer = new cMessage("sendVoIP");
        scheduleAt(startTime, timer);

        voipSilencePacketSize = voipHeaderSize;

        // initialize the sequence number
        pktID = 1;
    }
}

void VoipStreamSender::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        Packet *packet;

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
                scheduleAfter(packetTimeLength, packet);
                scheduleAfter(packetTimeLength, msg);
            }
        }
        else {
            packet = check_and_cast<Packet *>(msg);
            emit(packetSentSignal, packet);
            socket.sendTo(packet, destAddress, destPort);
        }
    }
    else
        delete msg;
}

void VoipStreamSender::finish()
{
    outFile.close();

    if (pCodecCtx) {
        avcodec_close(pCodecCtx);
    }

    if (pReSampleCtx) {
        swr_close(pReSampleCtx);
        swr_free(&pReSampleCtx);
    }

    if (pFormatCtx) {
        avformat_close_input(&pFormatCtx);
    }
}

void VoipStreamSender::openSoundFile(const char *name)
{
    int err;

    err = avformat_open_input(&pFormatCtx, name, nullptr, nullptr);
    if (err < 0)
        throw cRuntimeError("Audiofile '%s' open error: (%d) %s", name, err, av_err2str(err));

    err = avformat_find_stream_info(pFormatCtx, nullptr);
    if (err < 0)
        throw cRuntimeError("Audiofile '%s' avformat_find_stream_info() error: (%d) %s", name, err, av_err2str(err));

    // get stream number
    streamIndex = -1;
    for (unsigned int j = 0; j < pFormatCtx->nb_streams; j++) {
        if (pFormatCtx->streams[j]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            streamIndex = j;
            break;
        }
    }

    if (streamIndex == -1)
        throw cRuntimeError("The file '%s' not contains any audio stream.", name);

    AVCodecParameters *codecPar = pFormatCtx->streams[streamIndex]->codecpar;

    // find decoder and open the correct codec
    pCodec = avcodec_find_decoder(codecPar->codec_id);
    if (!pCodec)
        throw cRuntimeError("Audiofile '%s' avcodec_find_decoder() error: decoder not found", name);
    pCodecCtx = avcodec_alloc_context3(pCodec);
    if (!pCodecCtx)
        throw cRuntimeError("avcodec_alloc_context3() failed");
    err = avcodec_parameters_to_context(pCodecCtx, codecPar);
    if (err < 0)
        throw cRuntimeError("avcodec_parameters_to_context() error: (%d) %s", err, av_err2str(err));
    err = avcodec_open2(pCodecCtx, pCodec, nullptr);
    if (err < 0)
        throw cRuntimeError("avcodec_open() error on file '%s': (%d) %s", name, err, av_err2str(err));

    // allocate encoder
    pEncoderCtx = avcodec_alloc_context3(nullptr);
    if (!pEncoderCtx)
        throw cRuntimeError("error occured in avcodec_alloc_context3()");
    // set bitrate:
    pEncoderCtx->bit_rate = compressedBitRate;
    pEncoderCtx->sample_rate = sampleRate;
#if LIBAVCODEC_VERSION_MAJOR < 59
    pEncoderCtx->channels = 1;
#else /* LIBAVCODEC_VERSION_MAJOR < 59 */
    pEncoderCtx->ch_layout = AV_CHANNEL_LAYOUT_MONO;
#endif /* LIBAVCODEC_VERSION_MAJOR < 59 */

    pCodecEncoder = avcodec_find_encoder_by_name(codec);
    if (!pCodecEncoder)
        throw cRuntimeError("Codec '%s' not found!", codec);

    pEncoderCtx->sample_fmt = pCodecEncoder->sample_fmts[0];

    if (avcodec_open2(pEncoderCtx, pCodecEncoder, nullptr) < 0)
        throw cRuntimeError("could not open %s encoding codec!", codec);

#if LIBAVCODEC_VERSION_MAJOR >= 59
    pEncoderCtx->frame_size = samplesPerPacket; // TODO required for g726 codec in libavcodec: 60.3.100 (KLUDGE?)
#endif /* LIBAVCODEC_VERSION_MAJOR >= 59 */

    pReSampleCtx = nullptr;
    if (pCodecCtx->sample_rate != sampleRate
        || pCodecCtx->sample_fmt != pEncoderCtx->sample_fmt
#if LIBAVCODEC_VERSION_MAJOR < 59
        || pCodecCtx->channels != 1)
#else /* LIBAVCODEC_VERSION_MAJOR < 59 */
        || pCodecCtx->ch_layout.nb_channels != 1)
#endif /* LIBAVCODEC_VERSION_MAJOR < 59 */
    {
#if LIBAVCODEC_VERSION_MAJOR < 59
        pReSampleCtx = swr_alloc();
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
#else /* LIBAVCODEC_VERSION_MAJOR < 59 */
        AVChannelLayout inChannelLayout = pCodecCtx->ch_layout;
        if (inChannelLayout.u.mask == 0)
            av_channel_layout_default(&inChannelLayout, pCodecCtx->ch_layout.nb_channels);
        ASSERT(inChannelLayout.u.mask != 0);
        AVSampleFormat inSampleFmt = pCodecCtx->sample_fmt;
        int inSampleRate = pCodecCtx->sample_rate;
        AVChannelLayout outChannelLayout = AV_CHANNEL_LAYOUT_MONO;
        AVSampleFormat outSampleFmt = pEncoderCtx->sample_fmt;
        int outSampleRate = sampleRate;
        err = swr_alloc_set_opts2(&pReSampleCtx,
                &outChannelLayout, outSampleFmt, outSampleRate,
                &inChannelLayout, inSampleFmt, inSampleRate,
                0, nullptr);
        if (err < 0)
            throw cRuntimeError("Error opening context, swr_alloc_set_opts2() returns (%d) %s", err, av_err2str(err));
        if (!pReSampleCtx)
            throw cRuntimeError("error in swr_alloc()");
#endif
        if (av_opt_set_int(pReSampleCtx, "internal_sample_fmt", AV_SAMPLE_FMT_FLTP, 0))
            throw cRuntimeError("error in option setting of 'internal_sample_fmt'");

        err = swr_init(pReSampleCtx);
        if (err < 0)
            throw cRuntimeError("Error opening context, swr_init() returns (%d) %s", err, av_err2str(err));
    }

    if (traceFileName && *traceFileName) {
        inet::utils::makePathForFile(traceFileName);
        outFile.open(traceFileName, sampleRate, 8 * av_get_bytes_per_sample(pEncoderCtx->sample_fmt));
    }

    sampleBuffer.clear(samplesPerPacket * av_get_bytes_per_sample(pEncoderCtx->sample_fmt));
    av_seek_frame(pFormatCtx, streamIndex, 0, 0);
}

Packet *VoipStreamSender::generatePacket()
{
    readFrame();

    if (sampleBuffer.empty())
        return nullptr;

    short int bytesPerInSample = av_get_bytes_per_sample(pEncoderCtx->sample_fmt);
    int samples = std::min(sampleBuffer.length() / bytesPerInSample, samplesPerPacket);
    bool isSilent = checkSilence(pEncoderCtx->sample_fmt, sampleBuffer.readPtr(), samples);
    const auto& vp = makeShared<VoipStreamPacket>();

    if (samples < samplesPerPacket && repeatCount > 1) {
        //padding last frame when the sending will be repeating from start
        int dataSize = (samplesPerPacket - samples) * bytesPerInSample;
        memset(sampleBuffer.writePtr(), 0, dataSize);
        sampleBuffer.notifyWrote(dataSize);
        samples = std::min(sampleBuffer.length() / (bytesPerInSample), samplesPerPacket);
    }
    int inBytes = samples * bytesPerInSample;

    AVPacket *opacket = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();
    frame->nb_samples = samples;
    frame->sample_rate = pEncoderCtx->sample_rate;
#if LIBAVCODEC_VERSION_MAJOR < 59
    frame->channel_layout = AV_CH_LAYOUT_MONO;
    frame->channels = pEncoderCtx->channels;
#else /* LIBAVCODEC_VERSION_MAJOR < 59 */
    frame->ch_layout = pEncoderCtx->ch_layout;
#endif /* LIBAVCODEC_VERSION_MAJOR < 59 */
    frame->format = pEncoderCtx->sample_fmt;

#if LIBAVCODEC_VERSION_MAJOR < 59
    int err = avcodec_fill_audio_frame(frame, pEncoderCtx->channels, pEncoderCtx->sample_fmt, (const uint8_t *)(sampleBuffer.readPtr()), inBytes, 1);
#else /* LIBAVCODEC_VERSION_MAJOR < 59 */
    int err = avcodec_fill_audio_frame(frame, pEncoderCtx->ch_layout.nb_channels, pEncoderCtx->sample_fmt, (const uint8_t *)(sampleBuffer.readPtr()), inBytes, 1);
#endif /* LIBAVCODEC_VERSION_MAJOR < 59 */
    if (err < 0)
        throw cRuntimeError("Error in avcodec_fill_audio_frame(): (%d) %s", err, av_err2str(err));

    err = avcodec_send_frame(pEncoderCtx, frame);
    if (err < 0)
        throw cRuntimeError("avcodec_send_frame() error: (%d) %s", err, av_err2str(err));
    err = avcodec_receive_packet(pEncoderCtx, opacket);
    if (err < 0)
        throw cRuntimeError("avcodec_receive_packet() error: (%d) %s", err, av_err2str(err));

    if (outFile.isOpen())
        outFile.write(sampleBuffer.readPtr(), inBytes);
    sampleBuffer.notifyRead(inBytes);

    Packet *pk = new Packet();
    if (isSilent) {
        pk->setName("SILENCE");
        vp->setType(SILENCE);
        vp->setChunkLength(B(voipSilencePacketSize));
        vp->setHeaderLength(voipSilencePacketSize);
        vp->setDataLength(0);
    }
    else {
        pk->setName("VOICE");
        vp->setType(VOICE);
        vp->setDataLength(opacket->size);
        vp->setChunkLength(B(voipHeaderSize));
        vp->setHeaderLength(voipHeaderSize);
        const auto& voice = makeShared<BytesChunk>(opacket->data, opacket->size);
        pk->insertAtFront(voice);
    }

    vp->setTimeStamp(pktID);
    vp->setSeqNo(pktID);
    vp->setCodec(pEncoderCtx->codec_id);
    vp->setSampleRate(sampleRate);
    vp->setSampleBits(pEncoderCtx->bits_per_coded_sample);
    vp->setSamplesPerPacket(samplesPerPacket);
    vp->setTransmitBitrate(compressedBitRate);
    pk->insertAtFront(vp);

    pktID++;

    av_packet_free(&opacket);
    av_frame_free(&frame);
    return pk;
}

bool VoipStreamSender::checkSilence(AVSampleFormat sampleFormat, void *_buf, int samples)
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

void VoipStreamSender::Buffer::align()
{
    if (readOffset) {
        if (length())
            memmove(samples, samples + readOffset, length());
        writeOffset -= readOffset;
        readOffset = 0;
    }
}

void VoipStreamSender::readFrame()
{
    short int inBytesPerSample = av_get_bytes_per_sample(pCodecCtx->sample_fmt);

    if (sampleBuffer.length() >= samplesPerPacket * inBytesPerSample)
        return;

    sampleBuffer.align();

    AVPacket *packet = av_packet_alloc();

    while (sampleBuffer.length() < samplesPerPacket * inBytesPerSample) {
        // read one frame
        int err = av_read_frame(pFormatCtx, packet);
        if (err < 0) { // end of file
            if (pReSampleCtx)
                resampleFrame(nullptr, 0);  // resample remainder data in internal buffer
            break;
        }

        // if the frame doesn't belong to our audiostream, continue... is not supposed to happen,
        // since .wav contain only one media stream
        if (packet->stream_index != streamIndex)
            continue;

#if LIBAVUTIL_VERSION_MAJOR < 57
        int skip_samples_size = 0;
#else
        size_t skip_samples_size = 0;
#endif
        const uint32_t* skip_samples_ptr = reinterpret_cast<const uint32_t*>(
                av_packet_get_side_data(packet, AV_PKT_DATA_SKIP_SAMPLES, &skip_samples_size));

        if (skip_samples_ptr && *skip_samples_ptr > 0) {
            size_t skipped_samples = *skip_samples_ptr;
            if (pReSampleCtx)
                skipped_samples = skipped_samples * sampleRate / pCodecCtx->sample_rate;
            int dataSize = skipped_samples * inBytesPerSample;
            memset(sampleBuffer.writePtr(), 0, dataSize);
            sampleBuffer.notifyWrote(dataSize);
        }

        // packet length == 0 ? read next packet
        if (packet->size == 0)
            continue;

        err = avcodec_send_packet(pCodecCtx, packet);
        if (err < 0)
            throw cRuntimeError("Error in avcodec_send_packet(): (%d) %s", err, av_err2str(err));

        AVFrame *frame = av_frame_alloc();
        while (true) {
            // decode audio and save the decoded samples in our buffer
            err = avcodec_receive_frame(pCodecCtx, frame);
            if (err == AVERROR(EAGAIN) || err == AVERROR_EOF)
                break;
            else if (err < 0)
                throw cRuntimeError("Error in avcodec_receive_frame(): (%d) %s", err, av_err2str(err));

            if (!pReSampleCtx) {
                // copy frame to sampleBuffer
                int dataSize = frame->nb_samples * inBytesPerSample;
                memcpy(sampleBuffer.writePtr(), *frame->extended_data, dataSize);
                sampleBuffer.notifyWrote(dataSize);
            }
            else {
                const uint8_t **in_data = (const uint8_t **)(frame->extended_data);
                resampleFrame(in_data, frame->nb_samples);
            }
        }
        av_frame_free(&frame);
    }
    av_packet_free(&packet);
}

void VoipStreamSender::resampleFrame(const uint8_t **in_data, int in_nb_samples)
{
    short int outBytesPerSample = av_get_bytes_per_sample(pEncoderCtx->sample_fmt);
    uint8_t *tmpSamples = new uint8_t[Buffer::BUFSIZE];
    uint8_t *out_data[1] = { nullptr };
    int maxOutSamples = sampleBuffer.availableSpace() / outBytesPerSample;
    int out_linesize;
    int err;

    err = av_samples_fill_arrays(out_data, &out_linesize, tmpSamples, 1, maxOutSamples, pEncoderCtx->sample_fmt, 0);
    if (err < 0)
        throw cRuntimeError("failed out_data fill arrays: (%d) %s", err, av_err2str(err));

    int resampled = swr_convert(pReSampleCtx, out_data, out_linesize, in_data, in_nb_samples);
    if (resampled < 0)
        throw cRuntimeError("swr_convert() returns error (%d) %s", resampled, av_err2str(resampled));
    if (swr_get_delay(pReSampleCtx, 0) > 0)
        throw cRuntimeError("%ld delay samples not converted\n", swr_get_delay(pReSampleCtx, 0));
    if (resampled > 0) {
        memcpy(sampleBuffer.writePtr(), out_data[0], resampled * outBytesPerSample);
        sampleBuffer.notifyWrote(resampled * outBytesPerSample);
    }
    delete[] tmpSamples;
}

} // namespace inet

