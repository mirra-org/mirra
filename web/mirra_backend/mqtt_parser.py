import logging
from time import sleep
from typing import Any

import paho.mqtt.client as mqtt
from paho.mqtt.enums import CallbackAPIVersion
from paho.mqtt.properties import Properties
from paho.mqtt.reasoncodes import ReasonCode

from mirra_backend.config import config
from mirra_backend.crud.measurement import process_measurement_sync

log = logging.getLogger(__name__)


class _MQTTParser:
    def __init__(self) -> None:
        self.client = mqtt.Client(
            callback_api_version=CallbackAPIVersion.VERSION2, client_id="mirra_backend"
        )
        self.client.suppress_exceptions = True

    @staticmethod
    def mirra_on_connect(
        client: mqtt.Client,
        userdata: Any,
        flags: mqtt.ConnectFlags,
        reason: ReasonCode,
        properties: Properties | None,
    ) -> None:
        client.subscribe(config.mqtt_prefix)
        log.info(
            f"mqtt parser connected to broker with prefix {config.mqtt_prefix}. Result reason: {reason}"
        )

    @staticmethod
    def mirra_on_message(
        client: mqtt.Client, userdata: Any, msg: mqtt.MQTTMessage
    ) -> None:
        hierarchy = str(msg.topic).split("/")
        gateway_mac = hierarchy[1]
        node_mac = hierarchy[2]

        process_measurement_sync(gateway_mac, node_mac, msg.payload)

    def start(self) -> None:
        self.client.on_connect = self.mirra_on_connect
        self.client.on_message = self.mirra_on_message
        while True:
            try:
                self.client.connect(config.mqtt_host, config.mqtt_port, 60)
                self.client.loop_start()
                log.info("mqtt parser background thread started")
                return
            except ConnectionRefusedError:
                log.info(
                    f"mqtt parser connection to {config.mqtt_host}:{config.mqtt_port} refused. Retrying in 2s..."
                )
                sleep(2)

    def close(self) -> None:
        self.client.loop_stop()
        self.client.disconnect()


class _EmptyMQTTParser(_MQTTParser):
    def __init__(self) -> None:
        pass

    def start(self) -> None:
        pass

    def close(self) -> None:
        pass


if config.mqtt_enabled:
    mqtt_parser = _MQTTParser()
else:
    mqtt_parser = _EmptyMQTTParser()
