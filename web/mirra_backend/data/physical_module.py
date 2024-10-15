import sqlalchemy as sa
from sqlalchemy import orm

from .modelbase import SqlAlchemyBase
from .module import Module


class PhysicalModule(SqlAlchemyBase):
    __tablename__ = "physical_modules"

    id: orm.Mapped[int] = orm.mapped_column(primary_key=True)
    mac: orm.Mapped[str] = orm.mapped_column(unique=True)

    modules_list: orm.Mapped[list[Module]] = orm.relationship(
        back_populates="physical_module", cascade="all, delete", passive_deletes=True
    )
