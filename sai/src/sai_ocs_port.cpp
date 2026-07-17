#include "sai_adapter.h"
#include "virtual_ocs_dev_mgr.h"
#include <cstring>
#include <string>
#include <vector>

// OCS port
//
// A port is a simplex face-plate connector. Its operational status is derived
// from whether a cross-connect currently references it (see virtual_ocs_port_device),
// unless software forces it blocked via the override state.

static std::string u8list_to_string(const sai_u8_list_t &l)
{
    if (l.list == nullptr || l.count == 0) {
        return std::string();
    }
    uint32_t len = l.count;
    // Tolerate a trailing NUL terminator supplied by the caller.
    if (l.list[len - 1] == '\0') {
        len -= 1;
    }
    return std::string(reinterpret_cast<const char *>(l.list), len);
}

// Copy raw bytes into a caller-provided sai_u8_list_t. On input count is the
// buffer capacity; on output it is the actual byte count.
static sai_status_t bytes_to_u8list(const uint8_t *data, uint32_t size, sai_u8_list_t &out)
{
    if (out.list == nullptr) {
        out.count = size;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }
    uint32_t capacity = out.count;
    uint32_t copy = (size < capacity) ? size : capacity;
    if (copy > 0) {
        std::memcpy(out.list, data, copy);
    }
    out.count = size;
    return (size > capacity) ? SAI_STATUS_BUFFER_OVERFLOW : SAI_STATUS_SUCCESS;
}

sai_status_t
sai_adapter::create_ocs_port(sai_object_id_t *ocs_port_id,
                             sai_object_id_t switch_id,
                             uint32_t attr_count,
                             const sai_attribute_t *attr_list)
{
    CHECK_SWITCH_ID(switch_id);

    ocs_port_obj *obj = new ocs_port_obj(switch_metadata_ptr->sai_id_map);
    *ocs_port_id = obj->sai_object_id;

    logger::notice(std::string(__func__) + ", object id " + std::to_string(*ocs_port_id));

    auto& mgr = virtual_ocs_device_manager::instance();
    sai_status_t ret_status = mgr.create_device(obj->sai_object_id, SAI_OBJECT_TYPE_OCS_PORT);
    if (ret_status == SAI_STATUS_SUCCESS) {
        for (uint32_t i = 0; i < attr_count; i++) {
            sai_status_t tmp_status = set_ocs_port_attribute(obj->sai_object_id, attr_list + i);
            logger::debug(std::string(__func__) + " attr " + std::to_string(attr_list[i].id) + " ret " + std::to_string(tmp_status));
            if (tmp_status != SAI_STATUS_SUCCESS) {
                ret_status = tmp_status;
                break;
            }
        }
    }

    return ret_status;
}

sai_status_t
sai_adapter::remove_ocs_port(sai_object_id_t ocs_port_id)
{
    logger::notice(std::string(__func__) + ", object id " + std::to_string(ocs_port_id));

    auto& mgr = virtual_ocs_device_manager::instance();
    sai_status_t ret_status = mgr.delete_device(ocs_port_id);
    if (ret_status != SAI_STATUS_SUCCESS) {
        logger::warn(std::string(__func__) + ", did not find virtual device object id " + std::to_string(ocs_port_id));
    }
    switch_metadata_ptr->sai_id_map.free_id(ocs_port_id);
    return ret_status;
}

sai_status_t
sai_adapter::set_ocs_port_attribute(sai_object_id_t ocs_port_id,
                                    const sai_attribute_t *attr)
{
    logger::notice(std::string(__func__) +
                   ", object id " + std::to_string(ocs_port_id) +
                   ", attr id " + std::to_string(attr->id));

    sai_status_t rc = SAI_STATUS_SUCCESS;
    CAST_OBJ(obj, ocs_port_obj, ocs_port_id);

    auto& mgr = virtual_ocs_device_manager::instance();
    auto *port_dev = mgr.get_device<virtual_ocs_port_device>(ocs_port_id);
    if (!port_dev) {
        logger::error(std::string(__func__) + ", device not found for object id " + std::to_string(ocs_port_id));
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    switch (attr->id) {
    case SAI_OCS_PORT_ATTR_NAME:
    {
        std::string name = u8list_to_string(attr->value.u8list);
        obj->dev_name = name;
        port_dev->set_name(name);
        // Simulate a deterministic face-plate -> physical element mapping by
        // echoing the port name bytes.
        std::vector<uint8_t> mapping(name.begin(), name.end());
        port_dev->set_physical_mapping(mapping);
        break;
    }
    case SAI_OCS_PORT_ATTR_OVERRIDE_STATE:
        port_dev->set_override_state((sai_ocs_port_override_state_t)attr->value.s32);
        break;
    case SAI_OCS_PORT_ATTR_OPER_STATUS:
    case SAI_OCS_PORT_ATTR_PHYSICAL_MAPPING:
        rc = SAI_STATUS_NOT_SUPPORTED;
        logger::warn(std::string(__func__) + " attribute is read-only, " + std::to_string(attr->id));
        break;
    default:
        rc = SAI_STATUS_NOT_SUPPORTED;
        logger::warn("unsupported ocs port attribute, " + std::to_string(attr->id));
        break;
    }

    return rc;
}

sai_status_t
sai_adapter::get_ocs_port_attribute(sai_object_id_t ocs_port_id,
                                    uint32_t attr_count,
                                    sai_attribute_t *attr_list)
{
    logger::debug("enter " + std::string(__func__));

    auto& mgr = virtual_ocs_device_manager::instance();
    auto *port_dev = mgr.get_device<virtual_ocs_port_device>(ocs_port_id);
    if (!port_dev) {
        logger::error(std::string(__func__) + ", device not found for object id " + std::to_string(ocs_port_id));
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    sai_status_t rc = SAI_STATUS_SUCCESS;

    for (uint32_t i = 0; i < attr_count; i++) {
        switch (attr_list[i].id) {
        case SAI_OCS_PORT_ATTR_NAME:
        {
            const std::string &name = port_dev->get_name();
            rc = bytes_to_u8list(reinterpret_cast<const uint8_t *>(name.data()),
                                 (uint32_t)name.size(), attr_list[i].value.u8list);
            break;
        }
        case SAI_OCS_PORT_ATTR_OVERRIDE_STATE:
            attr_list[i].value.s32 = port_dev->get_override_state();
            break;
        case SAI_OCS_PORT_ATTR_OPER_STATUS:
            attr_list[i].value.s32 = port_dev->get_oper_status();
            break;
        case SAI_OCS_PORT_ATTR_PHYSICAL_MAPPING:
        {
            const std::vector<uint8_t> &m = port_dev->get_physical_mapping();
            rc = bytes_to_u8list(m.data(), (uint32_t)m.size(), attr_list[i].value.u8list);
            break;
        }
        default:
            rc = SAI_STATUS_NOT_SUPPORTED;
            logger::warn("unsupported ocs port attribute, " + std::to_string(attr_list[i].id));
            break;
        }
    }

    return rc;
}
