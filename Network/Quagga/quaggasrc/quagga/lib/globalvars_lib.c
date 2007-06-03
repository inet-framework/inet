
#include "globalvars_lib.h"

struct Globalvars_lib * activeVars_lib;

int* GlobalVars_errno()
{
    return &__activeVars_lib->errno__X;
}

int* GlobalVars_optind()
{
    return &__activeVars_lib->optind__X;
}

char** GlobalVars_optarg()
{
    return &__activeVars_lib->optarg__X;
}

struct GlobalVars_lib * GlobalVars_createActiveSet_lib()
{
    struct GlobalVars_lib * ptr = (struct GlobalVars_lib *)malloc(sizeof(struct GlobalVars_lib));
    memset(ptr, 0, sizeof(struct GlobalVars_lib));
    return ptr;
}

extern struct cmd_element rmap_show_name_cmd_lib;
extern struct cmd_element no_ip_prefix_list_seq_le_ge_cmd_lib;
extern struct cmd_element rmap_continue_seq_cmd_lib;
extern struct cmd_element interface_desc_cmd_lib;
extern struct cmd_element key_chain_cmd_lib;
extern struct cmd_element no_rmap_onmatch_next_cmd_lib;
extern struct cmd_element show_history_cmd_lib;
extern struct cmd_element show_ip_prefix_list_summary_name_cmd_lib;
extern struct cmd_element enable_password_cmd_lib;
extern struct cmd_element config_terminal_length_cmd_lib;
extern struct cmd_element ip_prefix_list_ge_cmd_lib;
extern struct cmd_element no_config_log_trap_cmd_lib;
extern struct cmd_element ipv6_distribute_list_prefix_all_cmd_lib;
extern struct cmd_element service_terminal_length_cmd_lib;
extern struct cmd_element accept_lifetime_day_month_day_month_cmd_lib;
extern struct cmd_element config_help_cmd_lib;
extern struct cmd_element send_lifetime_day_month_day_month_cmd_lib;
extern struct cmd_element terminal_no_monitor_cmd_lib;
extern struct cmd_element vty_login_cmd_lib;
extern struct cmd_element route_map_cmd_lib;
extern struct cmd_element send_lifetime_day_month_month_day_cmd_lib;
extern struct cmd_element no_access_list_standard_nomask_cmd_lib;
extern struct cmd_element distribute_list_cmd_lib;
extern struct cmd_element config_log_stdout_cmd_lib;
extern struct cmd_element service_password_encrypt_cmd_lib;
extern struct cmd_element clear_ip_prefix_list_cmd_lib;
extern struct cmd_element no_access_list_extended_mask_host_cmd_lib;
extern struct cmd_element no_if_ipv6_rmap_cmd_lib;
extern struct cmd_element access_list_standard_host_cmd_lib;
extern struct cmd_element no_service_password_encrypt_cmd_lib;
extern struct cmd_element show_ip_prefix_list_detail_cmd_lib;
extern struct cmd_element no_service_advanced_vty_cmd_lib;
extern struct cmd_element line_vty_cmd_lib;
extern struct cmd_element no_ip_prefix_list_seq_le_cmd_lib;
extern struct cmd_element no_hostname_cmd_lib;
extern struct cmd_element no_interface_desc_cmd_lib;
extern struct cmd_element ip_prefix_list_seq_ge_cmd_lib;
extern struct cmd_element no_enable_password_cmd_lib;
extern struct cmd_element key_cmd_lib;
extern struct cmd_element copy_runningconfig_startupconfig_cmd_lib;
extern struct cmd_element config_write_cmd_lib;
extern struct cmd_element show_memory_cmd_lib;
extern struct cmd_element no_access_list_extended_any_mask_cmd_lib;
extern struct cmd_element show_memory_ospf_cmd_lib;
extern struct cmd_element config_end_cmd_lib;
extern struct cmd_element no_route_map_cmd_lib;
extern struct cmd_element rmap_onmatch_goto_cmd_lib;
extern struct cmd_element no_access_list_extended_host_any_cmd_lib;
extern struct cmd_element no_access_list_remark_arg_cmd_lib;
extern struct cmd_element no_access_list_extended_cmd_lib;
extern struct cmd_element no_config_log_record_priority_cmd_lib;
extern struct cmd_element show_memory_lib_cmd_lib;
extern struct cmd_element no_config_log_monitor_cmd_lib;
extern struct cmd_element show_ip_prefix_list_name_cmd_lib;
extern struct cmd_element send_lifetime_month_day_day_month_cmd_lib;
extern struct cmd_element config_log_syslog_level_cmd_lib;
extern struct cmd_element no_banner_motd_cmd_lib;
extern struct cmd_element send_lifetime_infinite_month_day_cmd_lib;
extern struct cmd_element no_distribute_list_cmd_lib;
extern struct cmd_element no_config_log_facility_cmd_lib;
extern struct cmd_element rmap_continue_index_cmd_lib;
extern struct cmd_element no_ip_prefix_list_le_ge_cmd_lib;
extern struct cmd_element show_memory_ripng_cmd_lib;
extern struct cmd_element accept_lifetime_day_month_month_day_cmd_lib;
extern struct cmd_element accept_lifetime_duration_day_month_cmd_lib;
extern struct cmd_element send_lifetime_duration_day_month_cmd_lib;
extern struct cmd_element ip_prefix_list_le_ge_cmd_lib;
extern struct cmd_element ip_prefix_list_cmd_lib;
extern struct cmd_element exec_timeout_min_cmd_lib;
extern struct cmd_element no_access_list_remark_cmd_lib;
extern struct cmd_element distribute_list_all_cmd_lib;
extern struct cmd_element no_key_string_cmd_lib;
extern struct cmd_element no_interface_cmd_lib;
extern struct cmd_element no_access_list_extended_mask_any_cmd_lib;
extern struct cmd_element ipv6_distribute_list_all_cmd_lib;
extern struct cmd_element show_ip_prefix_list_summary_cmd_lib;
extern struct cmd_element ip_prefix_list_description_cmd_lib;
extern struct cmd_element no_if_rmap_cmd_lib;
extern struct cmd_element no_access_list_standard_host_cmd_lib;
extern struct cmd_element show_ip_prefix_list_prefix_first_match_cmd_lib;
extern struct cmd_element show_ip_access_list_cmd_lib;
extern struct cmd_element rmap_continue_cmd_lib;
extern struct cmd_element exec_timeout_sec_cmd_lib;
extern struct cmd_element ip_prefix_list_seq_le_cmd_lib;
extern struct cmd_element access_list_exact_cmd_lib;
extern struct cmd_element config_disable_cmd_lib;
extern struct cmd_element no_ip_prefix_list_le_cmd_lib;
extern struct cmd_element config_log_file_level_cmd_lib;
extern struct cmd_element vty_access_class_cmd_lib;
extern struct cmd_element clear_ip_prefix_list_name_cmd_lib;
extern struct cmd_element ip_prefix_list_seq_ge_le_cmd_lib;
extern struct cmd_element send_lifetime_infinite_day_month_cmd_lib;
extern struct cmd_element access_list_extended_host_mask_cmd_lib;
extern struct cmd_element echo_cmd_lib;
extern struct cmd_element accept_lifetime_infinite_day_month_cmd_lib;
extern struct cmd_element access_list_extended_cmd_lib;
extern struct cmd_element show_ip_prefix_list_name_seq_cmd_lib;
extern struct cmd_element no_ip_prefix_list_description_arg_cmd_lib;
extern struct cmd_element show_address_cmd_lib;
extern struct cmd_element config_write_terminal_cmd_lib;
extern struct cmd_element ip_prefix_list_ge_le_cmd_lib;
extern struct cmd_element config_log_file_cmd_lib;
extern struct cmd_element no_access_list_standard_cmd_lib;
extern struct cmd_element no_distribute_list_all_cmd_lib;
extern struct cmd_element password_text_cmd_lib;
extern struct cmd_element no_key_cmd_lib;
extern struct cmd_element no_route_map_all_cmd_lib;
extern struct cmd_element config_who_cmd_lib;
extern struct cmd_element send_lifetime_month_day_month_day_cmd_lib;
extern struct cmd_element config_log_syslog_facility_cmd_lib;
extern struct cmd_element no_ip_prefix_list_sequence_number_cmd_lib;
extern struct cmd_element no_ip_prefix_list_seq_ge_cmd_lib;
extern struct cmd_element no_access_list_extended_any_host_cmd_lib;
extern struct cmd_element access_list_extended_host_host_cmd_lib;
extern struct cmd_element no_rmap_continue_seq_lib;
extern struct cmd_element config_enable_cmd_lib;
extern struct cmd_element access_list_extended_any_any_cmd_lib;
extern struct cmd_element access_list_extended_any_host_cmd_lib;
extern struct cmd_element accept_lifetime_month_day_day_month_cmd_lib;
extern struct cmd_element config_write_file_cmd_lib;
extern struct cmd_element no_config_log_syslog_facility_cmd_lib;
extern struct cmd_element no_ip_prefix_list_description_cmd_lib;
extern struct cmd_element show_thread_cpu_cmd_lib;
extern struct cmd_element no_config_log_syslog_cmd_lib;
extern struct cmd_element no_exec_timeout_cmd_lib;
extern struct cmd_element no_ip_prefix_list_ge_le_cmd_lib;
extern struct cmd_element no_key_chain_cmd_lib;
extern struct cmd_element show_startup_config_cmd_lib;
extern struct cmd_element terminal_monitor_cmd_lib;
extern struct cmd_element no_config_log_file_level_cmd_lib;
extern struct cmd_element rmap_show_cmd_lib;
extern struct cmd_element access_list_extended_host_any_cmd_lib;
extern struct cmd_element clear_ip_prefix_list_name_prefix_cmd_lib;
extern struct cmd_element config_quit_cmd_lib;
extern struct cmd_element config_log_monitor_level_cmd_lib;
extern struct cmd_element ip_prefix_list_le_cmd_lib;
extern struct cmd_element accept_lifetime_infinite_month_day_cmd_lib;
extern struct cmd_element no_ipv6_distribute_list_prefix_cmd_lib;
extern struct cmd_element if_rmap_cmd_lib;
extern struct cmd_element no_config_log_file_cmd_lib;
extern struct cmd_element no_ipv6_distribute_list_prefix_all_cmd_lib;
extern struct cmd_element show_memory_bgp_cmd_lib;
extern struct cmd_element show_memory_isis_cmd_lib;
extern struct cmd_element no_access_list_cmd_lib;
extern struct cmd_element no_ipv6_distribute_list_cmd_lib;
extern struct cmd_element no_vty_login_cmd_lib;
extern struct cmd_element no_service_terminal_length_cmd_lib;
extern struct cmd_element show_memory_all_cmd_lib;
extern struct cmd_element no_access_list_standard_any_cmd_lib;
extern struct cmd_element config_log_facility_cmd_lib;
extern struct cmd_element service_advanced_vty_cmd_lib;
extern struct cmd_element no_distribute_list_prefix_cmd_lib;
extern struct cmd_element no_rmap_call_cmd_lib;
extern struct cmd_element show_running_config_cmd_lib;
extern struct cmd_element config_terminal_cmd_lib;
extern struct cmd_element accept_lifetime_duration_month_day_cmd_lib;
extern struct cmd_element banner_motd_default_cmd_lib;
extern struct cmd_element no_vty_access_class_cmd_lib;
extern struct cmd_element show_memory_ospf6_cmd_lib;
extern struct cmd_element show_ip_prefix_list_cmd_lib;
extern struct cmd_element ip_prefix_list_sequence_number_cmd_lib;
extern struct cmd_element no_access_list_exact_cmd_lib;
extern struct cmd_element config_write_memory_cmd_lib;
extern struct cmd_element config_log_record_priority_cmd_lib;
extern struct cmd_element show_ip_prefix_list_prefix_cmd_lib;
extern struct cmd_element show_version_cmd_lib;
extern struct cmd_element enable_password_text_cmd_lib;
extern struct cmd_element config_list_cmd_lib;
extern struct cmd_element config_exit_cmd_lib;
extern struct cmd_element show_logging_cmd_lib;
extern struct cmd_element key_string_cmd_lib;
extern struct cmd_element access_list_extended_any_mask_cmd_lib;
extern struct cmd_element interface_cmd_lib;
extern struct cmd_element access_list_standard_cmd_lib;
extern struct cmd_element distribute_list_prefix_all_cmd_lib;
extern struct cmd_element access_list_extended_mask_host_cmd_lib;
extern struct cmd_element access_list_cmd_lib;
extern struct cmd_element config_log_stdout_level_cmd_lib;
extern struct cmd_element show_ip_access_list_name_cmd_lib;
extern struct cmd_element no_config_log_stdout_cmd_lib;
extern struct cmd_element access_list_any_cmd_lib;
extern struct cmd_element config_log_trap_cmd_lib;
extern struct cmd_element no_ip_prefix_list_cmd_lib;
extern struct cmd_element no_access_list_extended_host_host_cmd_lib;
extern struct cmd_element hostname_cmd_lib;
extern struct cmd_element no_distribute_list_prefix_all_cmd_lib;
extern struct cmd_element password_cmd_lib;
extern struct cmd_element accept_lifetime_month_day_month_day_cmd_lib;
extern struct cmd_element no_access_list_extended_any_any_cmd_lib;
extern struct cmd_element no_access_list_all_cmd_lib;
extern struct cmd_element no_access_list_any_cmd_lib;
extern struct cmd_element if_ipv6_rmap_cmd_lib;
extern struct cmd_element no_access_list_extended_host_mask_cmd_lib;
extern struct cmd_element access_list_standard_nomask_cmd_lib;
extern struct cmd_element ipv6_distribute_list_cmd_lib;
extern struct cmd_element no_ip_prefix_list_seq_ge_le_cmd_lib;
extern struct cmd_element access_list_remark_cmd_lib;
extern struct cmd_element config_terminal_no_length_cmd_lib;
extern struct cmd_element ip_prefix_list_seq_le_ge_cmd_lib;
extern struct cmd_element no_ip_prefix_list_seq_cmd_lib;
extern struct cmd_element rmap_onmatch_next_cmd_lib;
extern struct cmd_element no_rmap_continue_cmd_lib;
extern struct cmd_element ipv6_distribute_list_prefix_cmd_lib;
extern struct cmd_element access_list_extended_mask_any_cmd_lib;
extern struct cmd_element show_memory_rip_cmd_lib;
extern struct cmd_element no_ip_prefix_list_ge_cmd_lib;
extern struct cmd_element config_logmsg_cmd_lib;
extern struct cmd_element no_rmap_onmatch_goto_cmd_lib;
extern struct cmd_element send_lifetime_duration_month_day_cmd_lib;
extern struct cmd_element rmap_call_cmd_lib;
extern struct cmd_element show_ip_prefix_list_prefix_longer_cmd_lib;
extern struct cmd_element access_list_standard_any_cmd_lib;
extern struct cmd_element ip_prefix_list_seq_cmd_lib;
extern struct cmd_element config_log_monitor_cmd_lib;
extern struct cmd_element no_ip_prefix_list_prefix_cmd_lib;
extern struct cmd_element show_ip_prefix_list_detail_name_cmd_lib;
extern struct cmd_element no_ipv6_distribute_list_all_cmd_lib;
extern struct cmd_element config_log_syslog_cmd_lib;
extern struct cmd_element distribute_list_prefix_cmd_lib;

