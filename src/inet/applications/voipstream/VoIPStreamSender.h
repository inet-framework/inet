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

#ifndef __INET_VOIPSTREAMSENDER_H
#define __INET_VOIPSTREAMSENDER_H

#ifndef HAVE_FFMPEG
#error Please install libavcodec, libavformat, libavutil or disable 'VoIPStream' feature
#endif // ifndef HAVE_FFMPEG

#include <fnmatch.h>
#include <vector>

#define __STDC_CONSTANT_MACROS

#include "inet/common/INETDefs.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(54, 28, 0)
#ifndef HAVE_FFMPEG_AVRESAMPLE
#error Please install libavresample or disable 'VoIPStream' feature
#endif // ifndef HAVE_FFMPEG_AVRESAMPLE
#include <libavresample/avresample.h>
#endif // if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(54, 28, 0)
};

#include "inet/applications/voipstream/AudioOutFile.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/contract/udp/UDPSocket.h"
#include "inet/applications/voipstream/VoIPStreamPacket_m.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/common/lifecycle/LifecycleOperation.h"

namespace inet {

//using namespace std;

class INET_API VoIPStreamSender : public cSimpleModule, public ILifecycle
{
  public:
    VoIPStreamSender();
    ~VoIPStreamSender();

    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override
    { Enter_Method_Silent(); throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName()); return true; }

  protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;

    virtual void openSoundFile(const char *name);
    virtual VoIPStreamPacket *generatePacket();
    virtual bool checkSilence(AVSampleFormat sampleFormat, void *_buf, int samples);
    virtual void readFrame();

  protected:
    class Buffer
    {
      public:
        enum { BUFSIZE = 48000*2*2 };   // 1 second of two channel 48kHz 16bit audio

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
    int voipSilenceThreshold = 0;    // the maximum amplitude of a silence packet
    int voipSilencePacketSize = 0;    // size of a silence packet
    int sampleRate = 0;    // samples/sec [Hz]
    const char *codec = nullptr;
    int compressedBitRate = 0;
    simtime_t packetTimeLength;
    const char *soundFile = nullptr;    // input audio file name
    int repeatCount = 0;

    const char *traceFileName = nullptr;    // name of the output trace file, nullptr or empty to turn off recording
    AudioOutFile outFile;

    // AVCodec parameters
    AVFormatContext *pFormatCtx = nullptr;
    AVCodecContext *pCodecCtx = nullptr;
    AVCodec *pCodec = nullptr;    // input decoder codec

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(54, 28, 0)
    AVAudioResampleContext *pReSampleCtx = nullptr;
#else // if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(54, 28, 0)
    ReSampleContext *pReSampleCtx = nullptr;
#endif // if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(54, 28, 0)

    AVCodecContext *pEncoderCtx = nullptr;
    AVCodec *pCodecEncoder = nullptr;    // output encoder codec

    // state variables
    UDPSocket socket;
    int streamIndex = -1;
    uint32_t pktID = 0;    // increasing packet sequence number
    int samplesPerPacket = 0;
    AVPacket packet;
    Buffer sampleBuffer;

    cMessage *timer = nullptr;

    // statistics:
    static simsignal_t sentPkSignal;
};

} // namespace inet

#endif // ifndef __INET_VOIPSTREAMSENDER_H

