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
#ifndef __OSPF_TED____H
#define __OSPF_TED____H

//#include "OspfTe.h"
#include <vector>


struct telinkstate{
IPAddress advrouter;
int type;
IPAddress linkid;
IPAddress local;
IPAddress remote;
double metric;
double MaxBandwith;
double MaxResvBandwith;
double UnResvBandwith[8];
int AdminGrp;
};

struct simple_link_t{

    int advRouter;
    int id;
};

class TED : public cSimpleModule
{
private:

    std::vector<telinkstate>    ted;

public:
    Module_Class_Members(TED, cSimpleModule, 16384);

    virtual void initialize();
    virtual void activity();
    void buildDatabase();
    inline std::vector<telinkstate>* getTED(){return &ted;}
    void printDatabase();
    void updateLink(simple_link_t* aLink, double metric, double bw);


};


#endif
