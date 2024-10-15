import pytest
from conftest import gateway_macs, node_macs

from mirra_backend.crud.module import get_current_module
from mirra_backend.types.mac_address import MACAddress


@pytest.mark.asyncio
async def test_get_current_module(test_db):
    for mac in node_macs:
        cur_module = await get_current_module(test_db, mac)
        physical_module = await cur_module.awaitable_attrs.physical_module
        assert physical_module.mac == mac
    for mac in gateway_macs:
        cur_module = await get_current_module(test_db, mac)
        physical_module = await cur_module.awaitable_attrs.physical_module
        assert physical_module.mac == mac
    for mac in (MACAddress(f"{i:012X}") for i in range(9990, 9995)):
        assert (await get_current_module(test_db, mac)) is None
