from enum import Enum
from pathlib import Path

from pydantic import field_validator
from pydantic_settings import BaseSettings, SettingsConfigDict

root_dir: Path = Path(__file__).parents[1].resolve()


class DBPrefill(str, Enum):
    default = "default"
    test = "test"
    custom = "custom"
    empty = "empty"


class Config(BaseSettings):
    model_config = SettingsConfigDict(
        env_file=(root_dir / "mirra_backend.env"),
        env_file_encoding="utf-8",
        env_prefix="MIRRA_",
        case_sensitive=False,
        extra="forbid",
    )

    host: str = "0.0.0.0"
    port: int = 80

    db_file: Path = root_dir / "db/db.sqlite"
    db_prefill: DBPrefill = DBPrefill.default

    prepopulation_folder: Path = root_dir / "mirra_backend/data/prepopulation"

    location_images_folder: Path = root_dir / "db/location_images"

    mqtt_enabled: bool = True
    mqtt_host: str = "127.0.0.1"
    mqtt_port: int = 8882
    mqtt_prefix: str = "mirra/#"
    mqtt_broker_config_file: Path = root_dir / "mosquitto/mosquitto.conf"
    mqtt_psk_file: Path = root_dir / "db/gateways.psk"

    vite_dist_folder: Path = root_dir / "front/dist"
    vite_host: str = "127.0.0.1"
    vite_port: int = 5173

    gateway_access_code_time: int = 180

    dev: bool = False

    @field_validator(
        "db_file",
        "prepopulation_folder",
        "location_images_folder",
        "mqtt_broker_config_file",
        "mqtt_psk_file",
        "vite_dist_folder",
    )
    @classmethod
    def make_path_absolute(cls, value: str) -> Path:
        path = Path(value)
        if not path.is_absolute():
            return (root_dir / Path(value)).resolve()
        return path


config = Config()
