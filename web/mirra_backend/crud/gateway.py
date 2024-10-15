from sqlalchemy.ext.asyncio import AsyncSession

from mirra_backend.data.gateway import Gateway
from mirra_backend.data.physical_module import PhysicalModule
from mirra_backend.mqtt_broker import mqtt_broker
from mirra_backend.types.mac_address import MACAddress

from .module import get_current_module
from .physical_module import get_physical_module


async def add_gateway(session: AsyncSession, mac: MACAddress, psk: str) -> Gateway:
    physical_module = await get_physical_module(session, mac)
    if physical_module is None:
        physical_module = PhysicalModule(mac=mac)
    gateway = Gateway(physical_module=physical_module)

    session.add(gateway)
    await session.commit()
    mqtt_broker.update_psk(mac, psk)

    return gateway


async def get_current_gateway(session: AsyncSession, mac: MACAddress) -> Gateway | None:
    module = await get_current_module(session, mac)
    if isinstance(module, Gateway):
        return module
    return None


async def remove_current_gateway(session: AsyncSession, mac: MACAddress) -> None:
    gateway = await get_current_gateway(session, mac)
    if gateway is None:
        return
    mqtt_broker.remove_psk(mac)
    await session.delete(gateway)
    await session.commit()
