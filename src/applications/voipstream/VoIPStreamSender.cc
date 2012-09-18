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

#include "VoIPStreamSender.h"

#include "UDPControlInfo_m.h"

Define_Module(VoIPStreamSender);


simsignal_t VoIPStreamSender::sentPkSignal = SIMSIGNAL_NULL;

VoIPStreamSender::~VoIPStreamSender()
{
    cancelAndDelete(timer);
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
    delete [] samples;
}

void VoIPStreamSender::Buffer::clear(int framesize)
{
    delete samples;
    bufferSize = BUFSIZE + framesize;
    samples = new char[bufferSize];
    readOffset = 0;
    writeOffset = 0;
}

void VoIPStreamSender::initialize(int stage)
{
    if (stage != 3)  //wait until stage 3 - The Address resolver does not work before that!
        return;

    // say HELLO to the world
    ev << "VoIPSourceApp -> initialize(" << stage << ")" << endl;

    // Hack for create results folder
    recordScalar("hackForCreateResultsFolder", 0);

    pReSampleCtx = NULL;
    localPort = par("localPort");
    destPort = par("destPort");
    destAddress = IPvXAddressResolver().resolve(par("destAddress").stringValue());
    socket.setOutputGate(gate("udpOut"));
    socket.bind(localPort);

    voipHeaderSize = par("voipHeaderSize");
    voipSilenceThreshold = par("voipSilenceThreshold");
    sampleRate = par("sampleRate");
    codec = par("codec").stringValue();
    compressedBitRate = par("compressedBitRate");
    packetTimeLength = par("packetTimeLength");

    soundFile = par("soundFile").stringValue();
    repeatCount = par("repeatCount");
    traceFileName = par("traceFileName").stringValue();
    simtime_t startTime = par("startTime");

    samplesPerPacket = (int)round(sampleRate * SIMTIME_DBL(packetTimeLength));
    if (samplesPerPacket & 1)
        samplesPerPacket++;
    ev << "The packetTimeLength parameter is " << packetTimeLength * 1000.0 << "ms, ";
    packetTimeLength = ((double)samplesPerPacket) / sampleRate;
    ev << "adjusted to " << packetTimeLength * 1000.0 << "ms" << endl;

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

    // statistics:
    sentPkSignal = registerSignal("sentPk");
}

void VoIPStreamSender::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        VoIPStreamPacket *packet;

        if (msg == timer)
        {
            packet = generatePacket();

            if (!packet)
            {
                if (repeatCount > 1)
                {
                    repeatCount--;
                    av_seek_frame(pFormatCtx, streamIndex, 0, 0);
                    packet = generatePacket();
                }
            }

            if (packet)
            {
                // reschedule trigger message
                scheduleAt(simTime() + packetTimeLength, packet);
                scheduleAt(simTime() + packetTimeLength, msg);
            }
        }
        else
        {
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

    if (pReSampleCtx)
        audio_resample_close(pReSampleCtx);

    pReSampleCtx = NULL;

    if (this->pFormatCtx)
        av_close_input_file(pFormatCtx);
}

/*
static void choose_sample_fmt(AVStream *st, AVCodec *codec)
{
    if (codec && codec->sample_fmts)
    {
        const AVSampleFormat *p = codec->sample_fmts;
        for (; *p != -1; p++)
        {
            if (*p == st->codec->sample_fmt)
                break;
        }

        if (*p == -1)
        {
            av_log(NULL, AV_LOG_WARNING,
                    "Incompatible sample format '%s' for codec '%s', auto-selecting format '%s'\n",
                    av_get_sample_fmt_name(st->codec->sample_fmt), codec->name, av_get_sample_fmt_name(
                            codec->sample_fmts[0]));
            st->codec->sample_fmt = codec->sample_fmts[0];
        }
    }
}
*/

