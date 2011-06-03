#include <stack>
#include "lwmpls_data.h"



LWMPLSDataStructure::LWMPLSDataStructure()
{
    /* reserva e inicializa las estructuras de datos usadas por el protocolo mpls ligero */


    LWMPLS_MAX_TIME = 15.0;
    LWMPLS_MAX_TIME_MAC = 0.5;
    /* timer limit of initialize forwarding struc after capture label */
    LWMPLS_MAX_TIME_CAPTURE_LABEL = 10;
    /* Limits for break path */
    /* Number of reintent before break */
    LWMPLS_MAX_RTR = 2;
    /* Timer after last message */

    forwardingTableOutput = new LWmplsFwMap;
    interfaceMap = new LWmplsInterfaceMap;
    num_label_in_use = 0;
    bad_pkt_rate = 0;
    broadCastCounter = 0;
}

LWMPLSDataStructure::~LWMPLSDataStructure()
{

    while (!interfaceMap->empty())
    {
        delete  interfaceMap->begin()->second;
        interfaceMap->erase(interfaceMap->begin());
    }
    for (unsigned int i=0; i<label_list.size(); i++)
    {
        if (label_list[i].data_f_ptr==NULL) continue;
        LWmpls_Forwarding_Structure *aux = label_list[i].data_f_ptr;
        deleteForwarding(aux);
    }
    label_list.clear();
    forwardingMacKey.clear();

    broadCastList.clear();
    destList.clear();

    forwardingTableOutput->clear();

    delete forwardingTableOutput;
    delete interfaceMap;
}



LWmpls_Forwarding_Structure * LWMPLSDataStructure::lwmpls_forwarding_data(int label_input, int label_output, uint64_t mac_dest)
{
    LWMPLSKey key;
    LWmpls_Forwarding_Structure * data_f_ptr = NULL;
    /* extrae la estructura de datos correspondiente al label, en esta estrutrua est� la informaci�n de que hacer  */
    /* y cual es el siguiente label */

    if (label_input>0)
    {

        if ((unsigned int)label_input > label_list.size())
            return (data_f_ptr);

        if (label_list[label_input-1].in_use)
        {
            data_f_ptr = label_list[label_input-1].data_f_ptr;
            if (data_f_ptr!=NULL)
            {
                simtime_t actual_time = simTime();
                if ((actual_time - data_f_ptr->last_use)>data_f_ptr->label_life_limit)
                {
                    data_f_ptr = lwmpls_interface_delete_label(label_input);
                    //delLWMPLSLabel(data_f_ptr->input_label);
                    //delLWMPLSLabel(data_f_ptr->return_label_input);
                    //forwardingTableOutput->erase (data_f_ptr->key_output);
                    //forwardingTableOutput->erase (data_f_ptr->return_key_output);
                    deleteForwarding(data_f_ptr);
                    data_f_ptr = NULL;
                }
                else
                {
                    if ((data_f_ptr->input_label!=label_input) && (data_f_ptr->return_label_input!=label_input))
                    {
                        printf("\n %d %d %d", label_input, data_f_ptr->input_label, data_f_ptr->return_label_input);
                        opp_error("lwmpls_forwarding_data Error in label database, label in use but struct not correct");
                    }
                }
            }
            else
                label_list[label_input-1].in_use = false;
        }
        else if (!label_list[label_input-1].in_use && label_list[label_input-1].data_f_ptr!=NULL)
        {
            opp_error("lwmpls_forwarding_data label no usada pero existe estructura");
        }
    }
    else if (label_output>0)
    {
        key.label = label_output;
        key.mac_addr = mac_dest;
        LWmplsFwMap::iterator i = forwardingTableOutput->find(key);
        if (i!=forwardingTableOutput->end())
            data_f_ptr = i->second;
        if (data_f_ptr!=NULL)
        {
            /* Check label */
            if ((data_f_ptr->return_label_output == label_output && data_f_ptr->input_mac_address == mac_dest) ||
                    (data_f_ptr->output_label == label_output && data_f_ptr->mac_address == mac_dest))
                return (data_f_ptr);
            else
            {
                deleteForwarding(data_f_ptr);
                forwardingTableOutput->erase(i);
                data_f_ptr = NULL;
                printf("lwmpls_forwarding_data error in forwarding_table_output");
            }
        }
    }
    return data_f_ptr;
}



