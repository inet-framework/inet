/*******************************************************************
*
*    This library is free software, you can redistribute it
*    and/or modify
*    it under  the terms of the GNU Lesser General Public License
*    as published by the Free Software Foundation;
*    either version 2 of the License, or any later version.
*    The library is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*    See the GNU Lesser General Public License for more details.
*
*
*********************************************************************/

#ifndef __MPLSMODULE_H__

#define __MPLSMODULE_H__

#include <vector>
#include <omnetpp.h>
#include "MPLSPacket.h"
#include "IPDatagram.h"
#include "LIBTableAccess.h"
#include "RoutingTableAccess.h"
#include "RoutingTable.h"
#include "ConstType.h"


using namespace std;

#define DEST_CLASSIFIER        1
#define DEST_SOURCE_CLASSIFIER 2
#define MAX_LSP_NO            25
#define PUSH_OPER            0
#define SWAP_OPER            1
#define POP_OPER            2



struct fec_color_mapping
{
    int fecId;
    int color;
};

struct FECElem
{
    FECElem() {fecId=-1;src=-1;dest=-1;}
    int fecId;
    int src;
    int dest;
};



class MPLSModule : public cSimpleModule
{
   private:
      RoutingTableAccess routingTableAccess;
      LIBTableAccess libTableAccess;

      bool isIR;
      bool isER;
      bool isSignallingReady;
      int  classifierType;
      simtime_t delay1;

      vector<FECElem> fecList;

      cArray ipdataQueue;  // Queue of packets need label from LDP -- FIXME should be cQueue!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      cQueue ldpQueue;   // Queue of queries to LDP when the LDP is not ready

   public:
      Module_Class_Members(MPLSModule,cSimpleModule,0);

      /**
       * Initialize the module parameters
       */
      virtual void initialize();

      /** @name Message handling routines */
      //@{
      virtual void handleMessage(cMessage *msg);

      virtual void processPacketFromL3(cMessage *msg);
      virtual void processPacketFromSignalling(cMessage *msg);
      virtual void processPacketFromL2(cMessage *msg);
      // FIXME introduce these:
      //virtual void processMPLSPacketFromL2(MPLSPacket *mplsPacket);
      //virtual void processIPDatagramFromL2(IPDatagram *ipdata);
      //@}

      /**
       * Classify FEC for packet
       * @param IPDatagram The ip packet to be classified
       * @param type Scheme of classification, i.g destination-based or destination and sender-based
       */
      int classifyPacket(IPDatagram *ipdata, int type);

};



#endif

