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
import logging
import time
import os
import entity.utils as utils

LOGGER = logging.getLogger(__name__)


def test_message_recognition(server_host: str, server_port: int, start_server: bool):
    if start_server is not False:
        LOGGER.info("Local server is active")

    res = requests.post(
        utils.get_recognise_message_url(server_host, server_port, "Turn off the light in the bedroom"),
        timeout=(3, 10)
    )
    assert res.ok is True, \
        f"Invalid response status code: {res.reason}"
    LOGGER.info(f"Response is received: elapsed <{res.elapsed}>")

    result = res.json()
    assert result["status"] is True, \
        f"Message recognition has failed {result['error']}"

    LOGGER.info(f"Waiting for automation")
    time.sleep(3)

    signal_file = "/tmp/light-turn-off-bedroom.txt"
    assert os.path.exists(signal_file), \
        "Signal file is absent"
    os.remove(signal_file)
