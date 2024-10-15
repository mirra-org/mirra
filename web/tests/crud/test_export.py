import pytest

from mirra_backend.crud.export import export_all_csv


@pytest.mark.asyncio
async def test_export_all_csv(test_db):
    await export_all_csv(test_db)
