import requests
import entity.utils as utils
import logging
import wave
import pathlib

LOGGER = logging.getLogger(__name__)
CHUNK_SIZE = 512
AUDIO_FILE = pathlib.Path.cwd().joinpath("asset/audio/turn-off-the-light-bedroom.wav")


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


def test_speech_recognition(server_host: str, server_port: int):
    r = requests.post(
        utils.get_recognise_speech_url(server_host, server_port),
        data=audio_data_gen(),
        timeout=(3, 10),
        headers={"Expect": "100-continue"}
    )
    assert r.ok is True, \
        f"Invalid response status code: {r.reason}"

    LOGGER.info(f"Response is received: elapsed <{r.elapsed}>")

    result = r.json()
    assert result["status"] is True, \
        f"Speech recognition has failed {result['error']}"
