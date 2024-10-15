FROM python:3.11-slim AS mirra-poetry-builder
WORKDIR /setup
ENV PYTHONUNBUFFERED=1 \
    # prevents python creating .pyc files
    PYTHONDONTWRITEBYTECODE=1 \
    # pip
    PIP_NO_CACHE_DIR=off \
    PIP_DISABLE_PIP_VERSION_CHECK=on \
    PIP_DEFAULT_TIMEOUT=100 \
    # make poetry install to this location
    POETRY_HOME="/opt/poetry" \
    # make poetry create the virtual environment in the project's root
    # it gets named `.venv`
    POETRY_VIRTUALENVS_IN_PROJECT=true \
    # do not ask any interactive question
    POETRY_NO_INTERACTION=1 \
    # this is where our virtual environment will live
    VENV_PATH="/setup/.venv"
# prepend poetry and venv to path
ENV PATH="$POETRY_HOME/bin:$VENV_PATH/bin:$PATH"
# install poetry - respects $POETRY_VERSION & $POETRY_HOME
RUN apt-get update \
    && apt-get install --no-install-recommends -y \
        curl \
        # deps for building python deps
        build-essential
RUN curl -sSL https://install.python-poetry.org | python3 -
# copy poetry requirement files
COPY pyproject.toml poetry.lock ./
# install poetry prod deps - uses $POETRY_VIRTUALENVS_IN_PROJECT internally
RUN poetry install --without dev --no-root
    
FROM node:22-slim AS mirra-vite-builder
WORKDIR /setup
# install pnpm
ENV PNPM_HOME="/pnpm"
ENV PATH="$PNPM_HOME:$PATH"
RUN corepack enable
# copy vite build requirement files
COPY package.json tsconfig.json vite.config.ts pnpm-lock.yaml ./ 
COPY ./front ./front/
# install pnpm deps
RUN pnpm install -P
# run pnpm vite build
RUN pnpm build

FROM python:3.11-slim AS mirra-web
#install mosquitto
RUN apt-get update \
    && apt-get install --no-install-recommends -y \
        mosquitto
# copy build artifacts to prod image
WORKDIR /mirra_web
ENV VENV_PATH="/mirra_web/.venv"
ENV PATH="$VENV_PATH/bin:$PATH"
COPY --from=mirra-poetry-builder /setup ./
COPY --from=mirra-vite-builder /setup/front/dist ./front/dist/
COPY ./mirra_backend ./mirra_backend/

COPY ./mosquitto ./mosquitto/

# run backend
CMD ["python3", "-m", "mirra_backend"]
