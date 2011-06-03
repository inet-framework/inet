#include "batman.h"

/* this function can be called when the daemon starts or at runtime */
void Batman::hna_local_task_add_ip(const Uint128 &ip_addr, uint16_t netmask, uint8_t route_action)
{
    Hna_task *hna_task;

    hna_task = new Hna_task;

    hna_task->addr = ip_addr;
    hna_task->netmask = netmask;
    hna_task->route_action = route_action;
    hna_chg_list.push_back(hna_task);
}


void Batman::hna_local_buffer_fill(void)
{

    hna_buff_local.clear();
    if (hna_list.empty())
        return;

    for (unsigned int i=0; i < hna_list.size(); i++)
    {
        BatmanHnaMsg aux;
        aux.addr = hna_list[i].addr;
        aux.netmask = hna_list[i].netmask;
        hna_buff_local.push_back(aux);
    }
}

void Batman::hna_local_task_exec(void)
{
    Hna_task *hna_task;
    Hna_local_entry *hna_local_entry;

    if (hna_chg_list.empty())
    return;

    while (!hna_chg_list.empty())
    {
        hna_task = hna_chg_list.front();
        hna_local_entry = NULL;
        bool found = false;

        for (HnaLocalEntryList::iterator it=hna_list.begin(); it!=hna_list.end(); it++)
        {
            hna_local_entry = &(*it);
            if ((hna_task->addr == hna_local_entry->addr) && (hna_task->netmask == hna_local_entry->netmask)) {
                found = true;
                if (hna_task->route_action == ROUTE_DEL) {
                    // EV << "Deleting HNA from announce network list: %s/%i\n", hna_addr_str, hna_task->netmask);
                    hna_local_update_routes(hna_local_entry, ROUTE_DEL);
                    hna_list.erase(it);
                } else {
                    // debug_output(3, "Can't add HNA - already announcing network: %s/%i\n", hna_addr_str, hna_task->netmask);
                }
                break;
            }
        }

        if (!found) {

            if (hna_task->route_action == ROUTE_ADD) {
                //debug_output(3, "Adding HNA to announce network list: %s/%i\n", hna_addr_str, hna_task->netmask);

                /* add node */
                //hna_local_entry = debugMalloc(sizeof(struct hna_local_entry), 702);
                Hna_local_entry hna_local_entry;
                hna_local_entry.addr = hna_task->addr;
                hna_local_entry.netmask = hna_task->netmask;
                hna_local_update_routes(&hna_local_entry, ROUTE_ADD);
                hna_list.push_back(hna_local_entry);
            } else {
                EV << "Can't delete HNA - network is not announced: ";
            }
        }

        hna_chg_list.erase(hna_chg_list.begin());
        delete hna_task;
    }
    /* rewrite local buffer */
    hna_local_buffer_fill();
}
/*
unsigned char *hna_local_update_vis_packet(unsigned char *vis_packet, uint16_t *vis_packet_size)
{
    struct list_head *list_pos;
    struct hna_local_entry *hna_local_entry;
    struct vis_data *vis_data;

    if (num_hna_local < 1)
        return vis_packet;

    list_for_each(list_pos, &hna_list) {
        hna_local_entry = list_entry(list_pos, struct hna_local_entry, list);

        *vis_packet_size += sizeof(struct vis_data);

        vis_packet = debugRealloc(vis_packet, *vis_packet_size, 107);
        vis_data = (struct vis_data *)(vis_packet + *vis_packet_size - sizeof(struct vis_data));

        memcpy(&vis_data->ip, (unsigned char *)&hna_local_entry->addr, 4);

        vis_data->data = hna_local_entry->netmask;
        vis_data->type = DATA_TYPE_HNA;
    }

    return vis_packet;
}
*/
void Batman::hna_local_update_routes(Hna_local_entry *hna_local_entry, int8_t route_action)
{
    /* add / delete throw routing entries for own hna */
//    add_del_route(hna_local_entry->addr, hna_local_entry->netmask, 0, 0, "unknown", BATMAN_RT_TABLE_NETWORKS, ROUTE_TYPE_THROW, route_action);
//    add_del_route(hna_local_entry->addr, hna_local_entry->netmask, 0, 0, "unknown", BATMAN_RT_TABLE_HOSTS, ROUTE_TYPE_THROW, route_action);
//    add_del_route(hna_local_entry->addr, hna_local_entry->netmask, 0, 0, "unknown", BATMAN_RT_TABLE_UNREACH, ROUTE_TYPE_THROW, route_action);
//    add_del_route(hna_local_entry->addr, hna_local_entry->netmask, 0, 0, "unknown", BATMAN_RT_TABLE_TUNNEL, ROUTE_TYPE_THROW, route_action);
    add_del_route(hna_local_entry->addr, hna_local_entry->netmask, 0, 0, NULL, BATMAN_RT_TABLE_TUNNEL, ROUTE_TYPE_THROW, route_action);
    /* do not NAT HNA networks automatically */
    //hna_local_update_nat(hna_local_entry->addr, hna_local_entry->netmask, route_action);
}

