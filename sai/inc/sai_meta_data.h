#pragma once

#include <iostream>
#include <list>
#include <map>
#include <saiextensions.h>
#include <vector>
#include <set>
#include <regex>
#include <optional>
#include <cstring>


#define NULL_HANDLE -1

// object pointer and it's id
class sai_id_map_t
{
public:
    sai_id_map_t()
    {
        // init
        unused_id.clear();
        id_map.clear();
    }

    ~sai_id_map_t()
    {
        free_all();
    }

    void free_id(sai_object_id_t sai_object_id);

    void free_all();

    sai_object_id_t get_new_id(void *obj_ptr)
    {
        sai_object_id_t id;
        if (!unused_id.empty()) {
            id = unused_id.back();
            unused_id.pop_back();
        } else {
            id = id_map.size() + 1;
        }
        id_map[id] = obj_ptr;
        return id;
    }

    void *get_object(sai_object_id_t sai_object_id)
    {
        return id_map[sai_object_id];
    }

    bool find_object(sai_object_id_t sai_object_id)
    {
        return id_map.find(sai_object_id) != id_map.end();
    }

    uint32_t get_ids(std::vector<sai_object_id_t> &ids) {
        for (const auto &it : id_map) {
            ids.push_back(it.first);
        }

        return ids.size();
    }

protected:
    std::map<sai_object_id_t, void *> id_map;
    std::vector<sai_object_id_t> unused_id;
};

class sai_obj
{
public:
    sai_obj(sai_id_map_t &sai_id_map, sai_object_type_t type)
    {
        sai_object_id = sai_id_map.get_new_id(this);
        sai_object_type = type;
    }

    virtual ~sai_obj()
    {
        // free_id(sai_object_id); TODO: fix this
    }

    // variable
    sai_object_id_t sai_object_id;
    sai_object_type_t sai_object_type;
};

class router_interface_obj : public sai_obj
{
public:
    router_interface_obj(sai_id_map_t &sai_id_map) :
        sai_obj(sai_id_map, SAI_OBJECT_TYPE_ROUTER_INTERFACE)
    {
        this->vid = 1;
        this->mac_valid = false;
        this->vid = 0;
        this->type = SAI_ROUTER_INTERFACE_TYPE_VLAN;
    }

    // variable
    sai_mac_t mac;
    bool mac_valid;
    uint16_t vid;
    sai_router_interface_type_t type;
};

// Base class for all OCS SAI objects (port / cross-connect / factory-data).
class ocs_obj : public sai_obj
{
public:
    ocs_obj(sai_id_map_t &sai_id_map, sai_object_type_extensions_t type) :
        sai_obj(sai_id_map, (sai_object_type_t)type)
    {
    }

    virtual ~ocs_obj() {}

    // SAI-facing name of the object (SAI_OCS_*_ATTR_NAME).
    std::string dev_name;
};

// SAI_OBJECT_TYPE_OCS_PORT
class ocs_port_obj : public ocs_obj
{
public:
    ocs_port_obj(sai_id_map_t &sai_id_map) :
        ocs_obj(sai_id_map, SAI_OBJECT_TYPE_OCS_PORT),
        override_state(SAI_OCS_PORT_OVERRIDE_STATE_NORMAL)
    {
    }

    // variable
    sai_ocs_port_override_state_t override_state;
};

// SAI_OBJECT_TYPE_OCS_CROSS_CONNECT
class ocs_cross_connect_obj : public ocs_obj
{
public:
    ocs_cross_connect_obj(sai_id_map_t &sai_id_map) :
        ocs_obj(sai_id_map, SAI_OBJECT_TYPE_OCS_CROSS_CONNECT),
        a_side_port_id(SAI_NULL_OBJECT_ID),
        b_side_port_id(SAI_NULL_OBJECT_ID)
    {
    }

    // variable
    sai_object_id_t a_side_port_id;
    sai_object_id_t b_side_port_id;
};

// SAI_OBJECT_TYPE_OCS_CROSS_CONNECT_FACTORY_DATA
class ocs_cross_connect_factory_data_obj : public ocs_obj
{
public:
    ocs_cross_connect_factory_data_obj(sai_id_map_t &sai_id_map) :
        ocs_obj(sai_id_map, SAI_OBJECT_TYPE_OCS_CROSS_CONNECT_FACTORY_DATA)
    {
    }
};

class switch_metadata
{ 
public:
    switch_metadata() : virtual_id(0x0F000000)
    {
        memset(default_switch_mac, 0, 6);

        sai_object_id_t id = virtual_id;
        vids[SAI_OBJECT_TYPE_VIRTUAL_ROUTER] = ++id;
        vids[SAI_OBJECT_TYPE_VLAN] = ++id;
        vids[SAI_OBJECT_TYPE_PORT] = ++id;
        vids[SAI_OBJECT_TYPE_BRIDGE] = ++id;
        vids[SAI_OBJECT_TYPE_HOSTIF_TRAP_GROUP] = ++id;
    }

    // variable
    const uint32_t virtual_id;
    sai_object_id_t switch_id;
    sai_mac_t default_switch_mac;
    sai_id_map_t sai_id_map;

    // id mapping for virtual devices
    std::map<sai_object_type_t, sai_object_id_t> vids;

};
