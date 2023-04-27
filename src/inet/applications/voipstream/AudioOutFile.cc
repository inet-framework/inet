//
// Copyright (C) 2005 M. Bohge (bohge@tkn.tu-berlin.de), M. Renwanz
// Copyright (C) 2010 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

// for INT64_C(x), UINT64_C(x):
#define __STDC_CONSTANT_MACROS

#include <cstdarg>
#include <stdint.h>

#include "inet/applications/voipstream/AudioOutFile.h"

namespace inet {

//#if defined(__clang__)
//#  pragma clang diagnostic ignored "-Wdeprecated-declarations"
//#elif defined(__GNUC__)
//#  pragma GCC diagnostic ignored "-Wdeprecated-declarations"
//#endif

static void *getThisPtr() { return cSimulation::getActiveSimulation()->getContextModule(); }
void inet_av_log(void *avcontext, int level, const char *format, va_list va)
{
    char buffer[1024];
    int l = vsnprintf(buffer, 1023, format, va);
    *(buffer + l) = 0;
    EV_DEBUG << "av_log: " << buffer;
}

void AudioOutFile::open(const char *resultFile, int sampleRate, short int sampleBits)
{
    int err;
    ASSERT(!opened);

    opened = true;

    // auto detect the output format from the name. default is WAV
#if LIBAVFORMAT_VERSION_MAJOR >= 59
    const
#endif
    AVOutputFormat *fmt = av_guess_format(nullptr, resultFile, nullptr);
    if (!fmt) {
        EV_WARN << "Could not deduce output format from file extension: using WAV.\n";
        fmt = av_guess_format("wav", nullptr, nullptr);
    }
    if (!fmt) {
        throw cRuntimeError("Could not find suitable output format for filename '%s'", resultFile);
    }

    // allocate the output media context
    oc = avformat_alloc_context();
    if (!oc)
        throw cRuntimeError("Memory error at avformat_alloc_context()");

    oc->oformat = fmt;
    oc->url = av_strdup(resultFile);

    // add the audio stream using the default format codecs and initialize the codecs
    audio_st = nullptr;
    if (fmt->audio_codec != AV_CODEC_ID_NONE) {
        audio_st = avformat_new_stream(oc, nullptr);

        if (!audio_st)
            throw cRuntimeError("Could not alloc stream\n");

        AVCodecParameters *p = audio_st->codecpar;
        p->codec_id = fmt->audio_codec;
        p->codec_type = AVMEDIA_TYPE_AUDIO;
        p->bit_rate = sampleRate * sampleBits;
        p->sample_rate = sampleRate;
#if LIBAVCODEC_VERSION_MAJOR < 59
        p->channels = 1;
#else /* LIBAVCODEC_VERSION_MAJOR < 59 */
        p->ch_layout = AV_CHANNEL_LAYOUT_MONO;
#endif /* LIBAVCODEC_VERSION_MAJOR < 59 */
    }

    av_dump_format(oc, 0, resultFile, 1);

    // now that all the parameters are set, we can open the audio and
    // video codecs and allocate the necessary encode buffers
    if (audio_st) {
        AVCodecParameters *codecPar = audio_st->codecpar;

        // find the audio encoder
#if LIBAVCODEC_VERSION_MAJOR >= 59
        const
#endif
        AVCodec *avcodec = avcodec_find_encoder(codecPar->codec_id);
        if (!avcodec)
            throw cRuntimeError("Codec %d not found", codecPar->codec_id);
        codecCtx = avcodec_alloc_context3(avcodec);
        if (!codecCtx)
            throw cRuntimeError("avcodec_alloc_context3() failed");
        err = avcodec_parameters_to_context(codecCtx, codecPar);
        if (err < 0)
            throw cRuntimeError("avcodec_parameters_to_context() error: (%d) %s", err, av_err2str(err));

        codecCtx->sample_fmt = AV_SAMPLE_FMT_S16; // FIXME hack!

        // open it
        err = avcodec_open2(codecCtx, avcodec, nullptr);
        if (err < 0)
            throw cRuntimeError("Could not open codec %d: error=(%d) %s", codecPar->codec_id, err, av_err2str(err));
    }

    // open the output file, if needed
    if (!(fmt->flags & AVFMT_NOFILE)) {
        err = avio_open(&oc->pb, resultFile, AVIO_FLAG_WRITE);
        if (err < 0)
            throw cRuntimeError("Could not open '%s': error=(%d) %s", resultFile, err, av_err2str(err));
    }

    // write the stream header
    err = avformat_write_header(oc, nullptr);
    if (err < 0)
        throw cRuntimeError("Could not write header to '%s', error=(%d) %s", resultFile, err, av_err2str(err));
}

void AudioOutFile::write(void *decBuf, int pktBytes)
{
    ASSERT(opened);

    short int bytesPerInSample = av_get_bytes_per_sample(codecCtx->sample_fmt);
    int samples = pktBytes / bytesPerInSample;

    AVFrame *frame = av_frame_alloc();
    frame->nb_samples = samples;
    frame->sample_rate = codecCtx->sample_rate;
#if LIBAVCODEC_VERSION_MAJOR < 59
    frame->channel_layout = AV_CH_LAYOUT_MONO;
    frame->channels = codecCtx->channels;
#else /* LIBAVCODEC_VERSION_MAJOR < 59 */
    frame->ch_layout = codecCtx->ch_layout;
    frame->ch_layout.u.mask = AV_CH_LAYOUT_MONO;
#endif /* LIBAVCODEC_VERSION_MAJOR < 59 */
    frame->format = codecCtx->sample_fmt;
    int err = avcodec_fill_audio_frame(frame, /*channels*/ 1, codecCtx->sample_fmt, (const uint8_t *)(decBuf), pktBytes, 1);
    if (err < 0)
        throw cRuntimeError("Error in avcodec_fill_audio_frame(): error (%d) %s", err, av_err2str(err));

    // The bitsPerOutSample is not 0 when codec is PCM.
#if LIBAVCODEC_VERSION_MAJOR < 59
    frame->channels = codecCtx->channels;
#endif /* LIBAVCODEC_VERSION_MAJOR < 59 */
    frame->format = codecCtx->sample_fmt;
    err = avcodec_send_frame(codecCtx, frame);
    if (err < 0)
        throw cRuntimeError("avcodec_send_frame() error: (%d) %s", err, av_err2str(err));
    AVPacket *pkt = av_packet_alloc();
    while(true) {
        err = avcodec_receive_packet(codecCtx, pkt);
        if (err == AVERROR(EAGAIN) || err == AVERROR_EOF)
            break;
        else if (err < 0)
            throw cRuntimeError("avcodec_receive_packet() error: (%d) %s", err, av_err2str(err));

        pkt->dts = 0; // HACK for libav 11

        // write the compressed frame into the media file
        err = av_interleaved_write_frame(oc, pkt);
        if (err != 0)
            throw cRuntimeError("Error while writing audio frame: (%d) %s", err, av_err2str(err));
    }
    av_packet_free(&pkt);
    av_frame_free(&frame);
}

bool AudioOutFile::close()
{
    if (!opened)
        return false;

    int err = avcodec_send_frame(codecCtx, nullptr);
    if (err < 0)
        throw cRuntimeError("avcodec_send_frame() error: (%d) %s", err, av_err2str(err));
    AVPacket *pkt = av_packet_alloc();
    while(true) {
        err = avcodec_receive_packet(codecCtx, pkt);
        if (err == AVERROR(EAGAIN) || err == AVERROR_EOF)
            break;
        else if (err < 0)
            throw cRuntimeError("avcodec_receive_packet() error: (%d) %s", err, av_err2str(err));

        pkt->dts = 0; // HACK for libav 11

        // write the compressed frame into the media file
        err = av_interleaved_write_frame(oc, pkt);
        if (err != 0)
            throw cRuntimeError("Error while writing audio frame: (%d) %s", err, av_err2str(err));
    }
    av_packet_free(&pkt);

    opened = false;

    // write the trailer, if any.  the trailer must be written
    // before you close the CodecContexts open when you wrote the
    // header; otherwise write_trailer may try to use memory that
    // was freed on av_codec_close()
    av_write_trailer(oc);

    // close each codec
    if (audio_st)
        avcodec_close(codecCtx);

    if (!(oc->oformat->flags & AVFMT_NOFILE)) {
        // close the output file
        avio_close(oc->pb);
    }

    // free the stream
    avformat_free_context(oc);
    oc = nullptr;
    return true;
}

AudioOutFile::~AudioOutFile()
{
    close();
}

} // namespace inet