void Batman::_hna_global_add(OrigNode *orig_node, Hna_element *hna_element)
{
    Hna_global_entry *hna_global_entry;
    OrigNode *old_orig_node = NULL;

    std::map<BatmanHnaMsg,Hna_global_entry*>::iterator it;
    it = hnaMap.find(*hna_element);

    /* add the hna node if it does not exist */
    if (it == hnaMap.end())
    {
        hna_global_entry = new Hna_global_entry;
        hna_global_entry->addr = hna_element->addr;
        hna_global_entry->netmask = hna_element->netmask;
        hna_global_entry->curr_orig_node = NULL;
        hnaMap.insert(std::pair<BatmanHnaMsg,Hna_global_entry*>(*hna_element, hna_global_entry));
    }

    /* the given orig_node already is the current orig node for this HNA */
    if (hna_global_entry->curr_orig_node == orig_node)
        return;

    bool notFound = false;
    for (unsigned int i=0; i<hna_global_entry->orig_list.size(); i++)
    {
        if (hna_global_entry->orig_list[i] == orig_node)
        {
            notFound = true;
            break;
        }

    }
    /* append the given orig node to the list */
    if (notFound)
        hna_global_entry->orig_list.push_back(orig_node);

    /* our TQ value towards the HNA is better */
    if ((!hna_global_entry->curr_orig_node) ||
        (orig_node->router->tq_avg > hna_global_entry->curr_orig_node->router->tq_avg)) {

        old_orig_node = hna_global_entry->curr_orig_node;
        hna_global_entry->curr_orig_node = orig_node;

        /**
         * if we change the orig node towards the HNA we may still route via the same next hop
         * which does not require any routing table changes
         */
        if ((old_orig_node) &&
            (hna_global_entry->curr_orig_node->router->addr == old_orig_node->router->addr))
            return;

        add_del_route(hna_element->addr, hna_element->netmask, orig_node->router->addr,
                    orig_node->router->if_incoming->if_index,
                    orig_node->router->if_incoming->dev,
                    BATMAN_RT_TABLE_NETWORKS, ROUTE_TYPE_UNICAST, ROUTE_ADD);
    }

    /* delete previous route */
    if (old_orig_node) {
        add_del_route(hna_element->addr, hna_element->netmask, old_orig_node->router->addr,
                    old_orig_node->router->if_incoming->if_index,
                    old_orig_node->router->if_incoming->dev,
                    BATMAN_RT_TABLE_NETWORKS, ROUTE_TYPE_UNICAST, ROUTE_DEL);
    }
}

void Batman::_hna_global_del(OrigNode *orig_node, Hna_element *hna_element)
{
    Hna_global_entry *hna_global_entry;
    OrigNode * orig_ptr = NULL;

    std::map<BatmanHnaMsg,Hna_global_entry*>::iterator it;
    it = hnaMap.find(*hna_element);

    /* add the hna node if it does not exist */
    if (it == hnaMap.end())
        return;
    hna_global_entry = it->second;
    for (unsigned int i=0; i<hna_global_entry->orig_list.size();)
    {
        orig_ptr = hna_global_entry->orig_list[i];
        /* delete old entry in orig list */
        if (orig_ptr == orig_node) {
            hna_global_entry->orig_list.erase(hna_global_entry->orig_list.begin()+i);
            continue;
        }

        /* find best alternative route */
        if ((!hna_global_entry->curr_orig_node) ||
            (orig_ptr->router->tq_avg > hna_global_entry->curr_orig_node->router->tq_avg))
            hna_global_entry->curr_orig_node = hna_global_entry->orig_list[i];
        i++;
    }

    /* set new route if available */
    if (hna_global_entry->curr_orig_node) {
        /**
         * if we delete one orig node towards the HNA but we switch to an alternative
         * which is reachable via the same next hop no routing table changes are necessary
         */
        if (hna_global_entry->curr_orig_node->router->addr == orig_node->router->addr)
            return;

        add_del_route(hna_element->addr, hna_element->netmask, hna_global_entry->curr_orig_node->router->addr,
                    hna_global_entry->curr_orig_node->router->if_incoming->if_index,
                    hna_global_entry->curr_orig_node->router->if_incoming->dev,
                    BATMAN_RT_TABLE_NETWORKS, ROUTE_TYPE_UNICAST, ROUTE_ADD);
    }

    add_del_route(hna_element->addr, hna_element->netmask, orig_node->router->addr,
                orig_node->router->if_incoming->if_index,
                orig_node->router->if_incoming->dev,
                BATMAN_RT_TABLE_NETWORKS, ROUTE_TYPE_UNICAST, ROUTE_DEL);

    /* if no alternative route is available remove the HNA entry completely */
    if (!hna_global_entry->curr_orig_node) {
        hnaMap.erase(it);
        delete hna_global_entry;
    }
}

