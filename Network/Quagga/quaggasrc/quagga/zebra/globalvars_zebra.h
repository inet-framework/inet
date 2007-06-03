#ifndef __GLOBALVARS_ZEBRA_H__
#define __GLOBALVARS_ZEBRA_H__

#include "allheaders.h"

extern struct GlobalVars_zebra * __activeVars_zebra;

void GlobalVars_initializeActiveSet_zebra();

struct GlobalVars_zebra
{
    struct cmd_element debug_zebra_packet_cmd__X;
    unsigned long zebra_debug_event__X;
    struct zebra_t zebrad__X;
    struct cmd_element debug_zebra_kernel_cmd__X;
    struct cmd_element no_debug_zebra_events_cmd__X;
    struct cmd_element debug_zebra_packet_direct_cmd__X;
    struct cmd_element show_zebra_client_cmd__X;
    struct cmd_element no_debug_zebra_packet_cmd__X;
    struct cmd_element debug_zebra_packet_detail_cmd__X;
    struct cmd_element no_debug_zebra_kernel_cmd__X;
    struct cmd_element debug_zebra_events_cmd__X;
    struct cmd_element no_debug_zebra_packet_direct_cmd__X;
    unsigned long zebra_debug_packet__X;
    struct cmd_element show_debugging_zebra_cmd__X;
    unsigned long zebra_debug_kernel__X;
    struct cmd_element zebra_interface_cmd__X;
};

#define debug_zebra_packet_cmd__VAR  (__activeVars_zebra->debug_zebra_packet_cmd__X)
#define zebra_debug_event__VAR  (__activeVars_zebra->zebra_debug_event__X)
#define zebrad__VAR  (__activeVars_zebra->zebrad__X)
#define debug_zebra_kernel_cmd__VAR  (__activeVars_zebra->debug_zebra_kernel_cmd__X)
#define no_debug_zebra_events_cmd__VAR  (__activeVars_zebra->no_debug_zebra_events_cmd__X)
#define debug_zebra_packet_direct_cmd__VAR  (__activeVars_zebra->debug_zebra_packet_direct_cmd__X)
#define show_zebra_client_cmd__VAR  (__activeVars_zebra->show_zebra_client_cmd__X)
#define no_debug_zebra_packet_cmd__VAR  (__activeVars_zebra->no_debug_zebra_packet_cmd__X)
#define debug_zebra_packet_detail_cmd__VAR  (__activeVars_zebra->debug_zebra_packet_detail_cmd__X)
#define no_debug_zebra_kernel_cmd__VAR  (__activeVars_zebra->no_debug_zebra_kernel_cmd__X)
#define debug_zebra_events_cmd__VAR  (__activeVars_zebra->debug_zebra_events_cmd__X)
#define no_debug_zebra_packet_direct_cmd__VAR  (__activeVars_zebra->no_debug_zebra_packet_direct_cmd__X)
#define zebra_debug_packet__VAR  (__activeVars_zebra->zebra_debug_packet__X)
#define show_debugging_zebra_cmd__VAR  (__activeVars_zebra->show_debugging_zebra_cmd__X)
#define zebra_debug_kernel__VAR  (__activeVars_zebra->zebra_debug_kernel__X)
#define zebra_interface_cmd__VAR  (__activeVars_zebra->zebra_interface_cmd__X)

#endif