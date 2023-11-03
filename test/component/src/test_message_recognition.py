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