/* hna_buff_delete searches in buf if element e is found.
 *
 * if found, delete it from the buf and return 1.
 * if not found, return 0.
 */
int Batman::hna_buff_delete(std::vector<Hna_element *> &buf, int *buf_len, Hna_element *e)
{
    for (unsigned int i = 0; i < buf.size(); i++) {
        if (*(buf[i])==*e) {
            /* move last element forward */
            buf.erase(buf.begin()+i);
            *buf_len -= SIZE_Hna_element;
            return 1;
        }
    }
    return 0;
}

void Batman::hna_global_add(OrigNode *orig_node, BatmanHnaMsg *new_hna, int16_t new_hna_len)
{
    Hna_element *e;
    int i, num_elements;
    // clean buffer
    while (!orig_node->hna_buff.empty())
    {
        delete orig_node->hna_buff.back();
        orig_node->hna_buff.pop_back();
    }
    if ((new_hna == NULL) || (new_hna_len == 0))
    {
        return;
    }

    /* add new routes */
    num_elements = new_hna_len;
    //debug_output(4, "HNA information received (%i HNA network%s): \n", num_elements, (num_elements > 1 ? "s": ""));
    for (i = 0; i < num_elements; i++) {
        e = new_hna[i].dup();
        orig_node->hna_buff.push_back(e);
        /*
        addr_to_string(e->addr, hna_str, sizeof(hna_str));

        if ((e->netmask > 0 ) && (e->netmask < 33))
            debug_output(4, "hna: %s/%i\n", hna_str, e->netmask);
        else
            debug_output(4, "hna: %s/%i -> ignoring (invalid netmask) \n", hna_str, e->netmask);
*/
        if ((e->netmask > 0) && (e->netmask <= 32))
            _hna_global_add(orig_node, e);
    }
}
// HNA methods
/**
 * hna_global_update() replaces the old add_del_hna function. This function
 * updates the new hna buffer for the supplied orig node and
 * adds/deletes/updates the announced routes.
 *
 * Instead of first deleting and then adding, we try to add new routes
 * before delting the old ones so that the kernel will not experience
 * a situation where no route is present.
 */
void Batman::hna_global_update(OrigNode *orig_node, BatmanHnaMsg *new_hna, int16_t new_hna_len, NeighNode *old_router)
{
    Hna_element *e;
    Hna_global_entry *hna_global_entry;
    int i, num_elements, old_hna_len;
    std::vector<BatmanHnaMsg *> old_hna;

    /* orig node stopped announcing any networks */
    if ((!orig_node->hna_buff.empty()) && ((new_hna == NULL) || (new_hna_len == 0))) {
        hna_global_del(orig_node);
        return;
    }

    /* orig node started to announce networks */
    if ((orig_node->hna_buff.empty()) && ((new_hna != NULL) || (new_hna_len != 0))) {
        hna_global_add(orig_node, new_hna, new_hna_len);
        return;
    }

    /**
     * next hop router changed - no need to change the global hna hash
     * we just have to make sure that the best orig node is still in place
     * NOTE: we may miss a changed HNA here which we will update with the next packet
     */
    if (old_router != orig_node->router) {
        num_elements = orig_node->hna_buff.size();

        for (unsigned int i = 0; i < orig_node->hna_buff.size(); i++) {
            e = orig_node->hna_buff[i];

            if ((e->netmask < 1) || (e->netmask > 32))
                continue;

            std::map<BatmanHnaMsg,Hna_global_entry*>::iterator it;
            it = hnaMap.find(*e);

            /* add the hna node if it does not exist */
            if (it == hnaMap.end())
                return;
            hna_global_entry = it->second;


            if (!hna_global_entry)
                continue;

            /* if the given orig node is not in use no routes need to change */
            if (hna_global_entry->curr_orig_node != orig_node)
                continue;

            add_del_route(e->addr, e->netmask, orig_node->router->addr,
                    orig_node->router->if_incoming->if_index,
                    orig_node->router->if_incoming->dev,
                    BATMAN_RT_TABLE_NETWORKS, ROUTE_TYPE_UNICAST, ROUTE_ADD);

            add_del_route(e->addr, e->netmask, old_router->addr,
                old_router->if_incoming->if_index,
                old_router->if_incoming->dev,
                BATMAN_RT_TABLE_NETWORKS, ROUTE_TYPE_UNICAST, ROUTE_DEL);
        }

        return;
    }

    /**
     * check if the buffers even changed. if its still the same, there is no need to
     * update the routes. if the router changed, then we have to update all the routes
     * NOTE: no NULL pointer checking here because memcmp() just returns if n == 0
     */

    bool change = false;
    if ((int)orig_node->hna_buff.size() != new_hna_len)
        change = true;
    else
    {
        for (unsigned int i=0; i<orig_node->hna_buff.size(); i++)
        {
            if (*(orig_node->hna_buff[i]) != new_hna[i])
            {
                change = true;
                break;
            }
        }
    }

    if (!change)
        return;    /* nothing to do */

    /* changed HNA */
    old_hna = orig_node->hna_buff;

    orig_node->hna_buff.empty();
    num_elements = new_hna_len / SIZE_Hna_element;
    for (int i = 0; i < num_elements; i++) {
        BatmanHnaMsg *aux = &new_hna[i];
        BatmanHnaMsg *e = aux->dup();
        orig_node->hna_buff.push_back(e);
    }

    /* add new routes and keep old routes */
    for (i = 0; i < num_elements; i++) {
        e = orig_node->hna_buff[i];
        /**
         * if the router is the same, and the announcement was already in the old
         * buffer, we can keep the route.
         */
        if (hna_buff_delete(old_hna, &old_hna_len, e) == 0) {
            /* not found / deleted, need to add this new route */
            if ((e->netmask > 0) && (e->netmask <= 32))
                _hna_global_add(orig_node, e);
        }
    }

    /* old routes which are not to be kept are deleted now. */
    num_elements = old_hna_len / SIZE_Hna_element;
    for (unsigned int i = 0; i < old_hna.size(); i++) {
        BatmanHnaMsg *e = (old_hna[i]);
        if ((e->netmask > 0) && (e->netmask <= 32))
            _hna_global_del(orig_node, e);
    }

    /* dispose old hna buffer now. */
    while (!old_hna.empty())
    {
        delete old_hna.back();
        old_hna.pop_back();
    }
}

