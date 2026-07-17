#include "sai_adapter.h"
#include "virtual_ocs_dev_mgr.h"
#include <string>

// OCS cross connect
//
// A cross-connect establishes a bidirectional connection between an A-side and
// a B-side OCS port. Creating it drives both referenced ports towards a tuned
// state; removing it releases them (see virtual_ocs_port_device oper status).

static void ocs_port_add_connection(sai_object_id_t port_id)
{
    if (port_id == SAI_NULL_OBJECT_ID) {
        return;
    }
    auto& mgr = virtual_ocs_device_manager::instance();
    auto *port_dev = mgr.get_device<virtual_ocs_port_device>(port_id);
    if (port_dev) {
        port_dev->add_connection();
    } else {
        logger::warn(std::string(__func__) + ", referenced port not found " + std::to_string(port_id));
    }
}

static void ocs_port_remove_connection(sai_object_id_t port_id)
{
    if (port_id == SAI_NULL_OBJECT_ID) {
        return;
    }
    auto& mgr = virtual_ocs_device_manager::instance();
    auto *port_dev = mgr.get_device<virtual_ocs_port_device>(port_id);
    if (port_dev) {
        port_dev->remove_connection();
    }
}

sai_status_t
sai_adapter::create_ocs_cross_connect(sai_object_id_t *ocs_cross_connect_id,
                                      sai_object_id_t switch_id,
                                      uint32_t attr_count,
                                      const sai_attribute_t *attr_list)
{
    CHECK_SWITCH_ID(switch_id);

    ocs_cross_connect_obj *obj = new ocs_cross_connect_obj(switch_metadata_ptr->sai_id_map);
    *ocs_cross_connect_id = obj->sai_object_id;

    logger::notice(std::string(__func__) + ", object id " + std::to_string(*ocs_cross_connect_id));

    auto& mgr = virtual_ocs_device_manager::instance();
    sai_status_t ret_status = mgr.create_device(obj->sai_object_id, SAI_OBJECT_TYPE_OCS_CROSS_CONNECT);
    if (ret_status == SAI_STATUS_SUCCESS) {
        for (uint32_t i = 0; i < attr_count; i++) {
            sai_status_t tmp_status = set_ocs_cross_connect_attribute(obj->sai_object_id, attr_list + i);
            logger::debug(std::string(__func__) + " attr " + std::to_string(attr_list[i].id) + " ret " + std::to_string(tmp_status));
            if (tmp_status != SAI_STATUS_SUCCESS) {
                ret_status = tmp_status;
                break;
            }
        }
    }

    if (ret_status == SAI_STATUS_SUCCESS) {
        // Link the referenced ports so their oper status reflects the connection.
        ocs_port_add_connection(obj->a_side_port_id);
        ocs_port_add_connection(obj->b_side_port_id);
    }

    return ret_status;
}

sai_status_t
sai_adapter::remove_ocs_cross_connect(sai_object_id_t ocs_cross_connect_id)
{
    logger::notice(std::string(__func__) + ", object id " + std::to_string(ocs_cross_connect_id));

    CAST_OBJ(obj, ocs_cross_connect_obj, ocs_cross_connect_id);
    if (obj) {
        ocs_port_remove_connection(obj->a_side_port_id);
        ocs_port_remove_connection(obj->b_side_port_id);
    }

    auto& mgr = virtual_ocs_device_manager::instance();
    sai_status_t ret_status = mgr.delete_device(ocs_cross_connect_id);
    if (ret_status != SAI_STATUS_SUCCESS) {
        logger::warn(std::string(__func__) + ", did not find virtual device object id " + std::to_string(ocs_cross_connect_id));
    }
    switch_metadata_ptr->sai_id_map.free_id(ocs_cross_connect_id);
    return ret_status;
}

sai_status_t
sai_adapter::set_ocs_cross_connect_attribute(sai_object_id_t ocs_cross_connect_id,
                                             const sai_attribute_t *attr)
{
    logger::notice(std::string(__func__) +
                   ", object id " + std::to_string(ocs_cross_connect_id) +
                   ", attr id " + std::to_string(attr->id));

    sai_status_t rc = SAI_STATUS_SUCCESS;
    CAST_OBJ(obj, ocs_cross_connect_obj, ocs_cross_connect_id);

    auto& mgr = virtual_ocs_device_manager::instance();
    auto *xc_dev = mgr.get_device<virtual_ocs_cross_connect_device>(ocs_cross_connect_id);
    if (!xc_dev) {
        logger::error(std::string(__func__) + ", device not found for object id " + std::to_string(ocs_cross_connect_id));
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    // A/B side ports are MANDATORY_ON_CREATE | CREATE_ONLY, so they are only
    // ever set through the create() attribute loop.
    switch (attr->id) {
    case SAI_OCS_CROSS_CONNECT_ATTR_A_SIDE_PORT_ID:
        obj->a_side_port_id = attr->value.oid;
        xc_dev->set_a_side_port_id(attr->value.oid);
        break;
    case SAI_OCS_CROSS_CONNECT_ATTR_B_SIDE_PORT_ID:
        obj->b_side_port_id = attr->value.oid;
        xc_dev->set_b_side_port_id(attr->value.oid);
        break;
    default:
        rc = SAI_STATUS_NOT_SUPPORTED;
        logger::warn("unsupported ocs cross connect attribute, " + std::to_string(attr->id));
        break;
    }

    return rc;
}

sai_status_t
sai_adapter::get_ocs_cross_connect_attribute(sai_object_id_t ocs_cross_connect_id,
                                             uint32_t attr_count,
                                             sai_attribute_t *attr_list)
{
    logger::debug("enter " + std::string(__func__));

    auto& mgr = virtual_ocs_device_manager::instance();
    auto *xc_dev = mgr.get_device<virtual_ocs_cross_connect_device>(ocs_cross_connect_id);
    if (!xc_dev) {
        logger::error(std::string(__func__) + ", device not found for object id " + std::to_string(ocs_cross_connect_id));
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    sai_status_t rc = SAI_STATUS_SUCCESS;

    for (uint32_t i = 0; i < attr_count; i++) {
        switch (attr_list[i].id) {
        case SAI_OCS_CROSS_CONNECT_ATTR_A_SIDE_PORT_ID:
            attr_list[i].value.oid = xc_dev->get_a_side_port_id();
            break;
        case SAI_OCS_CROSS_CONNECT_ATTR_B_SIDE_PORT_ID:
            attr_list[i].value.oid = xc_dev->get_b_side_port_id();
            break;
        default:
            rc = SAI_STATUS_NOT_SUPPORTED;
            logger::warn("unsupported ocs cross connect attribute, " + std::to_string(attr_list[i].id));
            break;
        }
    }

    return rc;
}
