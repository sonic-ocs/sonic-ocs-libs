#include "sai_adapter.h"
#include "logger.h"
#include <iostream>
#include <vector>
#include <string>
#include <cstring>

using namespace std;

static const char *port_status_str(int32_t s)
{
    switch (s) {
    case SAI_OCS_PORT_STATUS_BLOCKED: return "BLOCKED";
    case SAI_OCS_PORT_STATUS_TUNING:  return "TUNING";
    case SAI_OCS_PORT_STATUS_TUNED:   return "TUNED";
    case SAI_OCS_PORT_STATUS_FAILED:  return "FAILED";
    default:                          return "UNKNOWN";
    }
}

// Create an OCS port with the given name and return its object id.
static sai_object_id_t create_port(sai_ocs_api_t *ocs_api, sai_object_id_t switch_id, const string &name)
{
    vector<sai_attribute_t> attrs;
    sai_attribute_t attr;

    attr.id = SAI_OCS_PORT_ATTR_NAME;
    attr.value.u8list.count = (uint32_t)name.size();
    attr.value.u8list.list = (uint8_t *)name.data();
    attrs.push_back(attr);

    sai_object_id_t port_id = SAI_NULL_OBJECT_ID;
    sai_status_t rc = ocs_api->create_ocs_port(&port_id, switch_id, attrs.size(), attrs.data());
    cout << "create_ocs_port " << name << ", rc = " << rc << ", port_id = " << port_id << endl;
    return port_id;
}

static int32_t get_port_oper_status(sai_ocs_api_t *ocs_api, sai_object_id_t port_id)
{
    sai_attribute_t attr;
    attr.id = SAI_OCS_PORT_ATTR_OPER_STATUS;
    attr.value.s32 = -1;
    sai_status_t rc = ocs_api->get_ocs_port_attribute(port_id, 1, &attr);
    cout << "  get oper_status port " << port_id << ", rc = " << rc
         << ", status = " << port_status_str(attr.value.s32) << endl;
    return attr.value.s32;
}

void kvm_flow_test()
{
    sai_api_version_t ver = 0;
    sai_status_t rc = sai_query_api_version(&ver);
    cout << "sai_query_api_version, rc = " << rc << ", version = " << ver << endl;

    rc = sai_api_initialize(0, NULL);
    cout << "sai_api_initialize, rc = " << rc << endl;

    // switch
    sai_switch_api_t *switch_api = NULL;
    rc = sai_api_query(SAI_API_SWITCH, (void **)&switch_api);
    cout << "sai_api_query(SWITCH), rc = " << rc << ", switch_api = " << switch_api << endl;

    sai_attribute_t attr;
    vector<sai_attribute_t> attr_list;
    attr.id = SAI_SWITCH_ATTR_INIT_SWITCH;
    attr.value.booldata = true;
    attr_list.push_back(attr);

    sai_object_id_t switch_id = SAI_NULL_OBJECT_ID;
    rc = switch_api->create_switch(&switch_id, attr_list.size(), attr_list.data());
    cout << "create_switch, rc = " << rc << ", switch_id = " << switch_id << endl;

    // ocs
    sai_ocs_api_t *ocs_api = NULL;
    rc = sai_api_query((sai_api_t)SAI_API_OCS, (void **)&ocs_api);
    cout << "sai_api_query(OCS), rc = " << rc << ", ocs_api = " << ocs_api << endl;

    // --- Create two ports ---
    cout << "------------------------------------------------------" << endl;
    sai_object_id_t port_a = create_port(ocs_api, switch_id, "Ethernet0");
    sai_object_id_t port_b = create_port(ocs_api, switch_id, "Ethernet1");

    cout << "ports before cross-connect (expect BLOCKED):" << endl;
    get_port_oper_status(ocs_api, port_a);
    get_port_oper_status(ocs_api, port_b);

    // --- Create a cross-connect A <-> B ---
    cout << "------------------------------------------------------" << endl;
    attr_list.clear();
    attr.id = SAI_OCS_CROSS_CONNECT_ATTR_A_SIDE_PORT_ID;
    attr.value.oid = port_a;
    attr_list.push_back(attr);
    attr.id = SAI_OCS_CROSS_CONNECT_ATTR_B_SIDE_PORT_ID;
    attr.value.oid = port_b;
    attr_list.push_back(attr);

    sai_object_id_t xc_id = SAI_NULL_OBJECT_ID;
    rc = ocs_api->create_ocs_cross_connect(&xc_id, switch_id, attr_list.size(), attr_list.data());
    cout << "create_ocs_cross_connect A<->B, rc = " << rc << ", xc_id = " << xc_id << endl;

    cout << "ports after cross-connect (expect TUNED):" << endl;
    get_port_oper_status(ocs_api, port_a);
    get_port_oper_status(ocs_api, port_b);

    // --- Force-block port A via override ---
    cout << "------------------------------------------------------" << endl;
    attr.id = SAI_OCS_PORT_ATTR_OVERRIDE_STATE;
    attr.value.s32 = SAI_OCS_PORT_OVERRIDE_STATE_FORCE_BLOCKED;
    rc = ocs_api->set_ocs_port_attribute(port_a, &attr);
    cout << "set port A override FORCE_BLOCKED, rc = " << rc << endl;
    cout << "port A after force-block (expect BLOCKED):" << endl;
    get_port_oper_status(ocs_api, port_a);

    // --- Factory data ---
    cout << "------------------------------------------------------" << endl;
    sai_object_id_t fd_id = SAI_NULL_OBJECT_ID;
    rc = ocs_api->create_ocs_cross_connect_factory_data(&fd_id, switch_id, 0, NULL);
    cout << "create_ocs_cross_connect_factory_data, rc = " << rc << ", fd_id = " << fd_id << endl;

    int32_t freq_buf[8] = {0};
    sai_attribute_t fd_attr;
    fd_attr.id = SAI_OCS_CROSS_CONNECT_FACTORY_DATA_ATTR_FREQUENCY_THZ;
    fd_attr.value.s32list.count = 8;
    fd_attr.value.s32list.list = freq_buf;
    rc = ocs_api->get_ocs_cross_connect_factory_data_attribute(fd_id, 1, &fd_attr);
    cout << "get factory data FREQUENCY_THZ, rc = " << rc << ", count = " << fd_attr.value.s32list.count << ", values =";
    for (uint32_t i = 0; i < fd_attr.value.s32list.count && i < 8; i++) {
        cout << " " << fd_attr.value.s32list.list[i];
    }
    cout << endl;

    // --- Remove the cross-connect ---
    cout << "------------------------------------------------------" << endl;
    rc = ocs_api->remove_ocs_cross_connect(xc_id);
    cout << "remove_ocs_cross_connect, rc = " << rc << endl;
    cout << "port B after cross-connect removal (expect BLOCKED):" << endl;
    get_port_oper_status(ocs_api, port_b);

    // cleanup
    ocs_api->remove_ocs_cross_connect_factory_data(fd_id);
    ocs_api->remove_ocs_port(port_a);
    ocs_api->remove_ocs_port(port_b);

    sai_api_uninitialize();
}

int main()
{
    kvm_flow_test();

    return 0;
}
