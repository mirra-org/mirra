import asyncio as aio
from typing import (
    Annotated,
    Any,
    AsyncGenerator,
    Callable,
    Coroutine,
    TypeVar,
    TypeVarTuple,
)

from fastapi import Depends
from sqlalchemy.ext.asyncio import AsyncSession

from mirra_backend.data.database import create_async_session


async def db_session() -> AsyncGenerator[AsyncSession, None]:
    async with create_async_session() as session:
        yield session


T = TypeVar("T")
Ts = TypeVarTuple("Ts")


async def inject_session(
    coroutine: Callable[..., Coroutine[Any, Any, T]], *args, **kwargs
) -> T:
    async with create_async_session() as session:
        return await coroutine(session, *args, **kwargs)


def inject_session_sync(
    coroutine: Callable[..., Coroutine[Any, Any, T]], *args, **kwargs
) -> T:
    try:
        event_loop = aio.get_event_loop()
    except RuntimeError:
        return aio.run(inject_session(coroutine, *args, **kwargs))
    task = event_loop.create_task(inject_session(coroutine, *args, **kwargs))
    return event_loop.run_until_complete(task)


Session = Annotated[AsyncSession, Depends(db_session)]
