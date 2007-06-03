#ifndef __GLOBALVARS_RIPD_H__
#define __GLOBALVARS_RIPD_H__

#include "allheaders.h"

extern struct GlobalVars_ripd * __activeVars_ripd;

void GlobalVars_initializeActiveSet_ripd();

struct GlobalVars_ripd
{
    struct cmd_element no_rip_offset_list_ifname_cmd__X;
    struct cmd_element no_ip_rip_authentication_string_cmd__X;
    long rip_global_queries__X;
    struct cmd_element no_rip_offset_list_cmd__X;
    struct cmd_element no_rip_distance_cmd__X;
    struct cmd_node rip_node__X;
    struct cmd_element no_rip_distance_source_access_list_cmd__X;
    struct cmd_element ip_rip_receive_version_cmd__X;
    struct cmd_element rip_redistribute_type_routemap_cmd__X;
    struct cmd_element rip_passive_interface_cmd__X;
    struct route_table * rip_distance_table__X;
    struct cmd_element no_rip_redistribute_type_metric_routemap_cmd__X;
    struct cmd_element no_rip_default_information_originate_cmd__X;
    struct cmd_element rip_distance_cmd__X;
    struct cmd_element no_rip_timers_cmd__X;
    struct cmd_element ip_rip_split_horizon_cmd__X;
    struct cmd_element ip_rip_authentication_string_cmd__X;
    struct cmd_element rip_redistribute_type_metric_routemap_cmd__X;
    struct cmd_element no_ip_rip_send_version_num_cmd__X;
    struct cmd_element no_rip_redistribute_rip_cmd__X;
    struct cmd_element no_rip_version_val_cmd__X;
    struct cmd_element no_rip_passive_interface_cmd__X;
    struct cmd_element no_debug_rip_events_cmd__X;
    struct cmd_element ip_rip_authentication_key_chain_cmd__X;
    struct cmd_element rip_version_cmd__X;
    struct cmd_element no_ip_rip_split_horizon_poisoned_reverse_cmd__X;
    struct cmd_element no_ip_rip_authentication_mode_type_authlen_cmd__X;
    struct cmd_element no_rip_network_cmd__X;
    struct cmd_element no_ip_rip_authentication_key_chain2_cmd__X;
    struct cmd_element rip_redistribute_type_metric_cmd__X;
    struct cmd_element no_debug_rip_packet_cmd__X;
    struct cmd_element no_router_rip_cmd__X;
    struct cmd_element rip_redistribute_type_cmd__X;
    struct cmd_element no_ip_rip_split_horizon_cmd__X;
    struct cmd_element rip_network_cmd__X;
    struct cmd_element debug_rip_zebra_cmd__X;
    struct cmd_element ip_rip_send_version_1_cmd__X;
    struct cmd_element rip_route_cmd__X;
    struct cmd_element rip_offset_list_ifname_cmd__X;
    struct cmd_element rip_distance_source_access_list_cmd__X;
    struct route_table * rip_neighbor_table__X;
    long rip_global_route_changes__X;
    vector rip_enable_interface__X;
    struct cmd_element no_rip_default_metric_cmd__X;
    struct cmd_element rip_default_information_originate_cmd__X;
    struct cmd_element rip_default_metric_cmd__X;
    struct cmd_element ip_rip_authentication_mode_authlen_cmd__X;
    struct cmd_element rip_neighbor_cmd__X;
    struct cmd_element no_rip_default_metric_val_cmd__X;
    struct cmd_element ip_rip_receive_version_1_cmd__X;
    struct cmd_element debug_rip_packet_direct_cmd__X;
    struct cmd_element show_debugging_rip_cmd__X;
    struct cmd_element no_ip_rip_receive_version_num_cmd__X;
    unsigned long rip_debug_event__X;
    struct cmd_element no_rip_redistribute_type_cmd__X;
    struct cmd_element no_rip_route_cmd__X;
    struct cmd_element ip_rip_send_version_2_cmd__X;
    struct cmd_element no_ip_rip_authentication_mode_cmd__X;
    struct cmd_element no_rip_neighbor_cmd__X;
    struct list * rip_offset_list_master__X;
    struct cmd_element router_rip_cmd__X;
    struct cmd_element no_ip_rip_authentication_string2_cmd__X;
    struct cmd_element no_ip_rip_receive_version_cmd__X;
    struct cmd_element rip_timers_cmd__X;
    struct route_table * rip_enable_network__X;
    struct cmd_element debug_rip_packet_detail_cmd__X;
    struct cmd_element no_ip_rip_send_version_cmd__X;
    struct cmd_element ip_rip_send_version_cmd__X;
    struct cmd_element no_rip_distance_source_cmd__X;
    struct cmd_element rip_redistribute_rip_cmd__X;
    struct cmd_element no_ip_rip_authentication_key_chain_cmd__X;
    struct cmd_element no_rip_version_cmd__X;
    struct cmd_element ip_rip_split_horizon_poisoned_reverse_cmd__X;
    unsigned long rip_debug_zebra__X;
    struct cmd_element rip_offset_list_cmd__X;
    struct cmd_element show_ip_rip_cmd__X;
    struct cmd_element rip_distance_source_cmd__X;
    struct cmd_element ip_rip_authentication_mode_cmd__X;
    struct cmd_element no_rip_redistribute_type_routemap_cmd__X;
    struct cmd_element no_debug_rip_zebra_cmd__X;
    struct cmd_element debug_rip_packet_cmd__X;
    struct cmd_element no_rip_timers_val_cmd__X;
    struct cmd_element ip_rip_receive_version_2_cmd__X;
    struct cmd_element no_debug_rip_packet_direct_cmd__X;
    struct_rip * rip__X;
    struct cmd_element debug_rip_events_cmd__X;
    unsigned long rip_debug_packet__X;
    vector Vrip_passive_nondefault__X;
    struct cmd_element no_ip_rip_authentication_mode_type_cmd__X;
    struct cmd_element no_rip_redistribute_type_metric_cmd__X;
    struct cmd_element show_ip_rip_status_cmd__X;
};