LWmpls_Interface_Structure * LWMPLSDataStructure::lwmpls_interface_structure(uint64_t mac_addr)
{
    /* Extrae la estructura del mac donde figura la lista de todos los label que lo usan */
    if (mac_addr==(uint64_t)0)
        return NULL;

    LWmplsInterfaceMap::iterator  macIterator = interfaceMap->find(mac_addr);
    if (macIterator!=interfaceMap->end())
        if (macIterator->second->mac_address==mac_addr)
            return  macIterator->second;
    return NULL;
}

void LWMPLSDataStructure::lwmpls_interface_delete_list_mpls(uint64_t mac_addr)
{

    LWmpls_Forwarding_Structure *data_f_ptr = NULL;
    simtime_t actual_time = simTime();

    /* borra todas las estructuras de datos correspondientes a las etiquetas que usan la direcci�n mac como */
    /* siguiente salto */

    if (mac_addr==0)
        return;


    LWmplsInterfaceMap::iterator  macIterator = interfaceMap->find(mac_addr);
    if (macIterator!=interfaceMap->end())
    {
        delete  macIterator->second;
        interfaceMap->erase(macIterator);
    }

    for (unsigned int i = 1; i<-label_list.size(); i++)
    {
        if (label_list[i-1].in_use)
        {
            data_f_ptr = label_list[i-1].data_f_ptr;
            if (data_f_ptr == NULL)
            {
                delLWMPLSLabel(i);
                continue;
            }

            if ((mac_addr!=data_f_ptr->mac_address) && (mac_addr!=data_f_ptr->input_mac_address))
            {
                deleteForwarding(data_f_ptr);
                data_f_ptr = NULL;
            }
        }
    }

// is necessary this test?
    LWmplsFwMap::iterator it;
    std::stack<LWmplsFwMap::iterator> mystack;
    // Check integrity
    for ( it=forwardingTableOutput->begin(); it != forwardingTableOutput->end(); it++ )
    {
        if ((it->second->mac_address == mac_addr) || (it->second->input_mac_address == mac_addr))
        {
            mystack.push(it);
            printf("\n !!!!!!!!!!! test integrity !!!!!!!!!!!\n");
        }
    }

    while (!mystack.empty())
    {
        it = mystack.top();
        mystack.pop();
        delLWMPLSLabel(it->second->input_label);
        delLWMPLSLabel(it->second->return_label_input);
        forwardingTableOutput->erase(it->second->key_output);
        forwardingTableOutput->erase(it->second->return_key_output);
        forwardingTableOutput->erase(it);
    }
}

