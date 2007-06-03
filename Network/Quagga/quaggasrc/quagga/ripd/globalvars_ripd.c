
#include "globalvars_ripd.h"

struct Globalvars_ripd * activeVars_ripd;

struct GlobalVars_ripd * GlobalVars_createActiveSet_ripd()
{
    struct GlobalVars_ripd * ptr = (struct GlobalVars_ripd *)malloc(sizeof(struct GlobalVars_ripd));
    memset(ptr, 0, sizeof(struct GlobalVars_ripd));
    return ptr;
}

extern struct cmd_element no_rip_redistribute_type_cmd_ripd;
extern struct route_map_rule_cmd route_match_metric_cmd_ripd;
extern struct cmd_element rip_passive_interface_cmd_ripd;
extern struct cmd_element no_ip_rip_send_version_cmd_ripd;
extern struct cmd_element rip_offset_list_ifname_cmd_ripd;
extern struct cmd_element match_ip_address_cmd_ripd;
extern struct cmd_element no_rip_redistribute_type_routemap_cmd_ripd;
extern struct route_map_rule_cmd route_match_ip_next_hop_cmd_ripd;
extern struct cmd_element no_rip_offset_list_ifname_cmd_ripd;
extern struct cmd_element no_debug_rip_zebra_cmd_ripd;
extern struct cmd_element no_rip_route_cmd_ripd;
extern struct cmd_element no_set_metric_val_cmd_ripd;
extern struct cmd_element no_ip_rip_authentication_string_cmd_ripd;
extern struct cmd_element no_rip_network_cmd_ripd;
extern struct cmd_element no_ip_rip_authentication_key_chain2_cmd_ripd;
extern struct cmd_element rip_distance_source_access_list_cmd_ripd;
extern struct cmd_element ip_rip_send_version_2_cmd_ripd;
extern struct cmd_element ip_rip_send_version_cmd_ripd;
extern struct cmd_element no_rip_distance_source_cmd_ripd;
extern struct cmd_element no_router_zebra_cmd_ripd;
extern struct cmd_element debug_rip_packet_cmd_ripd;
extern struct cmd_element match_tag_cmd_ripd;
extern struct cmd_element no_ip_rip_authentication_mode_cmd_ripd;
extern struct cmd_element no_match_ip_address_cmd_ripd;
extern struct cmd_element no_rip_redistribute_type_metric_routemap_cmd_ripd;
extern struct cmd_element no_match_ip_address_prefix_list_val_cmd_ripd;
extern struct cmd_element no_rip_timers_val_cmd_ripd;
extern struct cmd_element ip_rip_authentication_string_cmd_ripd;
extern struct cmd_element set_metric_cmd_ripd;
extern struct cmd_element rip_redistribute_type_metric_cmd_ripd;
extern struct route_map_rule_cmd route_match_ip_address_cmd_ripd;
extern struct cmd_element ip_rip_receive_version_2_cmd_ripd;
extern struct cmd_element rip_redistribute_type_metric_routemap_cmd_ripd;
extern struct route_map_rule_cmd route_set_metric_cmd_ripd;
extern struct cmd_element no_rip_default_metric_cmd_ripd;
extern struct route_map_rule_cmd route_match_ip_next_hop_prefix_list_cmd_ripd;
extern struct cmd_element no_debug_rip_packet_direct_cmd_ripd;
extern struct cmd_element set_tag_cmd_ripd;
extern struct cmd_element no_debug_rip_packet_cmd_ripd;
extern struct cmd_element router_zebra_cmd_ripd;
extern struct cmd_element rip_default_information_originate_cmd_ripd;
extern struct cmd_element no_ip_rip_send_version_num_cmd_ripd;
extern struct cmd_element rip_default_metric_cmd_ripd;
extern struct cmd_element ip_rip_authentication_mode_authlen_cmd_ripd;
extern struct route_map_rule_cmd route_match_tag_cmd_ripd;
extern struct route_map_rule_cmd route_match_interface_cmd_ripd;
extern struct cmd_element debug_rip_events_cmd_ripd;
extern struct cmd_element no_rip_offset_list_cmd_ripd;
extern struct cmd_element no_rip_neighbor_cmd_ripd;
extern struct cmd_element match_ip_next_hop_prefix_list_cmd_ripd;
extern struct cmd_element no_set_tag_val_cmd_ripd;
extern struct cmd_element match_ip_address_prefix_list_cmd_ripd;
extern struct cmd_element no_match_ip_address_val_cmd_ripd;
extern struct cmd_element no_rip_redistribute_rip_cmd_ripd;
extern struct cmd_element no_match_metric_cmd_ripd;
extern struct cmd_element rip_neighbor_cmd_ripd;
extern struct cmd_element set_ip_nexthop_cmd_ripd;
extern struct cmd_element no_rip_default_metric_val_cmd_ripd;
extern struct cmd_element no_router_rip_cmd_ripd;
extern struct cmd_element no_match_ip_next_hop_val_cmd_ripd;
extern struct cmd_element ip_rip_receive_version_1_cmd_ripd;
extern struct route_map_rule_cmd route_set_ip_nexthop_cmd_ripd;
extern struct cmd_element no_rip_distance_cmd_ripd;
extern struct cmd_element rip_redistribute_type_cmd_ripd;
extern struct cmd_element no_ip_rip_split_horizon_cmd_ripd;
extern struct cmd_element rip_redistribute_rip_cmd_ripd;
extern struct cmd_element no_match_interface_val_cmd_ripd;
extern struct route_map_rule_cmd route_match_ip_address_prefix_list_cmd_ripd;
extern struct cmd_element no_ip_rip_authentication_key_chain_cmd_ripd;
extern struct cmd_element no_rip_version_val_cmd_ripd;
extern struct cmd_element rip_network_cmd_ripd;
extern struct cmd_element debug_rip_zebra_cmd_ripd;
extern struct cmd_element debug_rip_packet_direct_cmd_ripd;
extern struct cmd_element no_match_metric_val_cmd_ripd;
extern struct cmd_element router_rip_cmd_ripd;
extern struct cmd_element no_ip_rip_authentication_string2_cmd_ripd;
extern struct cmd_element no_rip_passive_interface_cmd_ripd;
extern struct cmd_element no_rip_version_cmd_ripd;
extern struct cmd_element no_set_metric_cmd_ripd;
extern struct cmd_element no_match_interface_cmd_ripd;
extern struct cmd_element no_debug_rip_events_cmd_ripd;
extern struct cmd_element match_interface_cmd_ripd;
extern struct cmd_element ip_rip_split_horizon_poisoned_reverse_cmd_ripd;
extern struct cmd_element ip_rip_send_version_1_cmd_ripd;
extern struct cmd_element no_match_tag_val_cmd_ripd;
extern struct cmd_element rip_offset_list_cmd_ripd;
extern struct cmd_element no_match_ip_next_hop_cmd_ripd;
extern struct cmd_element no_rip_distance_source_access_list_cmd_ripd;
extern struct cmd_element set_metric_addsub_cmd_ripd;
extern struct route_map_rule_cmd route_set_tag_cmd_ripd;
extern struct cmd_element no_match_ip_next_hop_prefix_list_val_cmd_ripd;
extern struct cmd_element match_ip_next_hop_cmd_ripd;
extern struct cmd_element no_set_tag_cmd_ripd;
extern struct cmd_element no_match_ip_next_hop_prefix_list_cmd_ripd;
extern struct cmd_element no_rip_default_information_originate_cmd_ripd;
extern struct cmd_element show_ip_rip_cmd_ripd;
extern struct cmd_element ip_rip_authentication_key_chain_cmd_ripd;
extern struct cmd_element rip_distance_cmd_ripd;
extern struct cmd_element no_match_ip_address_prefix_list_cmd_ripd;
extern struct cmd_element no_set_ip_nexthop_cmd_ripd;
extern struct cmd_element no_ip_rip_receive_version_cmd_ripd;
extern struct cmd_element rip_distance_source_cmd_ripd;
extern struct cmd_element ip_rip_receive_version_cmd_ripd;
extern struct cmd_element no_match_tag_cmd_ripd;
extern struct cmd_element rip_version_cmd_ripd;
extern struct cmd_element no_ip_rip_split_horizon_poisoned_reverse_cmd_ripd;
extern struct cmd_element no_ip_rip_authentication_mode_type_authlen_cmd_ripd;
extern struct cmd_element no_ip_rip_authentication_mode_type_cmd_ripd;
extern struct cmd_element ip_rip_authentication_mode_cmd_ripd;
extern struct cmd_element no_rip_timers_cmd_ripd;
extern struct cmd_element rip_route_cmd_ripd;
extern struct cmd_element show_debugging_rip_cmd_ripd;
extern struct cmd_element match_metric_cmd_ripd;
extern struct cmd_element rip_timers_cmd_ripd;
extern struct cmd_element no_rip_redistribute_type_metric_cmd_ripd;
extern struct cmd_element no_ip_rip_receive_version_num_cmd_ripd;
extern struct cmd_element no_set_ip_nexthop_val_cmd_ripd;
extern struct cmd_element rip_redistribute_type_routemap_cmd_ripd;
extern struct cmd_element show_ip_rip_status_cmd_ripd;
extern struct cmd_element ip_rip_split_horizon_cmd_ripd;
extern struct cmd_element debug_rip_packet_detail_cmd_ripd;

