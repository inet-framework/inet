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
#include "MPLSModule.h"
#include "TED.h"

Define_Module( TED);

void TED::initialize()
{

  buildDatabase();
  printDatabase();

}

void TED::buildDatabase()
{
    if(!(ted.empty()))
        ted.clear();

    telinkstate* entry = new telinkstate;
    
    cTopology topo;
    topo.extractByModuleType( "TCPClientTest","TCPServerTest","RSVP_LSR_Node", NULL );
    ev << "Total number of RSVP LSR nodes = " << topo.nodes() << "\n";
    
    for (int i=0; i<topo.nodes(); i++)
    {
       sTopoNode *node = topo.node(i);
       cModule *module = node->module();
       //Get the MPLS componet
    RoutingTable* myRT =
(RoutingTable*)(module->submodule("networkLayers")->submodule("routingTable"));



       //Get the RoutingTable component - Todo: Set it public
       entry->advrouter = IPAddress(module->par("local_addr").stringValue());

           //*(myRT->getLoopbackAddress()); //Todo: Add this function to rt
       
       for (int j=0; j<node->outLinks(); j++)
       {
            
            cModule *neighbour = node->out(j)->remoteNode()->module();

            RoutingTable* neighbourRT =(RoutingTable*)(neighbour->submodule("networkLayers")->submodule("routingTable") ) ;

            //For each link
            //Get linkId
            entry->linkid = IPAddress(neighbour->par("local_addr").stringValue());
            
            int local_gateIndex = node->out(j)->localGate()->index();
            int remote_gateIndex = node->out(j)->remoteGate()->index();
            
            //Get local address
            entry->local = *(myRT->getInterfaceByIndex(local_gateIndex)->inetAddr);
            //Get remote address
            entry->remote= *(neighbourRT->getInterfaceByIndex(remote_gateIndex)->inetAddr);
            
            double BW =node->out(j)->localGate()->datarate()->doubleValue();
            double delay = node->out(j)->localGate()->delay()->doubleValue();
            entry->MaxBandwith = BW;
            entry->MaxResvBandwith=BW;
            entry->metric = delay;
            for(int k=0;k<8;k++)
            entry->UnResvBandwith[k]=BW;
            entry->type = 1; 
            
            
            ted.push_back(*entry);
       }
    }

    printDatabase();

}

void TED::activity()

{

}

//Test purpose

void TED::printDatabase()
{
     std::vector<telinkstate>::iterator t_iterI;
     telinkstate                      t_iter;
        ev << "*************TED DATABASE*******************\n";
     for(t_iterI = ted.begin(); t_iterI != ted.end(); t_iterI++)
     {
         t_iter = *t_iterI;
    
         ev << "Adv Router: " << t_iter.advrouter.getString() << "\n";
         ev << "Link Id (neighbour IP): " << t_iter.linkid.getString() << "\n";
         ev << "Max Bandwidth: " << t_iter.MaxBandwith << "\n";
         ev << "Metrix: " << t_iter.metric << "\n\n";
     }


}

void TED::updateLink(simple_link_t *aLink, double metric, double bw)
{
       
     for(int i=0; i< ted.size(); i++)
     {
         if((ted[i].advrouter.getInt() == (aLink->advRouter)) &&
            (ted[i].linkid.getInt() ==(aLink->id)))
         {
             ev << "TED update an entry\n";
             ev << "Advrouter=" <<ted[i].advrouter.getString() << "\n";
             ev << "linkId=" << ted[i].linkid.getString() << "\n";
             double bwIncrease = bw - (ted[i].MaxBandwith);
            ted[i].MaxBandwith = ted[i].MaxBandwith + bwIncrease;
            ted[i].MaxResvBandwith = ted[i].MaxResvBandwith+ bwIncrease;
            ev << "Old metric= " << ted[i].metric << "\n";

            ted[i].metric = metric;
            ev << "New metric= " << ted[i].metric << "\n";

            for(int j=0;j<8;j++)
            ted[i].UnResvBandwith[j] = ted[i].UnResvBandwith[j] +bwIncrease;
            break;
         }
    
    
     }
     printDatabase();

}




