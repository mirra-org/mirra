import sqlalchemy.orm as orm
from sqlalchemy.ext.asyncio import AsyncAttrs


class SqlAlchemyBase(AsyncAttrs, orm.DeclarativeBase):
    pass
