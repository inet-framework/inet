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

#include "NetPerfMeter.h"
#include "NetPerfMeter_m.h"

#include "inet/networklayer/common/L3AddressResolver.h"

namespace inet {

Define_Module(NetPerfMeter);


// #define EV std::cout


// ###### Get pareto-distributed double random value ########################
// Parameters:
// location = the location parameter (also: scale parameter): x_m, x_min or m
// shape    = the shape parameter: alpha or k
//
// Based on rpareto from GNU R's VGAM package
// (http://cran.r-project.org/web/packages/VGAM/index.html):
// rpareto <- function (n, location, shape)
// {
//     ans <- location/runif(n)^(1/shape)
//     ans[location <= 0] <- NaN
//     ans[shape <= 0] <- NaN
//     ans
// }
//
// Some description:
// http://en.wikipedia.org/wiki/Pareto_distribution
//
// Mean: E(X) = shape*location / (shape - 1) for alpha > 1
// => location = E(X)*(shape - 1) / shape
//
static cNEDValue pareto(cComponent *context, cNEDValue argv[], int argc)
{
    const int    rng      = argc==3 ? (int)argv[2] : 0;
    const double location = argv[0].doubleValueInUnit(argv[0].getUnit());
    const double shape    = argv[1].doubleValueInUnit(argv[1].getUnit());

    const double r      = RNGCONTEXT uniform(0.0, 1.0, rng);
    const double result = location / pow(r, 1.0 / shape);

    // printf("%1.6f  => %1.6f   (location=%1.6f shape=%1.6f)\n", r, result, location, shape);
    return cNEDValue(result, argv[0].getUnit());
}

Define_NED_Function(pareto, "quantity pareto(quantity location, quantity shape, long rng?)");


// ###### Constructor #######################################################
NetPerfMeter::NetPerfMeter()
{
   SendingAllowed = false;
   ConnectionID   = 0;
   resetStatistics();
}


// ###### Destructor ########################################################
NetPerfMeter::~NetPerfMeter()
{
   cancelAndDelete(ConnectTimer);
   ConnectTimer = NULL;
   cancelAndDelete(StartTimer);
   StartTimer = NULL;
   cancelAndDelete(ResetTimer);
   ResetTimer = NULL;
   cancelAndDelete(StopTimer);
   StopTimer = NULL;
   for(auto iterator = TransmitTimerVector.begin(); iterator != TransmitTimerVector.end(); iterator++) {
      cancelAndDelete(*iterator);
      *iterator = NULL;
   }
}


// ###### Parse vector of cDynamicExpression from string ####################
void NetPerfMeter::parseExpressionVector(std::vector<cDynamicExpression>& expressionVector,
                                         const char*                      string,
                                         const char*                      delimiters)
{
   expressionVector.clear();
   cStringTokenizer tokenizer(string, delimiters);
   while(tokenizer.hasMoreTokens()) {
      const char*        token = tokenizer.nextToken();
      cDynamicExpression expression;
      expression.parse(token);
      expressionVector.push_back(expression);
   }
}


// ###### initialize() method ###############################################
void NetPerfMeter::initialize()
{
   // ====== Handle parameters ==============================================
   ActiveMode = (bool)par("activeMode");
   if(strcmp((const char*)par("protocol"), "TCP") == 0) {
      TransportProtocol = TCP;
   }
   else if(strcmp((const char*)par("protocol"), "SCTP") == 0) {
      TransportProtocol = SCTP;
   }
   else if(strcmp((const char*)par("protocol"), "UDP") == 0) {
      TransportProtocol = UDP;
   }
   else {
      throw cRuntimeError("Bad protocol setting!");
   }

   RequestedOutboundStreams = 1;
   MaxInboundStreams        = 1;
   ActualOutboundStreams    = 1;
   ActualInboundStreams     = 1;
   LastStreamID             = 1;
   MaxMsgSize               = par("maxMsgSize");
   QueueSize                = par("queueSize");
   DecoupleSaturatedStreams = par("decoupleSaturatedStreams");
   RequestedOutboundStreams = par("outboundStreams");
   if((RequestedOutboundStreams < 1) || (RequestedOutboundStreams > 65535)) {
      throw cRuntimeError("Invalid number of outbound streams; use range from [1, 65535]");
   }
   MaxInboundStreams = par("maxInboundStreams");
   if((MaxInboundStreams < 1) || (MaxInboundStreams > 65535)) {
      throw cRuntimeError("Invalid number of inbound streams; use range from [1, 65535]");
   }
   UnorderedMode = par("unordered");
   if((UnorderedMode < 0.0) || (UnorderedMode > 1.0)) {
      throw cRuntimeError("Bad value for unordered probability; use range from [0.0, 1.0]");
   }
   UnreliableMode = par("unreliable");
   if((UnreliableMode < 0.0) || (UnreliableMode > 1.0)) {
      throw cRuntimeError("Bad value for unreliable probability; use range from [0.0, 1.0]");
   }
   parseExpressionVector(FrameRateExpressionVector, par("frameRateString"), ";");
   parseExpressionVector(FrameSizeExpressionVector, par("frameSizeString"), ";");

   TraceIndex = ~0;
   if(strcmp((const char*)par("traceFile"), "") != 0) {
      std::fstream traceFile((const char*)par("traceFile"));
      if(!traceFile.good()) {
         throw cRuntimeError("Unable to load trace file");
      }
      while(!traceFile.eof()) {
        TraceEntry traceEntry;
        traceEntry.InterFrameDelay = 0;
        traceEntry.FrameSize       = 0;
        traceEntry.StreamID        = 0;

        char line[256];
        traceFile.getline((char*)&line, sizeof(line), '\n');
        if(sscanf(line, "%lf %u %u", &traceEntry.InterFrameDelay, &traceEntry.FrameSize, &traceEntry.StreamID) >= 2) {
           // std::cout << "Frame: " << traceEntry.InterFrameDelay << "\t" << traceEntry.FrameSize << "\t" << traceEntry.StreamID << endl;
           TraceVector.push_back(traceEntry);
        }
      }
   }

   // ====== Initialize and bind socket =====================================
   SocketSCTP = IncomingSocketSCTP = NULL;
   SocketTCP  = IncomingSocketTCP  = NULL;
   SocketUDP  = NULL;

   for(unsigned int i = 0;i < RequestedOutboundStreams;i++) {
      SenderStatistics* senderStatistics = new SenderStatistics;
      SenderStatisticsMap.insert(std::pair<unsigned int, SenderStatistics*>(i, senderStatistics));
   }
   for(unsigned int i = 0;i < MaxInboundStreams;i++) {
      ReceiverStatistics* receiverStatistics = new ReceiverStatistics;
      ReceiverStatisticsMap.insert(std::pair<unsigned int, ReceiverStatistics*>(i, receiverStatistics));
   }

   ConnectTime       = par("connectTime");
   StartTime         = par("startTime");
   ResetTime         = par("resetTime");
   StopTime          = par("stopTime");
   MaxOnOffCycles    = par("maxOnOffCycles");

   HasFinished       = false;
   OffTimer          = NULL;
   OnTimer           = NULL;
   OnOffCycleCounter = 0;

   EV << simTime() << ", " << getFullPath() << ": Initialize"
      << "\tConnectTime=" << ConnectTime
      << "\tStartTime="   << StartTime
      << "\tResetTime="   << ResetTime
      << "\tStopTime="    << StopTime
      << endl;

   if(ActiveMode == false) {
      // Passive mode: create and bind socket immediately.
      // For active mode, the socket will be created just before connect().
      createAndBindSocket();
   }

   // ====== Schedule Connect Timer =========================================
   ConnectTimer = new cMessage("ConnectTimer");
   ConnectTimer->setKind(TIMER_CONNECT);
   scheduleAt(ConnectTime, ConnectTimer);
}


// ###### finish() method ###################################################
void NetPerfMeter::finish()
{
   if(TransportProtocol == SCTP) {
   }
   else if(TransportProtocol == TCP) {
   }
   else if(TransportProtocol == UDP) {
   }

   std::map<unsigned int, SenderStatistics*>::iterator senderStatisticsIterator = SenderStatisticsMap.begin();
   while(senderStatisticsIterator != SenderStatisticsMap.end()) {
      delete senderStatisticsIterator->second;
      SenderStatisticsMap.erase(senderStatisticsIterator);
      senderStatisticsIterator = SenderStatisticsMap.begin();
   }
   std::map<unsigned int, ReceiverStatistics*>::iterator receiverStatisticsIterator = ReceiverStatisticsMap.begin();
   while(receiverStatisticsIterator != ReceiverStatisticsMap.end()) {
      delete receiverStatisticsIterator->second;
      ReceiverStatisticsMap.erase(receiverStatisticsIterator);
      receiverStatisticsIterator = ReceiverStatisticsMap.begin();
   }
}


// ###### Show I/O status ###################################################
void NetPerfMeter::showIOStatus() {
   if(hasGUI()) {
      unsigned long long totalSentBytes = 0;
      for(std::map<unsigned int, SenderStatistics*>::const_iterator iterator = SenderStatisticsMap.begin();
         iterator != SenderStatisticsMap.end(); iterator++) {
         const SenderStatistics* senderStatistics = iterator->second;
         totalSentBytes += senderStatistics->SentBytes;
      }

      unsigned long long totalReceivedBytes = 0;
      for(std::map<unsigned int, ReceiverStatistics*>::const_iterator iterator = ReceiverStatisticsMap.begin();
         iterator != ReceiverStatisticsMap.end(); iterator++) {
         const ReceiverStatistics* receiverStatistics = iterator->second;
         totalReceivedBytes += receiverStatistics->ReceivedBytes;
      }

      char status[64];
      snprintf((char*)&status, sizeof(status), "In: %llu, Out: %llu",
               totalReceivedBytes, totalSentBytes);
      getDisplayString().setTagArg("t", 0, status);
   }
}


// ###### Handle timer ######################################################
void NetPerfMeter::handleTimer(cMessage* msg)
{
   // ====== Transmit timer =================================================
   const NetPerfMeterTransmitTimer* transmitTimer =
      dynamic_cast<NetPerfMeterTransmitTimer*>(msg);
   if(transmitTimer) {
      TransmitTimerVector[transmitTimer->getStreamID()] = NULL;
      if(TraceVector.size() > 0) {
         sendDataOfTraceFile(QueueSize);
      }
      else {
         sendDataOfNonSaturatedStreams(QueueSize, transmitTimer->getStreamID());
      }
   }

   // ====== Off timer ======================================================
   else if(msg == OffTimer) {
      EV << simTime() << ", " << getFullPath() << ": Entering OFF mode" << endl;

      OffTimer = NULL;
      stopSending();
   }

   // ====== On timer =======================================================
   else if(msg == OnTimer) {
      EV << simTime() << ", " << getFullPath() << ": Entering ON mode" << endl;

      OnTimer = NULL;
      startSending();
   }

   // ====== Reset timer ====================================================
   else if(msg == ResetTimer) {
      EV << simTime() << ", " << getFullPath() << ": Reset" << endl;

      ResetTimer = NULL;
      resetStatistics();

      assert(StopTimer == NULL);
      if(StopTime > 0.0) {
         StopTimer = new cMessage("StopTimer");
         StopTimer->setKind(TIMER_STOP);
         scheduleAt(simTime() + StopTime, StopTimer);
      }
   }

   // ====== Stop timer =====================================================
   else if(msg == StopTimer) {
      EV << simTime() << ", " << getFullPath() << ": STOP" << endl;

      StopTimer = NULL;
      if(OffTimer) {
         cancelAndDelete(OffTimer);
         OffTimer = NULL;
      }
      if(OnTimer) {
         cancelAndDelete(OnTimer);
         OnTimer = NULL;
      }

      if(TransportProtocol == SCTP) {
         if(IncomingSocketSCTP != NULL) {
            IncomingSocketSCTP->close();
         }
         else if(SocketSCTP != NULL) {
            SocketSCTP->close();
         }
      }
      else if(TransportProtocol == TCP) {
         if(SocketTCP != NULL) {
            SocketTCP->close();
         }
      }
      teardownConnection(true);
   }

   // ====== Start timer ====================================================
   else if(msg == StartTimer) {
      EV << simTime() << ", " << getFullPath() << ": Start" << endl;

      StartTimer = NULL;
      startSending();
   }

   // ====== Connect timer ==================================================
   else if(msg == ConnectTimer) {
      EV << simTime() << ", " << getFullPath() << ": Connect" << endl;

      ConnectTimer = NULL;
      establishConnection();
   }
}


// ###### Handle message ####################################################
void NetPerfMeter::handleMessage(cMessage* msg)
{
   // ====== Timer handling =================================================
   if(msg->isSelfMessage()) {
      handleTimer(msg);
   }

   // ====== SCTP ===========================================================
   else if(TransportProtocol == SCTP) {
      switch(msg->getKind()) {
         // ------ Data -----------------------------------------------------
         case SCTP_I_DATA:
            receiveMessage(msg);
          break;
         // ------ Data Arrival Indication ----------------------------------
         case SCTP_I_DATA_NOTIFICATION: {
            // Data has arrived -> request it from the SCTP module.
            const SCTPCommand* dataIndication =
               check_and_cast<const SCTPCommand*>(msg->getControlInfo());
            SCTPSendInfo* command = new SCTPSendInfo("SendCommand");
            command->setAssocId(dataIndication->getAssocId());
            command->setSid(dataIndication->getSid());
            command->setNumMsgs(dataIndication->getNumMsgs());
            cPacket* cmsg = new cPacket("ReceiveRequest");
            cmsg->setKind(SCTP_C_RECEIVE);
            cmsg->setControlInfo(command);
            send(cmsg, "sctpOut");
           }
          break;
         // ------ Connection established -----------------------------------
         case SCTP_I_ESTABLISHED: {
            const SCTPConnectInfo* connectInfo =
               check_and_cast<const SCTPConnectInfo*>(msg->getControlInfo());
            ActualOutboundStreams = connectInfo->getOutboundStreams();
            if(ActualOutboundStreams > RequestedOutboundStreams) {
               ActualOutboundStreams = RequestedOutboundStreams;
            }
            ActualInboundStreams = connectInfo->getInboundStreams();
            if(ActualInboundStreams > MaxInboundStreams) {
               ActualInboundStreams = MaxInboundStreams;
            }
            LastStreamID = ActualOutboundStreams - 1;
            // NOTE: Start sending on stream 0!
            successfullyEstablishedConnection(msg, QueueSize);
           }
          break;
         // ------ Queue indication -----------------------------------------
         case SCTP_I_SENDQUEUE_ABATED: {
            const SCTPSendQueueAbated* sendQueueAbatedIndication =
               check_and_cast<SCTPSendQueueAbated*>(msg->getControlInfo());
            assert(sendQueueAbatedIndication != NULL);
            // Queue is underfull again -> give it more data.
            SendingAllowed = true;
            if(TraceVector.size() == 0) {
               sendDataOfSaturatedStreams(sendQueueAbatedIndication->getBytesAvailable(),
                                          sendQueueAbatedIndication);
            }
           }
          break;
         case SCTP_I_SENDQUEUE_FULL:
            SendingAllowed = false;
          break;
         // ------ Errors ---------------------------------------------------
         case SCTP_I_PEER_CLOSED:
         case SCTP_I_CLOSED:
         case SCTP_I_CONNECTION_REFUSED:
         case SCTP_I_CONNECTION_RESET:
         case SCTP_I_TIMED_OUT:
         case SCTP_I_ABORT:
         case SCTP_I_CONN_LOST:
         case SCTP_I_SHUTDOWN_RECEIVED:
            teardownConnection();
          break;
         default:
            // printf("SCTP: kind=%d\n", msg->getKind());
          break;
      }
   }

   // ====== TCP ============================================================
   else if(TransportProtocol == TCP) {
      short kind = msg->getKind();
      switch(kind) {
         // ------ Data -----------------------------------------------------
         case TCP_I_DATA:
         case TCP_I_URGENT_DATA:
            receiveMessage(msg);
          break;
         // ------ Connection established -----------------------------------
         case TCP_I_ESTABLISHED:
            successfullyEstablishedConnection(msg, 0);
          break;
         // ------ Queue indication -----------------------------------------
         case TCP_I_SEND_MSG: {
            const TCPCommand* tcpCommand =
               check_and_cast<TCPCommand*>(msg->getControlInfo());
            assert(tcpCommand != NULL);
            // Queue is underfull again -> give it more data.
            if(SocketTCP != NULL) {   // T.D. 16.11.2011: Ensure that there is still a TCP socket!
               SendingAllowed = true;
               if(TraceVector.size() == 0) {
                  sendDataOfSaturatedStreams(tcpCommand->getUserId(), NULL);
               }
            }
           }
          break;
         // ------ Errors ---------------------------------------------------
         case TCP_I_PEER_CLOSED:
         case TCP_I_CLOSED:
         case TCP_I_CONNECTION_REFUSED:
         case TCP_I_CONNECTION_RESET:
         case TCP_I_TIMED_OUT:
            teardownConnection();
          break;
      }
   }

   // ====== UDP ============================================================
   else if(TransportProtocol == UDP) {
      switch(msg->getKind()) {
         // ------ Data -----------------------------------------------------
         case UDP_I_DATA:
            receiveMessage(msg);
          break;
         // ------ Error ----------------------------------------------------
         case UDP_I_ERROR:
            teardownConnection();
          break;
      }
   }

   delete msg;
}


// ###### Establish connection ##############################################
void NetPerfMeter::establishConnection()
{
   const char* remoteAddress = par("remoteAddress");
   const int   remotePort    = par("remotePort");

   // ====== Establish connection ===========================================
   if(ActiveMode == true) {
      createAndBindSocket();

      const char* primaryPath   = par("primaryPath");
      PrimaryPath = (primaryPath[0] != 0x00) ?
                       L3AddressResolver().resolve(primaryPath) : L3Address();

      setStatusString("Connecting");
      if(TransportProtocol == SCTP) {
         AddressVector remoteAddressList;
         remoteAddressList.push_back(L3AddressResolver().resolve(remoteAddress));
         SocketSCTP->connectx(remoteAddressList, remotePort);
      }
      else if(TransportProtocol == TCP) {
         SocketTCP->renewSocket();
         SocketTCP->connect(L3AddressResolver().resolve(remoteAddress), remotePort);
      }
      else if(TransportProtocol == UDP) {
         SocketUDP->connect(L3AddressResolver().resolve(remoteAddress), remotePort);
         // Just start sending, since UDP is connection-less
         successfullyEstablishedConnection(NULL, 0);
      }
      ConnectionEstablishmentTime = simTime();
   }
   else {
      // ------ Handle UDP on passive side ----------------
      if(TransportProtocol == UDP) {
         SocketUDP->connect(L3AddressResolver().resolve(remoteAddress), remotePort);
         successfullyEstablishedConnection(NULL, 0);
      }
   }
   EV << simTime() << ", " << getFullPath() << ": Sending allowed" << endl;
   SendingAllowed = true;
}


// ###### Connection has been established ###################################
void NetPerfMeter::successfullyEstablishedConnection(cMessage*          msg,
                                                     const unsigned int queueSize)
{
   if(HasFinished) {
      EV << "Already finished -> no new connection!" << endl;
      SCTPSocket newSocket(msg);
      newSocket.abort();
      return;
   }

   // ====== Update queue size ==============================================
   if(queueSize != 0) {
      QueueSize = queueSize;
      EV << simTime() << ", " << getFullPath() << ": Got queue size " << QueueSize << " from transport protocol" << endl;
   }

   // ====== Get connection ID ==============================================
   if(TransportProtocol == TCP) {
      if(ActiveMode == false) {
         assert(SocketTCP != NULL);
         if(IncomingSocketTCP != NULL) {
            delete IncomingSocketTCP;
         }
         IncomingSocketTCP = new TCPSocket(msg);
         IncomingSocketTCP->setOutputGate(gate("tcpOut"));
         IncomingSocketTCP->readDataTransferModePar(*this);
      }

      TCPCommand* connectInfo = check_and_cast<TCPCommand*>(msg->getControlInfo());
      ConnectionID = connectInfo->getConnId();
      sendTCPQueueRequest(QueueSize);   // Limit the send queue as given.
   }
   else if(TransportProtocol == SCTP) {
      if(ActiveMode == false) {
         assert(SocketSCTP != NULL);
         if(IncomingSocketSCTP != NULL) {
            delete IncomingSocketSCTP;
         }
         IncomingSocketSCTP = new SCTPSocket(msg);
         IncomingSocketSCTP->setOutputGate(gate("sctpOut"));
      }

      SCTPConnectInfo* connectInfo = check_and_cast<SCTPConnectInfo*>(msg->getControlInfo());
      ConnectionID = connectInfo->getAssocId();
      sendSCTPQueueRequest(QueueSize);   // Limit the send queue as given.
   }

   // ====== Initialize TransmitTimerVector =================================
   TransmitTimerVector.resize(ActualOutboundStreams);
   for(unsigned int i = 0; i < ActualOutboundStreams; i++) {
      TransmitTimerVector[i] = NULL;
   }

   // ====== Schedule Start Timer to begin transmission =====================
   if(OnOffCycleCounter == 0) {
      assert(StartTimer == NULL);
      StartTimer = new cMessage("StartTimer");
      StartTimer->setKind(TIMER_START);
      TransmissionStartTime = ConnectTime + StartTime;
      if(TransmissionStartTime < simTime()) {
         throw cRuntimeError("Connection establishment has been too late. Check startTime parameter!");
      }
      scheduleAt(TransmissionStartTime, StartTimer);

      // ====== Schedule Reset Timer to reset statistics ====================
      assert(ResetTimer == NULL);
      ResetTimer = new cMessage("ResetTimer");
      ResetTimer->setKind(TIMER_RESET);
      StatisticsResetTime = ConnectTime + ResetTime;
      scheduleAt(StatisticsResetTime, ResetTimer);
   }
   else {
      // ====== Restart transmission immediately ============================
      StartTimer = new cMessage("StartTimer");
      scheduleAt(simTime(), StartTimer);
   }
}


// ###### Start sending #####################################################
void NetPerfMeter::startSending()
{
   if(TraceVector.size() > 0) {
      sendDataOfTraceFile(QueueSize);
   }
   else {
      for(unsigned int streamID = 0; streamID < ActualOutboundStreams; streamID++) {
         sendDataOfNonSaturatedStreams(QueueSize, streamID);
      }
      sendDataOfSaturatedStreams(QueueSize, NULL);
   }

   // ------ On/Off handling ------------------------------------------------
   const simtime_t onTime = par("onTime");
   if(onTime.dbl() > 0.0) {
      OffTimer = new cMessage("OffTimer");
      OffTimer->setKind(TIMER_OFF);
      scheduleAt(simTime() + onTime, OffTimer);
   }
}


// ###### Stop sending ######################################################
void NetPerfMeter::stopSending()
{
    // ------ Stop all transmission timers ----------------------------------
    for(std::vector<NetPerfMeterTransmitTimer*>::iterator iterator = TransmitTimerVector.begin();
       iterator != TransmitTimerVector.end(); iterator++) {
      cancelAndDelete(*iterator);
      *iterator = NULL;
   }
   OnOffCycleCounter++;

   // ------ Schedule On timer ----------------------------------------------
   const simtime_t offDuration = par("offTime");
   if( (offDuration.dbl() > 0.0) &&
       ((MaxOnOffCycles < 0) ||
        (OnOffCycleCounter <= (unsigned int)MaxOnOffCycles)) ) {
      OnTimer = new cMessage("OnTimer");
      OnTimer->setKind(TIMER_ON);
      scheduleAt(simTime() + offDuration, OnTimer);
   }
}


// ###### Create and bind socket ############################################
void NetPerfMeter::createAndBindSocket()
{
   const char* localAddress = par("localAddress");
   const int   localPort    = par("localPort");
   if( (ActiveMode == false) && (localPort == 0) ) {
      throw cRuntimeError("No local port number given in active mode!");
   }
   L3Address localAddr;
   if (*localAddress)
       localAddr = L3AddressResolver().resolve(localAddress);

   if(TransportProtocol == SCTP) {
      assert(SocketSCTP == NULL);
      SocketSCTP = new SCTPSocket;
      SocketSCTP->setInboundStreams(MaxInboundStreams);
      SocketSCTP->setOutboundStreams(RequestedOutboundStreams);
      SocketSCTP->setOutputGate(gate("sctpOut"));
      SocketSCTP->bind(localPort);
      if(ActiveMode == false) {
         SocketSCTP->listen(true);
      }
   }
   else if(TransportProtocol == TCP) {
      assert(SocketTCP == NULL);
      SocketTCP = new TCPSocket;
      SocketTCP->setOutputGate(gate("tcpOut"));
      SocketTCP->readDataTransferModePar(*this);
      SocketTCP->bind(localAddr, localPort);
      if(ActiveMode == false) {
         SocketTCP->listen();
      }
   }
   else if(TransportProtocol == UDP) {
      assert(SocketUDP == NULL);
      SocketUDP = new UDPSocket;
      SocketUDP->setOutputGate(gate("udpOut"));
      SocketUDP->bind(localAddr, localPort);
   }
}


// ###### Connection teardown ###############################################
void NetPerfMeter::teardownConnection(const bool stopTimeReached)
{
   for(std::vector<NetPerfMeterTransmitTimer*>::iterator iterator = TransmitTimerVector.begin();
       iterator != TransmitTimerVector.end(); iterator++) {
      cancelAndDelete(*iterator);
      *iterator = NULL;
   }

   if(ActiveMode == false) {
      if(TransportProtocol == SCTP) {
         if(IncomingSocketSCTP != NULL) {
            delete IncomingSocketSCTP;
            IncomingSocketSCTP = NULL;
         }
      }
      else if(TransportProtocol == TCP) {
         if(IncomingSocketTCP != NULL) {
            delete IncomingSocketTCP;
            IncomingSocketTCP = NULL;
         }
      }
   }
   if( (stopTimeReached) || (ActiveMode == true) ) {
      if(TransportProtocol == SCTP) {
         if(SocketSCTP != NULL) {
            SocketSCTP->close();
            delete SocketSCTP;
            SocketSCTP = NULL;
         }
      }
      else if(TransportProtocol == TCP) {
         if(SocketTCP != NULL) {
            SocketTCP->abort();
            delete SocketTCP;
            SocketTCP = NULL;
         }
      }
      else if(TransportProtocol == UDP) {
         if(SocketUDP != NULL) {
            delete SocketUDP;
            SocketUDP = NULL;
         }
      }
      SendingAllowed = false;
      ConnectionID   = 0;

      setStatusString("Closed");
   }

   if(stopTimeReached) {
      writeStatistics();
      HasFinished = true;
   }
}


// ###### Reset statistics ##################################################
void NetPerfMeter::resetStatistics()
{
   StatisticsStartTime = simTime();
   for(std::map<unsigned int, SenderStatistics*>::iterator iterator = SenderStatisticsMap.begin();
       iterator != SenderStatisticsMap.end(); iterator++) {
       SenderStatistics* senderStatistics = iterator->second;
       senderStatistics->reset();
   }
   for(std::map<unsigned int, ReceiverStatistics*>::iterator iterator = ReceiverStatisticsMap.begin();
       iterator != ReceiverStatisticsMap.end(); iterator++) {
       ReceiverStatistics* receiverStatistics = iterator->second;
       receiverStatistics->reset();
   }
}


// ###### Write scalar statistics ###########################################
void NetPerfMeter::writeStatistics()
{
   const simtime_t statisticsStopTime = simTime();
   const double    duration           = statisticsStopTime.dbl() - StatisticsStartTime.dbl();

   recordScalar("Total Measurement Duration", duration);
   recordScalar("On-Off Cycles",              OnOffCycleCounter);

   // ====== Per-Stream Statistics ==========================================
   unsigned long long totalSentBytes    = 0;
   unsigned long long totalSentMessages = 0;
   for(std::map<unsigned int, SenderStatistics*>::const_iterator iterator = SenderStatisticsMap.begin();
       iterator != SenderStatisticsMap.end(); iterator++) {
      const unsigned int streamID = iterator->first;
      if(streamID >= ActualOutboundStreams) {
         break;
      }
      const SenderStatistics* senderStatistics = iterator->second;
      totalSentBytes    += senderStatistics->SentBytes;
      totalSentMessages += senderStatistics->SentMessages;

      const double transmissionBitRate     = (duration > 0.0) ? (8 * senderStatistics->SentBytes / duration) : 0.0;
      const double transmissionByteRate    = (duration > 0.0) ? (senderStatistics->SentBytes / duration) : 0.0;
      const double transmissionMessageRate = (duration > 0.0) ? (senderStatistics->SentMessages / duration) : 0.0;
      recordScalar(format("Transmission Bit Rate Stream #%u", streamID).c_str(),     transmissionBitRate);
      recordScalar(format("Transmission Byte Rate Stream #%u", streamID).c_str(),    transmissionByteRate);
      recordScalar(format("Transmission Message Rate Stream #%u", streamID).c_str(), transmissionMessageRate);
      recordScalar(format("Sent Bytes Stream #%u", streamID).c_str(),                senderStatistics->SentBytes);
      recordScalar(format("Sent Messages Stream #%u", streamID).c_str(),             senderStatistics->SentMessages);
   }

   unsigned long long totalReceivedBytes    = 0;
   unsigned long long totalReceivedMessages = 0;
   for(std::map<unsigned int, ReceiverStatistics*>::iterator iterator = ReceiverStatisticsMap.begin();
       iterator != ReceiverStatisticsMap.end(); iterator++) {
      const unsigned int streamID = iterator->first;
      if(streamID >= ActualInboundStreams) {
         break;
      }
      ReceiverStatistics* receiverStatistics = iterator->second;
      totalReceivedBytes    += receiverStatistics->ReceivedBytes;
      totalReceivedMessages += receiverStatistics->ReceivedMessages;

      // NOTE: When sending "as much as possible", the transmission counters are
      //       set to 0 on reset. If the queue is long enough, there may be no
      //       need to fill it again => counters will be 0 here.
      const double receptionBitRate     = (duration > 0.0) ? (8 * receiverStatistics->ReceivedBytes / duration) : 0.0;
      const double receptionByteRate    = (duration > 0.0) ? (receiverStatistics->ReceivedBytes / duration) : 0.0;
      const double receptionMessageRate = (duration > 0.0) ? (receiverStatistics->ReceivedMessages / duration) : 0.0;
      recordScalar(format("Reception Bit Rate Stream #%u", streamID).c_str(),     receptionBitRate);
      recordScalar(format("Reception Byte Rate Stream #%u", streamID).c_str(),    receptionByteRate);
      recordScalar(format("Reception Message Rate Stream #%u", streamID).c_str(), receptionMessageRate);
      recordScalar(format("Received Bytes Stream #%u", streamID).c_str(),         receiverStatistics->ReceivedBytes);
      recordScalar(format("Received Messages Stream #%u", streamID).c_str(),      receiverStatistics->ReceivedMessages);
      receiverStatistics->ReceivedDelayHistogram.recordAs(format("Received Message Delay Stream #%u", streamID).c_str(), "s");
   }


   // ====== Total Statistics ===============================================
   // NOTE: When sending "as much as possible", the transmission counters are
   //       set to 0 on reset. If the queue is long enough, there may be no
   //       need to fill it again => counters will be 0 here.
   const double totalReceptionBitRate        = (duration > 0.0) ? (8 * totalReceivedBytes / duration) : 0.0;
   const double totalReceptionByteRate       = (duration > 0.0) ? (totalReceivedBytes / duration) : 0.0;
   const double totalReceptionMessageRate    = (duration > 0.0) ? (totalReceivedMessages / duration) : 0.0;
   const double totalTransmissionBitRate     = (duration > 0.0) ? (8 * totalSentBytes / duration) : 0.0;
   const double totalTransmissionByteRate    = (duration > 0.0) ? (totalSentBytes / duration) : 0.0;
   const double totalTransmissionMessageRate = (duration > 0.0) ? (totalSentMessages / duration) : 0.0;

   // NOTE: The byte rate is redundant, but having bits and bytes
   //       makes manual reading of the results easier.
   recordScalar("Total Transmission Bit Rate",     totalTransmissionBitRate);
   recordScalar("Total Transmission Byte Rate",    totalTransmissionByteRate);
   recordScalar("Total Transmission Message Rate", totalTransmissionMessageRate);
   recordScalar("Total Sent Bytes",                totalSentBytes);
   recordScalar("Total Sent Messages",             totalSentMessages);

   recordScalar("Total Reception Bit Rate",        totalReceptionBitRate);
   recordScalar("Total Reception Byte Rate",       totalReceptionByteRate);
   recordScalar("Total Reception Message Rate",    totalReceptionMessageRate);
   recordScalar("Total Received Bytes",            totalReceivedBytes);
   recordScalar("Total Received Messages",         totalReceivedMessages);

   resetStatistics();   // Make sure that it is not mistakenly used later
}


// ###### Transmit frame of given size via given stream #####################
unsigned long NetPerfMeter::transmitFrame(const unsigned int frameSize,
                                          const unsigned int streamID)
{
   EV << simTime() << ", " << getFullPath() << ": Transmit frame of size "
      << frameSize << " on stream #" << streamID << endl;
   assert(OnTimer == NULL);

   // ====== TCP ============================================================
   unsigned long newlyQueuedBytes = 0;
   if(TransportProtocol == TCP) {
      // TCP is stream-oriented: just pass the amount of frame data.
      NetPerfMeterDataMessage* tcpMessage = new NetPerfMeterDataMessage;
      tcpMessage->setCreationTime(simTime());
      tcpMessage->setByteLength(frameSize);
      tcpMessage->setKind(TCP_C_SEND);

      if(IncomingSocketTCP) {
         IncomingSocketTCP->send(tcpMessage);
      }
      else {
         SocketTCP->send(tcpMessage);
      }

      newlyQueuedBytes += frameSize;
      SenderStatistics* senderStatistics = getSenderStatistics(0);
      senderStatistics->SentBytes += frameSize;
      senderStatistics->SentMessages++;
   }

   // ====== Message-Oriented Protocols =====================================
   else {
      unsigned int bytesToSend = frameSize;
      do {
         const unsigned int msgSize =
            (bytesToSend > MaxMsgSize) ? MaxMsgSize : bytesToSend;

         // ====== SCTP =====================================================
         if(TransportProtocol == SCTP) {
            const bool sendUnordered  = (UnorderedMode > 0.0)  ? (uniform(0.0, 1.0) < UnorderedMode)  : false;
            const bool sendUnreliable = (UnreliableMode > 0.0) ? (uniform(0.0, 1.0) < UnreliableMode) : false;

            NetPerfMeterDataMessage* dataMessage = new NetPerfMeterDataMessage;
            dataMessage->setCreationTime(simTime());
            dataMessage->setByteLength(msgSize);
            /*
            dataMessage->setDataArraySize(msgSize);
            for(unsigned long i = 0; i < msgSize; i++)  {
               dataMessage->setData(i, (i & 1) ? 'D' : 'T');
            }
            */
            dataMessage->setDataLen(msgSize);

            SCTPSendInfo* command = new SCTPSendInfo("SendRequest");
            command->setAssocId(ConnectionID);
            command->setSid(streamID);
            command->setSendUnordered( (sendUnordered == true) ?
                                       COMPLETE_MESG_UNORDERED : COMPLETE_MESG_ORDERED );
            command->setLast(true);
            command->setPrimary(PrimaryPath.isUnspecified());
            command->setRemoteAddr(PrimaryPath);
            command->setPrValue(1);
            command->setPrMethod( (sendUnreliable == true) ? 2 : 0 );   // PR-SCTP policy: RTX

            SCTPSendInfo* cmsg = new SCTPSendInfo("ControlInfo");
            cmsg->encapsulate(dataMessage);
            cmsg->setKind(SCTP_C_SEND);
            cmsg->setControlInfo(command);
            cmsg->setByteLength(msgSize);
            send(cmsg, "sctpOut");

            SenderStatistics* senderStatistics = getSenderStatistics(streamID);
            senderStatistics->SentBytes += msgSize;
            senderStatistics->SentMessages++;
         }

         // ====== UDP ===================================================
         else if(TransportProtocol == UDP) {
            NetPerfMeterDataMessage* dataMessage = new NetPerfMeterDataMessage;
            dataMessage->setCreationTime(simTime());
            dataMessage->setByteLength(msgSize);
            SocketUDP->send(dataMessage);

            SenderStatistics* senderStatistics = getSenderStatistics(0);
            senderStatistics->SentBytes += msgSize;
            senderStatistics->SentMessages++;
         }

         newlyQueuedBytes += msgSize;
         bytesToSend      -= msgSize;
      } while(bytesToSend > 0);
   }
   showIOStatus();
   return(newlyQueuedBytes);
}


// ###### Get frame rate for next frame to be sent on given stream ##########
double NetPerfMeter::getFrameRate(const unsigned int streamID)
{
   double frameRate;
   if(FrameRateExpressionVector.size() == 0) {
      frameRate = par("frameRate");
   }
   else {
      frameRate =
         FrameRateExpressionVector[streamID % FrameRateExpressionVector.size()].
            doubleValue(this, "Hz");
      if(frameRate < 0) {
         frameRate = par("frameRate");
      }
   }
   return(frameRate);
}


// ###### Get frame rate for next frame to be sent on given stream ##########
unsigned long NetPerfMeter::getFrameSize(const unsigned int streamID)
{
   unsigned long frameSize;
   if(FrameSizeExpressionVector.size() == 0) {
      frameSize = par("frameSize");
   }
   else {
      double doubleSize =
         FrameSizeExpressionVector[streamID % FrameSizeExpressionVector.size()].doubleValue(this, "B");
      frameSize = (doubleSize >= 0.0) ? (long)doubleSize : par("frameSize");
   }
   return(frameSize);
}


// ###### Send data of saturated streams ####################################
void NetPerfMeter::sendDataOfSaturatedStreams(const unsigned long long   bytesAvailableInQueue,
                                              const SCTPSendQueueAbated* sendQueueAbatedIndication)
{
   if(OnTimer != NULL) {
      // We are in Off mode -> nothing to send!
      return;
   }

   // ====== Is sending allowed (i.e. space in the queue)? ==================
   if(SendingAllowed) {
      // ====== SCTP tells current queue occupation for each stream =========
      unsigned long long contingent;
      unsigned long long queued[ActualOutboundStreams];
      if(sendQueueAbatedIndication == NULL)  {
         // At the moment, the actual queue size is unknown.
         // => Assume it to be bytesAvailableInQueue.
         contingent = bytesAvailableInQueue / ActualOutboundStreams;
         for(unsigned int streamID = 0; streamID < ActualOutboundStreams; streamID++) {
            queued[streamID] = 0;
         }
      }
      else {
         assert(ActualOutboundStreams <= sendQueueAbatedIndication->getQueuedForStreamArraySize());
         for(unsigned int streamID = 0; streamID < ActualOutboundStreams; streamID++) {
            queued[streamID] = sendQueueAbatedIndication->getQueuedForStream(streamID);
         }
         contingent = sendQueueAbatedIndication->getBytesLimit() / ActualOutboundStreams;
      }

      // ====== Send, but care for per-stream contingent ====================
      LastStreamID = (LastStreamID + 1) % ActualOutboundStreams;
      unsigned int       startStreamID    = LastStreamID;
      unsigned long long newlyQueuedBytes = 0;
      bool               progress;
      do {
         // ====== Perform one round ========================================
         progress = false;   // Will be set to true on any transmission progress
                             // during the next round
         do {
            const double frameRate = getFrameRate(LastStreamID);
            if(frameRate < 0.0000001) {   // Saturated stream
               if( (DecoupleSaturatedStreams == false) ||
                   (queued[LastStreamID] < contingent) ) {
                  // ====== Send one frame ==================================
                  const unsigned long frameSize = getFrameSize(LastStreamID);
                  if(frameSize >= 1) {
                     const unsigned long long sent = transmitFrame(frameSize, LastStreamID);
                     newlyQueuedBytes     += sent;
                     queued[LastStreamID] += sent;
                     progress = true;
                  }
               }
            }
            LastStreamID = (LastStreamID + 1) % ActualOutboundStreams;
         } while(LastStreamID != startStreamID);
      } while( (newlyQueuedBytes < bytesAvailableInQueue) && (progress == true) );
   }
}


// ###### Send data of non-saturated streams ################################
void NetPerfMeter::sendDataOfNonSaturatedStreams(const unsigned long long bytesAvailableInQueue,
                                                 const unsigned int       streamID)
{
   assert(OnTimer == NULL);

   // ====== Is there something to send? ====================================
   const double frameRate = getFrameRate(streamID);
   if(frameRate <= 0.0) {
      // No non-saturated transmission on this stream
      // => no need to re-schedule timer!
      return;
   }

   // ====== Is sending allowed (i.e. space in the queue)? ==================
   if(SendingAllowed) {
      const long frameSize = getFrameSize(streamID);
      if(frameSize < 1) {
         return;
      }
      if( (frameRate <= 0.0) &&
         ((TransportProtocol == TCP) || (TransportProtocol == UDP)) ) {
         throw cRuntimeError("TCP and UDP do not support \"send as much as possible\" mode (frameRate=0)!");
      }

      // ====== Transmit frame ==============================================
/*
      EV << simTime() << ", " << getFullPath() << ": Stream #" << streamID
         << ":\tframeRate=" << frameRate
         << "\tframeSize=" << frameSize << endl;
*/
      transmitFrame(frameSize, streamID);
   }
   else {
      // No transmission allowed -> skip this frame!
   }

   // ====== Schedule next frame transmission ===============================
   assert(TransmitTimerVector[streamID] == NULL);
   TransmitTimerVector[streamID] = new NetPerfMeterTransmitTimer("TransmitTimer");
   TransmitTimerVector[streamID]->setKind(TIMER_TRANSMIT);
   TransmitTimerVector[streamID]->setStreamID(streamID);
   const double nextFrameTime = 1.0 / frameRate;
/*
   EV << simTime() << ", " << getFullPath()
      << ": Next on stream #" << streamID << " in " << nextFrameTime << "s" << endl;
*/
   scheduleAt(simTime() + nextFrameTime, TransmitTimerVector[streamID]);
}


// ###### Send data of non-saturated streams ################################
void NetPerfMeter::sendDataOfTraceFile(const unsigned long long bytesAvailableInQueue)
{
   if(TraceIndex < TraceVector.size()) {
      const unsigned int frameSize = TraceVector[TraceIndex].FrameSize;
      unsigned int streamID        = TraceVector[TraceIndex].StreamID;
      if(streamID >= ActualOutboundStreams) {
        if(TransportProtocol == SCTP) {
           throw cRuntimeError("Invalid streamID in trace");
        }
        streamID = 0;
      }
      transmitFrame(frameSize, streamID);
      TraceIndex++;
   }

   if(TraceIndex >= TraceVector.size()) {
      TraceIndex = 0;
   }

   // ====== Schedule next frame transmission ===============================
   if(TraceIndex < TraceVector.size()) {
      const double nextFrameTime = TraceVector[TraceIndex].InterFrameDelay;
      assert(TransmitTimerVector[0] == NULL);
      TransmitTimerVector[0] = new NetPerfMeterTransmitTimer("TransmitTimer");
      TransmitTimerVector[0]->setKind(TIMER_TRANSMIT);
      TransmitTimerVector[0]->setStreamID(0);

      // std::cout << simTime() << ", " << getFullPath()
      //           << ": Next in " << nextFrameTime << "s" << endl;

      scheduleAt(simTime() + nextFrameTime, TransmitTimerVector[0]);
   }
}


// ###### Receive data ######################################################
void NetPerfMeter::receiveMessage(cMessage* msg)
{
   const cPacket* dataMessage =
      dynamic_cast<const cPacket*>(msg);
   if(dataMessage != NULL) {
      unsigned int    streamID = 0;
      const simtime_t delay    = simTime() - dataMessage->getCreationTime();

      if(TransportProtocol == SCTP) {
         const SCTPRcvInfo* receiveCommand =
            check_and_cast<const SCTPRcvInfo*>(dataMessage->getControlInfo());
         streamID = receiveCommand->getSid();
      }

      ReceiverStatistics* receiverStatistics = getReceiverStatistics(streamID);
      receiverStatistics->ReceivedMessages++;
      receiverStatistics->ReceivedBytes += dataMessage->getByteLength();
      receiverStatistics->ReceivedDelayHistogram.collect(delay);
   }
   showIOStatus();
}


// ###### SCTP queue length configuration ###################################
void NetPerfMeter::sendSCTPQueueRequest(const unsigned int queueSize)
{
   assert(SocketSCTP != NULL);

   // Tell SCTP to limit the send queue to the number of bytes specified.
   // When the queue is able accept more data again, it will be indicated by
   // SCTP_I_SENDQUEUE_ABATED!

   SCTPInfo* queueInfo = new SCTPInfo();
   queueInfo->setText(queueSize);
   queueInfo->setAssocId(ConnectionID);

   cPacket* cmsg = new cPacket("QueueRequest");
   cmsg->setKind(SCTP_C_QUEUE_BYTES_LIMIT);
   cmsg->setControlInfo(queueInfo);
   if(IncomingSocketSCTP) {
      IncomingSocketSCTP->sendRequest(cmsg);
   }
   else {
      SocketSCTP->sendRequest(cmsg);
   }
}


// ###### TCP queue length configuration ####################################
void NetPerfMeter::sendTCPQueueRequest(const unsigned int queueSize)
{
   assert(SocketTCP != NULL);

   // Tell TCP to limit the send queue to the number of bytes specified.
   // When the queue is able accept more data again, it will be indicated by
   // TCP_I_SEND_MSG!

   TCPCommand* queueInfo = new TCPCommand();
   queueInfo->setUserId(queueSize);
   queueInfo->setConnId(ConnectionID);

   cPacket* cmsg = new cPacket("QueueRequest");
   cmsg->setKind(TCP_C_QUEUE_BYTES_LIMIT);
   cmsg->setControlInfo(queueInfo);
   if(IncomingSocketTCP) {
      IncomingSocketTCP->sendCommand(cmsg);
   }
   else {
      SocketTCP->sendCommand(cmsg);
   }
}


// ###### Return sprintf-formatted string ####################################
opp_string NetPerfMeter::format(const char* formatString, ...)
{
   char    str[1024];
   va_list args;
   va_start(args, formatString);
   vsnprintf((char*)&str, sizeof(str), formatString, args);
   va_end(args);
   return(opp_string(str));
}

}  // namespace inet
