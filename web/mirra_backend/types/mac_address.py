from __future__ import annotations

from mirra_backend.exceptions import ValidationException


class MACAddress(str):
    def __new__(cls, v: str) -> MACAddress:
        return str.__new__(cls, cls.validate(v))

    @staticmethod
    def validate(v: str) -> str:
        if not isinstance(v, str):
            raise ValidationException("mac_address_type", "Value must be a string.")
        v = v.strip().upper().replace(":", "")
        try:
            int(v, 16)
        except ValueError:
            raise ValidationException(
                "mac_address_hex", "Value must be valid hexadecimal."
            )
        if len(v) != 12:
            raise ValidationException(
                "mac_address_length", "Value must be exactly 6 bytes long."
            )
        v = f"{v[0:2]}:{v[2:4]}:{v[4:6]}:{v[6:8]}:{v[8:10]}:{v[10:12]}"
        return v

    def to_bytes(self) -> bytes:
        return bytes.fromhex(self.replace(":", ""))
