import pytest
from conftest import gateway_macs, node_macs

from mirra_backend.crud.physical_module import get_physical_module
from mirra_backend.types.mac_address import MACAddress


@pytest.mark.asyncio
async def test_get_physical_module(test_db):
    for mac in node_macs:
        assert (await get_physical_module(test_db, mac)).mac == mac
    for mac in gateway_macs:
        assert (await get_physical_module(test_db, mac)).mac == mac
    for mac in (MACAddress(f"{i:012X}") for i in range(9990, 9995)):
        assert (await get_physical_module(test_db, mac)) is None
