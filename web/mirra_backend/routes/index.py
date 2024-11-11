from fastapi import APIRouter, Request
from fastapi.responses import HTMLResponse
from htpy import a, h1, li, p, ul

from mirra_backend.layout import layout

router = APIRouter(default_response_class=HTMLResponse)


@router.get("/")
async def export_page(request: Request) -> HTMLResponse:
    request.url_for("add_gateway_page")
    return HTMLResponse(
        layout(
            [
                h1["mirra"],
                p["home page"],
                ul[
                    li[a(href=str(request.url_for("add_gateway_page")))["add gateway"]],
                    li[a(href=str(request.url_for("export_page")))["download db"]],
                ],
            ],
            titled="home",
        )
    )