int LWMPLSDataStructure::getLWMPLSLabel()
{
    simtime_t min_time;
    int label = -1;
    LWmpls_Forwarding_Structure* data_f_ptr = NULL;
    simtime_t actual_time = simTime();
    min_time = min_time.getMaxTime();

    for (unsigned int i=1; i<=label_list.size(); i++)
    {
        if ( label_list[i-1].in_use==false)
        {
            if (((label_list[i-1].time==0) || (actual_time-label_list[i-1].time>LWMPLS_MIN_REUSE_TIME)) && label != -1)
            {
                label = i;
            }
        }
        else
        {
            if (label_list[i-1].data_f_ptr!=NULL)
            {
                if ((actual_time-label_list[i-1].data_f_ptr->last_use)>label_list[i-1].data_f_ptr->label_life_limit)
                {
                    data_f_ptr = lwmpls_interface_delete_label(i);
                    deleteForwarding(data_f_ptr);
                    //label_list[i-1].data_f_ptr=NULL;
                    data_f_ptr = NULL;
                }
            }
            else
            {
                if (actual_time-label_list[i-1].capture_time>LWMPLS_MAX_TIME_CAPTURE_LABEL)
                    delLWMPLSLabel(i);
            }
        }
    }
    if (label!=-1)
    {
        label_list[label-1].in_use = true;
        num_label_in_use++;
        label_list[label-1].data_f_ptr = NULL;
        label_list[label-1].capture_time = actual_time;
    }
    else
    {
        if (label_list.size()<LWMPLS_MAX_LABEL-1)
        {
            LWmpls_label_list newData;
            newData.in_use = true;
            num_label_in_use++;
            newData.data_f_ptr = NULL;
            newData.capture_time = actual_time;
            label_list.push_back(newData);
            label = label_list.size();
        }
        else
        {
            for (unsigned int i=1; i<=label_list.size(); i++)
                if (!label_list[i-1].in_use)
                    if (min_time>label_list[i-1].time)
                    {
                        label = i;
                        min_time = label_list[i-1].time;
                    }
        }
    }
// purge unnecessary list elements
    while (label_list[label_list.size()-1].in_use == false &&
            actual_time-label_list[label_list.size()].time>LWMPLS_MIN_REUSE_TIME )
    {
        label_list.pop_back();

    }
    return label;
}

bool LWMPLSDataStructure::lwmpls_label_in_use(int label)
{
    bool in_use = false;
    LWmpls_Forwarding_Structure* data_f_ptr = NULL;

    if ((unsigned int) label > label_list.size() || label<=0)
        return false;
    /* indica si la etiqueta esta en uso comprueba si la etiqueta ha caducado por llevar demasiado tiempo sin usarse*/
    if (label_list[label-1].in_use==true)
    {
        simtime_t actual_time = simTime();
        if (label_list[label-1].data_f_ptr)
        {
            if ((actual_time - label_list[label-1].data_f_ptr->last_use)>label_list[label-1].data_f_ptr->label_life_limit)
            {
                data_f_ptr = lwmpls_interface_delete_label(label);
                deleteForwarding(data_f_ptr);
                //label_list[label-1].data_f_ptr=NULL;
            }
            else
            {
                in_use = true;
                if ((label_list[label-1].data_f_ptr->input_label!=label) && (label_list[label-1].data_f_ptr->return_label_input!=label))
                {
                    printf("\n %d %d %d", label, label_list[label-1].data_f_ptr->input_label, label_list[label-1].data_f_ptr->return_label_input);
                    opp_error("lwmpls_label_in_use Error in label database, label in use but struct not correct");
                }
            }
        }
        else
        {
            delLWMPLSLabel(label);
        }
    }
    return in_use;
}



void LWMPLSDataStructure::delLWMPLSLabel(int label)
{

    simtime_t actual_time = simTime();
    LWmpls_Forwarding_Structure *data_f_ptr = NULL;

    if ((unsigned int) label > label_list.size())
        return;
    if (label<=0)
        return;
    if (label_list[label-1].in_use)
    { /* label are in use, free label*/
        num_label_in_use--;
        label_list[label-1].in_use = false;
        label_list[label-1].time = actual_time;

        if (label_list[label-1].data_f_ptr!=NULL)
        {
            data_f_ptr = label_list[label-1].data_f_ptr;
            if ((data_f_ptr->input_label != label) && (data_f_ptr->return_label_input!=label))
                opp_error("delLWMPLSLabel error in label data");

            LWmplsInterfaceMap::iterator  macIterator;
            if (data_f_ptr->input_label == label)
                macIterator = interfaceMap->find(data_f_ptr->input_mac_address);
            else if (data_f_ptr->return_label_input == label)
                macIterator = interfaceMap->find(data_f_ptr->mac_address);
            else
                macIterator = interfaceMap->end();

            if (macIterator!=interfaceMap->end())
            {
                if (macIterator->second->numLabels()>0)
                    macIterator->second->numLabels()--;

                if (macIterator->second->numLabels()==0)
                {
                    delete   macIterator->second;
                    interfaceMap->erase(macIterator);
                }

            }

        }
        if (num_label_in_use<0)
            opp_error("LWMPLS label number in use less than 0");
    }
    label_list[label-1].data_f_ptr = NULL;
    label_list[label-1].in_use = false;
}