void GlobalVars_initializeActiveSet_lib()
{
    struct cmd_node vty_node__T = 
    {
        VTY_NODE,
        "%s(config-line)# ",
        1,
    };
    struct cmd_node enable_node__T = 
    {
        ENABLE_NODE,
        "%s# ",
    };
    struct cmd_node auth_enable_node__T = 
    {
        AUTH_ENABLE_NODE,
        "Password: ",
    };
    struct cmd_node prefix_node__T = 
    {
        PREFIX_NODE,
        "",               
    1
    };
    struct cmd_node view_node__T = 
    {
        VIEW_NODE,
        "%s> ",
    };
    struct cmd_node keychain_node__T = 
    {
        KEYCHAIN_NODE,
        "%s(config-keychain)# ",
        1
    };
    struct route_map_list route_map_master__T =  { NULL, NULL, NULL, NULL };
    struct prefix_master prefix_master_ipv4__T =  
    { 
        {NULL, NULL},
        {NULL, NULL},
        1,
        NULL,
        NULL,
    };
    struct cmd_node access_node__T = 
    {
        ACCESS_NODE,
        "",               
        1
    };
    struct cmd_node keychain_key_node__T = 
    {
        KEYCHAIN_KEY_NODE,
        "%s(config-keychain-key)# ",
        1
    };
    struct cmd_node rmap_node__T = 
    {
        RMAP_NODE,
        "%s(config-route-map)# ",
        1
    };
    struct prefix_master prefix_master_orf__T =  
    { 
        {NULL, NULL},
        {NULL, NULL},
        1,
        NULL,
        NULL,
    };
    struct cmd_node auth_node__T = 
    {
        AUTH_NODE,
        "Password: ",
    };
    struct cmd_node config_node__T = 
    {
        CONFIG_NODE,
        "%s(config)# ",
        1
    };
    struct access_master access_master_ipv4__T =  
    {
        {NULL, NULL},
        {NULL, NULL},
        NULL,
        NULL,
    };
    memcpy(&__activeVars_lib->rmap_show_name_cmd__X, &rmap_show_name_cmd_lib, sizeof(rmap_show_name_cmd_lib));
    memcpy(&__activeVars_lib->no_ip_prefix_list_seq_le_ge_cmd__X, &no_ip_prefix_list_seq_le_ge_cmd_lib, sizeof(no_ip_prefix_list_seq_le_ge_cmd_lib));
    memcpy(&__activeVars_lib->rmap_continue_seq_cmd__X, &rmap_continue_seq_cmd_lib, sizeof(rmap_continue_seq_cmd_lib));
    memcpy(&__activeVars_lib->vty_node__X, &vty_node__T, sizeof(vty_node__T));
    memcpy(&__activeVars_lib->interface_desc_cmd__X, &interface_desc_cmd_lib, sizeof(interface_desc_cmd_lib));
    memcpy(&__activeVars_lib->enable_node__X, &enable_node__T, sizeof(enable_node__T));
    __activeVars_lib->re_max_failures__X =  20000;
    memcpy(&__activeVars_lib->key_chain_cmd__X, &key_chain_cmd_lib, sizeof(key_chain_cmd_lib));
    memcpy(&__activeVars_lib->no_rmap_onmatch_next_cmd__X, &no_rmap_onmatch_next_cmd_lib, sizeof(no_rmap_onmatch_next_cmd_lib));
    memcpy(&__activeVars_lib->show_history_cmd__X, &show_history_cmd_lib, sizeof(show_history_cmd_lib));
    memcpy(&__activeVars_lib->show_ip_prefix_list_summary_name_cmd__X, &show_ip_prefix_list_summary_name_cmd_lib, sizeof(show_ip_prefix_list_summary_name_cmd_lib));
    memcpy(&__activeVars_lib->enable_password_cmd__X, &enable_password_cmd_lib, sizeof(enable_password_cmd_lib));
    memcpy(&__activeVars_lib->config_terminal_length_cmd__X, &config_terminal_length_cmd_lib, sizeof(config_terminal_length_cmd_lib));
    memcpy(&__activeVars_lib->ip_prefix_list_ge_cmd__X, &ip_prefix_list_ge_cmd_lib, sizeof(ip_prefix_list_ge_cmd_lib));
    memcpy(&__activeVars_lib->no_config_log_trap_cmd__X, &no_config_log_trap_cmd_lib, sizeof(no_config_log_trap_cmd_lib));
    memcpy(&__activeVars_lib->auth_enable_node__X, &auth_enable_node__T, sizeof(auth_enable_node__T));
    memcpy(&__activeVars_lib->ipv6_distribute_list_prefix_all_cmd__X, &ipv6_distribute_list_prefix_all_cmd_lib, sizeof(ipv6_distribute_list_prefix_all_cmd_lib));
    memcpy(&__activeVars_lib->service_terminal_length_cmd__X, &service_terminal_length_cmd_lib, sizeof(service_terminal_length_cmd_lib));
    memcpy(&__activeVars_lib->accept_lifetime_day_month_day_month_cmd__X, &accept_lifetime_day_month_day_month_cmd_lib, sizeof(accept_lifetime_day_month_day_month_cmd_lib));
    memcpy(&__activeVars_lib->config_help_cmd__X, &config_help_cmd_lib, sizeof(config_help_cmd_lib));
    memcpy(&__activeVars_lib->send_lifetime_day_month_day_month_cmd__X, &send_lifetime_day_month_day_month_cmd_lib, sizeof(send_lifetime_day_month_day_month_cmd_lib));
    memcpy(&__activeVars_lib->terminal_no_monitor_cmd__X, &terminal_no_monitor_cmd_lib, sizeof(terminal_no_monitor_cmd_lib));
    memcpy(&__activeVars_lib->vty_login_cmd__X, &vty_login_cmd_lib, sizeof(vty_login_cmd_lib));
    memcpy(&__activeVars_lib->route_map_cmd__X, &route_map_cmd_lib, sizeof(route_map_cmd_lib));
    memcpy(&__activeVars_lib->send_lifetime_day_month_month_day_cmd__X, &send_lifetime_day_month_month_day_cmd_lib, sizeof(send_lifetime_day_month_month_day_cmd_lib));
    memcpy(&__activeVars_lib->no_access_list_standard_nomask_cmd__X, &no_access_list_standard_nomask_cmd_lib, sizeof(no_access_list_standard_nomask_cmd_lib));
    memcpy(&__activeVars_lib->distribute_list_cmd__X, &distribute_list_cmd_lib, sizeof(distribute_list_cmd_lib));
    memcpy(&__activeVars_lib->config_log_stdout_cmd__X, &config_log_stdout_cmd_lib, sizeof(config_log_stdout_cmd_lib));
    memcpy(&__activeVars_lib->service_password_encrypt_cmd__X, &service_password_encrypt_cmd_lib, sizeof(service_password_encrypt_cmd_lib));
    memcpy(&__activeVars_lib->clear_ip_prefix_list_cmd__X, &clear_ip_prefix_list_cmd_lib, sizeof(clear_ip_prefix_list_cmd_lib));
    memcpy(&__activeVars_lib->no_access_list_extended_mask_host_cmd__X, &no_access_list_extended_mask_host_cmd_lib, sizeof(no_access_list_extended_mask_host_cmd_lib));
    __activeVars_lib->cpu_record__X =  NULL;
    memcpy(&__activeVars_lib->no_if_ipv6_rmap_cmd__X, &no_if_ipv6_rmap_cmd_lib, sizeof(no_if_ipv6_rmap_cmd_lib));
    memcpy(&__activeVars_lib->access_list_standard_host_cmd__X, &access_list_standard_host_cmd_lib, sizeof(access_list_standard_host_cmd_lib));
    memcpy(&__activeVars_lib->no_service_password_encrypt_cmd__X, &no_service_password_encrypt_cmd_lib, sizeof(no_service_password_encrypt_cmd_lib));
    memcpy(&__activeVars_lib->show_ip_prefix_list_detail_cmd__X, &show_ip_prefix_list_detail_cmd_lib, sizeof(show_ip_prefix_list_detail_cmd_lib));
    __activeVars_lib->zlog_default__X =  NULL;
    __activeVars_lib->vty_accesslist_name__X =  NULL;
    memcpy(&__activeVars_lib->no_service_advanced_vty_cmd__X, &no_service_advanced_vty_cmd_lib, sizeof(no_service_advanced_vty_cmd_lib));
    memcpy(&__activeVars_lib->line_vty_cmd__X, &line_vty_cmd_lib, sizeof(line_vty_cmd_lib));
    memcpy(&__activeVars_lib->no_ip_prefix_list_seq_le_cmd__X, &no_ip_prefix_list_seq_le_cmd_lib, sizeof(no_ip_prefix_list_seq_le_cmd_lib));
    memcpy(&__activeVars_lib->no_hostname_cmd__X, &no_hostname_cmd_lib, sizeof(no_hostname_cmd_lib));
    memcpy(&__activeVars_lib->no_interface_desc_cmd__X, &no_interface_desc_cmd_lib, sizeof(no_interface_desc_cmd_lib));
    memcpy(&__activeVars_lib->ip_prefix_list_seq_ge_cmd__X, &ip_prefix_list_seq_ge_cmd_lib, sizeof(ip_prefix_list_seq_ge_cmd_lib));
    memcpy(&__activeVars_lib->no_enable_password_cmd__X, &no_enable_password_cmd_lib, sizeof(no_enable_password_cmd_lib));
    memcpy(&__activeVars_lib->prefix_node__X, &prefix_node__T, sizeof(prefix_node__T));
    memcpy(&__activeVars_lib->key_cmd__X, &key_cmd_lib, sizeof(key_cmd_lib));
    memcpy(&__activeVars_lib->copy_runningconfig_startupconfig_cmd__X, &copy_runningconfig_startupconfig_cmd_lib, sizeof(copy_runningconfig_startupconfig_cmd_lib));
    memcpy(&__activeVars_lib->config_write_cmd__X, &config_write_cmd_lib, sizeof(config_write_cmd_lib));
    memcpy(&__activeVars_lib->show_memory_cmd__X, &show_memory_cmd_lib, sizeof(show_memory_cmd_lib));
    memcpy(&__activeVars_lib->no_access_list_extended_any_mask_cmd__X, &no_access_list_extended_any_mask_cmd_lib, sizeof(no_access_list_extended_any_mask_cmd_lib));
    __activeVars_lib->vty_ipv6_accesslist_name__X =  NULL;
    memcpy(&__activeVars_lib->show_memory_ospf_cmd__X, &show_memory_ospf_cmd_lib, sizeof(show_memory_ospf_cmd_lib));
    memcpy(&__activeVars_lib->view_node__X, &view_node__T, sizeof(view_node__T));
    memcpy(&__activeVars_lib->config_end_cmd__X, &config_end_cmd_lib, sizeof(config_end_cmd_lib));
    memcpy(&__activeVars_lib->no_route_map_cmd__X, &no_route_map_cmd_lib, sizeof(no_route_map_cmd_lib));
    memcpy(&__activeVars_lib->rmap_onmatch_goto_cmd__X, &rmap_onmatch_goto_cmd_lib, sizeof(rmap_onmatch_goto_cmd_lib));
    memcpy(&__activeVars_lib->keychain_node__X, &keychain_node__T, sizeof(keychain_node__T));
    memcpy(&__activeVars_lib->no_access_list_extended_host_any_cmd__X, &no_access_list_extended_host_any_cmd_lib, sizeof(no_access_list_extended_host_any_cmd_lib));
    memcpy(&__activeVars_lib->route_map_master__X, &route_map_master__T, sizeof(route_map_master__T));
    memcpy(&__activeVars_lib->no_access_list_remark_arg_cmd__X, &no_access_list_remark_arg_cmd_lib, sizeof(no_access_list_remark_arg_cmd_lib));
    memcpy(&__activeVars_lib->no_access_list_extended_cmd__X, &no_access_list_extended_cmd_lib, sizeof(no_access_list_extended_cmd_lib));
    memcpy(&__activeVars_lib->no_config_log_record_priority_cmd__X, &no_config_log_record_priority_cmd_lib, sizeof(no_config_log_record_priority_cmd_lib));
    __activeVars_lib->__getopt_initialized__X =  0;
    memcpy(&__activeVars_lib->show_memory_lib_cmd__X, &show_memory_lib_cmd_lib, sizeof(show_memory_lib_cmd_lib));
    memcpy(&__activeVars_lib->no_config_log_monitor_cmd__X, &no_config_log_monitor_cmd_lib, sizeof(no_config_log_monitor_cmd_lib));
    __activeVars_lib->optind__X =  1;
    memcpy(&__activeVars_lib->show_ip_prefix_list_name_cmd__X, &show_ip_prefix_list_name_cmd_lib, sizeof(show_ip_prefix_list_name_cmd_lib));
    memcpy(&__activeVars_lib->send_lifetime_month_day_day_month_cmd__X, &send_lifetime_month_day_day_month_cmd_lib, sizeof(send_lifetime_month_day_day_month_cmd_lib));
    memcpy(&__activeVars_lib->config_log_syslog_level_cmd__X, &config_log_syslog_level_cmd_lib, sizeof(config_log_syslog_level_cmd_lib));
    memcpy(&__activeVars_lib->no_banner_motd_cmd__X, &no_banner_motd_cmd_lib, sizeof(no_banner_motd_cmd_lib));
    memcpy(&__activeVars_lib->send_lifetime_infinite_month_day_cmd__X, &send_lifetime_infinite_month_day_cmd_lib, sizeof(send_lifetime_infinite_month_day_cmd_lib));
    memcpy(&__activeVars_lib->no_distribute_list_cmd__X, &no_distribute_list_cmd_lib, sizeof(no_distribute_list_cmd_lib));
    memcpy(&__activeVars_lib->no_config_log_facility_cmd__X, &no_config_log_facility_cmd_lib, sizeof(no_config_log_facility_cmd_lib));
    memcpy(&__activeVars_lib->rmap_continue_index_cmd__X, &rmap_continue_index_cmd_lib, sizeof(rmap_continue_index_cmd_lib));
    memcpy(&__activeVars_lib->no_ip_prefix_list_le_ge_cmd__X, &no_ip_prefix_list_le_ge_cmd_lib, sizeof(no_ip_prefix_list_le_ge_cmd_lib));
    memcpy(&__activeVars_lib->show_memory_ripng_cmd__X, &show_memory_ripng_cmd_lib, sizeof(show_memory_ripng_cmd_lib));
    memcpy(&__activeVars_lib->accept_lifetime_day_month_month_day_cmd__X, &accept_lifetime_day_month_month_day_cmd_lib, sizeof(accept_lifetime_day_month_month_day_cmd_lib));
    memcpy(&__activeVars_lib->accept_lifetime_duration_day_month_cmd__X, &accept_lifetime_duration_day_month_cmd_lib, sizeof(accept_lifetime_duration_day_month_cmd_lib));
    memcpy(&__activeVars_lib->prefix_master_ipv4__X, &prefix_master_ipv4__T, sizeof(prefix_master_ipv4__T));
    memcpy(&__activeVars_lib->send_lifetime_duration_day_month_cmd__X, &send_lifetime_duration_day_month_cmd_lib, sizeof(send_lifetime_duration_day_month_cmd_lib));
    memcpy(&__activeVars_lib->ip_prefix_list_le_ge_cmd__X, &ip_prefix_list_le_ge_cmd_lib, sizeof(ip_prefix_list_le_ge_cmd_lib));
    memcpy(&__activeVars_lib->ip_prefix_list_cmd__X, &ip_prefix_list_cmd_lib, sizeof(ip_prefix_list_cmd_lib));
    memcpy(&__activeVars_lib->exec_timeout_min_cmd__X, &exec_timeout_min_cmd_lib, sizeof(exec_timeout_min_cmd_lib));
    memcpy(&__activeVars_lib->no_access_list_remark_cmd__X, &no_access_list_remark_cmd_lib, sizeof(no_access_list_remark_cmd_lib));
    memcpy(&__activeVars_lib->distribute_list_all_cmd__X, &distribute_list_all_cmd_lib, sizeof(distribute_list_all_cmd_lib));
    memcpy(&__activeVars_lib->no_key_string_cmd__X, &no_key_string_cmd_lib, sizeof(no_key_string_cmd_lib));
    memcpy(&__activeVars_lib->no_interface_cmd__X, &no_interface_cmd_lib, sizeof(no_interface_cmd_lib));
    memcpy(&__activeVars_lib->no_access_list_extended_mask_any_cmd__X, &no_access_list_extended_mask_any_cmd_lib, sizeof(no_access_list_extended_mask_any_cmd_lib));
    memcpy(&__activeVars_lib->ipv6_distribute_list_all_cmd__X, &ipv6_distribute_list_all_cmd_lib, sizeof(ipv6_distribute_list_all_cmd_lib));
    __activeVars_lib->opterr__X =  1;
    memcpy(&__activeVars_lib->show_ip_prefix_list_summary_cmd__X, &show_ip_prefix_list_summary_cmd_lib, sizeof(show_ip_prefix_list_summary_cmd_lib));
    memcpy(&__activeVars_lib->ip_prefix_list_description_cmd__X, &ip_prefix_list_description_cmd_lib, sizeof(ip_prefix_list_description_cmd_lib));
    memcpy(&__activeVars_lib->no_if_rmap_cmd__X, &no_if_rmap_cmd_lib, sizeof(no_if_rmap_cmd_lib));
    memcpy(&__activeVars_lib->no_access_list_standard_host_cmd__X, &no_access_list_standard_host_cmd_lib, sizeof(no_access_list_standard_host_cmd_lib));
    memcpy(&__activeVars_lib->show_ip_prefix_list_prefix_first_match_cmd__X, &show_ip_prefix_list_prefix_first_match_cmd_lib, sizeof(show_ip_prefix_list_prefix_first_match_cmd_lib));
    memcpy(&__activeVars_lib->show_ip_access_list_cmd__X, &show_ip_access_list_cmd_lib, sizeof(show_ip_access_list_cmd_lib));
    memcpy(&__activeVars_lib->rmap_continue_cmd__X, &rmap_continue_cmd_lib, sizeof(rmap_continue_cmd_lib));
    __activeVars_lib->telnet_space_char__X =  ' ';
    memcpy(&__activeVars_lib->exec_timeout_sec_cmd__X, &exec_timeout_sec_cmd_lib, sizeof(exec_timeout_sec_cmd_lib));
    memcpy(&__activeVars_lib->ip_prefix_list_seq_le_cmd__X, &ip_prefix_list_seq_le_cmd_lib, sizeof(ip_prefix_list_seq_le_cmd_lib));
    memcpy(&__activeVars_lib->access_list_exact_cmd__X, &access_list_exact_cmd_lib, sizeof(access_list_exact_cmd_lib));
    memcpy(&__activeVars_lib->config_disable_cmd__X, &config_disable_cmd_lib, sizeof(config_disable_cmd_lib));
    memcpy(&__activeVars_lib->no_ip_prefix_list_le_cmd__X, &no_ip_prefix_list_le_cmd_lib, sizeof(no_ip_prefix_list_le_cmd_lib));
    memcpy(&__activeVars_lib->config_log_file_level_cmd__X, &config_log_file_level_cmd_lib, sizeof(config_log_file_level_cmd_lib));
    memcpy(&__activeVars_lib->vty_access_class_cmd__X, &vty_access_class_cmd_lib, sizeof(vty_access_class_cmd_lib));
    memcpy(&__activeVars_lib->clear_ip_prefix_list_name_cmd__X, &clear_ip_prefix_list_name_cmd_lib, sizeof(clear_ip_prefix_list_name_cmd_lib));
    memcpy(&__activeVars_lib->ip_prefix_list_seq_ge_le_cmd__X, &ip_prefix_list_seq_ge_le_cmd_lib, sizeof(ip_prefix_list_seq_ge_le_cmd_lib));
    memcpy(&__activeVars_lib->send_lifetime_infinite_day_month_cmd__X, &send_lifetime_infinite_day_month_cmd_lib, sizeof(send_lifetime_infinite_day_month_cmd_lib));
    memcpy(&__activeVars_lib->access_list_extended_host_mask_cmd__X, &access_list_extended_host_mask_cmd_lib, sizeof(access_list_extended_host_mask_cmd_lib));
    memcpy(&__activeVars_lib->echo_cmd__X, &echo_cmd_lib, sizeof(echo_cmd_lib));
    memcpy(&__activeVars_lib->accept_lifetime_infinite_day_month_cmd__X, &accept_lifetime_infinite_day_month_cmd_lib, sizeof(accept_lifetime_infinite_day_month_cmd_lib));
    memcpy(&__activeVars_lib->access_list_extended_cmd__X, &access_list_extended_cmd_lib, sizeof(access_list_extended_cmd_lib));
    memcpy(&__activeVars_lib->show_ip_prefix_list_name_seq_cmd__X, &show_ip_prefix_list_name_seq_cmd_lib, sizeof(show_ip_prefix_list_name_seq_cmd_lib));
    memcpy(&__activeVars_lib->no_ip_prefix_list_description_arg_cmd__X, &no_ip_prefix_list_description_arg_cmd_lib, sizeof(no_ip_prefix_list_description_arg_cmd_lib));
    memcpy(&__activeVars_lib->show_address_cmd__X, &show_address_cmd_lib, sizeof(show_address_cmd_lib));
    memcpy(&__activeVars_lib->config_write_terminal_cmd__X, &config_write_terminal_cmd_lib, sizeof(config_write_terminal_cmd_lib));
    memcpy(&__activeVars_lib->ip_prefix_list_ge_le_cmd__X, &ip_prefix_list_ge_le_cmd_lib, sizeof(ip_prefix_list_ge_le_cmd_lib));
    memcpy(&__activeVars_lib->config_log_file_cmd__X, &config_log_file_cmd_lib, sizeof(config_log_file_cmd_lib));
    memcpy(&__activeVars_lib->no_access_list_standard_cmd__X, &no_access_list_standard_cmd_lib, sizeof(no_access_list_standard_cmd_lib));
    memcpy(&__activeVars_lib->no_distribute_list_all_cmd__X, &no_distribute_list_all_cmd_lib, sizeof(no_distribute_list_all_cmd_lib));
    memcpy(&__activeVars_lib->password_text_cmd__X, &password_text_cmd_lib, sizeof(password_text_cmd_lib));
    __activeVars_lib->syslog_fd__X =  -1;
    memcpy(&__activeVars_lib->no_key_cmd__X, &no_key_cmd_lib, sizeof(no_key_cmd_lib));
    memcpy(&__activeVars_lib->no_route_map_all_cmd__X, &no_route_map_all_cmd_lib, sizeof(no_route_map_all_cmd_lib));
    memcpy(&__activeVars_lib->config_who_cmd__X, &config_who_cmd_lib, sizeof(config_who_cmd_lib));
    memcpy(&__activeVars_lib->send_lifetime_month_day_month_day_cmd__X, &send_lifetime_month_day_month_day_cmd_lib, sizeof(send_lifetime_month_day_month_day_cmd_lib));
    memcpy(&__activeVars_lib->config_log_syslog_facility_cmd__X, &config_log_syslog_facility_cmd_lib, sizeof(config_log_syslog_facility_cmd_lib));
    __activeVars_lib->optopt__X =  '?';
    memcpy(&__activeVars_lib->no_ip_prefix_list_sequence_number_cmd__X, &no_ip_prefix_list_sequence_number_cmd_lib, sizeof(no_ip_prefix_list_sequence_number_cmd_lib));
    memcpy(&__activeVars_lib->no_ip_prefix_list_seq_ge_cmd__X, &no_ip_prefix_list_seq_ge_cmd_lib, sizeof(no_ip_prefix_list_seq_ge_cmd_lib));
    memcpy(&__activeVars_lib->no_access_list_extended_any_host_cmd__X, &no_access_list_extended_any_host_cmd_lib, sizeof(no_access_list_extended_any_host_cmd_lib));
    memcpy(&__activeVars_lib->access_node__X, &access_node__T, sizeof(access_node__T));
    memcpy(&__activeVars_lib->keychain_key_node__X, &keychain_key_node__T, sizeof(keychain_key_node__T));
    memcpy(&__activeVars_lib->access_list_extended_host_host_cmd__X, &access_list_extended_host_host_cmd_lib, sizeof(access_list_extended_host_host_cmd_lib));
    memcpy(&__activeVars_lib->no_rmap_continue_seq__X, &no_rmap_continue_seq_lib, sizeof(no_rmap_continue_seq_lib));
    memcpy(&__activeVars_lib->config_enable_cmd__X, &config_enable_cmd_lib, sizeof(config_enable_cmd_lib));
    memcpy(&__activeVars_lib->access_list_extended_any_any_cmd__X, &access_list_extended_any_any_cmd_lib, sizeof(access_list_extended_any_any_cmd_lib));
    memcpy(&__activeVars_lib->access_list_extended_any_host_cmd__X, &access_list_extended_any_host_cmd_lib, sizeof(access_list_extended_any_host_cmd_lib));
    memcpy(&__activeVars_lib->accept_lifetime_month_day_day_month_cmd__X, &accept_lifetime_month_day_day_month_cmd_lib, sizeof(accept_lifetime_month_day_day_month_cmd_lib));
    memcpy(&__activeVars_lib->config_write_file_cmd__X, &config_write_file_cmd_lib, sizeof(config_write_file_cmd_lib));
    memcpy(&__activeVars_lib->no_config_log_syslog_facility_cmd__X, &no_config_log_syslog_facility_cmd_lib, sizeof(no_config_log_syslog_facility_cmd_lib));
    memcpy(&__activeVars_lib->rmap_node__X, &rmap_node__T, sizeof(rmap_node__T));
    __activeVars_lib->vty_timeout_val__X =  VTY_TIMEOUT_DEFAULT;
    memcpy(&__activeVars_lib->no_ip_prefix_list_description_cmd__X, &no_ip_prefix_list_description_cmd_lib, sizeof(no_ip_prefix_list_description_cmd_lib));
    memcpy(&__activeVars_lib->show_thread_cpu_cmd__X, &show_thread_cpu_cmd_lib, sizeof(show_thread_cpu_cmd_lib));
    memcpy(&__activeVars_lib->no_config_log_syslog_cmd__X, &no_config_log_syslog_cmd_lib, sizeof(no_config_log_syslog_cmd_lib));
    memcpy(&__activeVars_lib->no_exec_timeout_cmd__X, &no_exec_timeout_cmd_lib, sizeof(no_exec_timeout_cmd_lib));
    memcpy(&__activeVars_lib->no_ip_prefix_list_ge_le_cmd__X, &no_ip_prefix_list_ge_le_cmd_lib, sizeof(no_ip_prefix_list_ge_le_cmd_lib));
    memcpy(&__activeVars_lib->no_key_chain_cmd__X, &no_key_chain_cmd_lib, sizeof(no_key_chain_cmd_lib));
    memcpy(&__activeVars_lib->show_startup_config_cmd__X, &show_startup_config_cmd_lib, sizeof(show_startup_config_cmd_lib));
    memcpy(&__activeVars_lib->terminal_monitor_cmd__X, &terminal_monitor_cmd_lib, sizeof(terminal_monitor_cmd_lib));
    memcpy(&__activeVars_lib->no_config_log_file_level_cmd__X, &no_config_log_file_level_cmd_lib, sizeof(no_config_log_file_level_cmd_lib));
    memcpy(&__activeVars_lib->rmap_show_cmd__X, &rmap_show_cmd_lib, sizeof(rmap_show_cmd_lib));
    memcpy(&__activeVars_lib->access_list_extended_host_any_cmd__X, &access_list_extended_host_any_cmd_lib, sizeof(access_list_extended_host_any_cmd_lib));
    memcpy(&__activeVars_lib->clear_ip_prefix_list_name_prefix_cmd__X, &clear_ip_prefix_list_name_prefix_cmd_lib, sizeof(clear_ip_prefix_list_name_prefix_cmd_lib));
    __activeVars_lib->optarg__X =  NULL;
    memcpy(&__activeVars_lib->config_quit_cmd__X, &config_quit_cmd_lib, sizeof(config_quit_cmd_lib));
    memcpy(&__activeVars_lib->config_log_monitor_level_cmd__X, &config_log_monitor_level_cmd_lib, sizeof(config_log_monitor_level_cmd_lib));
    memcpy(&__activeVars_lib->ip_prefix_list_le_cmd__X, &ip_prefix_list_le_cmd_lib, sizeof(ip_prefix_list_le_cmd_lib));
    memcpy(&__activeVars_lib->accept_lifetime_infinite_month_day_cmd__X, &accept_lifetime_infinite_month_day_cmd_lib, sizeof(accept_lifetime_infinite_month_day_cmd_lib));
    memcpy(&__activeVars_lib->no_ipv6_distribute_list_prefix_cmd__X, &no_ipv6_distribute_list_prefix_cmd_lib, sizeof(no_ipv6_distribute_list_prefix_cmd_lib));
    memcpy(&__activeVars_lib->if_rmap_cmd__X, &if_rmap_cmd_lib, sizeof(if_rmap_cmd_lib));
    memcpy(&__activeVars_lib->no_config_log_file_cmd__X, &no_config_log_file_cmd_lib, sizeof(no_config_log_file_cmd_lib));
    memcpy(&__activeVars_lib->no_ipv6_distribute_list_prefix_all_cmd__X, &no_ipv6_distribute_list_prefix_all_cmd_lib, sizeof(no_ipv6_distribute_list_prefix_all_cmd_lib));
    memcpy(&__activeVars_lib->prefix_master_orf__X, &prefix_master_orf__T, sizeof(prefix_master_orf__T));
    memcpy(&__activeVars_lib->auth_node__X, &auth_node__T, sizeof(auth_node__T));
    __activeVars_lib->if_rmap_add_hook__X =  NULL;
    memcpy(&__activeVars_lib->show_memory_bgp_cmd__X, &show_memory_bgp_cmd_lib, sizeof(show_memory_bgp_cmd_lib));
    memcpy(&__activeVars_lib->show_memory_isis_cmd__X, &show_memory_isis_cmd_lib, sizeof(show_memory_isis_cmd_lib));
    memcpy(&__activeVars_lib->no_access_list_cmd__X, &no_access_list_cmd_lib, sizeof(no_access_list_cmd_lib));
    memcpy(&__activeVars_lib->no_ipv6_distribute_list_cmd__X, &no_ipv6_distribute_list_cmd_lib, sizeof(no_ipv6_distribute_list_cmd_lib));
    memcpy(&__activeVars_lib->no_vty_login_cmd__X, &no_vty_login_cmd_lib, sizeof(no_vty_login_cmd_lib));
    memcpy(&__activeVars_lib->no_service_terminal_length_cmd__X, &no_service_terminal_length_cmd_lib, sizeof(no_service_terminal_length_cmd_lib));
    memcpy(&__activeVars_lib->show_memory_all_cmd__X, &show_memory_all_cmd_lib, sizeof(show_memory_all_cmd_lib));
    memcpy(&__activeVars_lib->no_access_list_standard_any_cmd__X, &no_access_list_standard_any_cmd_lib, sizeof(no_access_list_standard_any_cmd_lib));
    memcpy(&__activeVars_lib->config_log_facility_cmd__X, &config_log_facility_cmd_lib, sizeof(config_log_facility_cmd_lib));
    memcpy(&__activeVars_lib->service_advanced_vty_cmd__X, &service_advanced_vty_cmd_lib, sizeof(service_advanced_vty_cmd_lib));
    memcpy(&__activeVars_lib->no_distribute_list_prefix_cmd__X, &no_distribute_list_prefix_cmd_lib, sizeof(no_distribute_list_prefix_cmd_lib));
    __activeVars_lib->zclient_debug__X =  0;
    memcpy(&__activeVars_lib->no_rmap_call_cmd__X, &no_rmap_call_cmd_lib, sizeof(no_rmap_call_cmd_lib));
    memcpy(&__activeVars_lib->show_running_config_cmd__X, &show_running_config_cmd_lib, sizeof(show_running_config_cmd_lib));
    memcpy(&__activeVars_lib->config_terminal_cmd__X, &config_terminal_cmd_lib, sizeof(config_terminal_cmd_lib));
    memcpy(&__activeVars_lib->accept_lifetime_duration_month_day_cmd__X, &accept_lifetime_duration_month_day_cmd_lib, sizeof(accept_lifetime_duration_month_day_cmd_lib));
    memcpy(&__activeVars_lib->banner_motd_default_cmd__X, &banner_motd_default_cmd_lib, sizeof(banner_motd_default_cmd_lib));
    memcpy(&__activeVars_lib->no_vty_access_class_cmd__X, &no_vty_access_class_cmd_lib, sizeof(no_vty_access_class_cmd_lib));
    memcpy(&__activeVars_lib->show_memory_ospf6_cmd__X, &show_memory_ospf6_cmd_lib, sizeof(show_memory_ospf6_cmd_lib));
    memcpy(&__activeVars_lib->show_ip_prefix_list_cmd__X, &show_ip_prefix_list_cmd_lib, sizeof(show_ip_prefix_list_cmd_lib));
    memcpy(&__activeVars_lib->ip_prefix_list_sequence_number_cmd__X, &ip_prefix_list_sequence_number_cmd_lib, sizeof(ip_prefix_list_sequence_number_cmd_lib));
    memcpy(&__activeVars_lib->no_access_list_exact_cmd__X, &no_access_list_exact_cmd_lib, sizeof(no_access_list_exact_cmd_lib));
    memcpy(&__activeVars_lib->config_write_memory_cmd__X, &config_write_memory_cmd_lib, sizeof(config_write_memory_cmd_lib));
    memcpy(&__activeVars_lib->config_log_record_priority_cmd__X, &config_log_record_priority_cmd_lib, sizeof(config_log_record_priority_cmd_lib));
    memcpy(&__activeVars_lib->show_ip_prefix_list_prefix_cmd__X, &show_ip_prefix_list_prefix_cmd_lib, sizeof(show_ip_prefix_list_prefix_cmd_lib));
    memcpy(&__activeVars_lib->show_version_cmd__X, &show_version_cmd_lib, sizeof(show_version_cmd_lib));
    memcpy(&__activeVars_lib->enable_password_text_cmd__X, &enable_password_text_cmd_lib, sizeof(enable_password_text_cmd_lib));
    memcpy(&__activeVars_lib->config_list_cmd__X, &config_list_cmd_lib, sizeof(config_list_cmd_lib));
    memcpy(&__activeVars_lib->config_exit_cmd__X, &config_exit_cmd_lib, sizeof(config_exit_cmd_lib));
    memcpy(&__activeVars_lib->show_logging_cmd__X, &show_logging_cmd_lib, sizeof(show_logging_cmd_lib));
    memcpy(&__activeVars_lib->key_string_cmd__X, &key_string_cmd_lib, sizeof(key_string_cmd_lib));
    memcpy(&__activeVars_lib->access_list_extended_any_mask_cmd__X, &access_list_extended_any_mask_cmd_lib, sizeof(access_list_extended_any_mask_cmd_lib));
    memcpy(&__activeVars_lib->interface_cmd__X, &interface_cmd_lib, sizeof(interface_cmd_lib));
    memcpy(&__activeVars_lib->access_list_standard_cmd__X, &access_list_standard_cmd_lib, sizeof(access_list_standard_cmd_lib));
    memcpy(&__activeVars_lib->distribute_list_prefix_all_cmd__X, &distribute_list_prefix_all_cmd_lib, sizeof(distribute_list_prefix_all_cmd_lib));
    memcpy(&__activeVars_lib->access_list_extended_mask_host_cmd__X, &access_list_extended_mask_host_cmd_lib, sizeof(access_list_extended_mask_host_cmd_lib));
    memcpy(&__activeVars_lib->access_list_cmd__X, &access_list_cmd_lib, sizeof(access_list_cmd_lib));
    memcpy(&__activeVars_lib->config_log_stdout_level_cmd__X, &config_log_stdout_level_cmd_lib, sizeof(config_log_stdout_level_cmd_lib));
    memcpy(&__activeVars_lib->show_ip_access_list_name_cmd__X, &show_ip_access_list_name_cmd_lib, sizeof(show_ip_access_list_name_cmd_lib));
    memcpy(&__activeVars_lib->no_config_log_stdout_cmd__X, &no_config_log_stdout_cmd_lib, sizeof(no_config_log_stdout_cmd_lib));
    memcpy(&__activeVars_lib->access_list_any_cmd__X, &access_list_any_cmd_lib, sizeof(access_list_any_cmd_lib));
    memcpy(&__activeVars_lib->config_log_trap_cmd__X, &config_log_trap_cmd_lib, sizeof(config_log_trap_cmd_lib));
    memcpy(&__activeVars_lib->no_ip_prefix_list_cmd__X, &no_ip_prefix_list_cmd_lib, sizeof(no_ip_prefix_list_cmd_lib));
    memcpy(&__activeVars_lib->no_access_list_extended_host_host_cmd__X, &no_access_list_extended_host_host_cmd_lib, sizeof(no_access_list_extended_host_host_cmd_lib));
    memcpy(&__activeVars_lib->hostname_cmd__X, &hostname_cmd_lib, sizeof(hostname_cmd_lib));
    memcpy(&__activeVars_lib->no_distribute_list_prefix_all_cmd__X, &no_distribute_list_prefix_all_cmd_lib, sizeof(no_distribute_list_prefix_all_cmd_lib));
    memcpy(&__activeVars_lib->password_cmd__X, &password_cmd_lib, sizeof(password_cmd_lib));
    memcpy(&__activeVars_lib->accept_lifetime_month_day_month_day_cmd__X, &accept_lifetime_month_day_month_day_cmd_lib, sizeof(accept_lifetime_month_day_month_day_cmd_lib));
    memcpy(&__activeVars_lib->no_access_list_extended_any_any_cmd__X, &no_access_list_extended_any_any_cmd_lib, sizeof(no_access_list_extended_any_any_cmd_lib));
    memcpy(&__activeVars_lib->no_access_list_all_cmd__X, &no_access_list_all_cmd_lib, sizeof(no_access_list_all_cmd_lib));
    memcpy(&__activeVars_lib->no_access_list_any_cmd__X, &no_access_list_any_cmd_lib, sizeof(no_access_list_any_cmd_lib));
    memcpy(&__activeVars_lib->if_ipv6_rmap_cmd__X, &if_ipv6_rmap_cmd_lib, sizeof(if_ipv6_rmap_cmd_lib));
    memcpy(&__activeVars_lib->no_access_list_extended_host_mask_cmd__X, &no_access_list_extended_host_mask_cmd_lib, sizeof(no_access_list_extended_host_mask_cmd_lib));
    memcpy(&__activeVars_lib->access_list_standard_nomask_cmd__X, &access_list_standard_nomask_cmd_lib, sizeof(access_list_standard_nomask_cmd_lib));
    memcpy(&__activeVars_lib->ipv6_distribute_list_cmd__X, &ipv6_distribute_list_cmd_lib, sizeof(ipv6_distribute_list_cmd_lib));
    __activeVars_lib->no_password_check__X =  0;
    __activeVars_lib->telnet_backward_char__X =  0x08;
    memcpy(&__activeVars_lib->no_ip_prefix_list_seq_ge_le_cmd__X, &no_ip_prefix_list_seq_ge_le_cmd_lib, sizeof(no_ip_prefix_list_seq_ge_le_cmd_lib));
    memcpy(&__activeVars_lib->access_list_remark_cmd__X, &access_list_remark_cmd_lib, sizeof(access_list_remark_cmd_lib));
    memcpy(&__activeVars_lib->config_terminal_no_length_cmd__X, &config_terminal_no_length_cmd_lib, sizeof(config_terminal_no_length_cmd_lib));
    memcpy(&__activeVars_lib->ip_prefix_list_seq_le_ge_cmd__X, &ip_prefix_list_seq_le_ge_cmd_lib, sizeof(ip_prefix_list_seq_le_ge_cmd_lib));
    memcpy(&__activeVars_lib->config_node__X, &config_node__T, sizeof(config_node__T));
    memcpy(&__activeVars_lib->no_ip_prefix_list_seq_cmd__X, &no_ip_prefix_list_seq_cmd_lib, sizeof(no_ip_prefix_list_seq_cmd_lib));
    memcpy(&__activeVars_lib->rmap_onmatch_next_cmd__X, &rmap_onmatch_next_cmd_lib, sizeof(rmap_onmatch_next_cmd_lib));
    memcpy(&__activeVars_lib->no_rmap_continue_cmd__X, &no_rmap_continue_cmd_lib, sizeof(no_rmap_continue_cmd_lib));
    memcpy(&__activeVars_lib->ipv6_distribute_list_prefix_cmd__X, &ipv6_distribute_list_prefix_cmd_lib, sizeof(ipv6_distribute_list_prefix_cmd_lib));
    memcpy(&__activeVars_lib->access_list_extended_mask_any_cmd__X, &access_list_extended_mask_any_cmd_lib, sizeof(access_list_extended_mask_any_cmd_lib));
    memcpy(&__activeVars_lib->show_memory_rip_cmd__X, &show_memory_rip_cmd_lib, sizeof(show_memory_rip_cmd_lib));
    memcpy(&__activeVars_lib->no_ip_prefix_list_ge_cmd__X, &no_ip_prefix_list_ge_cmd_lib, sizeof(no_ip_prefix_list_ge_cmd_lib));
    memcpy(&__activeVars_lib->config_logmsg_cmd__X, &config_logmsg_cmd_lib, sizeof(config_logmsg_cmd_lib));
    __activeVars_lib->vty_cwd__X =  NULL;
    memcpy(&__activeVars_lib->no_rmap_onmatch_goto_cmd__X, &no_rmap_onmatch_goto_cmd_lib, sizeof(no_rmap_onmatch_goto_cmd_lib));
    memcpy(&__activeVars_lib->send_lifetime_duration_month_day_cmd__X, &send_lifetime_duration_month_day_cmd_lib, sizeof(send_lifetime_duration_month_day_cmd_lib));
    memcpy(&__activeVars_lib->rmap_call_cmd__X, &rmap_call_cmd_lib, sizeof(rmap_call_cmd_lib));
    memcpy(&__activeVars_lib->show_ip_prefix_list_prefix_longer_cmd__X, &show_ip_prefix_list_prefix_longer_cmd_lib, sizeof(show_ip_prefix_list_prefix_longer_cmd_lib));
    memcpy(&__activeVars_lib->access_list_standard_any_cmd__X, &access_list_standard_any_cmd_lib, sizeof(access_list_standard_any_cmd_lib));
    memcpy(&__activeVars_lib->ip_prefix_list_seq_cmd__X, &ip_prefix_list_seq_cmd_lib, sizeof(ip_prefix_list_seq_cmd_lib));
    memcpy(&__activeVars_lib->config_log_monitor_cmd__X, &config_log_monitor_cmd_lib, sizeof(config_log_monitor_cmd_lib));
    memcpy(&__activeVars_lib->no_ip_prefix_list_prefix_cmd__X, &no_ip_prefix_list_prefix_cmd_lib, sizeof(no_ip_prefix_list_prefix_cmd_lib));
    memcpy(&__activeVars_lib->show_ip_prefix_list_detail_name_cmd__X, &show_ip_prefix_list_detail_name_cmd_lib, sizeof(show_ip_prefix_list_detail_name_cmd_lib));
    memcpy(&__activeVars_lib->no_ipv6_distribute_list_all_cmd__X, &no_ipv6_distribute_list_all_cmd_lib, sizeof(no_ipv6_distribute_list_all_cmd_lib));
    memcpy(&__activeVars_lib->config_log_syslog_cmd__X, &config_log_syslog_cmd_lib, sizeof(config_log_syslog_cmd_lib));
    memcpy(&__activeVars_lib->access_master_ipv4__X, &access_master_ipv4__T, sizeof(access_master_ipv4__T));
    memcpy(&__activeVars_lib->distribute_list_prefix_cmd__X, &distribute_list_prefix_cmd_lib, sizeof(distribute_list_prefix_cmd_lib));
    __activeVars_lib->if_rmap_delete_hook__X =  NULL;
}
