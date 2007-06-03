
#include "globalvars_zebra.h"

struct Globalvars_zebra * activeVars_zebra;

struct GlobalVars_zebra * GlobalVars_createActiveSet_zebra()
{
    struct GlobalVars_zebra * ptr = (struct GlobalVars_zebra *)malloc(sizeof(struct GlobalVars_zebra));
    memset(ptr, 0, sizeof(struct GlobalVars_zebra));
    return ptr;
}

extern struct cmd_element no_ip_route_mask_distance_cmd_zebra;
extern struct cmd_element no_bandwidth_if_cmd_zebra;
extern struct cmd_element router_id_cmd_zebra;
extern struct cmd_element ip_route_flags_distance2_cmd_zebra;
extern struct cmd_element no_ip_route_mask_flags2_cmd_zebra;
extern struct cmd_element ip_route_mask_cmd_zebra;
extern struct cmd_element no_debug_zebra_packet_direct_cmd_zebra;
extern struct cmd_element ip_route_mask_flags_distance_cmd_zebra;
extern struct cmd_element no_ip_route_mask_flags_distance2_cmd_zebra;
extern struct cmd_element show_zebra_client_cmd_zebra;
extern struct cmd_element show_ip_route_supernets_cmd_zebra;
extern struct cmd_element no_multicast_cmd_zebra;
extern struct cmd_element no_ip_forwarding_cmd_zebra;
extern struct cmd_element no_ip_route_distance_cmd_zebra;
extern struct cmd_element no_debug_zebra_packet_cmd_zebra;
extern struct cmd_element bandwidth_if_cmd_zebra;
extern struct cmd_element no_ip_route_flags_cmd_zebra;
extern struct cmd_element ip_address_cmd_zebra;
extern struct cmd_element show_ip_route_cmd_zebra;
extern struct cmd_element debug_zebra_packet_detail_cmd_zebra;
extern struct cmd_element multicast_cmd_zebra;
extern struct cmd_element show_ip_route_prefix_cmd_zebra;
extern struct cmd_element no_ip_address_label_cmd_zebra;
extern struct cmd_element zebra_interface_cmd_zebra;
extern struct cmd_element ip_forwarding_cmd_zebra;
extern struct cmd_element ip_route_flags_distance_cmd_zebra;
extern struct cmd_element show_ip_route_prefix_longer_cmd_zebra;
extern struct cmd_element no_ip_route_mask_flags_cmd_zebra;
extern struct cmd_element no_debug_zebra_kernel_cmd_zebra;
extern struct cmd_element no_ip_route_mask_flags_distance_cmd_zebra;
extern struct cmd_element no_ip_address_cmd_zebra;
extern struct cmd_element no_shutdown_if_cmd_zebra;
extern struct cmd_element debug_zebra_kernel_cmd_zebra;
extern struct cmd_element no_ip_route_cmd_zebra;
extern struct cmd_element show_debugging_zebra_cmd_zebra;
extern struct cmd_element shutdown_if_cmd_zebra;
extern struct cmd_element no_bandwidth_if_val_cmd_zebra;
extern struct cmd_element ip_route_flags_cmd_zebra;
extern struct cmd_element show_ip_route_summary_cmd_zebra;
extern struct cmd_element show_table_cmd_zebra;
extern struct cmd_element ip_route_cmd_zebra;
extern struct cmd_element show_ip_forwarding_cmd_zebra;
extern struct cmd_element debug_zebra_packet_direct_cmd_zebra;
extern struct cmd_element show_ip_route_protocol_cmd_zebra;
extern struct cmd_element config_table_cmd_zebra;
extern struct cmd_element no_linkdetect_cmd_zebra;
extern struct cmd_element no_ip_route_mask_cmd_zebra;
extern struct cmd_element show_interface_cmd_zebra;
extern struct cmd_element show_ip_route_addr_cmd_zebra;
extern struct cmd_element no_ip_route_flags_distance_cmd_zebra;
extern struct cmd_element no_router_id_cmd_zebra;
extern struct cmd_element debug_zebra_events_cmd_zebra;
extern struct cmd_element ip_route_mask_flags2_cmd_zebra;
extern struct cmd_element no_debug_zebra_events_cmd_zebra;
extern struct cmd_element ip_route_mask_distance_cmd_zebra;
extern struct cmd_element ip_route_flags2_cmd_zebra;
extern struct cmd_element ip_route_mask_flags_cmd_zebra;
extern struct cmd_element linkdetect_cmd_zebra;
extern struct cmd_element no_ip_route_flags2_cmd_zebra;
extern struct cmd_element ip_route_mask_flags_distance2_cmd_zebra;
extern struct cmd_element debug_zebra_packet_cmd_zebra;
extern struct cmd_element ip_address_label_cmd_zebra;
extern struct cmd_element no_ip_route_flags_distance2_cmd_zebra;
extern struct cmd_element ip_route_distance_cmd_zebra;