void LWMPLSDataStructure::lwmpls_init_interface(LWmpls_Interface_Structure** interface_str_ptr_ptr, int label_in, uint64_t mac_addr, int type)
{
    bool init;
    LWmpls_Interface_Structure * interface_str_ptr = NULL;


    if (mac_addr==(uint64_t)0)
        return;

    LWmplsInterfaceMap::iterator it = interfaceMap->find(mac_addr);
    if (it!=interfaceMap->end())
        interface_str_ptr = it->second;
    else
        interface_str_ptr = NULL;

    init = false;
    if (*interface_str_ptr_ptr!=NULL && interface_str_ptr!=NULL && *interface_str_ptr_ptr!=interface_str_ptr)
        opp_error(" ERROR INTERFACE 1");
    else if (interface_str_ptr!=NULL && *interface_str_ptr_ptr == NULL)
        *interface_str_ptr_ptr = interface_str_ptr;

    if (interface_str_ptr==NULL)/* no existe crear */
    {
        interface_str_ptr = new LWmpls_Interface_Structure;
        *interface_str_ptr_ptr = interface_str_ptr;
        interface_str_ptr->mac_address = mac_addr;
        init = true;
    }
    else
    {
        if (interface_str_ptr->mac_address!=mac_addr)
        {
            opp_error("lwmpls_init_interface initial data different 2");
        }
    }

    interface_str_ptr->num_rtr = 0;

    if (*interface_str_ptr_ptr!=interface_str_ptr)
    {
        printf(" ERROR INTERFACE 3");
        opp_error("lwmpls_initialize_interface ");
    }

    if (type!=-1)
    {
        interface_str_ptr->numLabels()++;

    }
    if (init)
    {
        interface_str_ptr->last_use = simTime();
        interfaceMap->insert(std::pair<uint64_t,LWmpls_Interface_Structure *>(mac_addr, interface_str_ptr));
    }
}



void LWMPLSDataStructure::lwmpls_forwarding_input_data_add(int label, LWmpls_Forwarding_Structure *data_f_ptr)
{

    if (label<=0)
        opp_error("lwmpls_forwarding_input_data_add Error in label label <=0 ");
    if ((unsigned int)label > label_list.size())
        opp_error("lwmpls_forwarding_input_data_add Error in label label label > LWMPLS_MAX_LABEL");


    if ((data_f_ptr->input_label!=label) && (data_f_ptr->return_label_input!=label))
        opp_error("lwmpls_forwarding_input_data_add Error in label database, label exist but struct not correct %d %d %d", label, data_f_ptr->input_label, data_f_ptr->return_label_input);

    if (label_list[label-1].in_use==true)
    {
        if (label_list[label-1].data_f_ptr==NULL)
            label_list[label-1].data_f_ptr = data_f_ptr;
        else
        {
            opp_error("lwmpls_forwarding_input_data_add Error in label database, data not null  %d %d %d", label, data_f_ptr->input_label, data_f_ptr->return_label_input);
        }
    }
    else
    {
        label_list[label-1].in_use = true;
        label_list[label-1].data_f_ptr = data_f_ptr;
        label_list[label-1].capture_time = simTime();
        num_label_in_use++;
    }
}

