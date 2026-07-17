#pragma once

#include "virtual_ocs_device.h"
#include <memory>
#include <unordered_map>

class virtual_ocs_device_manager {
public:
    // Singleton accessor
    static virtual_ocs_device_manager& instance();

    // Factory functions
    sai_status_t create_device(sai_object_id_t object_id, sai_object_type_extensions_t type);
    sai_status_t delete_device(sai_object_id_t object_id);

    // Base getter
    virtual_ocs_device* get_device_base(sai_object_id_t object_id);

    // Typed getter
    template <typename T>
    T* get_device(sai_object_id_t object_id) {
        virtual_ocs_device* base = get_device_base(object_id);
        if (base) {
            return dynamic_cast<T*>(base);
        }
        return nullptr;
    }

private:
    virtual_ocs_device_manager() = default;
    ~virtual_ocs_device_manager() = default;

    // Disable copy/move
    virtual_ocs_device_manager(const virtual_ocs_device_manager&) = delete;
    virtual_ocs_device_manager& operator=(const virtual_ocs_device_manager&) = delete;
    virtual_ocs_device_manager(virtual_ocs_device_manager&&) = delete;
    virtual_ocs_device_manager& operator=(virtual_ocs_device_manager&&) = delete;

    // Internal map
    std::unordered_map<sai_object_id_t, std::unique_ptr<virtual_ocs_device>> g_ocs_devices;
};
