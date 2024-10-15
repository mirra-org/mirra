from .gateway import Gateway
from .location import Location
from .measurement import Measurement
from .modelbase import SqlAlchemyBase
from .module import Module
from .node import Node
from .physical_module import PhysicalModule
from .sensor import Sensor, SensorClass

SqlAlchemyBase.registry.configure()
