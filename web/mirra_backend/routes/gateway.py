import asyncio
from secrets import compare_digest, token_hex
from typing import Annotated

from fastapi import APIRouter, BackgroundTasks, Form, Header
from fastapi.responses import HTMLResponse, PlainTextResponse
from htpy import button, div, form, h3, input, label, li, ul
from sqlalchemy.ext.asyncio import AsyncSession

import mirra_backend.crud.gateway as crud_gateway
from mirra_backend.config import config
from mirra_backend.crud.common import Session
from mirra_backend.layout import layout
from mirra_backend.types.mac_address import MACAddress

router = APIRouter(prefix="/gateway", default_response_class=HTMLResponse)


@router.get("/add")
async def add_gateway_page() -> HTMLResponse:
    return HTMLResponse(
        layout(
            [
                h3["add gateway form"],
                form(hx_post="/gateway/add", hx_target="next div")[
                    label["gateway mac address"],
                    input(name="gateway_mac"),
                    button(class_="btn primary")["Submit"],
                ],
                div,
            ],
            titled="add gateway",
        )
    )


gateways_codes_psks: dict[MACAddress, tuple[str, str]] = {}


async def timeout_gateway_code(session: AsyncSession, gateway_mac: MACAddress) -> None:
    access_code = gateways_codes_psks[gateway_mac][0]
    await asyncio.sleep(config.gateway_access_code_time)
    if gateway_mac in gateways_codes_psks:
        # do not pop if access code has changed: this implies a new request was done while asleep
        if gateways_codes_psks[gateway_mac][0] == access_code:
            gateways_codes_psks.pop(gateway_mac)


@router.post("/add")
async def add_gateway(
    session: Session,
    gateway_mac: Annotated[str, Form()],
    background_tasks: BackgroundTasks,
) -> HTMLResponse:
    gateway_mac = MACAddress(gateway_mac)
    access_code = token_hex(4)
    psk = token_hex(32)
    gateways_codes_psks[gateway_mac] = (access_code, psk)
    background_tasks.add_task(timeout_gateway_code, session, gateway_mac)
    return ul[li[f"Access code : {access_code}"]]


"""
@router.get("/psk")
async def show_psk(
    session: Session, gateway_mac: Annotated[str, Form()]
) -> HTMLResponse:
    gateway_mac = MACAddress(gateway_mac)
    if gateway_mac not in gateways_codes_psks:
        return
    _, psk = gateways_codes_psks.pop(gateway_mac)
    await crud_gateway.add_gateway(session, gateway_mac, psk)
"""


@router.get("/code", response_class=PlainTextResponse)
async def verify_gateway_code(
    session: Session,
    mirra_gateway: Annotated[str, Header()],
    mirra_access_code: Annotated[str, Header()],
) -> PlainTextResponse:
    mirra_gateway = MACAddress(mirra_gateway)
    if mirra_gateway not in gateways_codes_psks:
        return PlainTextResponse(status_code=404)
    correct_code, psk = gateways_codes_psks[mirra_gateway]
    if not compare_digest(mirra_access_code, correct_code):
        return PlainTextResponse(status_code=401)
    gateways_codes_psks.pop(mirra_gateway)
    await crud_gateway.add_gateway(session, mirra_gateway, psk)
    return PlainTextResponse(psk)
