import sqlalchemy as sa
from sqlalchemy import orm

from .module import Module


class Gateway(Module):
    nodes_list: orm.Mapped[list["Node"]] = orm.relationship(
        back_populates="gateway",
        cascade="all, delete",
        passive_deletes=True,
    )

    __mapper_args__ = {"polymorphic_identity": "gateway", "polymorphic_load": "inline"}
