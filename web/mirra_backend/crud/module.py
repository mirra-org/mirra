import sqlalchemy as sa
from sqlalchemy.ext.asyncio import AsyncSession

from mirra_backend.data.module import Module
from mirra_backend.types.mac_address import MACAddress

from .physical_module import get_physical_module


async def get_current_module(session: AsyncSession, mac: MACAddress) -> Module | None:
    physical_module = await get_physical_module(session, mac)
    if physical_module is None:
        return None
    query = (
        sa.select(Module)
        .where(Module.physical_module_id == physical_module.id)
        .order_by(sa.desc(Module.id))
        .limit(1)
    )
    return (await session.execute(query)).scalar_one_or_none()