bool LWMPLSDataStructure::lwmpls_forwarding_output_data_add(int label, uint64_t mac_addr, LWmpls_Forwarding_Structure *data_f_ptr, bool is_return)
{
    LWmpls_Forwarding_Structure * old_contents_ptr = NULL;
    LWMPLSKey key;

    if (mac_addr==(uint64_t)0)
        opp_error("Error mac not exist");

    key.label = label;
    key.mac_addr = mac_addr;

    if (is_return)
        data_f_ptr->return_key_output = key;
    else
        data_f_ptr->key_output = key;

    std::pair<LWmplsFwMap::iterator, bool > pr;
    pr = forwardingTableOutput->insert(std::pair<LWMPLSKey, LWmpls_Forwarding_Structure*>(key, data_f_ptr));
    if (pr.second == false )
    {
        old_contents_ptr = (pr.first)->second;
        if (old_contents_ptr!=data_f_ptr)
        {
            delLWMPLSLabel(data_f_ptr->input_label);
            delLWMPLSLabel(data_f_ptr->return_label_input);
            LWmplsFwMap::iterator it;
            it = forwardingTableOutput->find(data_f_ptr->key_output);
            if (it!=forwardingTableOutput->end() && it->second != data_f_ptr)
                deleteForwarding(it->second);
            if (it!=forwardingTableOutput->end())
                forwardingTableOutput->erase(it);
            it = forwardingTableOutput->find(data_f_ptr->return_key_output);
            if (it!=forwardingTableOutput->end() && it->second != data_f_ptr)
                deleteForwarding(it->second);
            if (it!=forwardingTableOutput->end())
                forwardingTableOutput->erase(it);
            if (old_contents_ptr)
            {
                delLWMPLSLabel(old_contents_ptr->input_label);
                delLWMPLSLabel(old_contents_ptr->return_label_input);
                it = forwardingTableOutput->find(old_contents_ptr->key_output);
                if (it!=forwardingTableOutput->end() && it->second != old_contents_ptr)
                    deleteForwarding(it->second);
                if (it!=forwardingTableOutput->end())
                    forwardingTableOutput->erase(it);
                it = forwardingTableOutput->find(old_contents_ptr->return_key_output);
                if (it!=forwardingTableOutput->end() && it->second != old_contents_ptr)
                    deleteForwarding(it->second);
                if (it!=forwardingTableOutput->end())
                    forwardingTableOutput->erase(it);
            }
            return false;
        }

    }
    return true;
}

LWmpls_Forwarding_Structure * LWMPLSDataStructure::lwmpls_interface_delete_label(int label)
{
    LWmpls_Forwarding_Structure *data_f_ptr = NULL;
    LWmpls_Interface_Structure *data_interface_ptr = NULL;

    if (label<=0)
        return NULL;
    if ((unsigned int)label > label_list.size())
        return NULL;

    if (!label_list[label-1].in_use)
    {
        if (label_list[label-1].data_f_ptr!=NULL)
            opp_error("lwmpls_interface_delete_label label not used but struct not null");
        return NULL;
    }
    else
        data_f_ptr = label_list[label-1].data_f_ptr;

    if (data_f_ptr!=NULL)
    {
        /* clean mac struct data  */
        if ((data_f_ptr->input_label != label) && (data_f_ptr->return_label_input!=label))
            opp_error("lwmpls_interface_delete_label error in label data");

        delLWMPLSLabel(label);
        delLWMPLSLabel(data_f_ptr->return_label_input);
        delLWMPLSLabel(data_f_ptr->input_label);

        LWmplsFwMap::iterator it2;
        it2 = forwardingTableOutput->find(data_f_ptr->key_output);
        LWmpls_Forwarding_Structure *data_aux_ptr = NULL;
        if (it2!=forwardingTableOutput->end())
        {
            data_aux_ptr = it2->second;
            forwardingTableOutput->erase(it2);
            if (data_aux_ptr!=data_f_ptr)
                opp_error("lwmpls_interface_delete_label error in label data 2");
        }

        it2 = forwardingTableOutput->find(data_f_ptr->return_key_output);
        if (it2!=forwardingTableOutput->end())
        {
            data_aux_ptr = it2->second;
            forwardingTableOutput->erase(it2);
            if (data_aux_ptr!=data_f_ptr)
                opp_error("lwmpls_interface_delete_label error in label data 2");

        }
        if (data_f_ptr->mac_address!=0)
        {
            LWmplsInterfaceMap::iterator it = interfaceMap->find(data_f_ptr->mac_address);
            if (it!=interfaceMap->end())
            {
                data_interface_ptr = it->second;
                unsigned int numLabel = data_interface_ptr->numLabels();
                if (numLabel > 0)
                    numLabel--;
                data_interface_ptr->numLabels() = numLabel;
                if (data_interface_ptr->numLabels()==0)
                {
                    interfaceMap->erase(it);
                    delete data_interface_ptr;
                }
            }
        }

        if (data_f_ptr->input_mac_address!=0)
        {
            LWmplsInterfaceMap::iterator it = interfaceMap->find(data_f_ptr->input_mac_address);
            if (it!=interfaceMap->end())
            {
                data_interface_ptr = it->second;
                unsigned int numLabel = data_interface_ptr->numLabels();
                if (numLabel > 0)
                    numLabel--;
                data_interface_ptr->numLabels() = numLabel;
                if (data_interface_ptr->numLabels()==0)
                {
                    interfaceMap->erase(it);
                    delete data_interface_ptr;
                }
            }
        }

        if (num_label_in_use<0)
            opp_error("LWMPLS label number in use less than 0");


    }
    else
        delLWMPLSLabel(label);
    /* */
    return  data_f_ptr;
}



