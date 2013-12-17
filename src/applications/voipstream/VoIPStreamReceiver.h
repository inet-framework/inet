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


#ifndef VOIPSTREAM_VOIPSTREAMRECEIVER_H
#define VOIPSTREAM_VOIPSTREAMRECEIVER_H

#ifndef HAVE_FFMPEG
#error Please install libavcodec, libavformat, libavutil or disable 'VoIPStream' feature
#endif


#define __STDC_CONSTANT_MACROS

#include "INETDefs.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
};

#include <iostream>
#include <sys/stat.h>

#include "IPvXAddressResolver.h"
#include "UDPControlInfo_m.h"
#include "UDPSocket.h"
#include "VoIPStreamPacket_m.h"
#include "AudioOutFile.h"
#include "ILifecycle.h"
#include "LifecycleOperation.h"

class VoIPStreamReceiver : public cSimpleModule, public ILifecycle
{
  public:
    VoIPStreamReceiver() { resultFile = ""; }
    ~VoIPStreamReceiver();

    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
    { Enter_Method_Silent(); throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName()); return true; }

  protected:
    virtual void initialize(int stage);
    virtual int numInitStages() const { return 2; }
    virtual void handleMessage(cMessage *msg);
    virtual void finish();

    virtual void createConnection(VoIPStreamPacket *vp);
    virtual void checkSourceAndParameters(VoIPStreamPacket *vp);
    virtual void closeConnection();
    virtual void decodePacket(VoIPStreamPacket *vp);
    static void initSignals();

    class Connection
    {
      public:
        Connection() : offline(true), oc(NULL), fmt(NULL), audio_st(NULL), decCtx(NULL), pCodecDec(NULL) {}
        void addAudioStream(enum CodecID codec_id);
        void openAudio(const char *fileName);
        void writeAudioFrame(uint8_t *buf, int len);
        void writeLostSamples(int sampleCount);
        void closeAudio();

        bool offline;
        uint16_t seqNo;
        uint32_t timeStamp;
        uint32_t ssrc;
        enum CodecID codec;
        short sampleBits;
        int sampleRate;
        int samplesPerPacket;
        int transmitBitrate;
        simtime_t lastPacketFinish;
        AVFormatContext *oc;
        AVOutputFormat *fmt;
        AVStream *audio_st;
        AVCodecContext *decCtx;
        AVCodec *pCodecDec;
        AudioOutFile outFile;
        IPvXAddress srcAddr;
        int srcPort;
        IPvXAddress destAddr;
        int destPort;
    };

  protected:
    int localPort;
    simtime_t playoutDelay;
    const char *resultFile;

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

#endif // VOIPSTREAM_VOIPSTREAMRECEIVER_H
