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

import requests
import entity.utils as utils
import logging
import wave
import pathlib
import time
import os

LOGGER = logging.getLogger(__name__)
CHUNK_SIZE = 512
AUDIO_FILE = pathlib.Path.cwd().joinpath("asset/audio/turn-on-light-bedroom.wav")


def audio_data_gen():
    with wave.open(str(AUDIO_FILE), "rb") as file:
        LOGGER.debug(f"Number of channels: {file.getnchannels()}")
        LOGGER.debug(f"Sample width: {file.getsampwidth()}")
        LOGGER.debug(f"Frame rage: {file.getframerate()}")
        LOGGER.debug(f"Number of frames: {file.getnframes()}")

        data = file.readframes(CHUNK_SIZE)
        while len(data) > 0:
            yield data
            data = file.readframes(CHUNK_SIZE)


def test_speech_recognition(server_host: str, server_port: int, start_server: bool):
    res = requests.post(
        utils.get_recognise_speech_url(server_host, server_port),
        data=audio_data_gen(),
        timeout=(3, 10),
        headers={"Expect": "100-continue"},
    )
    assert res.ok is True, \
        f"Invalid response status code: {res.reason}"
    LOGGER.info(f"Response is received: elapsed <{res.elapsed}>")

    result = res.json()
    assert result["status"] is True, \
        f"Speech recognition has failed {result['error']}"

    LOGGER.info(f"Waiting for automation")
    time.sleep(3)

    signal_file = "/tmp/light-turn-on-bedroom.txt"
    assert os.path.exists(signal_file), \
        "Signal file is absent"
    os.remove(signal_file)
