import struct
from datetime import datetime, timezone

from sqlalchemy.ext.asyncio import AsyncSession

from mirra_backend.data.measurement import Measurement
from mirra_backend.types.mac_address import MACAddress

from .common import inject_session_sync
from .node import add_node
from .sensor import get_sensor


async def add_measurement(
    session: AsyncSession,
    gateway_mac: MACAddress,
    node_mac: MACAddress,
    timestamp: int,
    sensor_key: int,
    instance_tag: int,
    value: float,
) -> Measurement | None:
    node = await add_node(session, node_mac, gateway_mac)
    if node is None:
        return None
    sensor = await get_sensor(session, sensor_key)
    if sensor is None:
        return None
    insertion_timestamp = datetime.now(timezone.utc)
    measurement = Measurement(
        node=node,
        timestamp=timestamp,
        insertion_timestamp=insertion_timestamp,
        value=value,
        sensor_id=sensor_key,
    )

    session.add(measurement)
    await session.commit()

    return measurement


async def process_measurement(
    session: AsyncSession, gateway_mac: MACAddress, node_mac: MACAddress, payload: bytes
) -> None:
    """
    Message format:
    `[mac 6]:[timestamp 4]:[flags 1]:[sensorvalue_1 6]:...[sensorvalue_n 6]`

    Each sensorvalue has the following format:
    `[sensor_id 2]:[data 4 (float)]`
    """
    payload = payload[6:]  # skip over mac
    timestamp: int = struct.unpack("I", payload[:4])[0]
    payload = payload[5:]  # skip over timestamp, flags
    print(f"sensor message: {gateway_mac}-{node_mac}-{timestamp}")
    instance_tags: dict[int, int] = {}
    while len(payload) > 0:
        sensor_key: int = payload[0]
        if sensor_key not in instance_tags:
            instance_tags[sensor_key] = 0

        value: float = struct.unpack("f", payload[2:6])[0]
        print(f"    sensor value: {sensor_key}-{instance_tags[sensor_key]}: {value}")

        await add_measurement(
            session,
            gateway_mac,
            node_mac,
            timestamp,
            sensor_key,
            instance_tags[sensor_key],
            value,
        )

        instance_tags[sensor_key] += 1
        payload = payload[6:]  # skip over sensorvalue


def process_measurement_sync(gateway_mac, node_mac, payload) -> None:
    return inject_session_sync(process_measurement, gateway_mac, node_mac, payload)
