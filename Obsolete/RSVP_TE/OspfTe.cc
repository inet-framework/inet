/*******************************************************************
*
*	This library is free software, you can redistribute it 
*	and/or modify 
*	it under  the terms of the GNU Lesser General Public License 
*	as published by the Free Software Foundation; 
*	either version 2 of the License, or any later version.
*	The library is distributed in the hope that it will be useful, 
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
*	See the GNU Lesser General Public License for more details.
*
*
*********************************************************************/
#include "OspfTe.h"
#include "ip_address.h"


Define_Module( OspfTe);


void OspfTe::initialize(int stage)
{


switch(stage)
{
case 0:



	ev << "OSPF Enter Stage 1\n";

	//Get the local address
	//Get the ted
	//Get the IP Routing component
	local_addr = IPAddress(par("local_addr").stringValue()).getInt();

        break;




case 1:
	ev << "OSPF Enter Stage 2\n";
	break;
default:
	ev << "OSPF : Omnetpp is so fo\n";
	break;

}

}

void OspfTe::activity()
{
bool IsIR =false;
cModule* curmod = this;
ev << "OSPF TE: I am starting \n";


   for (curmod = parentModule(); curmod != NULL; curmod = curmod->parentModule())
	{

		if (curmod->hasPar("isIR"))
		{
			IsIR = curmod->par("isIR").boolValue();
			break;
	

		}

	}
	if(IsIR)
	{
	
		updateTED();
	

	}
}


/*******************************************************************************
*                           OSPF-TE (CSPF)ROUTING
*
*******************************************************************************/
void OspfTe::TEAddCandidates(FlowSpecObj_t* fspec,
                             std::vector<CSPFVertex_Struct>  *CandidatesList )
 { 

 //CSPFVertex_Struct VertexVV = *(CShortestPathTree.back());
 CSPFVertex_Struct* VertexV = &CShortestPathTree.back();

	CSPFVertex_Struct* VertexW = new CSPFVertex_Struct;
    std::vector<telinkstate>::iterator ted_iterI;
	telinkstate ted_iter;
    std::vector<CSPFVertex_Struct>::iterator spt_iterI;
	CSPFVertex_Struct spt_iter;
   
         
    for (ted_iterI=ted.begin(); ted_iterI != ted.end(); ted_iterI++)
    {
		ted_iter = (telinkstate)*ted_iterI;
        //Other ends of the links to V only
        if(ted_iter.advrouter!= VertexV->VertexId)
            continue;
        
        //Not in the ConstrainedShortestPathTree only
        bool IsFound =false;
        for(spt_iterI = CShortestPathTree.begin();spt_iterI !=CShortestPathTree.end();spt_iterI++)
        {
			spt_iter = (CSPFVertex_Struct)*spt_iterI;
                if(ted_iter.linkid == spt_iter.VertexId)
                {
                      IsFound = true;
                      break;
                }
        }
        if(IsFound)
             continue;
        
        //Satisfy the resource constraint of BW only
        if((ted_iter.UnResvBandwith[0]) < (fspec->req_bandwidth))
             continue;
        
        //Normal Dijkstra Algorithm for adding candidate
        VertexW->VertexId = (ted_iter.linkid);
        VertexW->DistanceToRoot = (VertexV->DistanceToRoot) + (ted_iter.metric);
        VertexW->Parent = VertexV;
        
        IsFound = false; 
        if( !(CandidatesList->empty()) )
        {     
                for (spt_iterI=CandidatesList->begin(); spt_iterI != CandidatesList->end(); spt_iterI++)
                {
					spt_iter = (CSPFVertex_Struct)*spt_iterI;

                    if( spt_iter.VertexId == VertexW->VertexId) 
                     {
                          IsFound = true;
                           break;
                     }
               
                 }
          }
          
          //Relaxation
          if(IsFound)
          {
                 if(spt_iter.DistanceToRoot > VertexW->DistanceToRoot )
                 {
                      spt_iter.DistanceToRoot = VertexW->DistanceToRoot;
                      spt_iter.Parent = VertexV;
                      //TECalculateNextHops( VertexW, VertexV, *ted_iter);               
                 }
          }
          else
          CandidatesList->push_back(*VertexW);

               
    }

    CspfBuildSPT(fspec, CandidatesList);
    
       
}

std::vector<int>
OspfTe::CalculateERO(IPAddress* dest, std::vector<CSPFVertex_Struct> *CandidatesList, double* totalMetric )
{
    std::vector<int> EROList;
    std::vector<CSPFVertex_Struct>::iterator spt_iterI;
	CSPFVertex_Struct spt_iter;
    CSPFVertex_Struct* curVertex;
    
    if( !(CandidatesList->empty()) )
        {     
            for (spt_iterI=CandidatesList->begin(); spt_iterI != CandidatesList->end(); spt_iterI++)
            {
				spt_iter = (CSPFVertex_Struct)*spt_iterI;
            
            if(spt_iter.VertexId == (*dest))
                break;
            
            }
            
            curVertex = &spt_iter;

			(*totalMetric) = curVertex->DistanceToRoot;
            
            while(curVertex !=NULL)
            {
                 EROList.push_back(curVertex->VertexId.getInt());
                 curVertex = curVertex->Parent;
            }
                
        }
        return EROList;
    
}



