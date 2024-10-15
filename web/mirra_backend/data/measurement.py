from datetime import datetime, timezone

import sqlalchemy as sa
from sqlalchemy import orm

from .modelbase import SqlAlchemyBase


class Measurement(SqlAlchemyBase):
    __tablename__ = "measurements"

    id: orm.Mapped[int] = orm.mapped_column(primary_key=True)
    timestamp: orm.Mapped[int]
    value: orm.Mapped[float]

    node_id: orm.Mapped[int] = orm.mapped_column(
        sa.ForeignKey("modules.id", ondelete="CASCADE")
    )
    node: orm.Mapped["Node"] = orm.relationship(back_populates="measurements_list")

    sensor_id: orm.Mapped[int] = orm.mapped_column(
        sa.ForeignKey("sensors.id", ondelete="CASCADE")
    )
    sensor: orm.Mapped["Sensor"] = orm.relationship(back_populates="measurements_list")

    @property
    def timestamp_iso(self) -> str:
        return str(datetime.fromtimestamp(self.timestamp, tz=timezone.utc))
