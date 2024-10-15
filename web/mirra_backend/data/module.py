import sqlalchemy as sa
from sqlalchemy import orm

from .modelbase import SqlAlchemyBase


class Module(SqlAlchemyBase):
    __tablename__ = "modules"

    id: orm.Mapped[int] = orm.mapped_column(primary_key=True)
    name: orm.Mapped[str | None]

    physical_module_id: orm.Mapped[int] = orm.mapped_column(
        sa.ForeignKey("physical_modules.id", ondelete="CASCADE")
    )

    physical_module: orm.Mapped["PhysicalModule"] = orm.relationship(
        back_populates="modules_list"
    )

    location_id: orm.Mapped[int | None] = orm.mapped_column(
        sa.ForeignKey("locations.id", ondelete="SET NULL")
    )
    location: orm.Mapped["Location"] = orm.relationship()

    gateway_id: orm.Mapped[int | None] = orm.mapped_column(
        sa.ForeignKey("modules.id", ondelete="CASCADE")
    )

    __mapper_args__ = {
        "polymorphic_on": sa.sql.expression.case(
            (gateway_id == None, "gateway"), (gateway_id != None, "node")
        ),
        "polymorphic_abstract": True,
    }
