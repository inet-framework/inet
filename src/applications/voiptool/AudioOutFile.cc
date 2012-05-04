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

#include "AudioOutFile.h"

#include "INETEndians.h"


void AudioOutFile::addAudioStream(enum CodecID codec_id, int sampleRate, short int sampleBits)
{
#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(53,21,0)
    AVStream *st = av_new_stream(oc, 1);
#else
    AVStream *st = avformat_new_stream(oc, NULL);
#endif
    if (!st)
        throw cRuntimeError("Could not alloc stream\n");

    AVCodecContext *c = st->codec;
    c->codec_id = codec_id;
    c->codec_type = AVMEDIA_TYPE_AUDIO;

    /* put sample parameters */
    c->bit_rate = sampleRate * sampleBits;
    c->sample_rate = sampleRate;
    c->sample_fmt = SAMPLE_FMT_S16;  //FIXME hack!
    c->channels = 1;
    audio_st = st;
}

void AudioOutFile::open(const char *resultFile, int sampleRate, short int sampleBits)
{
    ASSERT(!opened);

    opened = true;

    // auto detect the output format from the name. default is WAV
    AVOutputFormat *fmt = av_guess_format(NULL, resultFile, NULL);
    if (!fmt)
    {
        ev << "Could not deduce output format from file extension: using WAV.\n";
        fmt = av_guess_format("wav", NULL, NULL);
    }
    if (!fmt)
    {
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
    if (fmt->audio_codec != CODEC_ID_NONE)
        addAudioStream(fmt->audio_codec, sampleRate, sampleBits);

    // set the output parameters (must be done even if no parameters).
    if (av_set_parameters(oc, NULL) < 0)
        throw cRuntimeError("Invalid output format parameters");

#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(53,21,0)
    dump_format(oc, 0, resultFile, 1);
#else
    av_dump_format(oc, 0, resultFile, 1);
#endif

    /* now that all the parameters are set, we can open the audio and
       video codecs and allocate the necessary encode buffers */
    if (audio_st)
    {
        AVCodecContext *c = audio_st->codec;

        /* find the audio encoder */
        AVCodec *avcodec = avcodec_find_encoder(c->codec_id);
        if (!avcodec)
            throw cRuntimeError("Codec %d not found", c->codec_id);

        /* open it */
#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(53,21,0)
        if (avcodec_open(c, avcodec) < 0)
#else
        if (avcodec_open2(c, avcodec, NULL) < 0)
#endif
            throw cRuntimeError("Could not open codec %d", c->codec_id);
    }

    /* open the output file, if needed */
    if (!(fmt->flags & AVFMT_NOFILE))
    {
#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(53,21,0)
        if (url_fopen(&oc->pb, resultFile, URL_WRONLY) < 0)
#else
        if (avio_open(&oc->pb, resultFile, URL_WRONLY) < 0)
#endif
            throw cRuntimeError("Could not open '%s'", resultFile);
    }

    // write the stream header
#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(53,21,0)
    av_write_header(oc);
#else
    avformat_write_header(oc, NULL);
#endif
}

void AudioOutFile::write(void *decBuf, int pktBytes)
{
    ASSERT(opened);

    AVCodecContext *c = audio_st->codec;
    uint8_t outbuf[pktBytes + FF_MIN_BUFFER_SIZE];
    AVPacket pkt;

    av_init_packet(&pkt);

    short int bitsPerInSample = av_get_bits_per_sample_format(c->sample_fmt);
    short int bitsPerOutSample = av_get_bits_per_sample(c->codec->id);
    // FFMPEG doc bug:
    // When codec is pcm or g726, the return value is count of output bytes,
    // and read (buf_size/(av_get_bits_per_sample(avctx->codec->id)/8)) samples from input buffer
    int samples = pktBytes * 8 / bitsPerInSample;
    int buf_size = (bitsPerOutSample) ? samples * bitsPerOutSample / 8 : samples;
    pkt.size = avcodec_encode_audio(c, outbuf, buf_size, (short int*)decBuf);
    if (c->coded_frame->pts != (int64_t)AV_NOPTS_VALUE)
        pkt.pts = av_rescale_q(c->coded_frame->pts, c->time_base, audio_st->time_base);
    pkt.flags |= AV_PKT_FLAG_KEY;
    pkt.stream_index = audio_st->index;
    pkt.data = outbuf;

    // write the compressed frame into the media file
    int ret = av_interleaved_write_frame(oc, &pkt);
    if (ret != 0)
        throw cRuntimeError("Error while writing audio frame: %d", ret);
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

    /* free the streams */
    for (unsigned int i = 0; i < oc->nb_streams; i++)
    {
        av_freep(&oc->streams[i]->codec);
        av_freep(&oc->streams[i]);
    }

    if (!(oc->oformat->flags & AVFMT_NOFILE))
    {
        /* close the output file */
#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(53,21,0)
        url_fclose(oc->pb);
#else
        avio_close(oc->pb);
#endif
    }

    /* free the stream */
    av_free(oc);
    oc = NULL;
    return true;
}

AudioOutFile::~AudioOutFile()
{
    close();
}
