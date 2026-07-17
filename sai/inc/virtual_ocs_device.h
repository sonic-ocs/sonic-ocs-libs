#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <saiextensions.h>
#include "logger.h"

/* Base class for a simulated OCS device (port / cross-connect / factory-data). */
class virtual_ocs_device {
public:
    virtual_ocs_device(sai_object_id_t sai_object_id, sai_object_type_extensions_t sai_object_type)
        : sai_object_id(sai_object_id),
          en_sai_object_type(sai_object_type)
    {
        logger::debug(std::string(__func__) + ", sai_id " + std::to_string(sai_object_id) +
                      ", sai_type " + std::to_string(sai_object_type));
    }

    virtual ~virtual_ocs_device() {}

    void set_name(const std::string& name) { attr_name = name; }
    const std::string& get_name() const { return attr_name; }

    sai_object_id_t get_object_id() const { return sai_object_id; }

protected:
    sai_object_id_t sai_object_id;
    sai_object_type_extensions_t en_sai_object_type;
    std::string attr_name;
};

// ===================== OCS Port =====================
// A simplex face-plate port. Its operational status is derived from whether a
// cross-connect currently references it, unless overridden by software.
class virtual_ocs_port_device : public virtual_ocs_device {
public:
    virtual_ocs_port_device(sai_object_id_t id, sai_object_type_extensions_t type)
        : virtual_ocs_device(id, type),
          attr_override_state(SAI_OCS_PORT_OVERRIDE_STATE_NORMAL),
          connection_count(0)
    {
        logger::debug(std::string(__func__) + ", OCS port device created with ID " + std::to_string(id));
    }

    virtual ~virtual_ocs_port_device() {}

    // Configurable attribute
    void set_override_state(sai_ocs_port_override_state_t state) { attr_override_state = state; }
    sai_ocs_port_override_state_t get_override_state() const { return attr_override_state; }

    // Read-only, simulated: face-plate -> physical element mapping.
    void set_physical_mapping(const std::vector<uint8_t>& mapping) { physical_mapping = mapping; }
    const std::vector<uint8_t>& get_physical_mapping() const { return physical_mapping; }

    // Cross-connect bookkeeping (drives the simulated oper status).
    void add_connection() { ++connection_count; }
    void remove_connection() { if (connection_count > 0) --connection_count; }
    uint32_t get_connection_count() const { return connection_count; }

    // Read-only, simulated operational status.
    sai_ocs_port_status_t get_oper_status() const
    {
        if (attr_override_state == SAI_OCS_PORT_OVERRIDE_STATE_FORCE_BLOCKED) {
            return SAI_OCS_PORT_STATUS_BLOCKED;
        }
        // A cross-connect referencing the port drives it to a tuned state; with
        // none, the port is blocked. Tuning is assumed instantaneous in the sim.
        return (connection_count > 0) ? SAI_OCS_PORT_STATUS_TUNED
                                      : SAI_OCS_PORT_STATUS_BLOCKED;
    }

private:
    sai_ocs_port_override_state_t attr_override_state;
    uint32_t connection_count;
    std::vector<uint8_t> physical_mapping;
};

// ===================== OCS Cross Connect =====================
// A bidirectional connection between an A-side and a B-side OCS port.
class virtual_ocs_cross_connect_device : public virtual_ocs_device {
public:
    virtual_ocs_cross_connect_device(sai_object_id_t id, sai_object_type_extensions_t type)
        : virtual_ocs_device(id, type),
          a_side_port_id(SAI_NULL_OBJECT_ID),
          b_side_port_id(SAI_NULL_OBJECT_ID)
    {
        logger::debug(std::string(__func__) + ", OCS cross-connect device created with ID " + std::to_string(id));
    }

    virtual ~virtual_ocs_cross_connect_device() {}

    void set_a_side_port_id(sai_object_id_t id) { a_side_port_id = id; }
    void set_b_side_port_id(sai_object_id_t id) { b_side_port_id = id; }
    sai_object_id_t get_a_side_port_id() const { return a_side_port_id; }
    sai_object_id_t get_b_side_port_id() const { return b_side_port_id; }

private:
    sai_object_id_t a_side_port_id;
    sai_object_id_t b_side_port_id;
};

// ===================== OCS Cross Connect Factory Data =====================
// Read-only inventory of factory insertion-loss measurements for a possible
// cross-connection. Populated by the vendor implementation (simulated here).
class virtual_ocs_cross_connect_factory_data_device : public virtual_ocs_device {
public:
    virtual_ocs_cross_connect_factory_data_device(sai_object_id_t id, sai_object_type_extensions_t type)
        : virtual_ocs_device(id, type)
    {
        logger::debug(std::string(__func__) + ", OCS cross-connect factory data device created with ID " + std::to_string(id));
    }

    virtual ~virtual_ocs_cross_connect_factory_data_device() {}

    void set_a_side_port_name(const std::string& name) { a_side_port_name = name; }
    void set_b_side_port_name(const std::string& name) { b_side_port_name = name; }
    const std::string& get_a_side_port_name() const { return a_side_port_name; }
    const std::string& get_b_side_port_name() const { return b_side_port_name; }

    void set_frequency_thz(const std::vector<int32_t>& v) { frequency_thz = v; }
    void set_measured_temperature(const std::vector<int32_t>& v) { measured_temperature = v; }
    void set_insertion_loss_db(const std::vector<int32_t>& v) { insertion_loss_db = v; }
    void set_insertion_loss_accuracy_db(const std::vector<int32_t>& v) { insertion_loss_accuracy_db = v; }

    const std::vector<int32_t>& get_frequency_thz() const { return frequency_thz; }
    const std::vector<int32_t>& get_measured_temperature() const { return measured_temperature; }
    const std::vector<int32_t>& get_insertion_loss_db() const { return insertion_loss_db; }
    const std::vector<int32_t>& get_insertion_loss_accuracy_db() const { return insertion_loss_accuracy_db; }

private:
    std::string a_side_port_name;
    std::string b_side_port_name;
    std::vector<int32_t> frequency_thz;
    std::vector<int32_t> measured_temperature;
    std::vector<int32_t> insertion_loss_db;
    std::vector<int32_t> insertion_loss_accuracy_db;
};
