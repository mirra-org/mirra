import sqlalchemy as sa
from sqlalchemy.ext.asyncio import AsyncSession

from mirra_backend.data.physical_module import PhysicalModule
from mirra_backend.types.mac_address import MACAddress


async def get_physical_module(
    session: AsyncSession, mac: MACAddress
) -> PhysicalModule | None:
    query = sa.select(PhysicalModule).where(PhysicalModule.mac == mac)
    return (await session.execute(query)).scalar_one_or_none()