#define no_rip_offset_list_ifname_cmd__VAR  (__activeVars_ripd->no_rip_offset_list_ifname_cmd__X)
#define no_ip_rip_authentication_string_cmd__VAR  (__activeVars_ripd->no_ip_rip_authentication_string_cmd__X)
#define rip_global_queries__VAR  (__activeVars_ripd->rip_global_queries__X)
#define no_rip_offset_list_cmd__VAR  (__activeVars_ripd->no_rip_offset_list_cmd__X)
#define no_rip_distance_cmd__VAR  (__activeVars_ripd->no_rip_distance_cmd__X)
#define rip_node__VAR  (__activeVars_ripd->rip_node__X)
#define no_rip_distance_source_access_list_cmd__VAR  (__activeVars_ripd->no_rip_distance_source_access_list_cmd__X)
#define ip_rip_receive_version_cmd__VAR  (__activeVars_ripd->ip_rip_receive_version_cmd__X)
#define rip_redistribute_type_routemap_cmd__VAR  (__activeVars_ripd->rip_redistribute_type_routemap_cmd__X)
#define rip_passive_interface_cmd__VAR  (__activeVars_ripd->rip_passive_interface_cmd__X)
#define rip_distance_table__VAR  (__activeVars_ripd->rip_distance_table__X)
#define no_rip_redistribute_type_metric_routemap_cmd__VAR  (__activeVars_ripd->no_rip_redistribute_type_metric_routemap_cmd__X)
#define no_rip_default_information_originate_cmd__VAR  (__activeVars_ripd->no_rip_default_information_originate_cmd__X)
#define rip_distance_cmd__VAR  (__activeVars_ripd->rip_distance_cmd__X)
#define no_rip_timers_cmd__VAR  (__activeVars_ripd->no_rip_timers_cmd__X)
#define ip_rip_split_horizon_cmd__VAR  (__activeVars_ripd->ip_rip_split_horizon_cmd__X)
#define ip_rip_authentication_string_cmd__VAR  (__activeVars_ripd->ip_rip_authentication_string_cmd__X)
#define rip_redistribute_type_metric_routemap_cmd__VAR  (__activeVars_ripd->rip_redistribute_type_metric_routemap_cmd__X)
#define no_ip_rip_send_version_num_cmd__VAR  (__activeVars_ripd->no_ip_rip_send_version_num_cmd__X)
#define no_rip_redistribute_rip_cmd__VAR  (__activeVars_ripd->no_rip_redistribute_rip_cmd__X)
#define no_rip_version_val_cmd__VAR  (__activeVars_ripd->no_rip_version_val_cmd__X)
#define no_rip_passive_interface_cmd__VAR  (__activeVars_ripd->no_rip_passive_interface_cmd__X)
#define no_debug_rip_events_cmd__VAR  (__activeVars_ripd->no_debug_rip_events_cmd__X)
#define ip_rip_authentication_key_chain_cmd__VAR  (__activeVars_ripd->ip_rip_authentication_key_chain_cmd__X)
#define rip_version_cmd__VAR  (__activeVars_ripd->rip_version_cmd__X)
#define no_ip_rip_split_horizon_poisoned_reverse_cmd__VAR  (__activeVars_ripd->no_ip_rip_split_horizon_poisoned_reverse_cmd__X)
#define no_ip_rip_authentication_mode_type_authlen_cmd__VAR  (__activeVars_ripd->no_ip_rip_authentication_mode_type_authlen_cmd__X)
#define no_rip_network_cmd__VAR  (__activeVars_ripd->no_rip_network_cmd__X)
#define no_ip_rip_authentication_key_chain2_cmd__VAR  (__activeVars_ripd->no_ip_rip_authentication_key_chain2_cmd__X)
#define rip_redistribute_type_metric_cmd__VAR  (__activeVars_ripd->rip_redistribute_type_metric_cmd__X)
#define no_debug_rip_packet_cmd__VAR  (__activeVars_ripd->no_debug_rip_packet_cmd__X)
#define no_router_rip_cmd__VAR  (__activeVars_ripd->no_router_rip_cmd__X)
#define rip_redistribute_type_cmd__VAR  (__activeVars_ripd->rip_redistribute_type_cmd__X)
#define no_ip_rip_split_horizon_cmd__VAR  (__activeVars_ripd->no_ip_rip_split_horizon_cmd__X)
#define rip_network_cmd__VAR  (__activeVars_ripd->rip_network_cmd__X)
#define debug_rip_zebra_cmd__VAR  (__activeVars_ripd->debug_rip_zebra_cmd__X)
#define ip_rip_send_version_1_cmd__VAR  (__activeVars_ripd->ip_rip_send_version_1_cmd__X)
#define rip_route_cmd__VAR  (__activeVars_ripd->rip_route_cmd__X)
#define rip_offset_list_ifname_cmd__VAR  (__activeVars_ripd->rip_offset_list_ifname_cmd__X)
#define rip_distance_source_access_list_cmd__VAR  (__activeVars_ripd->rip_distance_source_access_list_cmd__X)
#define rip_neighbor_table__VAR  (__activeVars_ripd->rip_neighbor_table__X)
#define rip_global_route_changes__VAR  (__activeVars_ripd->rip_global_route_changes__X)
#define rip_enable_interface__VAR  (__activeVars_ripd->rip_enable_interface__X)
#define no_rip_default_metric_cmd__VAR  (__activeVars_ripd->no_rip_default_metric_cmd__X)
#define rip_default_information_originate_cmd__VAR  (__activeVars_ripd->rip_default_information_originate_cmd__X)
#define rip_default_metric_cmd__VAR  (__activeVars_ripd->rip_default_metric_cmd__X)
#define ip_rip_authentication_mode_authlen_cmd__VAR  (__activeVars_ripd->ip_rip_authentication_mode_authlen_cmd__X)
#define rip_neighbor_cmd__VAR  (__activeVars_ripd->rip_neighbor_cmd__X)
#define no_rip_default_metric_val_cmd__VAR  (__activeVars_ripd->no_rip_default_metric_val_cmd__X)
#define ip_rip_receive_version_1_cmd__VAR  (__activeVars_ripd->ip_rip_receive_version_1_cmd__X)
#define debug_rip_packet_direct_cmd__VAR  (__activeVars_ripd->debug_rip_packet_direct_cmd__X)
#define show_debugging_rip_cmd__VAR  (__activeVars_ripd->show_debugging_rip_cmd__X)
#define no_ip_rip_receive_version_num_cmd__VAR  (__activeVars_ripd->no_ip_rip_receive_version_num_cmd__X)
#define rip_debug_event__VAR  (__activeVars_ripd->rip_debug_event__X)
#define no_rip_redistribute_type_cmd__VAR  (__activeVars_ripd->no_rip_redistribute_type_cmd__X)
#define no_rip_route_cmd__VAR  (__activeVars_ripd->no_rip_route_cmd__X)
#define ip_rip_send_version_2_cmd__VAR  (__activeVars_ripd->ip_rip_send_version_2_cmd__X)
#define no_ip_rip_authentication_mode_cmd__VAR  (__activeVars_ripd->no_ip_rip_authentication_mode_cmd__X)
#define no_rip_neighbor_cmd__VAR  (__activeVars_ripd->no_rip_neighbor_cmd__X)
#define rip_offset_list_master__VAR  (__activeVars_ripd->rip_offset_list_master__X)
#define router_rip_cmd__VAR  (__activeVars_ripd->router_rip_cmd__X)
#define no_ip_rip_authentication_string2_cmd__VAR  (__activeVars_ripd->no_ip_rip_authentication_string2_cmd__X)
#define no_ip_rip_receive_version_cmd__VAR  (__activeVars_ripd->no_ip_rip_receive_version_cmd__X)
#define rip_timers_cmd__VAR  (__activeVars_ripd->rip_timers_cmd__X)
#define rip_enable_network__VAR  (__activeVars_ripd->rip_enable_network__X)
#define debug_rip_packet_detail_cmd__VAR  (__activeVars_ripd->debug_rip_packet_detail_cmd__X)
#define no_ip_rip_send_version_cmd__VAR  (__activeVars_ripd->no_ip_rip_send_version_cmd__X)
#define ip_rip_send_version_cmd__VAR  (__activeVars_ripd->ip_rip_send_version_cmd__X)
#define no_rip_distance_source_cmd__VAR  (__activeVars_ripd->no_rip_distance_source_cmd__X)
#define rip_redistribute_rip_cmd__VAR  (__activeVars_ripd->rip_redistribute_rip_cmd__X)
#define no_ip_rip_authentication_key_chain_cmd__VAR  (__activeVars_ripd->no_ip_rip_authentication_key_chain_cmd__X)
#define no_rip_version_cmd__VAR  (__activeVars_ripd->no_rip_version_cmd__X)
#define ip_rip_split_horizon_poisoned_reverse_cmd__VAR  (__activeVars_ripd->ip_rip_split_horizon_poisoned_reverse_cmd__X)
#define rip_debug_zebra__VAR  (__activeVars_ripd->rip_debug_zebra__X)
#define rip_offset_list_cmd__VAR  (__activeVars_ripd->rip_offset_list_cmd__X)
#define show_ip_rip_cmd__VAR  (__activeVars_ripd->show_ip_rip_cmd__X)
#define rip_distance_source_cmd__VAR  (__activeVars_ripd->rip_distance_source_cmd__X)
#define ip_rip_authentication_mode_cmd__VAR  (__activeVars_ripd->ip_rip_authentication_mode_cmd__X)
#define no_rip_redistribute_type_routemap_cmd__VAR  (__activeVars_ripd->no_rip_redistribute_type_routemap_cmd__X)
#define no_debug_rip_zebra_cmd__VAR  (__activeVars_ripd->no_debug_rip_zebra_cmd__X)
#define debug_rip_packet_cmd__VAR  (__activeVars_ripd->debug_rip_packet_cmd__X)
#define no_rip_timers_val_cmd__VAR  (__activeVars_ripd->no_rip_timers_val_cmd__X)
#define ip_rip_receive_version_2_cmd__VAR  (__activeVars_ripd->ip_rip_receive_version_2_cmd__X)
#define no_debug_rip_packet_direct_cmd__VAR  (__activeVars_ripd->no_debug_rip_packet_direct_cmd__X)
#define rip__VAR  (__activeVars_ripd->rip__X)
#define debug_rip_events_cmd__VAR  (__activeVars_ripd->debug_rip_events_cmd__X)
#define rip_debug_packet__VAR  (__activeVars_ripd->rip_debug_packet__X)
#define Vrip_passive_nondefault__VAR  (__activeVars_ripd->Vrip_passive_nondefault__X)
#define no_ip_rip_authentication_mode_type_cmd__VAR  (__activeVars_ripd->no_ip_rip_authentication_mode_type_cmd__X)
#define no_rip_redistribute_type_metric_cmd__VAR  (__activeVars_ripd->no_rip_redistribute_type_metric_cmd__X)
#define show_ip_rip_status_cmd__VAR  (__activeVars_ripd->show_ip_rip_status_cmd__X)

#endif
