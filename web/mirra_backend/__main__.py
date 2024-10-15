import os

import uvicorn
from fastapi.staticfiles import StaticFiles

from mirra_backend.app import app, templates
from mirra_backend.config import config
from mirra_backend.data.database import global_init
from mirra_backend.data.prepopulation import prepopulate


def main() -> None:
    print(f"Running as EUID '{os.geteuid()}'.")
    configure_db()
    configure_mqtt()
    configure_routes()
    configure_templates()
    uvicorn.run(
        "mirra_backend.app:app",
        host=config.host,
        port=config.port,
        reload=config.dev,
        proxy_headers=True,
        forwarded_allow_ips="*",
    )


def configure_db() -> None:
    if not config.db_file.exists():
        global_init(delete_tables=False)
        prepopulate()
    else:
        global_init(delete_tables=False)

    if not config.mqtt_psk_file.exists():
        open(config.mqtt_psk_file, "a").close()


def configure_mqtt() -> None:
    from mirra_backend.mqtt_broker import mqtt_broker

    mqtt_broker.start()
    from mirra_backend.mqtt_parser import mqtt_parser

    mqtt_parser.start()


def configure_templates() -> None:
    from mirra_backend.vite_loader import vite_asset, vite_hmr_client

    templates.env.globals["vite_asset"] = vite_asset
    templates.env.globals["vite_hmr_client"] = vite_hmr_client


def configure_routes() -> None:
    app.mount(
        "/" + config.vite_dist_folder.name,
        StaticFiles(directory=config.vite_dist_folder),
        name=config.vite_dist_folder.name,
    )

    from mirra_backend.routes import export, gateway, index

    app.include_router(export.router)
    app.include_router(gateway.router)
    app.include_router(index.router)


if __name__ == "__main__":
    main()