void LWMPLSDataStructure::lwmpls_interface_delete_old_path()
{
    LWmpls_Interface_Structure *mac_struct_ptr = NULL;
    LWmpls_Forwarding_Structure *data_f_ptr = NULL;
    simtime_t time;

    time = simTime();

    if (num_label_in_use==0)
        return;


    for (unsigned int i=1; i<=label_list.size(); i++)
    {
        if (!label_list[i-1].in_use)
        {
            label_list[i-1].data_f_ptr = NULL;
            continue;
        }
        data_f_ptr = label_list[i-1].data_f_ptr;
        if (data_f_ptr==NULL)
        {
            delLWMPLSLabel(i);
            continue;
        }

        if (data_f_ptr->input_label!=(int) i && data_f_ptr->return_label_input!=(int) i)
        {
            delLWMPLSLabel(i);
            continue;
        }

        if (time-data_f_ptr->last_use > data_f_ptr->label_life_limit)
        {
            if (data_f_ptr->mac_address!=0)
            {
                LWmplsInterfaceMap::iterator it = interfaceMap->find(data_f_ptr->mac_address);
                if (it!=interfaceMap->end())
                {
                    mac_struct_ptr = it->second;
                    if (mac_struct_ptr->numLabels() > 0)
                        mac_struct_ptr->numLabels() --;
                    if (mac_struct_ptr->numLabels()==0)
                    {
                        interfaceMap->erase(it);
                        delete mac_struct_ptr;
                        mac_struct_ptr = NULL;
                    }

                }
            }

            if (data_f_ptr->input_mac_address!=0)
            {
                LWmplsInterfaceMap::iterator it = interfaceMap->find(data_f_ptr->input_mac_address);
                if (it!=interfaceMap->end())
                {

                    mac_struct_ptr = it->second;
                    if (mac_struct_ptr->numLabels() > 0)
                        mac_struct_ptr->numLabels() --;
                    if (mac_struct_ptr->numLabels()==0)
                    {
                        interfaceMap->erase(it);
                        delete mac_struct_ptr;
                        mac_struct_ptr = NULL;
                    }

                }
            }
            deleteForwarding(data_f_ptr);
            label_list[i-1].data_f_ptr = NULL;
            label_list[i-1].in_use = false;
        }
    }
}



bool LWMPLSDataStructure::lwmpls_mac_last_access(simtime_t &time, uint64_t addr)
{

    /* Extrae la estructura del mac donde figura la lista de todos los label que lo usan */
    time = -1;
    LWmplsInterfaceMap::iterator it = interfaceMap->find(addr);
    if (it!=interfaceMap->end())
    {
        time = it->second->last_use;
        return true;
    }
    return false;
}


