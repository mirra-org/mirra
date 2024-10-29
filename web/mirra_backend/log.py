import logging
from mirra_backend.config import config
from uvicorn.logging import AccessFormatter, DefaultFormatter


def init_log() -> None:
    logging.basicConfig(
        level=logging.INFO,
        format="[%(asctime)s] %(name)s %(levelname)s: %(message)s",
        handlers=[logging.StreamHandler()]
        + (
            [logging.FileHandler(config.log_file)]
            if config.log_file is not None
            else []
        ),
    )


def configure_app_logging():
    default_fmt_string = "[%(asctime)s] %(name)s %(levelprefix)s %(message)s"
    access_fmt_string = '[%(asctime)s] %(name)s %(levelprefix)s %(client_addr)s - "%(request_line)s" %(status_code)s'

    uvicorn_default_logger = logging.getLogger("uvicorn")
    uvicorn_access_logger = logging.getLogger("uvicorn.access")

    for logger, fmt_cls, fmt_string in (
        (uvicorn_default_logger, DefaultFormatter, default_fmt_string),
        (uvicorn_access_logger, AccessFormatter, access_fmt_string),
    ):
        logger.handlers[0].setFormatter(fmt_cls(fmt_string, use_colors=True))
        if config.log_file is not None:
            handler = logging.FileHandler(config.log_file)
            handler.setFormatter(fmt_cls(fmt_string, use_colors=False))
            logger.addHandler(handler)