void GlobalVars_initializeActiveSet_ripd()
{
    struct cmd_node zebra_node__T = 
    {
        ZEBRA_NODE,
        "%s(config-router)# ",
    };
    struct cmd_node rip_node__T = 
    {
        RIP_NODE,
        "%s(config-router)# ",
        1
    };
    struct cmd_node debug_node__T = 
    {
        DEBUG_NODE,
        "",               
        1
    };
    struct cmd_node interface_node__T = 
    {
        INTERFACE_NODE,
        "%s(config-if)# ",
        1,
    };
    memcpy(&__activeVars_lib->route_match_metric_cmd__X, &route_match_metric_cmd_ripd, sizeof(route_match_metric_cmd_ripd));
    
    memcpy(&__activeVars_ripd->no_rip_redistribute_type_cmd__X, &no_rip_redistribute_type_cmd_ripd, sizeof(no_rip_redistribute_type_cmd_ripd));
    memcpy(&__activeVars_ripd->rip_passive_interface_cmd__X, &rip_passive_interface_cmd_ripd, sizeof(rip_passive_interface_cmd_ripd));
    memcpy(&__activeVars_ripd->no_ip_rip_send_version_cmd__X, &no_ip_rip_send_version_cmd_ripd, sizeof(no_ip_rip_send_version_cmd_ripd));
    __activeVars_lib->retain_mode__X =  0;
    memcpy(&__activeVars_ripd->rip_offset_list_ifname_cmd__X, &rip_offset_list_ifname_cmd_ripd, sizeof(rip_offset_list_ifname_cmd_ripd));
    memcpy(&__activeVars_lib->match_ip_address_cmd__X, &match_ip_address_cmd_ripd, sizeof(match_ip_address_cmd_ripd));
    memcpy(&__activeVars_ripd->no_rip_redistribute_type_routemap_cmd__X, &no_rip_redistribute_type_routemap_cmd_ripd, sizeof(no_rip_redistribute_type_routemap_cmd_ripd));
    memcpy(&__activeVars_lib->route_match_ip_next_hop_cmd__X, &route_match_ip_next_hop_cmd_ripd, sizeof(route_match_ip_next_hop_cmd_ripd));
    __activeVars_lib->pid_file__X =  PATH_RIPD_PID;
    memcpy(&__activeVars_ripd->no_rip_offset_list_ifname_cmd__X, &no_rip_offset_list_ifname_cmd_ripd, sizeof(no_rip_offset_list_ifname_cmd_ripd));
    memcpy(&__activeVars_ripd->no_debug_rip_zebra_cmd__X, &no_debug_rip_zebra_cmd_ripd, sizeof(no_debug_rip_zebra_cmd_ripd));
    memcpy(&__activeVars_ripd->no_rip_route_cmd__X, &no_rip_route_cmd_ripd, sizeof(no_rip_route_cmd_ripd));
    memcpy(&__activeVars_lib->no_set_metric_val_cmd__X, &no_set_metric_val_cmd_ripd, sizeof(no_set_metric_val_cmd_ripd));
    memcpy(&__activeVars_ripd->no_ip_rip_authentication_string_cmd__X, &no_ip_rip_authentication_string_cmd_ripd, sizeof(no_ip_rip_authentication_string_cmd_ripd));
    memcpy(&__activeVars_ripd->no_rip_network_cmd__X, &no_rip_network_cmd_ripd, sizeof(no_rip_network_cmd_ripd));
    memcpy(&__activeVars_ripd->no_ip_rip_authentication_key_chain2_cmd__X, &no_ip_rip_authentication_key_chain2_cmd_ripd, sizeof(no_ip_rip_authentication_key_chain2_cmd_ripd));
    memcpy(&__activeVars_ripd->rip_distance_source_access_list_cmd__X, &rip_distance_source_access_list_cmd_ripd, sizeof(rip_distance_source_access_list_cmd_ripd));
    memcpy(&__activeVars_ripd->ip_rip_send_version_2_cmd__X, &ip_rip_send_version_2_cmd_ripd, sizeof(ip_rip_send_version_2_cmd_ripd));
    memcpy(&__activeVars_ripd->ip_rip_send_version_cmd__X, &ip_rip_send_version_cmd_ripd, sizeof(ip_rip_send_version_cmd_ripd));
    memcpy(&__activeVars_ripd->no_rip_distance_source_cmd__X, &no_rip_distance_source_cmd_ripd, sizeof(no_rip_distance_source_cmd_ripd));
    memcpy(&__activeVars_lib->no_router_zebra_cmd__X, &no_router_zebra_cmd_ripd, sizeof(no_router_zebra_cmd_ripd));
    memcpy(&__activeVars_ripd->debug_rip_packet_cmd__X, &debug_rip_packet_cmd_ripd, sizeof(debug_rip_packet_cmd_ripd));
    memcpy(&__activeVars_lib->match_tag_cmd__X, &match_tag_cmd_ripd, sizeof(match_tag_cmd_ripd));
    __activeVars_ripd->rip_global_route_changes__X =  0;
    __activeVars_lib->vty_addr__X =  NULL;
    memcpy(&__activeVars_ripd->no_ip_rip_authentication_mode_cmd__X, &no_ip_rip_authentication_mode_cmd_ripd, sizeof(no_ip_rip_authentication_mode_cmd_ripd));
    memcpy(&__activeVars_lib->no_match_ip_address_cmd__X, &no_match_ip_address_cmd_ripd, sizeof(no_match_ip_address_cmd_ripd));
    memcpy(&__activeVars_ripd->no_rip_redistribute_type_metric_routemap_cmd__X, &no_rip_redistribute_type_metric_routemap_cmd_ripd, sizeof(no_rip_redistribute_type_metric_routemap_cmd_ripd));
    memcpy(&__activeVars_lib->no_match_ip_address_prefix_list_val_cmd__X, &no_match_ip_address_prefix_list_val_cmd_ripd, sizeof(no_match_ip_address_prefix_list_val_cmd_ripd));
    memcpy(&__activeVars_lib->zebra_node__X, &zebra_node__T, sizeof(zebra_node__T));
    memcpy(&__activeVars_ripd->no_rip_timers_val_cmd__X, &no_rip_timers_val_cmd_ripd, sizeof(no_rip_timers_val_cmd_ripd));
    memcpy(&__activeVars_ripd->ip_rip_authentication_string_cmd__X, &ip_rip_authentication_string_cmd_ripd, sizeof(ip_rip_authentication_string_cmd_ripd));
    memcpy(&__activeVars_lib->set_metric_cmd__X, &set_metric_cmd_ripd, sizeof(set_metric_cmd_ripd));
    memcpy(&__activeVars_ripd->rip_redistribute_type_metric_cmd__X, &rip_redistribute_type_metric_cmd_ripd, sizeof(rip_redistribute_type_metric_cmd_ripd));
    memcpy(&__activeVars_lib->route_match_ip_address_cmd__X, &route_match_ip_address_cmd_ripd, sizeof(route_match_ip_address_cmd_ripd));
    memcpy(&__activeVars_ripd->ip_rip_receive_version_2_cmd__X, &ip_rip_receive_version_2_cmd_ripd, sizeof(ip_rip_receive_version_2_cmd_ripd));
    memcpy(&__activeVars_ripd->rip_redistribute_type_metric_routemap_cmd__X, &rip_redistribute_type_metric_routemap_cmd_ripd, sizeof(rip_redistribute_type_metric_routemap_cmd_ripd));
    memcpy(&__activeVars_lib->route_set_metric_cmd__X, &route_set_metric_cmd_ripd, sizeof(route_set_metric_cmd_ripd));
    memcpy(&__activeVars_ripd->no_rip_default_metric_cmd__X, &no_rip_default_metric_cmd_ripd, sizeof(no_rip_default_metric_cmd_ripd));
    memcpy(&__activeVars_lib->route_match_ip_next_hop_prefix_list_cmd__X, &route_match_ip_next_hop_prefix_list_cmd_ripd, sizeof(route_match_ip_next_hop_prefix_list_cmd_ripd));
    memcpy(&__activeVars_ripd->no_debug_rip_packet_direct_cmd__X, &no_debug_rip_packet_direct_cmd_ripd, sizeof(no_debug_rip_packet_direct_cmd_ripd));
    memcpy(&__activeVars_lib->set_tag_cmd__X, &set_tag_cmd_ripd, sizeof(set_tag_cmd_ripd));
    memcpy(&__activeVars_ripd->no_debug_rip_packet_cmd__X, &no_debug_rip_packet_cmd_ripd, sizeof(no_debug_rip_packet_cmd_ripd));
    memcpy(&__activeVars_lib->router_zebra_cmd__X, &router_zebra_cmd_ripd, sizeof(router_zebra_cmd_ripd));
    memcpy(&__activeVars_ripd->rip_default_information_originate_cmd__X, &rip_default_information_originate_cmd_ripd, sizeof(rip_default_information_originate_cmd_ripd));
    memcpy(&__activeVars_ripd->no_ip_rip_send_version_num_cmd__X, &no_ip_rip_send_version_num_cmd_ripd, sizeof(no_ip_rip_send_version_num_cmd_ripd));
    memcpy(&__activeVars_ripd->rip_default_metric_cmd__X, &rip_default_metric_cmd_ripd, sizeof(rip_default_metric_cmd_ripd));
    memcpy(&__activeVars_ripd->ip_rip_authentication_mode_authlen_cmd__X, &ip_rip_authentication_mode_authlen_cmd_ripd, sizeof(ip_rip_authentication_mode_authlen_cmd_ripd));
    __activeVars_ripd->rip__X =  NULL;
    memcpy(&__activeVars_lib->route_match_tag_cmd__X, &route_match_tag_cmd_ripd, sizeof(route_match_tag_cmd_ripd));
    memcpy(&__activeVars_lib->route_match_interface_cmd__X, &route_match_interface_cmd_ripd, sizeof(route_match_interface_cmd_ripd));
    __activeVars_ripd->rip_global_queries__X =  0;
    memcpy(&__activeVars_ripd->debug_rip_events_cmd__X, &debug_rip_events_cmd_ripd, sizeof(debug_rip_events_cmd_ripd));
    memcpy(&__activeVars_ripd->no_rip_offset_list_cmd__X, &no_rip_offset_list_cmd_ripd, sizeof(no_rip_offset_list_cmd_ripd));
    memcpy(&__activeVars_ripd->no_rip_neighbor_cmd__X, &no_rip_neighbor_cmd_ripd, sizeof(no_rip_neighbor_cmd_ripd));
    memcpy(&__activeVars_lib->match_ip_next_hop_prefix_list_cmd__X, &match_ip_next_hop_prefix_list_cmd_ripd, sizeof(match_ip_next_hop_prefix_list_cmd_ripd));
    memcpy(&__activeVars_lib->no_set_tag_val_cmd__X, &no_set_tag_val_cmd_ripd, sizeof(no_set_tag_val_cmd_ripd));
    memcpy(&__activeVars_lib->match_ip_address_prefix_list_cmd__X, &match_ip_address_prefix_list_cmd_ripd, sizeof(match_ip_address_prefix_list_cmd_ripd));
    memcpy(&__activeVars_lib->no_match_ip_address_val_cmd__X, &no_match_ip_address_val_cmd_ripd, sizeof(no_match_ip_address_val_cmd_ripd));
    __activeVars_lib->config_file__X =  NULL;
    memcpy(&__activeVars_ripd->no_rip_redistribute_rip_cmd__X, &no_rip_redistribute_rip_cmd_ripd, sizeof(no_rip_redistribute_rip_cmd_ripd));
    memcpy(&__activeVars_lib->no_match_metric_cmd__X, &no_match_metric_cmd_ripd, sizeof(no_match_metric_cmd_ripd));
    memcpy(&__activeVars_ripd->rip_neighbor_cmd__X, &rip_neighbor_cmd_ripd, sizeof(rip_neighbor_cmd_ripd));
    memcpy(&__activeVars_lib->set_ip_nexthop_cmd__X, &set_ip_nexthop_cmd_ripd, sizeof(set_ip_nexthop_cmd_ripd));
    memcpy(&__activeVars_ripd->no_rip_default_metric_val_cmd__X, &no_rip_default_metric_val_cmd_ripd, sizeof(no_rip_default_metric_val_cmd_ripd));
    memcpy(&__activeVars_ripd->no_router_rip_cmd__X, &no_router_rip_cmd_ripd, sizeof(no_router_rip_cmd_ripd));
    memcpy(&__activeVars_lib->no_match_ip_next_hop_val_cmd__X, &no_match_ip_next_hop_val_cmd_ripd, sizeof(no_match_ip_next_hop_val_cmd_ripd));
    memcpy(&__activeVars_ripd->ip_rip_receive_version_1_cmd__X, &ip_rip_receive_version_1_cmd_ripd, sizeof(ip_rip_receive_version_1_cmd_ripd));
    memcpy(&__activeVars_lib->route_set_ip_nexthop_cmd__X, &route_set_ip_nexthop_cmd_ripd, sizeof(route_set_ip_nexthop_cmd_ripd));
    memcpy(&__activeVars_ripd->no_rip_distance_cmd__X, &no_rip_distance_cmd_ripd, sizeof(no_rip_distance_cmd_ripd));
    __activeVars_ripd->rip_debug_packet__X =  0;
    memcpy(&__activeVars_ripd->rip_redistribute_type_cmd__X, &rip_redistribute_type_cmd_ripd, sizeof(rip_redistribute_type_cmd_ripd));
    memcpy(&__activeVars_ripd->no_ip_rip_split_horizon_cmd__X, &no_ip_rip_split_horizon_cmd_ripd, sizeof(no_ip_rip_split_horizon_cmd_ripd));
    memcpy(&__activeVars_ripd->rip_redistribute_rip_cmd__X, &rip_redistribute_rip_cmd_ripd, sizeof(rip_redistribute_rip_cmd_ripd));
    memcpy(&__activeVars_lib->no_match_interface_val_cmd__X, &no_match_interface_val_cmd_ripd, sizeof(no_match_interface_val_cmd_ripd));
    memcpy(&__activeVars_ripd->rip_node__X, &rip_node__T, sizeof(rip_node__T));
    memcpy(&__activeVars_lib->route_match_ip_address_prefix_list_cmd__X, &route_match_ip_address_prefix_list_cmd_ripd, sizeof(route_match_ip_address_prefix_list_cmd_ripd));
    memcpy(&__activeVars_ripd->no_ip_rip_authentication_key_chain_cmd__X, &no_ip_rip_authentication_key_chain_cmd_ripd, sizeof(no_ip_rip_authentication_key_chain_cmd_ripd));
    memcpy(&__activeVars_ripd->no_rip_version_val_cmd__X, &no_rip_version_val_cmd_ripd, sizeof(no_rip_version_val_cmd_ripd));
    memcpy(&__activeVars_ripd->rip_network_cmd__X, &rip_network_cmd_ripd, sizeof(rip_network_cmd_ripd));
    memcpy(&__activeVars_ripd->debug_rip_zebra_cmd__X, &debug_rip_zebra_cmd_ripd, sizeof(debug_rip_zebra_cmd_ripd));
    memcpy(&__activeVars_ripd->debug_rip_packet_direct_cmd__X, &debug_rip_packet_direct_cmd_ripd, sizeof(debug_rip_packet_direct_cmd_ripd));
    memcpy(&__activeVars_lib->no_match_metric_val_cmd__X, &no_match_metric_val_cmd_ripd, sizeof(no_match_metric_val_cmd_ripd));
    memcpy(&__activeVars_ripd->router_rip_cmd__X, &router_rip_cmd_ripd, sizeof(router_rip_cmd_ripd));
    memcpy(&__activeVars_ripd->no_ip_rip_authentication_string2_cmd__X, &no_ip_rip_authentication_string2_cmd_ripd, sizeof(no_ip_rip_authentication_string2_cmd_ripd));
    memcpy(&__activeVars_ripd->no_rip_passive_interface_cmd__X, &no_rip_passive_interface_cmd_ripd, sizeof(no_rip_passive_interface_cmd_ripd));
    memcpy(&__activeVars_ripd->no_rip_version_cmd__X, &no_rip_version_cmd_ripd, sizeof(no_rip_version_cmd_ripd));
    memcpy(&__activeVars_lib->no_set_metric_cmd__X, &no_set_metric_cmd_ripd, sizeof(no_set_metric_cmd_ripd));
    memcpy(&__activeVars_lib->no_match_interface_cmd__X, &no_match_interface_cmd_ripd, sizeof(no_match_interface_cmd_ripd));
    memcpy(&__activeVars_ripd->no_debug_rip_events_cmd__X, &no_debug_rip_events_cmd_ripd, sizeof(no_debug_rip_events_cmd_ripd));
    memcpy(&__activeVars_lib->match_interface_cmd__X, &match_interface_cmd_ripd, sizeof(match_interface_cmd_ripd));
    memcpy(&__activeVars_ripd->ip_rip_split_horizon_poisoned_reverse_cmd__X, &ip_rip_split_horizon_poisoned_reverse_cmd_ripd, sizeof(ip_rip_split_horizon_poisoned_reverse_cmd_ripd));
    __activeVars_ripd->rip_debug_zebra__X =  0;
    __activeVars_lib->vty_port__X =  RIP_VTY_PORT;
    __activeVars_lib->zclient__X =  NULL;
    memcpy(&__activeVars_ripd->ip_rip_send_version_1_cmd__X, &ip_rip_send_version_1_cmd_ripd, sizeof(ip_rip_send_version_1_cmd_ripd));
    memcpy(&__activeVars_lib->debug_node__X, &debug_node__T, sizeof(debug_node__T));
    memcpy(&__activeVars_lib->no_match_tag_val_cmd__X, &no_match_tag_val_cmd_ripd, sizeof(no_match_tag_val_cmd_ripd));
    memcpy(&__activeVars_ripd->rip_offset_list_cmd__X, &rip_offset_list_cmd_ripd, sizeof(rip_offset_list_cmd_ripd));
    memcpy(&__activeVars_lib->no_match_ip_next_hop_cmd__X, &no_match_ip_next_hop_cmd_ripd, sizeof(no_match_ip_next_hop_cmd_ripd));
    memcpy(&__activeVars_ripd->no_rip_distance_source_access_list_cmd__X, &no_rip_distance_source_access_list_cmd_ripd, sizeof(no_rip_distance_source_access_list_cmd_ripd));
    memcpy(&__activeVars_lib->set_metric_addsub_cmd__X, &set_metric_addsub_cmd_ripd, sizeof(set_metric_addsub_cmd_ripd));
    memcpy(&__activeVars_lib->route_set_tag_cmd__X, &route_set_tag_cmd_ripd, sizeof(route_set_tag_cmd_ripd));
    memcpy(&__activeVars_lib->no_match_ip_next_hop_prefix_list_val_cmd__X, &no_match_ip_next_hop_prefix_list_val_cmd_ripd, sizeof(no_match_ip_next_hop_prefix_list_val_cmd_ripd));
    memcpy(&__activeVars_lib->match_ip_next_hop_cmd__X, &match_ip_next_hop_cmd_ripd, sizeof(match_ip_next_hop_cmd_ripd));
    memcpy(&__activeVars_lib->no_set_tag_cmd__X, &no_set_tag_cmd_ripd, sizeof(no_set_tag_cmd_ripd));
    memcpy(&__activeVars_lib->no_match_ip_next_hop_prefix_list_cmd__X, &no_match_ip_next_hop_prefix_list_cmd_ripd, sizeof(no_match_ip_next_hop_prefix_list_cmd_ripd));
    memcpy(&__activeVars_ripd->no_rip_default_information_originate_cmd__X, &no_rip_default_information_originate_cmd_ripd, sizeof(no_rip_default_information_originate_cmd_ripd));
    memcpy(&__activeVars_ripd->show_ip_rip_cmd__X, &show_ip_rip_cmd_ripd, sizeof(show_ip_rip_cmd_ripd));
    memcpy(&__activeVars_ripd->ip_rip_authentication_key_chain_cmd__X, &ip_rip_authentication_key_chain_cmd_ripd, sizeof(ip_rip_authentication_key_chain_cmd_ripd));
    memcpy(&__activeVars_ripd->rip_distance_cmd__X, &rip_distance_cmd_ripd, sizeof(rip_distance_cmd_ripd));
    memcpy(&__activeVars_lib->no_match_ip_address_prefix_list_cmd__X, &no_match_ip_address_prefix_list_cmd_ripd, sizeof(no_match_ip_address_prefix_list_cmd_ripd));
    memcpy(&__activeVars_lib->no_set_ip_nexthop_cmd__X, &no_set_ip_nexthop_cmd_ripd, sizeof(no_set_ip_nexthop_cmd_ripd));
    memcpy(&__activeVars_ripd->no_ip_rip_receive_version_cmd__X, &no_ip_rip_receive_version_cmd_ripd, sizeof(no_ip_rip_receive_version_cmd_ripd));
    memcpy(&__activeVars_ripd->rip_distance_source_cmd__X, &rip_distance_source_cmd_ripd, sizeof(rip_distance_source_cmd_ripd));
    memcpy(&__activeVars_ripd->ip_rip_receive_version_cmd__X, &ip_rip_receive_version_cmd_ripd, sizeof(ip_rip_receive_version_cmd_ripd));
    memcpy(&__activeVars_lib->no_match_tag_cmd__X, &no_match_tag_cmd_ripd, sizeof(no_match_tag_cmd_ripd));
    memcpy(&__activeVars_ripd->rip_version_cmd__X, &rip_version_cmd_ripd, sizeof(rip_version_cmd_ripd));
    memcpy(&__activeVars_lib->interface_node__X, &interface_node__T, sizeof(interface_node__T));
    memcpy(&__activeVars_ripd->no_ip_rip_split_horizon_poisoned_reverse_cmd__X, &no_ip_rip_split_horizon_poisoned_reverse_cmd_ripd, sizeof(no_ip_rip_split_horizon_poisoned_reverse_cmd_ripd));
    memcpy(&__activeVars_ripd->no_ip_rip_authentication_mode_type_authlen_cmd__X, &no_ip_rip_authentication_mode_type_authlen_cmd_ripd, sizeof(no_ip_rip_authentication_mode_type_authlen_cmd_ripd));
    memcpy(&__activeVars_ripd->no_ip_rip_authentication_mode_type_cmd__X, &no_ip_rip_authentication_mode_type_cmd_ripd, sizeof(no_ip_rip_authentication_mode_type_cmd_ripd));
    memcpy(&__activeVars_ripd->ip_rip_authentication_mode_cmd__X, &ip_rip_authentication_mode_cmd_ripd, sizeof(ip_rip_authentication_mode_cmd_ripd));
    memcpy(&__activeVars_ripd->no_rip_timers_cmd__X, &no_rip_timers_cmd_ripd, sizeof(no_rip_timers_cmd_ripd));
    memcpy(&__activeVars_ripd->rip_route_cmd__X, &rip_route_cmd_ripd, sizeof(rip_route_cmd_ripd));
    memcpy(&__activeVars_ripd->show_debugging_rip_cmd__X, &show_debugging_rip_cmd_ripd, sizeof(show_debugging_rip_cmd_ripd));
    memcpy(&__activeVars_lib->match_metric_cmd__X, &match_metric_cmd_ripd, sizeof(match_metric_cmd_ripd));
    memcpy(&__activeVars_ripd->rip_timers_cmd__X, &rip_timers_cmd_ripd, sizeof(rip_timers_cmd_ripd));
    memcpy(&__activeVars_ripd->no_rip_redistribute_type_metric_cmd__X, &no_rip_redistribute_type_metric_cmd_ripd, sizeof(no_rip_redistribute_type_metric_cmd_ripd));
    memcpy(&__activeVars_ripd->no_ip_rip_receive_version_num_cmd__X, &no_ip_rip_receive_version_num_cmd_ripd, sizeof(no_ip_rip_receive_version_num_cmd_ripd));
    memcpy(&__activeVars_lib->no_set_ip_nexthop_val_cmd__X, &no_set_ip_nexthop_val_cmd_ripd, sizeof(no_set_ip_nexthop_val_cmd_ripd));
    memcpy(&__activeVars_ripd->rip_redistribute_type_routemap_cmd__X, &rip_redistribute_type_routemap_cmd_ripd, sizeof(rip_redistribute_type_routemap_cmd_ripd));
    memcpy(&__activeVars_ripd->show_ip_rip_status_cmd__X, &show_ip_rip_status_cmd_ripd, sizeof(show_ip_rip_status_cmd_ripd));
    memcpy(&__activeVars_ripd->ip_rip_split_horizon_cmd__X, &ip_rip_split_horizon_cmd_ripd, sizeof(ip_rip_split_horizon_cmd_ripd));
    __activeVars_ripd->rip_debug_event__X =  0;
    memcpy(&__activeVars_ripd->debug_rip_packet_detail_cmd__X, &debug_rip_packet_detail_cmd_ripd, sizeof(debug_rip_packet_detail_cmd_ripd));
}

