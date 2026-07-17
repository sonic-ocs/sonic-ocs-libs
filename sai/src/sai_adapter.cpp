#include "sai_adapter.h"
#include <cstring>

std::vector<sai_object_id_t> *sai_adapter::switch_list_ptr;
switch_metadata *sai_adapter::switch_metadata_ptr;


sai_adapter::sai_adapter()
{
    // ptr
    switch_list_ptr = &obj_list;
    switch_metadata_ptr = &metadata;

    // api set
    switch_api.create_switch = &sai_adapter::create_switch;
    switch_api.remove_switch = &sai_adapter::remove_switch;
    switch_api.get_switch_attribute = &sai_adapter::get_switch_attribute;
    switch_api.set_switch_attribute = &sai_adapter::set_switch_attribute;

    router_interface_api.create_router_interface = &sai_adapter::create_router_interface;
    router_interface_api.remove_router_interface = &sai_adapter::remove_router_interface;
    router_interface_api.set_router_interface_attribute = &sai_adapter::set_router_interface_attribute;
    router_interface_api.get_router_interface_attribute = &sai_adapter::get_router_interface_attribute;

    // Zero-init so unimplemented bulk methods are NULL.
    memset(&ocs_api, 0, sizeof(ocs_api));
    ocs_api.create_ocs_port = &sai_adapter::create_ocs_port;
    ocs_api.remove_ocs_port = &sai_adapter::remove_ocs_port;
    ocs_api.set_ocs_port_attribute = &sai_adapter::set_ocs_port_attribute;
    ocs_api.get_ocs_port_attribute = &sai_adapter::get_ocs_port_attribute;
    ocs_api.create_ocs_cross_connect = &sai_adapter::create_ocs_cross_connect;
    ocs_api.remove_ocs_cross_connect = &sai_adapter::remove_ocs_cross_connect;
    ocs_api.set_ocs_cross_connect_attribute = &sai_adapter::set_ocs_cross_connect_attribute;
    ocs_api.get_ocs_cross_connect_attribute = &sai_adapter::get_ocs_cross_connect_attribute;
    ocs_api.create_ocs_cross_connect_factory_data = &sai_adapter::create_ocs_cross_connect_factory_data;
    ocs_api.remove_ocs_cross_connect_factory_data = &sai_adapter::remove_ocs_cross_connect_factory_data;
    ocs_api.set_ocs_cross_connect_factory_data_attribute = &sai_adapter::set_ocs_cross_connect_factory_data_attribute;
    ocs_api.get_ocs_cross_connect_factory_data_attribute = &sai_adapter::get_ocs_cross_connect_factory_data_attribute;

    logger::notice("sai adapter initialized");
}

sai_adapter::~sai_adapter()
{
    logger::notice("sai adapter closed");
}

sai_status_t
sai_adapter::sai_api_query(sai_api_t sai_api_id, void **api_method_table)
{
    switch ((uint32_t)sai_api_id) {
    case SAI_API_SWITCH:
        *api_method_table = &switch_api;
        break;
    case SAI_API_ROUTER_INTERFACE:
        *api_method_table = &router_interface_api;
        break;
    case SAI_API_OCS:
        *api_method_table = &ocs_api;
        break;
    default:
        logger::notice("unsupported api request made " + std::to_string(sai_api_id));
        return SAI_STATUS_FAILURE;
    }
    return SAI_STATUS_SUCCESS;
}

sai_object_type_t
sai_adapter::sai_object_type_query(sai_object_id_t sai_object_id) {
    // top level, switch
    if (switch_metadata_ptr->switch_id == sai_object_id) {
        return SAI_OBJECT_TYPE_SWITCH;
    }

    // ocs sai
    if (switch_metadata_ptr->sai_id_map.find_object(sai_object_id)) {
        return static_cast<sai_obj *>(switch_metadata_ptr->sai_id_map.get_object(sai_object_id))->sai_object_type;
    }

    // vid
    for (const auto &id : switch_metadata_ptr->vids) {
        if (sai_object_id == id.second) {
            return id.first;
        }
    }

    return SAI_OBJECT_TYPE_NULL;
}
