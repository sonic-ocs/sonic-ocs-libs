# OCS kvm Driver Libs
This repository provides the source code and pre-built debian package used in the ocs kvm platform and devices.
- SAI driver implementing the experimental OCS API (ports + cross-connects) to simulate an optical circuit switch.

> Note: the `sai/` virtual adapter implements the experimental OCS SAI API
> (`SAI_API_OCS`: `OCS_PORT`, `OCS_CROSS_CONNECT`, `OCS_CROSS_CONNECT_FACTORY_DATA`)
> from `sonic-sairedis/SAI/experimental/saiexperimentalocs.h`.
>
> The SONiC platform (PMON) HAL driver is **no longer part of this repo**. The
> previous thrift-based design (a python client talking to a `HalPlatformApiServer`
> thrift server in the syncd container) has been removed. The platform APIs are now
> implemented natively, in-process, inside the `sonic_platform` python package that
> ships with the `ocs-kvm` platform in `sonic-ocs/sonic-buildimage`
> (`platform/ocs-kvm/sonic-platform-modules-ocs-kvm/ocs-v/sonic_platform/hal.py`).

## Packages
Building this repo produces the debian package consumed by the `ocs-kvm` platform in
`sonic-ocs/sonic-buildimage` (published under this repo's GitHub Releases):
- `libsai-ocs-<version>-amd64.deb` &mdash; vendor SAI implementation (see `sai/`)
