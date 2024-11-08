from contextlib import asynccontextmanager

from fastapi import FastAPI
from fastapi.staticfiles import StaticFiles

from mirra_backend.config import config
from mirra_backend.data.database import global_init
from mirra_backend.data.prepopulation import prepopulate_sync
from mirra_backend.exceptions import MIRRAException, custom_exception_handler


@asynccontextmanager
async def lifespan(app: FastAPI):
    print("Starting MIRRA web service...")
    configure_db()
    if config.mqtt_enabled:
        print("Starting MQTT broker and parser...")
        configure_mqtt()
    else:
        print("MQTT has been disabled. The broker and parser will not be started.")
    configure_routes()
    yield


def configure_db() -> None:
    global_init()
    if not config.db_file.exists():
        config.db_file.parent.mkdir(parents=True, exist_ok=True)
        prepopulate_sync()
    config.location_images_folder.mkdir(parents=True, exist_ok=True)
    if not config.mqtt_psk_file.exists():
        config.mqtt_psk_file.parent.mkdir(parents=True, exist_ok=True)
        open(config.mqtt_psk_file, "a").close()


def configure_mqtt() -> None:
    from mirra_backend.mqtt_broker import mqtt_broker

    mqtt_broker.start()

    from mirra_backend.mqtt_parser import mqtt_parser

    mqtt_parser.start()


def configure_routes() -> None:
    if not config.vite_dist_folder.exists():
        raise FileNotFoundError(
            f"The vite dist folder '{config.vite_dist_folder}' does not exist. Ensure it has been built with `npm build`."
        )
    app.mount(
        "/" + config.vite_dist_folder.name,
        StaticFiles(directory=config.vite_dist_folder),
        name=config.vite_dist_folder.name,
    )

    from mirra_backend.routes import export, gateway, index

    app.include_router(export.router)
    app.include_router(gateway.router)
    app.include_router(index.router)


app = FastAPI(
    lifespan=lifespan, exception_handlers={MIRRAException: custom_exception_handler}
)
