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

#ifndef __INET_VOIPSTREAMRECEIVER_H
#define __INET_VOIPSTREAMRECEIVER_H

#ifndef HAVE_FFMPEG
#error Please install libavcodec, libavformat, libavutil or disable 'VoIPStream' feature
#endif // ifndef HAVE_FFMPEG

#define __STDC_CONSTANT_MACROS

#include "inet/common/INETDefs.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
};

#include <iostream>
#include <sys/stat.h>

#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/contract/udp/UDPControlInfo_m.h"
#include "inet/transportlayer/contract/udp/UDPSocket.h"
#include "inet/applications/voipstream/VoIPStreamPacket_m.h"
#include "inet/applications/voipstream/AudioOutFile.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/common/lifecycle/LifecycleOperation.h"

namespace inet {

class INET_API VoIPStreamReceiver : public cSimpleModule, public ILifecycle
{
  public:
    VoIPStreamReceiver() {}
    ~VoIPStreamReceiver();

    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override;

  protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;

    virtual void createConnection(VoIPStreamPacket *vp);
    virtual void checkSourceAndParameters(VoIPStreamPacket *vp);
    virtual void closeConnection();
    virtual void decodePacket(VoIPStreamPacket *vp);

    class Connection
    {
      public:
        Connection() {}
        void addAudioStream(enum AVCodecID codec_id);
        void openAudio(const char *fileName);
        void writeAudioFrame(uint8_t *buf, int len);
        void writeLostSamples(int sampleCount);
        void closeAudio();

      public:
        bool offline = true;
        uint16_t seqNo = 0;
        uint32_t timeStamp = 0;
        uint32_t ssrc = 0;
        enum AVCodecID codec = AV_CODEC_ID_NONE;
        short sampleBits = 0;
        int sampleRate = 0;
        int samplesPerPacket = 0;
        int transmitBitrate = 0;
        simtime_t lastPacketFinish;
        AVFormatContext *oc = nullptr;
        AVOutputFormat *fmt = nullptr;
        AVStream *audio_st = nullptr;
        AVCodecContext *decCtx = nullptr;
        AVCodec *pCodecDec = nullptr;
        AudioOutFile outFile;
        L3Address srcAddr;
        int srcPort = -1;
        L3Address destAddr;
        int destPort = -1;
    };

  protected:
    int localPort = -1;
    simtime_t playoutDelay;
    const char *resultFile = nullptr;

    UDPSocket socket;

    Connection curConn;

    static simsignal_t rcvdPkSignal;
    static simsignal_t dropPkSignal;
    static simsignal_t lostSamplesSignal;
    static simsignal_t lostPacketsSignal;
    static simsignal_t packetHasVoiceSignal;
    static simsignal_t connStateSignal;
    static simsignal_t delaySignal;
};

} // namespace inet

#endif // ifndef __INET_VOIPSTREAMRECEIVER_H

