import fastapi
from fastapi.responses import JSONResponse
from pydantic import RootModel
from pydantic.dataclasses import dataclass


class MIRRAException(Exception):
    pass


@dataclass
class ValidationException(MIRRAException):
    constraint: str
    msg: str
    status_code: int = 422


async def custom_exception_handler(
    request: fastapi.Request, exc: MIRRAException
) -> JSONResponse:
    return JSONResponse(
        content=RootModel[type(exc)](exc).model_dump(
            exclude=set(("status_code", "headers"))
        ),
        status_code=exc.status_code,
        headers=exc.headers if hasattr(exc, "headers") else None,
    )
