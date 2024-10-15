from fastapi import APIRouter
from fastapi.responses import HTMLResponse

from .common import Templater

router = APIRouter(default_response_class=HTMLResponse)


@router.get("/")
async def export_page(template: Templater) -> HTMLResponse:
    return template("index.html")
