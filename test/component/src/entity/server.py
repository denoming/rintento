import time
import subprocess
import pathlib
import logging

LOGGER = logging.getLogger(__name__)


class Server:
    __process: subprocess.Popen | None
    __executable: pathlib.Path

    def __init__(self, executable: pathlib.Path, stdout: pathlib.Path = "/tmp/stdout.txt"):
        self.__stdout = open(stdout, "wt")
        self.__process = None
        self.__executable = executable

    def __del__(self):
        self.__stdout.close()

    def active(self) -> bool:
        if self.__process is not None:
            rc = self.__process.poll()
            return rc is None
        else:
            return False

    def start(self, delay: int = 1) -> bool:
        assert self.__process is None, \
            "Close service first"
        try:
            self.__process = subprocess.Popen(
                args=[f"{self.__executable.absolute()}"],
                stdout=self.__stdout,
                stderr=subprocess.STDOUT
            )
            rc = self.__process.poll()
            if rc is None:
                time.sleep(delay)
            return rc is None
        except OSError as e:
            LOGGER.error(f"Unable to run '{self.__executable.name}' service: {e.strerror}")
            return False
        except ValueError:
            LOGGER.error(f"Unable to run '{self.__executable.name}' service")
            return False

    def stop(self) -> bool:
        if self.__process is not None:
            self.__process.terminate()
            try:
                self.__process.wait()
            except TimeoutError:
                LOGGER.error(f"Timeout occurred on stop server")
                return False
            self.__process = None
        return True
