import sqlalchemy as sa
from sqlalchemy.ext.asyncio import AsyncSession

from mirra_backend.data.gateway import Gateway
from mirra_backend.data.node import Node
from mirra_backend.data.physical_module import PhysicalModule
from mirra_backend.mqtt_broker import mqtt_broker
from mirra_backend.types.mac_address import MACAddress

from .gateway import get_current_gateway
from .module import get_current_module
from .physical_module import get_physical_module


async def add_node(
    session: AsyncSession, node_mac: MACAddress, gateway_mac: MACAddress
) -> Node | None:
    current_module = await get_current_module(session, node_mac)
    if isinstance(current_module, Node):
        return current_module
    if isinstance(current_module, Gateway):
        mqtt_broker.remove_psk(node_mac)
    physical_module = await get_physical_module(session, node_mac)
    if physical_module is None:
        physical_module = PhysicalModule(mac=node_mac)
    gateway = await get_current_gateway(session, gateway_mac)
    if gateway is None:
        # gateway should have been created by this point
        return None
    node = Node(gateway=gateway, physical_module=physical_module)

    session.add(node)
    await session.commit()

    return node


async def get_current_node(session: AsyncSession, mac: MACAddress) -> Node | None:
    module = await get_current_module(session, mac)
    if isinstance(module, Node):
        return module
    return None
