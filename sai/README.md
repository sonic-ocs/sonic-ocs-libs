## Description
Build shared libraries in Debian packages for:
- ``libsai-ocs-$(version)-amd64.deb``: Vendor implementation of SONiC SAI API for ocs-kvm platform.

This virtual SAI adapter implements the experimental OCS (Optical Circuit Switch)
API defined in ``saiexperimentalocs.h`` (``SAI_API_OCS``):
- ``SAI_OBJECT_TYPE_OCS_PORT``: simplex face-plate ports, with software override
  and simulated operational status (blocked/tuned).
- ``SAI_OBJECT_TYPE_OCS_CROSS_CONNECT``: bidirectional A-side/B-side port
  connections; creating one drives the referenced ports to a tuned state.
- ``SAI_OBJECT_TYPE_OCS_CROSS_CONNECT_FACTORY_DATA``: read-only factory
  insertion-loss inventory (simulated).

Source layout:
- ``src/sai_ocs_port.cpp`` &mdash; OCS port create/remove/set/get
- ``src/sai_ocs_cross_connect.cpp`` &mdash; OCS cross-connect create/remove/set/get
- ``src/sai_ocs_cross_connect_factory_data.cpp`` &mdash; factory-data get
- ``inc/virtual_ocs_device.h`` + ``src/virtual_ocs_dev_mgr.cpp`` &mdash; simulated device model

## Build instruction
```
cd sai
mkdir build
cd build
cmake .. -DSONIC_PATH={SONIC_BUILD_IMAGE_PATH}
make -j4
make package
```

For how to specify sonic build-image path, please refer to the below example:
e.g.
cmake .. -DSONIC_PATH=/home/sonic/sonic/sonic-buildimage
