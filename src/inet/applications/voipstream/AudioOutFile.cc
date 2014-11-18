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

// for INT64_C(x), UINT64_C(x):
#define __STDC_CONSTANT_MACROS
#include <stdint.h>

#include "inet/applications/voipstream/AudioOutFile.h"

#include "inet/common/INETEndians.h"

extern "C" {
#include <libavutil/audioconvert.h>
}

namespace inet {

void AudioOutFile::addAudioStream(enum AVCodecID codec_id, int sampleRate, short int sampleBits)
{
    AVStream *st = avformat_new_stream(oc, NULL);

    if (!st)
        throw cRuntimeError("Could not alloc stream\n");

    AVCodecContext *c = st->codec;
    c->codec_id = codec_id;
    c->codec_type = AVMEDIA_TYPE_AUDIO;

    /* put sample parameters */
    c->bit_rate = sampleRate * sampleBits;
    c->sample_rate = sampleRate;
    c->sample_fmt = AV_SAMPLE_FMT_S16;    //FIXME hack!
    c->channels = 1;
    audio_st = st;
}

void AudioOutFile::open(const char *resultFile, int sampleRate, short int sampleBits)
{
    ASSERT(!opened);

    opened = true;

    // auto detect the output format from the name. default is WAV
    AVOutputFormat *fmt = av_guess_format(NULL, resultFile, NULL);
    if (!fmt) {
        EV_WARN << "Could not deduce output format from file extension: using WAV.\n";
        fmt = av_guess_format("wav", NULL, NULL);
    }
    if (!fmt) {
        throw cRuntimeError("Could not find suitable output format for filename '%s'", resultFile);
    }

    // allocate the output media context
    oc = avformat_alloc_context();
    if (!oc)
        throw cRuntimeError("Memory error at avformat_alloc_context()");

    oc->oformat = fmt;
    snprintf(oc->filename, sizeof(oc->filename), "%s", resultFile);

    // add the audio stream using the default format codecs and initialize the codecs
    audio_st = NULL;
    if (fmt->audio_codec != AV_CODEC_ID_NONE)
        addAudioStream(fmt->audio_codec, sampleRate, sampleBits);

    av_dump_format(oc, 0, resultFile, 1);

    /* now that all the parameters are set, we can open the audio and
       video codecs and allocate the necessary encode buffers */
    if (audio_st) {
        AVCodecContext *c = audio_st->codec;

        /* find the audio encoder */
        AVCodec *avcodec = avcodec_find_encoder(c->codec_id);
        if (!avcodec)
            throw cRuntimeError("Codec %d not found", c->codec_id);

        /* open it */
        if (avcodec_open2(c, avcodec, NULL) < 0)
            throw cRuntimeError("Could not open codec %d", c->codec_id);
    }

    /* open the output file, if needed */
    if (!(fmt->flags & AVFMT_NOFILE)) {
        if (avio_open(&oc->pb, resultFile, AVIO_FLAG_WRITE) < 0)
            throw cRuntimeError("Could not open '%s'", resultFile);
    }

    // write the stream header
    avformat_write_header(oc, NULL);
}

void AudioOutFile::write(void *decBuf, int pktBytes)
{
    ASSERT(opened);

    AVCodecContext *c = audio_st->codec;
    short int bytesPerInSample = av_get_bytes_per_sample(c->sample_fmt);
    int samples = pktBytes / bytesPerInSample;

    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;
    AVFrame *frame = av_frame_alloc();

    frame->nb_samples = samples;

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(54, 28, 0)
    frame->channel_layout = AV_CH_LAYOUT_MONO;
    frame->sample_rate = c->sample_rate;
#endif // if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(54, 28, 0)

    int ret = avcodec_fill_audio_frame(frame,    /*channels*/ 1, c->sample_fmt,
                (const uint8_t *)(decBuf), pktBytes, 1);
    if (ret < 0)
        throw cRuntimeError("Error in avcodec_fill_audio_frame(): err=%d", ret);

    // The bitsPerOutSample is not 0 when codec is PCM.
    int gotPacket;
    ret = avcodec_encode_audio2(c, &pkt, frame, &gotPacket);
    if (ret < 0 || gotPacket != 1)
        throw cRuntimeError("avcodec_encode_audio() error: %d gotPacket: %d", ret, gotPacket);

    pkt.dts = 0;    //HACK for libav 11

    // write the compressed frame into the media file
    ret = av_interleaved_write_frame(oc, &pkt);
    if (ret != 0)
        throw cRuntimeError("Error while writing audio frame: %d", ret);
    av_frame_free(&frame);
}

bool AudioOutFile::close()
{
    if (!opened)
        return false;

    opened = false;

    /* write the trailer, if any.  the trailer must be written
     * before you close the CodecContexts open when you wrote the
     * header; otherwise write_trailer may try to use memory that
     * was freed on av_codec_close() */
    av_write_trailer(oc);

    /* close each codec */
    if (audio_st)
        avcodec_close(audio_st->codec);


    if (!(oc->oformat->flags & AVFMT_NOFILE)) {
        /* close the output file */
        avio_close(oc->pb);
    }

    /* free the stream */
    avformat_free_context(oc);
    oc = NULL;
    return true;
}

AudioOutFile::~AudioOutFile()
{
    close();
}

} // namespace inet

