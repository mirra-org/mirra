from fastapi import FastAPI
from fastapi.templating import Jinja2Templates

from mirra_backend.config import config
from mirra_backend.exceptions import MIRRAException, custom_exception_handler

app = FastAPI()
app.add_exception_handler(MIRRAException, custom_exception_handler)
templates = Jinja2Templates(directory=config.templates_folder)
