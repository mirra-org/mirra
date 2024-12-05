import json
import logging
from pathlib import Path

import sqlalchemy as sa
from sqlalchemy.ext.asyncio import AsyncSession

from mirra_backend.config import DBPrefill, config

from .modelbase import SqlAlchemyBase

log = logging.getLogger(__name__)


async def prepopulate(
    session: AsyncSession, db_prefill: DBPrefill | None = None
) -> None:
    if db_prefill is None:
        db_prefill = config.db_prefill
    match db_prefill:
        case DBPrefill.empty:
            return
        case DBPrefill.default:
            prepopulation_folder = (
                Path(__file__).parent.resolve() / "prepopulation_default"
            )
        case DBPrefill.test:
            prepopulation_folder = (
                Path(__file__).parent.resolve() / "prepopulation_test"
            )
        case DBPrefill.custom:
            prepopulation_folder = config.prepopulation_folder
    await _prepopulate(session, prepopulation_folder)


async def _prepopulate(session: AsyncSession, folder: Path) -> None:
    for table in SqlAlchemyBase.metadata.sorted_tables:
        log.info(f"Prefilling database with table '{table.name}'...")
        file_path = folder / f"{table.name}.json"
        if not file_path.exists():
            continue
        with open(file_path) as file:
            rows = json.load(file)
        await session.execute(sa.insert(table), rows)
    await session.commit()
