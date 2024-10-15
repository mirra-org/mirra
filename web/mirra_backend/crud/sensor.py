import sqlalchemy as sa
from sqlalchemy.ext.asyncio import AsyncSession

from mirra_backend.data.sensor import Sensor


async def get_sensor(session: AsyncSession, sensor_key: int) -> Sensor | None:
    query = sa.select(Sensor).where(Sensor.id == sensor_key)
    return (await session.execute(query)).scalar_one_or_none()
