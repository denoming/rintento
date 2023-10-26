from datetime import date
from urllib.parse import urlencode


def get_recognise_message_url(host: str, port: int, message: str, scheme: str = "http") -> str:
    return f"{scheme}://{host}:{port}{get_message_target(message)}"


def get_recognise_speech_url(host: str, port: int, scheme: str = "http") -> str:
    return f"{scheme}://{host}:{port}{get_speech_target()}"


def get_message_target(message: str) -> str:
    return "/message?" + urlencode({
        'v': date.today().strftime("%Y%m%d"),
        'q': message
    })


def get_speech_target() -> str:
    return "/speech?" + urlencode({
        'v': date.today().strftime("%Y%m%d")
    })
