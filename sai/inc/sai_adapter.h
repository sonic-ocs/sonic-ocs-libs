#pragma once

// SAI
#ifdef __cplusplus
extern "C" {
#endif
#include "saiextensions.h"
#ifdef __cplusplus
}
#endif

#include "sai_meta_data.h"

// LOG
#include "logger.h"


#define CHECK_SWITCH_ID(id) \
do {\
    if ((id) != switch_metadata_ptr->switch_id) {\
        return SAI_STATUS_FAILURE;\
    }\
} while(0)

#define CAST_OBJ(o, t, id) t *o = static_cast<t *>(switch_metadata_ptr->sai_id_map.get_object(id))


class sai_adapter
{
public:
    sai_adapter();
    ~sai_adapter();
    // sai api functions
    static sai_status_t create_switch(
            sai_object_id_t *switch_id,
            uint32_t attr_count,
            const sai_attribute_t *attr_list);
    static sai_status_t remove_switch(sai_object_id_t switch_id);
    static sai_status_t get_switch_attribute(
            sai_object_id_t switch_id,
            uint32_t attr_count,
            sai_attribute_t *attr_list);
    static sai_status_t set_switch_attribute(
            sai_object_id_t switch_id,
            const sai_attribute_t *attr);

    static sai_status_t create_router_interface(
            sai_object_id_t *lag_member_id,
            sai_object_id_t switch_id,
            uint32_t attr_count,
            const sai_attribute_t *attr_list);
    static sai_status_t remove_router_interface(sai_object_id_t router_interface_id);
    static sai_status_t set_router_interface_attribute(
            sai_object_id_t rif_id,
            const sai_attribute_t *attr);
    static sai_status_t get_router_interface_attribute(
            sai_object_id_t rif_id,
            uint32_t attr_count,
            sai_attribute_t *attr_list);

    // OCS port
    static sai_status_t create_ocs_port(
            sai_object_id_t *ocs_port_id,
            sai_object_id_t switch_id,
            uint32_t attr_count,
            const sai_attribute_t *attr_list);
    static sai_status_t remove_ocs_port(sai_object_id_t ocs_port_id);
    static sai_status_t set_ocs_port_attribute(
            sai_object_id_t ocs_port_id,
            const sai_attribute_t *attr);
    static sai_status_t get_ocs_port_attribute(
            sai_object_id_t ocs_port_id,
            uint32_t attr_count,
            sai_attribute_t *attr_list);

    // OCS cross connect
    static sai_status_t create_ocs_cross_connect(
            sai_object_id_t *ocs_cross_connect_id,
            sai_object_id_t switch_id,
            uint32_t attr_count,
            const sai_attribute_t *attr_list);
    static sai_status_t remove_ocs_cross_connect(sai_object_id_t ocs_cross_connect_id);
    static sai_status_t set_ocs_cross_connect_attribute(
            sai_object_id_t ocs_cross_connect_id,
            const sai_attribute_t *attr);
    static sai_status_t get_ocs_cross_connect_attribute(
            sai_object_id_t ocs_cross_connect_id,
            uint32_t attr_count,
            sai_attribute_t *attr_list);

    // OCS cross connect factory data
    static sai_status_t create_ocs_cross_connect_factory_data(
            sai_object_id_t *ocs_cross_connect_factory_data_id,
            sai_object_id_t switch_id,
            uint32_t attr_count,
            const sai_attribute_t *attr_list);
    static sai_status_t remove_ocs_cross_connect_factory_data(sai_object_id_t ocs_cross_connect_factory_data_id);
    static sai_status_t set_ocs_cross_connect_factory_data_attribute(
            sai_object_id_t ocs_cross_connect_factory_data_id,
            const sai_attribute_t *attr);
    static sai_status_t get_ocs_cross_connect_factory_data_attribute(
            sai_object_id_t ocs_cross_connect_factory_data_id,
            uint32_t attr_count,
            sai_attribute_t *attr_list);

    // Others
    sai_status_t sai_api_query(sai_api_t sai_api_id, void **api_method_table);
    sai_object_type_t sai_object_type_query(sai_object_id_t);

    //variables
    static std::vector<sai_object_id_t> *switch_list_ptr;
    static switch_metadata *switch_metadata_ptr;

    // api table
    sai_switch_api_t switch_api;
    sai_router_interface_api_t router_interface_api;
    sai_ocs_api_t ocs_api;

private:
    static sai_status_t init_switch();

    std::vector<sai_object_id_t> obj_list;
    switch_metadata metadata;
};
