import sqlalchemy as sa
from sqlalchemy import orm

from .modelbase import SqlAlchemyBase


class SensorClass(SqlAlchemyBase):
    __tablename__ = "sensor_classes"
    id: orm.Mapped[int] = orm.mapped_column(primary_key=True)
    desc: orm.Mapped[str] = orm.mapped_column(unique=True)

    sensors_list: orm.Mapped[list["Sensor"]] = orm.relationship(
        back_populates="sensor_class", cascade="all, delete", passive_deletes=True
    )


class Sensor(SqlAlchemyBase):
    __tablename__ = "sensors"

    id: orm.Mapped[int] = orm.mapped_column(primary_key=True)
    name: orm.Mapped[str]
    unit: orm.Mapped[str]
    accuracy: orm.Mapped[float]
    description: orm.Mapped[str | None]

    sensor_class_id: orm.Mapped[int] = orm.mapped_column(
        sa.ForeignKey("sensor_classes.id", ondelete="CASCADE")
    )
    sensor_class: orm.Mapped["SensorClass"] = orm.relationship(
        back_populates="sensors_list"
    )

    measurements_list: orm.Mapped[list["Measurement"]] = orm.relationship(
        back_populates="sensor", cascade="all, delete", passive_deletes=True
    )
