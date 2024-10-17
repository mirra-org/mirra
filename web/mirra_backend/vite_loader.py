import json
from urllib.parse import urljoin

from markupsafe import Markup

from mirra_backend.config import config

_manifest: dict[str] | None = None

if not config.dev:
    try:
        with open(config.vite_dist_folder / "manifest.json", "r") as infile:
            _manifest = json.load(infile)
    except (FileNotFoundError, json.JSONDecodeError) as e:
        e.add_note(
            "Error while attempting to open the Vite manifest. Ensure that `npm build` has been run before starting the server."
        )
        raise e


def _script_tag(src: str, attrs: dict[str] | None = None) -> str:
    attrs_str = ""
    if attrs is not None:
        attrs_str = " ".join(f'{key}="{value}"' for key, value in attrs.items())

    return f'<script {attrs_str} src="{src}"></script>'


def _css_tag(href: str) -> str:
    return f'<link rel="stylesheet" href="{href}" />'


def _vite_server_asset(path: str = "") -> str:
    base = f"http://{config.vite_host}:{config.vite_port}"
    return urljoin(base, path)


def vite_hmr_client() -> Markup:
    if config.dev:
        return Markup(
            _script_tag(_vite_server_asset("@vite/client"), {"type": "module"})
        )
    return Markup("")


def _dist_path(path: str) -> str:
    return f"/{config.vite_dist_folder.name}/{path}"


def _vite_asset(
    path: str,
    scripts_attrs: dict[str] = {"type": "module", "async": "", "defer": ""},
) -> str:
    if config.dev:
        return _script_tag(_vite_server_asset(path), scripts_attrs)
    if path not in _manifest:
        raise FileNotFoundError(f"Could not find path '{path}' in manifest.")

    entry = _manifest[path]
    tags = []

    # add dependent stylesheets
    if "css" in entry:
        for path in entry["css"]:
            tags.append(_css_tag(_dist_path(path)))

    # add dependent imports
    if "imports" in entry:
        for path in entry["imports"]:
            tags.append(_vite_asset(path))

    # add main script
    tags.append(_script_tag(_dist_path(entry["file"]), scripts_attrs))

    return "\n".join(tags)


def vite_asset(path: str) -> Markup:
    return Markup(_vite_asset(path))


__all__ = ["vite_asset", "vite_hmr_client"]
