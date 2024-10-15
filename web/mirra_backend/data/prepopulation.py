import asyncio
import json

import sqlalchemy as sa
from sqlalchemy.ext.asyncio import AsyncSession

from mirra_backend.config import config

from .database import create_async_session
from .sensor import Sensor, SensorClass


def prepopulate() -> None:
    session = create_async_session()
    asyncio.run(_prepopulate(session))


async def _prepopulate(session: AsyncSession) -> None:
    with open(config.prepopulation_folder / "sensor_classes.json") as file:
        sensor_classes = json.load(file)
        await session.execute(sa.insert(SensorClass), sensor_classes)
    with open(config.prepopulation_folder / "sensors.json") as file:
        sensors = json.load(file)
        await session.execute(sa.insert(Sensor), sensors)
    await session.commit()
