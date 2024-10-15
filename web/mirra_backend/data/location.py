import sqlalchemy as sa
from sqlalchemy import orm

from .modelbase import SqlAlchemyBase


class Location(SqlAlchemyBase):
    __tablename__ = "locations"

    id: orm.Mapped[int] = orm.mapped_column(primary_key=True)
    lat: orm.Mapped[float]
    lng: orm.Mapped[float]
    alt: orm.Mapped[float | None]
