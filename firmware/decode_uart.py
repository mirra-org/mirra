import csv
import json
from argparse import ArgumentParser
from datetime import datetime, timezone
from pathlib import Path

sensors = {}


def get_sensors(path: Path):
    with path.open() as infile:
        sensors_json = json.load(infile)
    for sensor in sensors_json:
        sensors.update({sensor["id"]: (sensor["name"], sensor["unit"])})


def main(gateway_mac: str, out_path: Path, *in_paths: Path) -> None:
    with out_path.open("a") as out:
        out_csv = csv.writer(out)
        for path in in_paths:
            with path.open() as infile:
                timestamp = None
                node_mac = None
                for line in infile:
                    if len(line) <= 1:
                        node_mac = None
                        continue
                    if node_mac is None:
                        split_line = line.split()[:3]
                        node_mac = split_line[0]
                        timestamp = " ".join(split_line[1:])
                        timestamp = str(datetime.fromisoformat(timestamp))
                    else:
                        sensor_key, value = line.split()[:2]
                        sensor_name, sensor_unit = sensors[int(sensor_key)]
                        out_csv.writerow(
                            [
                                timestamp,
                                str(datetime.fromtimestamp(0, tz=timezone.utc)),
                                gateway_mac,
                                node_mac,
                                value,
                                sensor_name,
                                sensor_unit,
                            ]
                        )


if __name__ == "__main__":
    parser = ArgumentParser(prog="mirra_decode")
    parser.add_argument("file", nargs="+")
    parser.add_argument("-g", "--gateway", required=True)
    parser.add_argument("-s", "--sensors", required=True)
    parser.add_argument("-o", "--out", default="decoded.csv")
    parsed = parser.parse_args()
    get_sensors(Path(parsed.sensors).resolve())
    main(
        parsed.gateway,
        Path(parsed.out).resolve(),
        *[Path(file).resolve() for file in parsed.file],
    )
