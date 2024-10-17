from random import random
from typing import AsyncGenerator

import aiosqlite
import pytest_asyncio
import sqlalchemy as sa
from sqlalchemy import orm
from sqlalchemy.ext.asyncio import AsyncSession, create_async_engine

import mirra_backend.data as data
import mirra_backend.data.database as database
from mirra_backend.data.prepopulation import _prepopulate as populate_default
from mirra_backend.types.mac_address import MACAddress

pytest_plugins = ("pytest_asyncio",)

gateway_macs = [MACAddress(f"{i:012X}") for i in range(256, 259)]
node_macs = [MACAddress(f"{i:012X}") for i in range(1, 10)]
timestamps = range(0, 7201, 1200)


async def populate_test(session: AsyncSession) -> None:
    sensor_classes = [
        data.SensorClass(desc=desc) for desc in ("TEMPERATURE", "CO2", "AIR_PRESSURE")
    ]
    session.add_all(sensor_classes)
    sensors = [
        data.Sensor(
            name=f"{sensor_class.desc.lower()}",
            unit=unit,
            accuracy=0.5,
            sensor_class=sensor_class,
        )
        for sensor_class, unit in zip(sensor_classes, ("C", "ppm", "bar"))
    ]
    session.add_all(sensors)
    gateways = [
        data.Gateway(physical_module=data.PhysicalModule(mac=mac))
        for mac in gateway_macs
    ]
    session.add_all(gateways)
    nodes = [
        data.Node(
            physical_module=data.PhysicalModule(mac=mac), gateway=gateways[i // 3]
        )
        for i, mac in enumerate(node_macs)
    ]
    session.add_all(nodes)
    measurements = [
        data.Measurement(timestamp=timestamp, value=random(), node=node, sensor=sensor)
        for sensor in sensors
        for timestamp in timestamps
        for node in nodes
    ]
    session.add_all(measurements)
    await session.commit()


async def get_raw_connection(session: AsyncSession) -> aiosqlite.Connection:
    return (await (await session.connection()).get_raw_connection()).driver_connection


@pytest_asyncio.fixture(scope="session", autouse=True)
async def init():
    # make cached copy of default and test db
    default_cache_async_engine = create_async_engine(
        sa.make_url(
            "sqlite+aiosqlite:///file:test_default?mode=memory&cache=shared&uri=true"
        ),
        echo=False,
    )
    test_cache_async_engine = create_async_engine(
        sa.make_url(
            "sqlite+aiosqlite:///file:test_cache?mode=memory&cache=shared&uri=true"
        ),
        echo=False,
    )

    global default_cache_async_session, test_cache_async_session
    default_cache_async_session = orm.sessionmaker(
        default_cache_async_engine, class_=AsyncSession, expire_on_commit=False
    )()
    test_cache_async_session = orm.sessionmaker(
        test_cache_async_engine, class_=AsyncSession, expire_on_commit=False
    )()

    await (await default_cache_async_session.connection()).run_sync(
        data.SqlAlchemyBase.metadata.create_all
    )
    await (await test_cache_async_session.connection()).run_sync(
        data.SqlAlchemyBase.metadata.create_all
    )

    await populate_default(default_cache_async_session)
    await populate_test(test_cache_async_session)

    database.global_init(
        overwrite_factory=True,
        db_conn_url=sa.make_url("sqlite+pysqlite:///:memory:"),
        db_async_conn_url=sa.make_url("sqlite+aiosqlite:///:memory:"),
    )


@pytest_asyncio.fixture
async def empty_db() -> AsyncGenerator[AsyncSession, None]:
    yield database.create_async_session()


@pytest_asyncio.fixture
async def default_db() -> AsyncGenerator[AsyncSession, None]:
    async with database.create_async_session() as session:
        await (await get_raw_connection(default_cache_async_session)).backup(
            await get_raw_connection(session)
        )
        yield session


@pytest_asyncio.fixture
async def test_db() -> AsyncGenerator[AsyncSession, None]:
    async with database.create_async_session() as session:
        await (await get_raw_connection(test_cache_async_session)).backup(
            await get_raw_connection(session)
        )
        yield session
