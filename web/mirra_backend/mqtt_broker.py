from signal import SIGHUP
from subprocess import Popen

from mirra_backend.config import config
from mirra_backend.types.mac_address import MACAddress


class _MQTTBroker:
    process: Popen = None

    def start(self) -> None:
        self.process = Popen(["/sbin/mosquitto", "-c", config.mqtt_broker_config_file])

    def add_psk(self, mac: MACAddress, psk: str) -> None:
        with open(config.mqtt_psk_file, "a") as pskfile:
            pskfile.write(f"{mac.replace(':', '')}:{psk}\n")
        self.reload()

    def remove_psk(self, mac: MACAddress, reload: bool = True) -> None:
        mac_str = mac.replace(":", "")
        with open(config.mqtt_psk_file, "r+") as pskfile:
            lines = pskfile.readlines()
            pskfile.seek(0)
            for line in lines:
                if mac_str not in line:
                    pskfile.write(line)
            pskfile.truncate()
        if reload:
            self.reload()

    def update_psk(self, mac: MACAddress, psk: str) -> None:
        self.remove_psk(mac, reload=False)
        self.add_psk(mac, psk)

    def reload(self) -> None:
        self.process.send_signal(SIGHUP)


mqtt_broker = _MQTTBroker()