void VoIPStreamSender::openSoundFile(const char *name)
{
    int ret = av_open_input_file(&pFormatCtx, name, NULL, 0, NULL);

    if (ret)
        error("Audiofile '%s' open error: %d", name, ret);

    av_find_stream_info(pFormatCtx);

    //get stream number
    streamIndex = -1;
    for (unsigned int j = 0; j < pFormatCtx->nb_streams; j++)
    {
        if (pFormatCtx->streams[j]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            streamIndex = j;
            break;
        }
    }

    if (streamIndex == -1)
        error("The file '%s' not contains any audio stream.", name);

    pCodecCtx = pFormatCtx->streams[streamIndex]->codec;

    //find decoder and open the correct codec
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    ret = avcodec_open(pCodecCtx, pCodec);

    if (ret)
        error("avcodec_open() error on file '%s': %d", name, ret);

    //allocate encoder
    pEncoderCtx = avcodec_alloc_context();
    //set bitrate:
    pEncoderCtx->bit_rate = compressedBitRate;

    pEncoderCtx->sample_rate = sampleRate;
    pEncoderCtx->channels = 1;

    pCodecEncoder = avcodec_find_encoder_by_name(codec);

    if (pCodecEncoder == NULL)
        error("Codec '%s' not found!", codec);

    pEncoderCtx->sample_fmt = pCodecCtx->sample_fmt; // FIXME hack!

    if (avcodec_open(pEncoderCtx, pCodecEncoder) < 0)
        error("could not open %s encoding codec!", codec);

    if (pCodecCtx->sample_rate != sampleRate
            || pEncoderCtx->sample_fmt != pCodecCtx->sample_fmt
            || pCodecCtx->channels != 1)
    {
        pReSampleCtx = av_audio_resample_init(1, pCodecCtx->channels, sampleRate, pCodecCtx->sample_rate,
                pEncoderCtx->sample_fmt, pCodecCtx->sample_fmt, 16, 10, 0, 0.8);
                // parameters copied from the implementation of deprecated audio_resample_init()
        // begin HACK
        long int sec = 2;
        short int *inb = new short int[sec * pCodecCtx->channels * pCodecCtx->sample_rate * av_get_bits_per_sample_format(pCodecCtx->sample_fmt) / (8 * sizeof(short int))];
        short int *outb = new short int[sec * sampleRate * av_get_bits_per_sample_format(pEncoderCtx->sample_fmt) / (8 * sizeof(short int))+16];
        int decoded = audio_resample(pReSampleCtx, outb, inb, sec * pCodecCtx->sample_rate);
        EV << "decoded:" <<decoded << endl;
        delete [] inb;
        delete [] outb;
        // end HACK
    }
    else
    {
        pReSampleCtx = NULL;
    }

    if (traceFileName && *traceFileName)
        outFile.open(traceFileName, sampleRate, av_get_bits_per_sample_format(pEncoderCtx->sample_fmt));

    sampleBuffer.clear(samplesPerPacket * av_get_bits_per_sample_format(pEncoderCtx->sample_fmt) / 8);
}

VoIPStreamPacket* VoIPStreamSender::generatePacket()
{
    readFrame();

    if (sampleBuffer.empty())
        return NULL;

    short int bitsPerInSample = av_get_bits_per_sample_format(pEncoderCtx->sample_fmt);
    short int bitsPerOutSample = av_get_bits_per_sample(pEncoderCtx->codec->id);
    int samples = std::min(sampleBuffer.length() / (bitsPerInSample/8), samplesPerPacket);
    bool isSilent = checkSilence(pEncoderCtx->sample_fmt, sampleBuffer.readPtr(), samples);
    VoIPStreamPacket *vp = new VoIPStreamPacket();
    int outByteCount = 0;
    uint8_t *outBuf = NULL;

//    if (pEncoderCtx->frame_size > 1)
//    {
//        error("Unsupported codec");
//        // int encoderBufSize = (int)(compressedBitRate * SIMTIME_DBL(packetTimeLength)) / 8 + 256;
//    }
//    else
    {
        pEncoderCtx->frame_size = samples; //HACK!!!!

        int encoderBufSize = std::max(std::max(pEncoderCtx->frame_size, samples * bitsPerOutSample/8 + 256), samples);
        outBuf = new uint8_t[encoderBufSize];
        memset(outBuf, 0, encoderBufSize);

        // FFMPEG doc bug:
        // When codec is PCM, the return value is count of output bytes,
        // and read (buf_size/(av_get_bits_per_sample(avctx->codec->id)/8)) samples from input buffer
        // When codec is g726, the return value is count of output bytes,
        // and read buf_size samples from input buffer
        int buf_size = (bitsPerOutSample) ? samples * bitsPerOutSample / 8 : samples;

        // The bitsPerOutSample is not 0 when codec is PCM.
        outByteCount = avcodec_encode_audio(pEncoderCtx, outBuf, buf_size, (short int*)(sampleBuffer.readPtr()));
        if(encoderBufSize < outByteCount)
            error("avcodec_encode_audio() error: too small buffer: %d instead %d", encoderBufSize, outByteCount);
        if (outByteCount <= 0)
            error("avcodec_encode_audio() error: %d", outByteCount);

        if (outFile.isOpen())
            outFile.write(sampleBuffer.readPtr(), samples * bitsPerInSample/8);
        sampleBuffer.notifyRead(samples * bitsPerInSample/8);
    }

    vp->setDataFromBuffer(outBuf, outByteCount);

    if (isSilent)
    {
        vp->setName("SILENCE");
        vp->setType(SILENCE);
        vp->setByteLength(voipSilencePacketSize);
    }
    else
    {

        vp->setName("VOICE");
        vp->setType(VOICE);
        vp->setByteLength(voipHeaderSize + outByteCount);
    }

    vp->setTimeStamp(pktID);
    vp->setSeqNo(pktID);
    vp->setCodec(pEncoderCtx->codec_id);
    vp->setSampleRate(sampleRate);
    vp->setSampleBits(pEncoderCtx->bits_per_coded_sample);
    vp->setSamplesPerPacket(samplesPerPacket);
    vp->setTransmitBitrate(compressedBitRate);

    pktID++;

    delete [] outBuf;
    return vp;
}

