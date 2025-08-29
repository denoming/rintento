# Copyright 2025 Denys Asauliak
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import pathlib
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


def replace_keywords(file_path: pathlib.Path, keywords: dict):
    data = file_path.read_text()
    for key, value in keywords.items():
        data = data.replace(key, value)
    file_path.write_text(data)
