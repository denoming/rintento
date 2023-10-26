import requests
import logging
import entity.utils as utils

LOGGER = logging.getLogger(__name__)


def test_message_recognition(server_host: str, server_port: int):
    message = "Turn off the light in the bedroom"
    r = requests.post(
        utils.get_recognise_message_url(server_host, server_port, message),
        timeout=(3, 10)
    )
    assert r.ok is True, \
        f"Invalid response status code: {r.reason}"

    LOGGER.info(f"Response is received: elapsed <{r.elapsed}>")

    result = r.json()
    assert result["status"] is True, \
        f"Message recognition has failed {result['error']}"
