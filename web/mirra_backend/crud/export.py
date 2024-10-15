import csv
from io import StringIO

import sqlalchemy as sa
from sqlalchemy.orm import joinedload

from mirra_backend.data.gateway import Gateway
from mirra_backend.data.measurement import Measurement
from mirra_backend.data.node import Node
from mirra_backend.data.sensor import Sensor

from .common import Session


async def export_all_csv(session: Session) -> str:
    query = (
        sa.select(Measurement)
        .options(
            joinedload(Measurement.node).options(
                joinedload(Node.physical_module),
                joinedload(Node.gateway).options(joinedload(Gateway.physical_module)),
            ),
            joinedload(Measurement.sensor),
        )
        .order_by(Measurement.timestamp)
    )
    s = StringIO()
    writer = csv.writer(s)
    writer.writerow(
        (
            "timestamp(utc)",
            "gateway_mac",
            "node_mac",
            "value",
            "sensor_name",
            "sensor_unit",
        )
    )
    results = (await session.stream_scalars(query)).unique()
    async for meas in results:
        node: Node = meas.node
        gateway: Gateway = node.gateway
        sensor: Sensor = meas.sensor
        writer.writerow(
            (
                meas.timestamp_iso,
                gateway.physical_module.mac,
                node.physical_module.mac,
                meas.value,
                sensor.name,
                sensor.unit,
            )
        )
    await results.close()
    return s.getvalue()
