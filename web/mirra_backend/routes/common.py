from typing import Annotated, Callable

from fastapi import Depends, Request
from starlette.templating import _TemplateResponse

from mirra_backend.app import templates


def templater(request: Request):
    def templater_wrapper(name: str, response_options: dict[str] = {}, **kwargs):
        return templates.TemplateResponse(request, name, kwargs, **response_options)

    return templater_wrapper


Templater = Annotated[Callable[[str, Request], _TemplateResponse], Depends(templater)]