void GlobalVars_initializeActiveSet_zebra()
{
    struct cmd_node table_node__T = 
    {
        TABLE_NODE,
        "",               
        1
    };
    struct cmd_node forwarding_node__T = 
    {
        FORWARDING_NODE,
        "",               
        1
    };
    struct nlsock netlink_addr__T =  { -1, 0, {0}, "netlink-addr"};
    struct cmd_node debug_node__T = 
    {
        DEBUG_NODE,
        "",               
        1
    };
    struct zebra_t zebrad__T = 
    {
        NULL,
        NULL,
        0
    };
    struct nlsock netlink_cmd__T =  { -1, 0, {0}, "netlink-cmd"};
    struct cmd_node ip_node__T =  { IP_NODE,  "",  1 };
    struct cmd_node interface_node__T = 
    {
        INTERFACE_NODE,
        "%s(config-if)# ",
        1
    };
    struct nlsock netlink__T =  { -1, 0, {0}, "n""etlink-listen"};
    __activeVars_lib->retain_mode__X =  0;
    memcpy(&__activeVars_lib->no_ip_route_mask_distance_cmd__X, &no_ip_route_mask_distance_cmd_zebra, sizeof(no_ip_route_mask_distance_cmd_zebra));
    memcpy(&__activeVars_lib->no_bandwidth_if_cmd__X, &no_bandwidth_if_cmd_zebra, sizeof(no_bandwidth_if_cmd_zebra));
    memcpy(&__activeVars_lib->router_id_cmd__X, &router_id_cmd_zebra, sizeof(router_id_cmd_zebra));
    memcpy(&__activeVars_lib->ip_route_flags_distance2_cmd__X, &ip_route_flags_distance2_cmd_zebra, sizeof(ip_route_flags_distance2_cmd_zebra));
    __activeVars_lib->pid_file__X =  PATH_ZEBRA_PID;
    memcpy(&__activeVars_lib->table_node__X, &table_node__T, sizeof(table_node__T));
    memcpy(&__activeVars_lib->no_ip_route_mask_flags2_cmd__X, &no_ip_route_mask_flags2_cmd_zebra, sizeof(no_ip_route_mask_flags2_cmd_zebra));
    memcpy(&__activeVars_lib->ip_route_mask_cmd__X, &ip_route_mask_cmd_zebra, sizeof(ip_route_mask_cmd_zebra));
    memcpy(&__activeVars_zebra->no_debug_zebra_packet_direct_cmd__X, &no_debug_zebra_packet_direct_cmd_zebra, sizeof(no_debug_zebra_packet_direct_cmd_zebra));
    memcpy(&__activeVars_lib->ip_route_mask_flags_distance_cmd__X, &ip_route_mask_flags_distance_cmd_zebra, sizeof(ip_route_mask_flags_distance_cmd_zebra));
    memcpy(&__activeVars_lib->no_ip_route_mask_flags_distance2_cmd__X, &no_ip_route_mask_flags_distance2_cmd_zebra, sizeof(no_ip_route_mask_flags_distance2_cmd_zebra));
    memcpy(&__activeVars_zebra->show_zebra_client_cmd__X, &show_zebra_client_cmd_zebra, sizeof(show_zebra_client_cmd_zebra));
    memcpy(&__activeVars_lib->show_ip_route_supernets_cmd__X, &show_ip_route_supernets_cmd_zebra, sizeof(show_ip_route_supernets_cmd_zebra));
    memcpy(&__activeVars_lib->no_multicast_cmd__X, &no_multicast_cmd_zebra, sizeof(no_multicast_cmd_zebra));
    memcpy(&__activeVars_lib->no_ip_forwarding_cmd__X, &no_ip_forwarding_cmd_zebra, sizeof(no_ip_forwarding_cmd_zebra));
    memcpy(&__activeVars_lib->no_ip_route_distance_cmd__X, &no_ip_route_distance_cmd_zebra, sizeof(no_ip_route_distance_cmd_zebra));
    memcpy(&__activeVars_zebra->no_debug_zebra_packet_cmd__X, &no_debug_zebra_packet_cmd_zebra, sizeof(no_debug_zebra_packet_cmd_zebra));
    memcpy(&__activeVars_lib->bandwidth_if_cmd__X, &bandwidth_if_cmd_zebra, sizeof(bandwidth_if_cmd_zebra));
    memcpy(&__activeVars_lib->no_ip_route_flags_cmd__X, &no_ip_route_flags_cmd_zebra, sizeof(no_ip_route_flags_cmd_zebra));
    memcpy(&__activeVars_lib->ip_address_cmd__X, &ip_address_cmd_zebra, sizeof(ip_address_cmd_zebra));
    memcpy(&__activeVars_lib->show_ip_route_cmd__X, &show_ip_route_cmd_zebra, sizeof(show_ip_route_cmd_zebra));
    memcpy(&__activeVars_zebra->debug_zebra_packet_detail_cmd__X, &debug_zebra_packet_detail_cmd_zebra, sizeof(debug_zebra_packet_detail_cmd_zebra));
    memcpy(&__activeVars_lib->multicast_cmd__X, &multicast_cmd_zebra, sizeof(multicast_cmd_zebra));
    memcpy(&__activeVars_lib->show_ip_route_prefix_cmd__X, &show_ip_route_prefix_cmd_zebra, sizeof(show_ip_route_prefix_cmd_zebra));
    memcpy(&__activeVars_lib->no_ip_address_label_cmd__X, &no_ip_address_label_cmd_zebra, sizeof(no_ip_address_label_cmd_zebra));
    memcpy(&__activeVars_lib->forwarding_node__X, &forwarding_node__T, sizeof(forwarding_node__T));
    memcpy(&__activeVars_zebra->zebra_interface_cmd__X, &zebra_interface_cmd_zebra, sizeof(zebra_interface_cmd_zebra));
    memcpy(&__activeVars_lib->ip_forwarding_cmd__X, &ip_forwarding_cmd_zebra, sizeof(ip_forwarding_cmd_zebra));
    memcpy(&__activeVars_lib->ip_route_flags_distance_cmd__X, &ip_route_flags_distance_cmd_zebra, sizeof(ip_route_flags_distance_cmd_zebra));
    memcpy(&__activeVars_lib->show_ip_route_prefix_longer_cmd__X, &show_ip_route_prefix_longer_cmd_zebra, sizeof(show_ip_route_prefix_longer_cmd_zebra));
    memcpy(&__activeVars_lib->no_ip_route_mask_flags_cmd__X, &no_ip_route_mask_flags_cmd_zebra, sizeof(no_ip_route_mask_flags_cmd_zebra));
    memcpy(&__activeVars_lib->netlink_addr__X, &netlink_addr__T, sizeof(netlink_addr__T));
    memcpy(&__activeVars_lib->debug_node__X, &debug_node__T, sizeof(debug_node__T));
    memcpy(&__activeVars_zebra->no_debug_zebra_kernel_cmd__X, &no_debug_zebra_kernel_cmd_zebra, sizeof(no_debug_zebra_kernel_cmd_zebra));
    memcpy(&__activeVars_lib->no_ip_route_mask_flags_distance_cmd__X, &no_ip_route_mask_flags_distance_cmd_zebra, sizeof(no_ip_route_mask_flags_distance_cmd_zebra));
    memcpy(&__activeVars_lib->no_ip_address_cmd__X, &no_ip_address_cmd_zebra, sizeof(no_ip_address_cmd_zebra));
    memcpy(&__activeVars_lib->no_shutdown_if_cmd__X, &no_shutdown_if_cmd_zebra, sizeof(no_shutdown_if_cmd_zebra));
    memcpy(&__activeVars_zebra->zebrad__X, &zebrad__T, sizeof(zebrad__T));
    memcpy(&__activeVars_zebra->debug_zebra_kernel_cmd__X, &debug_zebra_kernel_cmd_zebra, sizeof(debug_zebra_kernel_cmd_zebra));
    memcpy(&__activeVars_lib->no_ip_route_cmd__X, &no_ip_route_cmd_zebra, sizeof(no_ip_route_cmd_zebra));
    memcpy(&__activeVars_lib->netlink_cmd__X, &netlink_cmd__T, sizeof(netlink_cmd__T));
    memcpy(&__activeVars_zebra->show_debugging_zebra_cmd__X, &show_debugging_zebra_cmd_zebra, sizeof(show_debugging_zebra_cmd_zebra));
    memcpy(&__activeVars_lib->shutdown_if_cmd__X, &shutdown_if_cmd_zebra, sizeof(shutdown_if_cmd_zebra));
    memcpy(&__activeVars_lib->no_bandwidth_if_val_cmd__X, &no_bandwidth_if_val_cmd_zebra, sizeof(no_bandwidth_if_val_cmd_zebra));
    memcpy(&__activeVars_lib->ip_node__X, &ip_node__T, sizeof(ip_node__T));
    memcpy(&__activeVars_lib->ip_route_flags_cmd__X, &ip_route_flags_cmd_zebra, sizeof(ip_route_flags_cmd_zebra));
    memcpy(&__activeVars_lib->show_ip_route_summary_cmd__X, &show_ip_route_summary_cmd_zebra, sizeof(show_ip_route_summary_cmd_zebra));
    memcpy(&__activeVars_lib->show_table_cmd__X, &show_table_cmd_zebra, sizeof(show_table_cmd_zebra));
    memcpy(&__activeVars_lib->ip_route_cmd__X, &ip_route_cmd_zebra, sizeof(ip_route_cmd_zebra));
    memcpy(&__activeVars_lib->interface_node__X, &interface_node__T, sizeof(interface_node__T));
    memcpy(&__activeVars_lib->show_ip_forwarding_cmd__X, &show_ip_forwarding_cmd_zebra, sizeof(show_ip_forwarding_cmd_zebra));
    memcpy(&__activeVars_zebra->debug_zebra_packet_direct_cmd__X, &debug_zebra_packet_direct_cmd_zebra, sizeof(debug_zebra_packet_direct_cmd_zebra));
    memcpy(&__activeVars_lib->show_ip_route_protocol_cmd__X, &show_ip_route_protocol_cmd_zebra, sizeof(show_ip_route_protocol_cmd_zebra));
    memcpy(&__activeVars_lib->config_table_cmd__X, &config_table_cmd_zebra, sizeof(config_table_cmd_zebra));
    memcpy(&__activeVars_lib->no_linkdetect_cmd__X, &no_linkdetect_cmd_zebra, sizeof(no_linkdetect_cmd_zebra));
    __activeVars_lib->nl_rcvbufsize__X =  0;
    memcpy(&__activeVars_lib->netlink__X, &netlink__T, sizeof(netlink__T));
    memcpy(&__activeVars_lib->no_ip_route_mask_cmd__X, &no_ip_route_mask_cmd_zebra, sizeof(no_ip_route_mask_cmd_zebra));
    memcpy(&__activeVars_lib->show_interface_cmd__X, &show_interface_cmd_zebra, sizeof(show_interface_cmd_zebra));
    memcpy(&__activeVars_lib->show_ip_route_addr_cmd__X, &show_ip_route_addr_cmd_zebra, sizeof(show_ip_route_addr_cmd_zebra));
    memcpy(&__activeVars_lib->no_ip_route_flags_distance_cmd__X, &no_ip_route_flags_distance_cmd_zebra, sizeof(no_ip_route_flags_distance_cmd_zebra));
    memcpy(&__activeVars_lib->no_router_id_cmd__X, &no_router_id_cmd_zebra, sizeof(no_router_id_cmd_zebra));
    memcpy(&__activeVars_zebra->debug_zebra_events_cmd__X, &debug_zebra_events_cmd_zebra, sizeof(debug_zebra_events_cmd_zebra));
    memcpy(&__activeVars_lib->ip_route_mask_flags2_cmd__X, &ip_route_mask_flags2_cmd_zebra, sizeof(ip_route_mask_flags2_cmd_zebra));
    memcpy(&__activeVars_zebra->no_debug_zebra_events_cmd__X, &no_debug_zebra_events_cmd_zebra, sizeof(no_debug_zebra_events_cmd_zebra));
    memcpy(&__activeVars_lib->ip_route_mask_distance_cmd__X, &ip_route_mask_distance_cmd_zebra, sizeof(ip_route_mask_distance_cmd_zebra));
    memcpy(&__activeVars_lib->ip_route_flags2_cmd__X, &ip_route_flags2_cmd_zebra, sizeof(ip_route_flags2_cmd_zebra));
    memcpy(&__activeVars_lib->ip_route_mask_flags_cmd__X, &ip_route_mask_flags_cmd_zebra, sizeof(ip_route_mask_flags_cmd_zebra));
    memcpy(&__activeVars_lib->linkdetect_cmd__X, &linkdetect_cmd_zebra, sizeof(linkdetect_cmd_zebra));
    memcpy(&__activeVars_lib->no_ip_route_flags2_cmd__X, &no_ip_route_flags2_cmd_zebra, sizeof(no_ip_route_flags2_cmd_zebra));
    memcpy(&__activeVars_lib->ip_route_mask_flags_distance2_cmd__X, &ip_route_mask_flags_distance2_cmd_zebra, sizeof(ip_route_mask_flags_distance2_cmd_zebra));
    memcpy(&__activeVars_zebra->debug_zebra_packet_cmd__X, &debug_zebra_packet_cmd_zebra, sizeof(debug_zebra_packet_cmd_zebra));
    __activeVars_lib->keep_kernel_mode__X =  0;
    memcpy(&__activeVars_lib->ip_address_label_cmd__X, &ip_address_label_cmd_zebra, sizeof(ip_address_label_cmd_zebra));
    memcpy(&__activeVars_lib->no_ip_route_flags_distance2_cmd__X, &no_ip_route_flags_distance2_cmd_zebra, sizeof(no_ip_route_flags_distance2_cmd_zebra));
    memcpy(&__activeVars_lib->ip_route_distance_cmd__X, &ip_route_distance_cmd_zebra, sizeof(ip_route_distance_cmd_zebra));
}