bool VoIPStreamSender::checkSilence(SampleFormat sampleFormat, void* _buf, int samples)
{
    int max = 0;
    int i;

    switch (sampleFormat)
    {
    case SAMPLE_FMT_U8:
        {
            uint8_t *buf = (uint8_t *)_buf;
            for (i=0; i<samples; ++i)
            {
                int s = abs(int(buf[i]) - 0x80);
                if (s > max)
                    max = s;
            }
        }
        break;

    case SAMPLE_FMT_S16:
        {
            int16_t *buf = (int16_t *)_buf;
            for (i=0; i<samples; ++i)
            {
                int s = abs(buf[i]);
                if (s > max)
                    max = s;
            }
        }
        break;

    case SAMPLE_FMT_S32:
        {
            int32_t *buf = (int32_t *)_buf;

            for (i=0; i<samples; ++i)
            {
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
    if (readOffset)
    {
        if (length())
            memcpy(samples, samples+readOffset, length());
        writeOffset -= readOffset;
        readOffset = 0;
    }
}

void VoIPStreamSender::readFrame()
{
    short int inBytesPerSample = av_get_bits_per_sample_format(pEncoderCtx->sample_fmt) / 8;
    if (sampleBuffer.length() >= samplesPerPacket * inBytesPerSample)
        return;

    sampleBuffer.align();

    char *tmpSamples = NULL;

    if (pReSampleCtx)
        tmpSamples = new char[Buffer::BUFSIZE];

    while (sampleBuffer.length() < samplesPerPacket * inBytesPerSample)
    {
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
        av_init_packet(&avpkt);
        avpkt.data = packet.data;
        avpkt.size = packet.size;

        while (avpkt.size > 0)
        {
            // decode audio and save the decoded samples in our buffer
            int16_t *rbuf, *nbuf;
            nbuf = (int16_t*)(sampleBuffer.writePtr());
            rbuf = (pReSampleCtx) ? (int16_t*)tmpSamples : nbuf;

            int frame_size = (pReSampleCtx) ? Buffer::BUFSIZE : sampleBuffer.availableSpace();
            memset(rbuf, 0, frame_size);
            int decoded = avcodec_decode_audio3(pCodecCtx, rbuf, &frame_size, &avpkt);

            if (decoded < 0)
                error("Error decoding frame, err=%d", decoded);

            avpkt.data += decoded;
            avpkt.size -= decoded;

            if (frame_size == 0)
                continue;

            decoded = frame_size / (inBytesPerSample * pCodecCtx->channels);
            ASSERT(frame_size == decoded * inBytesPerSample * pCodecCtx->channels);

            if (pReSampleCtx)
                decoded = audio_resample(pReSampleCtx, nbuf, rbuf, decoded);

            sampleBuffer.notifyWrote(decoded * inBytesPerSample);
        }
    }

    if (pReSampleCtx)
        delete [] tmpSamples;
}

