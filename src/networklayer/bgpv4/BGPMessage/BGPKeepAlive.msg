
cplusplus {{
#include "BGPHeader_m.h"
}}


class BGPHeader;

//
// Represents a \BGP Msg KeepAlive, to be used with the BGP module.
//
// \BGP KeepALive fields the same that a BGP header
//Les messages KeepAlive sont échangés aussi souvent qu’il faut entre les systèmes BGP 
//pour ne dépasser le Hold Timer (compteur de retenue defini dans BGPOpen). 
//
//						


packet BGPKeepAliveMessage extends BGPHeader
{
    @customize(true);
    @omitGetVerb(true);
}
