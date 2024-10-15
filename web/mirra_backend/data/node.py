import sqlalchemy as sa
from sqlalchemy import orm

from .module import Module


class Node(Module):
    gateway: orm.Mapped["Gateway"] = orm.relationship(
        back_populates="nodes_list", remote_side="Gateway.id"
    )

    measurements_list: orm.Mapped[list["Measurement"] | None] = orm.relationship(
        back_populates="node", cascade="all, delete", passive_deletes=True
    )

    __mapper_args__ = {"polymorphic_identity": "node", "polymorphic_load": "inline"}