void OspfTe::CspfBuildSPT(FlowSpecObj_t* fspec, std::vector<CSPFVertex_Struct> *CandidatesList )
{
	int Inx, i;
	CSPFVertex_Struct* VertexW = new CSPFVertex_Struct;

	double shortestDist = OSPFType::LSInfinity;
       
   // CSPFVertex_Struct* tmpCandidateEle;
        
    if( !(CandidatesList->empty()) )
    {
         std::vector<CSPFVertex_Struct>::iterator iterI;
		 CSPFVertex_Struct iter;
         
         for (iterI=CandidatesList->begin(); iterI != CandidatesList->end(); iterI++)
         {
			 iter=(CSPFVertex_Struct)*iterI;
             if(iter.DistanceToRoot < shortestDist)
                 shortestDist = iter.DistanceToRoot;           
         }
         
         for (iterI=CandidatesList->begin(); iterI != CandidatesList->end(); iterI++)
         {
			 iter=(CSPFVertex_Struct)*iterI;
             if(iter.DistanceToRoot == shortestDist)
                 break;     
         }
         //Add to Tree

         VertexW->DistanceToRoot = shortestDist;
         VertexW->Parent = iter.Parent;
         VertexW->VertexId = iter.VertexId;

         //CShortestPathTree.push_back(*VertexW);

         //ev << "OSPF add " << VertexW->VertexId.getString() << "\n";
         CShortestPathTree.push_back(*VertexW);
         CandidatesList->erase(iterI);

         TEAddCandidates(fspec, CandidatesList);

    }     
}


std::vector<int> OspfTe::CalculateERO(IPAddress* dest, FlowSpecObj_t* fspec, double* metric)
{
    std::vector<CSPFVertex_Struct> candidates;
    //Get Latest TED
    updateTED();
    
    //Reset the Constraint Shortest Path Tree
    CShortestPathTree.clear();
    CSPFVertex_Struct* RootVertex = new CSPFVertex_Struct;
    RootVertex->DistanceToRoot =0;
    RootVertex->Parent = NULL;
    RootVertex->VertexId = IPAddress(local_addr);

    candidates.push_back(*RootVertex);
    CspfBuildSPT(fspec, &candidates );
    return CalculateERO(dest, &CShortestPathTree, metric );
    
    
}

/******************************************************************************
*This is a tricky part of the simulation since only computation part of OSPF is built
*This force the immediate database convergence
******************************************************************************/
void OspfTe::updateTED()
{
    
	// find TED
   ev << "************************OSPF TE *******************\n" ;
    cTopology topo;
    topo.extractByModuleType( "TED", NULL );
    
    sTopoNode *node = topo.node(0);
    TED *module = (TED*)(node->module());
    if(module == NULL)
    {
    ev << "OSPF Error: Fail to locate TED";
    return;
    }

    ted = *(module->getTED());


     //std::vector<telinkstate>::iterator t_iterI;
	 //telinkstate			 t_iter;

	 /*
	 for(t_iterI = ted.begin(); t_iterI != ted.end(); t_iterI++)
	 {
		 t_iter = *t_iterI;
		 ev << "*****************OSPF TED INITIAL*****************\n";
		 ev << "Adv Router: " << t_iter.advrouter.getString() << "\n";
		 ev << "Link Id (neighbour IP): " << t_iter.linkid.getString() << "\n";
		 ev << "Max Bandwidth: " << t_iter.MaxBandwith << "\n";
		 ev << "Metric: " << t_iter.metric << "\n\n";
	 }
	*/

    
}

