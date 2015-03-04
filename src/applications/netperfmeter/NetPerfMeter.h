// * --------------------------------------------------------------------------
// *
// *     //====//  //===== <===//===>  //====//
// *    //        //          //      //    //    SCTP Optimization Project
// *   //=====   //          //      //====//   ==============================
// *        //  //          //      //           University of Duisburg-Essen
// *  =====//  //=====     //      //
// *
// * --------------------------------------------------------------------------
// *
// *   Copyright (C) 2009-2015 by Thomas Dreibholz
// *
// *   This program is free software: you can redistribute it and/or modify
// *   it under the terms of the GNU General Public License as published by
// *   the Free Software Foundation, either version 3 of the License, or
// *   (at your option) any later version.
// *
// *   This program is distributed in the hope that it will be useful,
// *   but WITHOUT ANY WARRANTY; without even the implied warranty of
// *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// *   GNU General Public License for more details.
// *
// *   You should have received a copy of the GNU General Public License
// *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
// *
// *   Contact: dreibh@iem.uni-due.de

#ifndef __NETPERFMETER_H
#define __NETPERFMETER_H

#include <omnetpp.h>
#include <assert.h>
#include <fstream>

#include "IPvXAddress.h"
#include "UDPSocket.h"
#include "TCPSocket.h"
#include "SCTPSocket.h"
#include "NetPerfMeter_m.h"
#include "SCTPAssociation.h"
#include "TCPCommand_m.h"
#include "SCTPCommand_m.h"
#include "UDPControlInfo_m.h"

class INET_API NetPerfMeter : public cSimpleModule
{
   public:
   NetPerfMeter();
   ~NetPerfMeter();
   virtual void initialize();
   virtual void finish();
   virtual void handleMessage(cMessage *msg);

   inline void setStatusString(const char* status) {
      if(ev.isGUI()) {
         getDisplayString().setTagArg("t", 0, status);
      }
   }
   void showIOStatus();

   void establishConnection();
   void successfullyEstablishedConnection(cMessage*          msg,
                                          const unsigned int queueSize);
   void teardownConnection(const bool stopTimeReached = false);
   void receiveMessage(cMessage* msg);
   void resetStatistics();
   void writeStatistics();

   void sendSCTPQueueRequest(const unsigned int queueSize);
   void sendTCPQueueRequest(const unsigned int queueSize);


   protected:
   // ====== Parameters =====================================================
   enum Protocol {
      SCTP = 0,
      TCP  = 1,
      UDP  = 2
   };
   enum TimerType {
      TIMER_CONNECT    = 1,
      TIMER_START      = 2,
      TIMER_RESET      = 3,
      TIMER_STOP       = 4,
      TIMER_TRANSMIT   = 5,
      TIMER_DISCONNECT = 6,
      TIMER_RECONNECT  = 7
   };

   Protocol              TransportProtocol;
   bool                  ActiveMode;
   bool                  SendingAllowed;
   bool                  HasFinished;
   unsigned int          MaxMsgSize;
   unsigned int          QueueSize;
   double                UnorderedMode;
   double                UnreliableMode;
   bool                  DecoupleSaturatedStreams;
   simtime_t             ConnectTime;
   simtime_t             StartTime;
   simtime_t             ResetTime;
   simtime_t             StopTime;
   cMessage*             ConnectTimer;
   cMessage*             StartTimer;
   cMessage*             StopTimer;
   cMessage*             ResetTimer;
   cMessage*             DisconnectTimer;
   cMessage*             ReconnectTimer;
   unsigned int          EstablishedConnections;
   unsigned int          MaxReconnects;
   std::vector<NetPerfMeterTransmitTimer*>
                         TransmitTimerVector;

   unsigned int          RequestedOutboundStreams;
   unsigned int          MaxInboundStreams;
   unsigned int          ActualOutboundStreams;
   unsigned int          ActualInboundStreams;
   std::vector<cDynamicExpression>
                         FrameRateExpressionVector;
   std::vector<cDynamicExpression>
                         FrameSizeExpressionVector;

   // ====== Sockets and Connection Information =============================
   SCTPSocket*           SocketSCTP;
   SCTPSocket*           IncomingSocketSCTP;
   TCPSocket*            SocketTCP;
   TCPSocket*            IncomingSocketTCP;
   UDPSocket*            SocketUDP;
   int                   ConnectionID;
   IPvXAddress           PrimaryPath;

   // ====== Trace File Handling ============================================
   struct TraceEntry {
      double       InterFrameDelay;
      unsigned int FrameSize;
      unsigned int StreamID;
   };
   std::vector<TraceEntry> TraceVector;                // Frame trace from file
   size_t                  TraceIndex;                 // Position in trace file

   // ====== Timers =========================================================
   simtime_t             TransmissionStartTime;        // Absolute transmission start time
   simtime_t             ConnectionEstablishmentTime;  // Absolute connection establishment time
   simtime_t             StatisticsResetTime;          // Absolute statistics reset time

   // ====== Variables ======================================================
   unsigned int          LastStreamID;                 // Stream number of last message being sent

   // ====== Statistics =====================================================
   simtime_t             StatisticsStartTime;          // Absolute start time of statistics recording

   private:
   class SenderStatistics {
      public:
      SenderStatistics() {
         reset();
      }

      inline void reset() {
         SentBytes    = 0;
         SentMessages = 0;
      }

      unsigned long long SentBytes;
      unsigned long long SentMessages;
   };

   class ReceiverStatistics {
      public:
      ReceiverStatistics() {
         ReceivedDelayHistogram.setName("Received Message Delay");
         reset();
      }

      inline void reset() {
         ReceivedBytes    = 0;
         ReceivedMessages = 0;
         ReceivedDelayHistogram.clearResult();
      }

      unsigned long long ReceivedBytes;
      unsigned long long ReceivedMessages;
      cDoubleHistogram   ReceivedDelayHistogram;
   };

   std::map<unsigned int, SenderStatistics*>   SenderStatisticsMap;
   std::map<unsigned int, ReceiverStatistics*> ReceiverStatisticsMap;

   inline SenderStatistics* getSenderStatistics(const unsigned int streamID) {
      std::map<unsigned int, SenderStatistics*>::iterator found = SenderStatisticsMap.find(streamID);
      assert(found != SenderStatisticsMap.end());
      return(found->second);
   }
   inline ReceiverStatistics* getReceiverStatistics(const unsigned int streamID) {
      std::map<unsigned int, ReceiverStatistics*>::iterator found = ReceiverStatisticsMap.find(streamID);
      assert(found != ReceiverStatisticsMap.end());
      return(found->second);
   }
   double        getFrameRate(const unsigned int streamID);
   unsigned long getFrameSize(const unsigned int streamID);
   void sendDataOfTraceFile(const unsigned long long bytesAvailableInQueue);
   void sendDataOfSaturatedStreams(const unsigned long long   bytesAvailableInQueue,
                                   const SCTPSendQueueAbated* sendQueueAbatedIndication);

   void sendDataOfNonSaturatedStreams(const unsigned long long bytesAvailableInQueue,
                                      const unsigned int       streamID);
   unsigned long transmitFrame(const unsigned int frameSize,
                               const unsigned int streamID);
   static opp_string format(const char* formatString, ...);
   static void parseExpressionVector(std::vector<cDynamicExpression>& expressionVector,
                                     const char*                      string,
                                     const char*                      delimiters = NULL);
   void createAndBindSocket();
   void handleTimer(cMessage* msg);
};

#endif
