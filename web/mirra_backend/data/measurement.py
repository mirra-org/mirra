from datetime import datetime, timezone

import sqlalchemy as sa
from sqlalchemy import orm

from .modelbase import SqlAlchemyBase


class Measurement(SqlAlchemyBase):
    __tablename__ = "measurements"
    __table_args__ = {"sqlite_with_rowid": False}

    timestamp: orm.Mapped[int] = orm.mapped_column(primary_key=True)

    node_id: orm.Mapped[int] = orm.mapped_column(
        sa.ForeignKey("modules.id", ondelete="CASCADE"), primary_key=True
    )
    node: orm.Mapped["Node"] = orm.relationship(back_populates="measurements_list")

    sensor_id: orm.Mapped[int] = orm.mapped_column(
        sa.ForeignKey("sensors.id", ondelete="CASCADE"), primary_key=True
    )
    sensor: orm.Mapped["Sensor"] = orm.relationship(back_populates="measurements_list")

    insertion_timestamp: orm.Mapped[int]

    value: orm.Mapped[float]

    @property
    def timestamp_iso(self) -> str:
        return str(datetime.fromtimestamp(self.timestamp, tz=timezone.utc))

    @property
    def insertion_timestamp_iso(self) -> str:
        return str(datetime.fromtimestamp(self.insertion_timestamp, tz=timezone.utc))
