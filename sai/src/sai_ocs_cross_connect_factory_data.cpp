#include "sai_adapter.h"
#include "virtual_ocs_dev_mgr.h"
#include <string>
#include <vector>

// OCS cross connect factory data
//
// Read-only inventory of factory insertion-loss measurements for a possible
// cross-connection. The vendor implementation provides these; here we populate
// deterministic simulated values on create and serve them back on get.

// Copy int32 values into a caller-provided sai_s32_list_t. On input count is the
// buffer capacity; on output it is the actual element count.
static sai_status_t s32vec_to_list(const std::vector<int32_t> &vec, sai_s32_list_t &out)
{
    uint32_t size = (uint32_t)vec.size();
    if (out.list == nullptr) {
        out.count = size;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }
    uint32_t capacity = out.count;
    uint32_t copy = (size < capacity) ? size : capacity;
    for (uint32_t i = 0; i < copy; i++) {
        out.list[i] = vec[i];
    }
    out.count = size;
    return (size > capacity) ? SAI_STATUS_BUFFER_OVERFLOW : SAI_STATUS_SUCCESS;
}

static sai_status_t str_to_u8list(const std::string &s, sai_u8_list_t &out)
{
    uint32_t size = (uint32_t)s.size();
    if (out.list == nullptr) {
        out.count = size;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }
    uint32_t capacity = out.count;
    uint32_t copy = (size < capacity) ? size : capacity;
    for (uint32_t i = 0; i < copy; i++) {
        out.list[i] = (uint8_t)s[i];
    }
    out.count = size;
    return (size > capacity) ? SAI_STATUS_BUFFER_OVERFLOW : SAI_STATUS_SUCCESS;
}

sai_status_t
sai_adapter::create_ocs_cross_connect_factory_data(sai_object_id_t *ocs_cross_connect_factory_data_id,
                                                   sai_object_id_t switch_id,
                                                   uint32_t attr_count,
                                                   const sai_attribute_t *attr_list)
{
    CHECK_SWITCH_ID(switch_id);
    (void)attr_count;
    (void)attr_list;

    ocs_cross_connect_factory_data_obj *obj =
        new ocs_cross_connect_factory_data_obj(switch_metadata_ptr->sai_id_map);
    *ocs_cross_connect_factory_data_id = obj->sai_object_id;

    logger::notice(std::string(__func__) + ", object id " + std::to_string(*ocs_cross_connect_factory_data_id));

    auto& mgr = virtual_ocs_device_manager::instance();
    sai_status_t ret_status = mgr.create_device(obj->sai_object_id, SAI_OBJECT_TYPE_OCS_CROSS_CONNECT_FACTORY_DATA);
    if (ret_status != SAI_STATUS_SUCCESS) {
        return ret_status;
    }

    // Populate deterministic simulated factory data. All attributes are
    // READ_ONLY, so they are not provided by the caller. Decimal values use the
    // fixed-point conventions documented in saiexperimentalocs.h (THz: 3 frac
    // digits, temperature / insertion loss: 2 frac digits).
    auto *fd_dev = mgr.get_device<virtual_ocs_cross_connect_factory_data_device>(obj->sai_object_id);
    if (fd_dev) {
        fd_dev->set_frequency_thz({193100, 193500, 229100});          // 193.100, 193.500, 229.100 THz
        fd_dev->set_measured_temperature({2500, 2500, 2500});         // 25.00 C
        fd_dev->set_insertion_loss_db({123, 125, 130});               // 1.23, 1.25, 1.30 dB
        fd_dev->set_insertion_loss_accuracy_db({10, 10, 10});         // 0.10 dB
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t
sai_adapter::remove_ocs_cross_connect_factory_data(sai_object_id_t ocs_cross_connect_factory_data_id)
{
    logger::notice(std::string(__func__) + ", object id " + std::to_string(ocs_cross_connect_factory_data_id));

    auto& mgr = virtual_ocs_device_manager::instance();
    sai_status_t ret_status = mgr.delete_device(ocs_cross_connect_factory_data_id);
    if (ret_status != SAI_STATUS_SUCCESS) {
        logger::warn(std::string(__func__) + ", did not find virtual device object id " + std::to_string(ocs_cross_connect_factory_data_id));
    }
    switch_metadata_ptr->sai_id_map.free_id(ocs_cross_connect_factory_data_id);
    return ret_status;
}

sai_status_t
sai_adapter::set_ocs_cross_connect_factory_data_attribute(sai_object_id_t ocs_cross_connect_factory_data_id,
                                                          const sai_attribute_t *attr)
{
    // All factory-data attributes are READ_ONLY.
    logger::warn(std::string(__func__) +
                 ", object id " + std::to_string(ocs_cross_connect_factory_data_id) +
                 ", attr id " + std::to_string(attr->id) + " is read-only");
    return SAI_STATUS_NOT_SUPPORTED;
}

sai_status_t
sai_adapter::get_ocs_cross_connect_factory_data_attribute(sai_object_id_t ocs_cross_connect_factory_data_id,
                                                          uint32_t attr_count,
                                                          sai_attribute_t *attr_list)
{
    logger::debug("enter " + std::string(__func__));

    auto& mgr = virtual_ocs_device_manager::instance();
    auto *fd_dev = mgr.get_device<virtual_ocs_cross_connect_factory_data_device>(ocs_cross_connect_factory_data_id);
    if (!fd_dev) {
        logger::error(std::string(__func__) + ", device not found for object id " + std::to_string(ocs_cross_connect_factory_data_id));
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    sai_status_t rc = SAI_STATUS_SUCCESS;

    for (uint32_t i = 0; i < attr_count; i++) {
        switch (attr_list[i].id) {
        case SAI_OCS_CROSS_CONNECT_FACTORY_DATA_ATTR_A_SIDE_PORT_NAME:
            rc = str_to_u8list(fd_dev->get_a_side_port_name(), attr_list[i].value.u8list);
            break;
        case SAI_OCS_CROSS_CONNECT_FACTORY_DATA_ATTR_B_SIDE_PORT_NAME:
            rc = str_to_u8list(fd_dev->get_b_side_port_name(), attr_list[i].value.u8list);
            break;
        case SAI_OCS_CROSS_CONNECT_FACTORY_DATA_ATTR_FREQUENCY_THZ:
            rc = s32vec_to_list(fd_dev->get_frequency_thz(), attr_list[i].value.s32list);
            break;
        case SAI_OCS_CROSS_CONNECT_FACTORY_DATA_ATTR_MEASURED_TEMPERATURE:
            rc = s32vec_to_list(fd_dev->get_measured_temperature(), attr_list[i].value.s32list);
            break;
        case SAI_OCS_CROSS_CONNECT_FACTORY_DATA_ATTR_INSERTION_LOSS_DB:
            rc = s32vec_to_list(fd_dev->get_insertion_loss_db(), attr_list[i].value.s32list);
            break;
        case SAI_OCS_CROSS_CONNECT_FACTORY_DATA_ATTR_INSERTION_LOSS_ACCURACY_DB:
            rc = s32vec_to_list(fd_dev->get_insertion_loss_accuracy_db(), attr_list[i].value.s32list);
            break;
        default:
            rc = SAI_STATUS_NOT_SUPPORTED;
            logger::warn("unsupported ocs cross connect factory data attribute, " + std::to_string(attr_list[i].id));
            break;
        }
    }

    return rc;
}
