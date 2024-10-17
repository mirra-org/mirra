import uvicorn

from mirra_backend.config import config


def main() -> None:
    uvicorn.run(
        "mirra_backend.app:app",
        host=config.host,
        port=config.port,
        reload=config.dev,
        proxy_headers=True,
        forwarded_allow_ips="*",
    )


if __name__ == "__main__":
    main()
