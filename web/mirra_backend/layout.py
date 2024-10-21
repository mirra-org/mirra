from htpy import Element, HTMLElement, body, head, html, meta, title

from mirra_backend.vite_loader import vite_asset, vite_hmr_client


def layout(body_elements: list[Element], titled: str = "") -> HTMLElement:
    return html(lang="en")[
        head[
            meta(charset="utf-8"),
            meta(
                name="description",
                content="Microclimate Instrument for Remote Realtime Applications",
            ),
            vite_hmr_client(),
            vite_asset("layout.ts"),
            title[f"mirra | {titled}"],
        ],
        body[*body_elements],
    ]
