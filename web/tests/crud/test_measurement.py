import struct
from random import random

import pytest
from conftest import gateway_macs, node_macs

from mirra_backend.crud.measurement import process_measurement


@pytest.mark.asyncio
async def test_process_measurement(test_db):
    gateway_mac = gateway_macs[0]
    node_mac = node_macs[0]
    payload = node_mac.to_bytes() + struct.pack("I", 0) + struct.pack("B", 0)
    for sensor_key in range(1, 4):
        payload += struct.pack("Bx", sensor_key) + struct.pack("f", random())
    await process_measurement(test_db, gateway_mac, node_mac, payload)
