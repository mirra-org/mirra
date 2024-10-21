from datetime import datetime

from fastapi import APIRouter, Request
from fastapi.responses import (
    FileResponse,
    HTMLResponse,
    PlainTextResponse,
    RedirectResponse,
)
from htpy import a, button, code, h3, h4, pre

from mirra_backend.config import config
from mirra_backend.crud.common import Session
from mirra_backend.crud.export import export_all_csv
from mirra_backend.data.database import get_ddl
from mirra_backend.layout import layout

router = APIRouter(prefix="/export", default_response_class=HTMLResponse)


def generate_filename(extension: str) -> str:
    return f"mirra-db-{datetime.now().strftime('%Y-%m-%d')}.{extension}"


@router.get("/")
async def export_page(request: Request) -> HTMLResponse:
    csv_filename = generate_filename("csv")
    db_filename = generate_filename("sqlite")

    return HTMLResponse(
        layout(
            [
                h3["export"],
                h4["csv file"],
                a(
                    href=str(request.url_for("download_csv", filename=csv_filename)),
                    download=csv_filename,
                )[button["Download"]],
                h4["database file"],
                a(
                    href=str(request.url_for("download_db", filename=db_filename)),
                    download=db_filename,
                )[button["Download"]],
                h4["database DDL schema:"],
                pre[code[get_ddl()]],
            ],
            titled="export",
        )
    )


@router.get("/download_db/{filename}", response_class=FileResponse, response_model=None)
async def download_db(
    filename: str, request: Request
) -> RedirectResponse | FileResponse:
    if filename != generate_filename("sqlite"):
        return RedirectResponse(
            f"{request.url.path[:-len(filename)]}{generate_filename('sqlite')}"
        )
    return FileResponse(
        config.db_file, media_type="application/vnd.sqlite3", filename=filename
    )


@router.get(
    "/download_csv/{filename}", response_class=PlainTextResponse, response_model=None
)
async def download_csv(
    filename: str, request: Request, session: Session
) -> RedirectResponse | PlainTextResponse:
    if filename != generate_filename("csv"):
        return RedirectResponse(
            f"{request.url.path[:-len(filename)]}{generate_filename('csv')}"
        )
    return PlainTextResponse(await export_all_csv(session), media_type="text/csv")