void  LWMPLSDataStructure::lwmpls_refresh_mac(uint64_t mac, simtime_t time)
{
    LWmplsInterfaceMap::iterator it = interfaceMap->find(mac);
    if (it!=interfaceMap->end())
    {
        it->second->last_use = time;
        it->second->num_rtr = 0;
    }
}


int LWMPLSDataStructure::lwmpls_nun_labels_in_use()
{
    return num_label_in_use;
}


double LWMPLSDataStructure::lwmpls_label_last_use(int label)
{
    if (lwmpls_label_in_use(label)==true)
    {
        return SIMTIME_DBL(label_list[label-1].data_f_ptr->last_use);
    }
    return -1;
}


int LWMPLSDataStructure::lwmpls_label_status(int label)
{
    int in_use = LWMPLS_STATUS_NOT_USE;
    LWmpls_Forwarding_Structure* data_f_ptr = NULL;

    if ((label<=0) || ((unsigned int)label > label_list.size()))
        return in_use;

    /* indica si la etiqueta esta en uso comprueba si la etiqueta ha caducado por llevar demasiado tiempo sin usarse*/
    if (label_list[label-1].in_use)
    {
        if (label_list[label-1].data_f_ptr!=NULL)
        {
            if (label_list[label-1].data_f_ptr->input_label!=label && label_list[label-1].data_f_ptr->return_label_input!=label)
            {
                delLWMPLSLabel(label);
                return in_use;
            }

            if ((simTime()-label_list[label-1].data_f_ptr->last_use)>label_list[label-1].data_f_ptr->label_life_limit)
            {
                data_f_ptr = lwmpls_interface_delete_label(label);
                label_list[label-1].data_f_ptr = NULL;
                deleteForwarding(data_f_ptr);
            }
            else
            {
                if (((label_list[label-1].data_f_ptr->input_label==label) && (label_list[label-1].data_f_ptr->output_label==0)) ||
                        ((label_list[label-1].data_f_ptr->return_label_input==label) && (label_list[label-1].data_f_ptr->return_label_output==0)))
//              if ((label_list[label-1].data_f_ptr->input_label==label) && (label_list[label-1].data_f_ptr->return_label_output==0) ||
//                  (label_list[label-1].data_f_ptr->return_label_input==label) && (label_list[label-1].data_f_ptr->output_label==0))
                    in_use = LWMPLS_STATUS_PROC;
                else
                    in_use = LWMPLS_STATUS_STBL;
            }
        }
        else
        {
            delLWMPLSLabel(label);

        }
    }
    return in_use;
}



void LWMPLSDataStructure::lwmpls_check_label(int label, const char * message)
{
    LWmpls_Forwarding_Structure* data_f_ptr = NULL;

    if (((unsigned int)label > label_list.size()) || (label<=0))
        return;
    if (label_list[label-1].data_f_ptr!=NULL)
    {
        data_f_ptr = label_list[label-1].data_f_ptr;
        //printf(" %p \n",data_f_ptr);
        if ((data_f_ptr->input_label!=label) && (data_f_ptr->return_label_input!=label))
        {
            printf("\n %s %d %d %d %p", message, label, data_f_ptr->input_label, data_f_ptr->return_label_input, data_f_ptr);
            opp_error("lwmpls_check_label Error in label database, label exist but struct not correct");
        }
    }
    return;
}



void LWMPLSDataStructure::deleteRegisterRoute(uint64_t destination)
{
    DestinationList::iterator it = destList.find(destination);
    if (it!=destList.end())
        destList.erase(it);
}

void LWMPLSDataStructure::registerRoute(uint64_t destination, int label)
{
    deleteRegisterRoute(destination);
    destList.insert(std::pair<uint64_t,int>(destination, label));
}

