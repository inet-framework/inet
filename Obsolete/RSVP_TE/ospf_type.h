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


/*
*	File Name OSPF_type.h
*	LDP library
*	This file defines constants used for the OSPF module
**/
#ifndef __OSPF_TYPE____H
#define __OSPF_TYPE____H

namespace OSPFType
{
/*Maximum number of neigbors*/
const int MAX_NO_NEIGHBORS =    10;

/*Maximum number of Shortest Paths*/
const int MAX_NO_SPROUTERS =    100;

/*Maximum number of links to one interface*/
const int MAX_NO_LINKS =        10;

/*Maximum number of LSA*/
const int MAX_NO_LSAS	=  		200;

/*Maximum number of RRA LSA*/
const int MAX_NO_RRA_LSAS =	  	200;

/*Maximum number of areas (unused)*/
const int MAX_NO_AREAS	=		5;

/*Maximum number of multicat group (unused)*/
const int MAX_NO_GRPS  =        30;

/*Maximum number of networks*/
const int MAX_NO_NETWORKS   =   110;

/*Hop types*/
const int ROUTER        =		1;
const int TRANSITNET    =		2;
const int  STUBNET      =		3;

/*Maximum cost*/
const int LSInfinity 		   = 100000;

};

#endif