std::vector<int> OspfTe::CalculateERO(IPAddress* dest, std::vector<simple_link_t> *links,
								FlowSpecObj_t* old_fspec, FlowSpecObj_t* new_fspec, double* totalDelay)
{
	std::vector<CSPFVertex_Struct> candidates;
    //Get Latest TED
    updateTED();
    
    //Reset the Constraint Shortest Path Tree
    CShortestPathTree.clear();
    CSPFVertex_Struct* RootVertex = new CSPFVertex_Struct;
    RootVertex->DistanceToRoot =0;
    RootVertex->Parent = NULL;
    RootVertex->VertexId = IPAddress(local_addr);

    candidates.push_back(*RootVertex);
    CspfBuildSPT(links, old_fspec,new_fspec, &candidates );
    return CalculateERO(dest, &CShortestPathTree, totalDelay );

}
void OspfTe::CspfBuildSPT(std::vector<simple_link_t> *links, FlowSpecObj_t *old_fspec,
						  FlowSpecObj_t *new_fspec,
						  std::vector<CSPFVertex_Struct> *CandidatesList )
{

	int Inx, i;
	CSPFVertex_Struct* VertexW = new CSPFVertex_Struct;

	double shortestDist = OSPFType::LSInfinity;
       
   // CSPFVertex_Struct* tmpCandidateEle;
        
    if( !(CandidatesList->empty()) )
    {
         std::vector<CSPFVertex_Struct>::iterator iterI;
		 CSPFVertex_Struct iter;
         
         for (iterI=CandidatesList->begin(); iterI != CandidatesList->end(); iterI++)
         {
			 iter=(CSPFVertex_Struct)*iterI;
             if(iter.DistanceToRoot < shortestDist)
                 shortestDist = iter.DistanceToRoot;           
         }
         
         for (iterI=CandidatesList->begin(); iterI != CandidatesList->end(); iterI++)
         {
			 iter=(CSPFVertex_Struct)*iterI;
             if(iter.DistanceToRoot == shortestDist)
                 break;     
         }
         //Add to Tree

         VertexW->DistanceToRoot = shortestDist;
         VertexW->Parent = iter.Parent;
         VertexW->VertexId = iter.VertexId;

         //CShortestPathTree.push_back(*VertexW);

         //ev << "OSPF add " << VertexW->VertexId.getString() << "\n";
         CShortestPathTree.push_back(*VertexW);
         CandidatesList->erase(iterI);

         TEAddCandidates(links, old_fspec, new_fspec, CandidatesList);

    }     

}

void OspfTe::TEAddCandidates(std::vector<simple_link_t> *links, FlowSpecObj_t *old_fspec,
						  FlowSpecObj_t *new_fspec,
						  std::vector<CSPFVertex_Struct> *CandidatesList )
{
	 //CSPFVertex_Struct VertexVV = *(CShortestPathTree.back());
	CSPFVertex_Struct* VertexV = &CShortestPathTree.back();

	CSPFVertex_Struct* VertexW = new CSPFVertex_Struct;
    std::vector<telinkstate>::iterator ted_iterI;
	telinkstate ted_iter;
    std::vector<CSPFVertex_Struct>::iterator spt_iterI;
	CSPFVertex_Struct spt_iter;
	std::vector<simple_link_t>::iterator iterS;
	simple_link_t aLink;
   
         
    for (ted_iterI=ted.begin(); ted_iterI != ted.end(); ted_iterI++)
    {
		ted_iter = (telinkstate)*ted_iterI;
        //Other ends of the links to V only
        if(ted_iter.advrouter!= VertexV->VertexId)
            continue;
        
        //Not in the ConstrainedShortestPathTree only
        bool IsFound =false;
        for(spt_iterI = CShortestPathTree.begin();spt_iterI !=CShortestPathTree.end();spt_iterI++)
        {
			spt_iter = (CSPFVertex_Struct)*spt_iterI;
                if(ted_iter.linkid == spt_iter.VertexId)
                {
                      IsFound = true;
                      break;
                }
        }
        if(IsFound)
             continue;
        
        //Satisfy the resource constraint of BW only
		bool sharedLink= false;
		for(iterS= links->begin(); iterS != links->end();iterS++)
		{
			aLink =(simple_link_t)(*iterS);
			if(aLink.advRouter == ted_iter.advrouter.getInt() &&
				aLink.id == ted_iter.linkid.getInt())
			{
				sharedLink =true;
				break;
			}
		
		}
		if(sharedLink)
		{
			if((ted_iter.UnResvBandwith[0]) + (old_fspec->req_bandwidth) < (new_fspec->req_bandwidth))
				continue;
		}
		else
		{
			if((ted_iter.UnResvBandwith[0]) < (new_fspec->req_bandwidth))
             continue;
		}
        
        //Normal Dijkstra Algorithm for adding candidate
        VertexW->VertexId = (ted_iter.linkid);
        VertexW->DistanceToRoot = (VertexV->DistanceToRoot) + (ted_iter.metric);
        VertexW->Parent = VertexV;
        
        IsFound = false; 
        if( !(CandidatesList->empty()) )
        {     
                for (spt_iterI=CandidatesList->begin(); spt_iterI != CandidatesList->end(); spt_iterI++)
                {
					spt_iter = (CSPFVertex_Struct)*spt_iterI;

                    if( spt_iter.VertexId == VertexW->VertexId) 
                     {
                          IsFound = true;
                           break;
                     }
               
                 }
          }
          
          //Relaxation
          if(IsFound)
          {
                 if(spt_iter.DistanceToRoot > VertexW->DistanceToRoot )
                 {
                      spt_iter.DistanceToRoot = VertexW->DistanceToRoot;
                      spt_iter.Parent = VertexV;
                      //TECalculateNextHops( VertexW, VertexV, *ted_iter);               
                 }
          }
          else
          CandidatesList->push_back(*VertexW);

               
    }

    CspfBuildSPT(links, old_fspec,new_fspec, CandidatesList);
    

}








