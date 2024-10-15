from sqlite3 import Connection
from typing import Any, Callable

import sqlalchemy as sa
import sqlalchemy.orm as orm
from sqlalchemy import event
from sqlalchemy.engine import Engine
from sqlalchemy.ext.asyncio import AsyncEngine, AsyncSession, create_async_engine
from sqlalchemy.pool.base import _ConnectionRecord
from sqlalchemy.schema import ExecutableDDLElement

from mirra_backend.config import config
from mirra_backend.data.modelbase import SqlAlchemyBase

__factory: Callable[[], AsyncSession] | None = None


@event.listens_for(Engine, "connect")
def set_sqlite_pragma(
    dbapi_connection: Connection, connection_record: _ConnectionRecord
) -> None:
    cursor = dbapi_connection.cursor()
    cursor.execute(f"PRAGMA foreign_keys=ON")
    cursor.close()


# Updated using https://stackoverflow.com/questions/69751346/how-to-make-the-sqlalchemy-async-sessionasync-session-generator-as-class-base
def global_init(
    overwrite_factory: bool = False,
    delete_tables: bool = True,
    db_conn_url: str | None = None,
    db_async_conn_url: str | None = None,
) -> None:
    global __factory
    if __factory and not overwrite_factory:
        return

    db_conn_url: str = (
        sa.make_url("sqlite+pysqlite:///" + config.db_file.as_posix())
        if db_conn_url is None
        else db_conn_url
    )
    db_async_conn_url: str = (
        sa.make_url("sqlite+aiosqlite:///" + config.db_file.as_posix())
        if db_async_conn_url is None
        else db_async_conn_url
    )

    engine = sa.create_engine(db_conn_url, echo=False)
    async_engine: AsyncEngine = create_async_engine(db_async_conn_url, echo=False)

    engine.update_execution_options(connect_args={"check_same_thread": False})
    async_engine.update_execution_options(connect_args={"check_same_thread": False})

    __factory = orm.sessionmaker(
        async_engine, class_=AsyncSession, expire_on_commit=False
    )  # type: ignore[call-overload]

    import mirra_backend.data  # noqa: F401

    if delete_tables:
        SqlAlchemyBase.metadata.drop_all(engine)
    # create new tables according to our current schema
    SqlAlchemyBase.metadata.create_all(engine)


def get_ddl() -> str:
    ddl = []

    def dump(sql: ExecutableDDLElement, *args: Any, **kwargs: Any) -> None:
        ddl.append(str(sql.compile(dialect=engine.dialect)))

    import mirra_backend.data  # noqa: F401

    db_conn_url = sa.make_url("sqlite+pysqlite:///" + ":memory:")
    engine = sa.create_mock_engine(db_conn_url, executor=dump)
    SqlAlchemyBase.metadata.create_all(engine, checkfirst=False)

    return "".join(ddl)


def create_async_session() -> AsyncSession:
    if __factory is None:
        raise Exception("You must call global_init() before using this method.")

    return __factory()


if __name__ == "__main__":
    print(get_ddl())
