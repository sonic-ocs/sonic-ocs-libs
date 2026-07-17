#include "virtual_ocs_device.h"
#include "virtual_ocs_dev_mgr.h"
#include <memory>
#include "logger.h"

// ===================== Singleton Access =====================
virtual_ocs_device_manager& virtual_ocs_device_manager::instance() {
    static virtual_ocs_device_manager instance;
    return instance;
}

virtual_ocs_device* virtual_ocs_device_manager::get_device_base(sai_object_id_t object_id)
{
    auto it = g_ocs_devices.find(object_id);
    if (it != g_ocs_devices.end()) {
        return it->second.get();
    }
    return nullptr;
}

// ===================== Factory Function =====================
sai_status_t virtual_ocs_device_manager::create_device(sai_object_id_t object_id, sai_object_type_extensions_t type)
{
    logger::debug(std::string(__func__) + " called with type " + std::to_string(type));
    sai_status_t ret_status = SAI_STATUS_SUCCESS;

    /* Check if object id is already in the map */
    if (g_ocs_devices.find(object_id) != g_ocs_devices.end()) {
        logger::warn(std::string(__func__) + " object_id " + std::to_string(object_id) + " already exists");
        return SAI_STATUS_ITEM_ALREADY_EXISTS;
    }

    try {
        switch (type) {
            case SAI_OBJECT_TYPE_OCS_PORT:
                g_ocs_devices[object_id] = std::make_unique<virtual_ocs_port_device>(object_id, type);
                break;

            case SAI_OBJECT_TYPE_OCS_CROSS_CONNECT:
                g_ocs_devices[object_id] = std::make_unique<virtual_ocs_cross_connect_device>(object_id, type);
                break;

            case SAI_OBJECT_TYPE_OCS_CROSS_CONNECT_FACTORY_DATA:
                g_ocs_devices[object_id] = std::make_unique<virtual_ocs_cross_connect_factory_data_device>(object_id, type);
                break;

            default:
                logger::warn(std::string(__func__) + " current implementation cant handle object type " + std::to_string(type));
                ret_status = SAI_STATUS_NOT_SUPPORTED;
                break;
        }
    } catch (const std::bad_alloc& e) {
        logger::error(std::string(__func__) + " memory allocation failed: " + e.what());
        ret_status = SAI_STATUS_NO_MEMORY;
    } catch (const std::exception& e) {
        logger::error(std::string(__func__) + " constructor exception: " + e.what());
        ret_status = SAI_STATUS_FAILURE;
    }

    logger::debug(std::string(__func__) + " returning " + std::to_string(ret_status));
    return ret_status;
}

sai_status_t virtual_ocs_device_manager::delete_device(sai_object_id_t object_id)
{
    auto it = g_ocs_devices.find(object_id);

    if (it == g_ocs_devices.end()) {
        logger::warn(std::string(__func__) + " object_id " + std::to_string(object_id) + " not found");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    logger::debug(std::string(__func__) + " destroying virtual device with object_id " +
                  std::to_string(object_id));

    g_ocs_devices.erase(it);

    return SAI_STATUS_SUCCESS;
}
