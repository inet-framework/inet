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
#ifndef __OSPF_TE__H__
#define __OSPF_TE__H__


#include "ospf_type.h"
#include <omnetpp.h>
#include "tcp.h"
#include "ip_address.h"
#include "intserv.h"
#include "TED.h"
//#include <list>
#include <vector>


/*******************************************************************************
*                                 OSPF TE TYPES 
*******************************************************************************/

struct CSPFVertex_Struct{
    IPAddress VertexId;
    struct CSPFVertex_Struct * Parent;
    double DistanceToRoot; 
};




class OspfTe : public cSimpleModule
{
private:

	std::vector<CSPFVertex_Struct> CShortestPathTree;
	std::vector<telinkstate>	ted;
	int local_addr;

	void
	TEAddCandidates(FlowSpecObj_t* fspec,
                                                    std::vector<CSPFVertex_Struct> *CandidatesList );
    
    std::vector<int>
    CalculateERO(IPAddress* dest, std::vector<CSPFVertex_Struct> *CandidatesList, double* totalMetric );


    void CspfBuildSPT( FlowSpecObj_t* fspec, std::vector<CSPFVertex_Struct> *CandidatesList );
	void CspfBuildSPT(std::vector<simple_link_t> *links, FlowSpecObj_t *old_fspec,
						  FlowSpecObj_t *new_fspec,
						  std::vector<CSPFVertex_Struct> *CandidatesList );
	void TEAddCandidates(std::vector<simple_link_t> *links, FlowSpecObj_t *old_fspec,
						  FlowSpecObj_t *new_fspec,
						  std::vector<CSPFVertex_Struct> *CandidatesList );
    void updateTED();

public:
	Module_Class_Members(OspfTe, cSimpleModule, 16384);
	void initialize(int stage);
	int numInitStages() { return 2; }
	virtual void activity();
	std::vector<int> CalculateERO(IPAddress* dest, FlowSpecObj_t* fspec, double* totalMetric);
	std::vector<int> CalculateERO(IPAddress* dest, std::vector<simple_link_t> *links,
								FlowSpecObj_t* old_fspec, FlowSpecObj_t* new_fspec, double* totalDelay);

						





};

#endif