int LWMPLSDataStructure::getRegisterRoute(uint64_t destination)
{
    DestinationList::iterator it = destList.find(destination);
    int label = -1;
    if (it!=destList.end())
    {

        //if ((*it).second.size()>0)
        //  label = (*it).second[0];
        label = (*it).second;
    }
    return label;
}


bool LWMPLSDataStructure::getBroadCastCounter(uint64_t destination, uint32_t &counter)
{
    bool find = false;
    BroadcastList::iterator it = broadCastList.find(destination);
    if (it!=broadCastList.end())
    {
        counter = (*it).second;
        find = true;
    }
    return find;
}

void LWMPLSDataStructure::setBroadCastCounter(uint64_t destination, uint32_t counter)
{
    BroadcastList::iterator it = broadCastList.find(destination);
    if (it!=broadCastList.end())
        (*it).second = counter;
    else
        broadCastList.insert(std::pair<uint64_t,uint64_t>(destination, counter));

}

void LWMPLSDataStructure::deleteForwarding(LWmpls_Forwarding_Structure* data_f_ptr)
{

    LWmpls_Forwarding_Structure* data_aux;

    delLWMPLSLabel(data_f_ptr->input_label);
    delLWMPLSLabel(data_f_ptr->return_label_input);

    LWmplsFwMap::iterator it = forwardingTableOutput->find(data_f_ptr->key_output);
    if (it!=forwardingTableOutput->end())
    {
        data_aux = it->second;
        forwardingTableOutput->erase(it);
        if (data_aux!=data_f_ptr)
            deleteForwarding(data_aux);
    }

    it = forwardingTableOutput->find(data_f_ptr->return_key_output);
    if (it!=forwardingTableOutput->end())
    {
        data_aux = it->second;
        forwardingTableOutput->erase(it);
        if (data_aux!=data_f_ptr)
            deleteForwarding(data_aux);
    }
    delete data_f_ptr;
    data_f_ptr = NULL;
}


void LWMPLSDataStructure::setForwardingMacKey(uint64_t srcMacAddrs, uint64_t dstMacAddrs, uint64_t prevMacAddr, uint64_t nextMacAddr, int32_t label)
{
    LWMPSMACAddressKey key;
    key.srcAddr = srcMacAddrs;
    key.destAddr = dstMacAddrs;
    key.prevAddr = prevMacAddr;
    key.label = label;
    LWmplsFwMacKey::iterator it = forwardingMacKey.find(key);
    if (it!=forwardingMacKey.end())
    {
        if (nextMacAddr!=0)
            it->second = nextMacAddr;
        else
            forwardingMacKey.erase(it);
    }
    else if (nextMacAddr!=0)
    {
        forwardingMacKey.insert(std::pair<LWMPSMACAddressKey,uint64_t>(key, nextMacAddr));
    }

}

uint64_t LWMPLSDataStructure::getForwardingMacKey(uint64_t srcMacAddrs, uint64_t dstMacAddrs, uint64_t prevMacAddr, int32_t label)
{
    LWMPSMACAddressKey key;

    key.srcAddr = srcMacAddrs;
    key.destAddr = dstMacAddrs;
    key.prevAddr = prevMacAddr;
    key.label = label;
    LWmplsFwMacKey::iterator it = forwardingMacKey.find(key);
    if (it!=forwardingMacKey.end())
    {
        return it->second;
    }
    else
        return (uint64_t) 0;
}

bool LWMPLSDataStructure::delForwardingMacKey(uint64_t srcMacAddrs, uint64_t dstMacAddrs, uint64_t prevMacAddr, int32_t label)
{
    LWMPSMACAddressKey key;

    key.srcAddr = srcMacAddrs;
    key.destAddr = dstMacAddrs;
    key.prevAddr = prevMacAddr;
    key.label = label;
    LWmplsFwMacKey::iterator it = forwardingMacKey.find(key);
    if (it!=forwardingMacKey.end())
    {
        forwardingMacKey.erase(it);
        return true;
    }
    return false;
}