void Batman::hna_global_check_tq(OrigNode *orig_node)
{
    Hna_element *e;
    Hna_global_entry *hna_global_entry;
    int i, num_elements;

    if (orig_node->hna_buff.empty())
        return;

    num_elements = orig_node->hna_buff.size();
    for (i = 0; i < num_elements; i++) {
        e = orig_node->hna_buff[i];

        if ((e->netmask < 1) || (e->netmask > 32))
            continue;

        std::map<BatmanHnaMsg,Hna_global_entry*>::iterator it;
        it = hnaMap.find(*e);

        if (it==hnaMap.end())
            continue;

        hna_global_entry = it->second;
        /* if the given orig node is not in use no routes need to change */
        if (hna_global_entry->curr_orig_node == orig_node)
            continue;

        /* the TQ value has to better than the currently selected orig node */
        if (hna_global_entry->curr_orig_node->router->tq_avg > orig_node->router->tq_avg)
            continue;

        /**
         * if we change the orig node towards the HNA we may still route via the same next hop
         * which does not require any routing table changes
         */
        if (hna_global_entry->curr_orig_node->router->addr != orig_node->router->addr)
        {
            add_del_route(e->addr, e->netmask, orig_node->router->addr,
                    orig_node->router->if_incoming->if_index,
                    orig_node->router->if_incoming->dev,
                    BATMAN_RT_TABLE_NETWORKS, ROUTE_TYPE_UNICAST, ROUTE_ADD);

            add_del_route(e->addr, e->netmask, hna_global_entry->curr_orig_node->router->addr,
                    hna_global_entry->curr_orig_node->router->if_incoming->if_index,
                    hna_global_entry->curr_orig_node->router->if_incoming->dev,
                    BATMAN_RT_TABLE_NETWORKS, ROUTE_TYPE_UNICAST, ROUTE_DEL);
        }
        hna_global_entry->curr_orig_node = orig_node;
    }
}

void Batman::hna_global_del(OrigNode *orig_node)
{
    Hna_element *e;

    if (orig_node->hna_buff.empty())
        return;

    /* delete routes */
    while (!orig_node->hna_buff.empty())
    {
        e = orig_node->hna_buff.back();
        orig_node->hna_buff.pop_back();
        /* not found / deleted, need to add this new route */
        if ((e->netmask > 0) && (e->netmask <= 32))
            _hna_global_del(orig_node, e);
        delete e;
    }
}

void Batman::hna_free(void)
{
    Hna_local_entry *hna_local_entry;

    /* hna local */
    while (!hna_list.empty())
    {
        hna_local_entry = &hna_list.back();
        hna_local_update_routes(hna_local_entry, ROUTE_DEL);
        hna_list.pop_back();
    }
    while (!hnaMap.empty())
    {
        delete hnaMap.begin()->second;
        hnaMap.erase(hnaMap.begin());
    }
    hna_buff_local.clear();
}
