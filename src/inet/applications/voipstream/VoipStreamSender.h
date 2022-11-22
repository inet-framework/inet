//
// Copyright (C) 2005 M. Bohge (bohge@tkn.tu-berlin.de), M. Renwanz
// Copyright (C) 2010 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_VOIPSTREAMSENDER_H
#define __INET_VOIPSTREAMSENDER_H

#ifndef HAVE_FFMPEG
#error Please install libavcodec, libavformat, libavutil or disable 'VoipStream' feature
#endif // ifndef HAVE_FFMPEG

#include <fnmatch.h>

#include <vector>

#define __STDC_CONSTANT_MACROS

#include "inet/common/INETDefs.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

#ifndef HAVE_FFMPEG_SWRESAMPLE
#error Please install libswresample or disable 'VoipStream' feature
#endif // ifndef HAVE_FFMPEG_SWRESAMPLE

#include <libswresample/swresample.h>
};

#include "inet/applications/voipstream/AudioOutFile.h"
#include "inet/common/lifecycle/LifecycleUnsupported.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"

namespace inet {

class INET_API VoipStreamSender : public cSimpleModule, public LifecycleUnsupported
{
  public:
    VoipStreamSender();
    ~VoipStreamSender();

  protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;

    virtual void openSoundFile(const char *name);
    virtual Packet *generatePacket();
    virtual bool checkSilence(AVSampleFormat sampleFormat, void *_buf, int samples);
    virtual void readFrame();
    virtual void resampleFrame(const uint8_t **in_data, int in_nb_samples);

  protected:
    class INET_API Buffer {
      public:
        enum { BUFSIZE = 48000 * 2 * 2 }; // 1 second of two channel 48kHz 16bit audio

      protected:
        char *samples;
        int bufferSize;
        int readOffset;
        int writeOffset;

      public:
        Buffer();
        ~Buffer();
        void clear(int framesize);
        int length() const { return writeOffset - readOffset; }
        bool empty() const { return writeOffset <= readOffset; }
        char *readPtr() { return samples + readOffset; }
        char *writePtr() { return samples + writeOffset; }
        int availableSpace() const { return bufferSize - writeOffset; }
        void notifyRead(int length) { readOffset += length; ASSERT(readOffset <= writeOffset); }
        void notifyWrote(int length) { writeOffset += length; ASSERT(writeOffset <= bufferSize); }
        void align();
    };

  protected:
    // general parameters
    int localPort = -1;
    int destPort = -1;
    L3Address destAddress;

    int voipHeaderSize = 0;
    int voipSilenceThreshold = 0; // the maximum amplitude of a silence packet
    int voipSilencePacketSize = 0; // size of a silence packet
    int sampleRate = 0; // samples/sec [Hz]
    const char *codec = nullptr;
    int compressedBitRate = 0;
    simtime_t packetTimeLength;
    const char *soundFile = nullptr; // input audio file name
    int repeatCount = 0;

    const char *traceFileName = nullptr; // name of the output trace file, nullptr or empty to turn off recording
    AudioOutFile outFile;

    // AVCodec parameters
    AVFormatContext *pFormatCtx = nullptr;
    AVCodecContext *pCodecCtx = nullptr;

#if LIBAVCODEC_VERSION_MAJOR >= 59
    const
#endif
    AVCodec *pCodec = nullptr; // input decoder codec
    SwrContext *pReSampleCtx = nullptr;
    AVCodecContext *pEncoderCtx = nullptr;

#if LIBAVCODEC_VERSION_MAJOR >= 59
    const
#endif
    AVCodec *pCodecEncoder = nullptr; // output encoder codec

    // state variables
    UdpSocket socket;
    int streamIndex = -1;
    uint32_t pktID = 0; // increasing packet sequence number
    int samplesPerPacket = 0;
    Buffer sampleBuffer;

    cMessage *timer = nullptr;
};

} // namespace inet

#endif

