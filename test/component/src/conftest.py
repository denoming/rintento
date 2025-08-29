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
import pytest
import entity.utils as utils
import entity.server
import entity.config as config
import shutil
import os


def pytest_addoption(parser: pytest.Parser):
    parser.addoption("--server-host", action="store", type=str, default="127.0.0.1",
                     help="The server hostname with operable service")
    parser.addoption("--server-port", action="store", type=int,
                     help="The server port", default=8080)
    parser.addoption("--exec-path", action="store", type=str,
                     help="The server executable file")
    parser.addoption("--wit-auth", action="store", type=str,
                     help="The Wit.AI authentication token")


@pytest.fixture(scope="session")
def server_host(pytestconfig: pytest.Config) -> str:
    return pytestconfig.getoption("--server-host")


@pytest.fixture(scope="session")
def server_port(pytestconfig: pytest.Config) -> int:
    return pytestconfig.getoption("--server-port")


@pytest.fixture(scope="session")
def server_path(pytestconfig: pytest.Config) -> pathlib.Path | None:
    option = pytestconfig.getoption("--exec-path")
    return pathlib.Path(option) if option is not None else None


@pytest.fixture(scope="session")
def wit_auth(pytestconfig: pytest.Config) -> str | None:
    option = pytestconfig.getoption("--wit-auth")
    if option is None:
        option = os.environ["RINTENTO_WIT_AUTH"]
    return option


@pytest.fixture(scope="session")
def init_config(wit_auth: str | None) -> str:
    tmp_path = pathlib.Path("/tmp")

    shutil.copy(config.TEST_ROOT_PATH / "asset" / config.TEST_CONFIG_FILE, tmp_path)
    shutil.copy(config.TEST_ROOT_PATH / "asset" / config.TEST_CONFIG_SCRIPT1, tmp_path)
    shutil.copy(config.TEST_ROOT_PATH / "asset" / config.TEST_CONFIG_SCRIPT2, tmp_path)

    if wit_auth is not None:
        utils.replace_keywords(tmp_path / config.TEST_CONFIG_FILE, {"<WIT_AUTH>": wit_auth})

    old_config_path = os.environ.get("RINTENTO_CONFIG")
    os.environ["RINTENTO_CONFIG"] = str(tmp_path / config.TEST_CONFIG_FILE)
    new_config_path = os.environ.get("RINTENTO_CONFIG")
    yield new_config_path
    if old_config_path is not None:
        os.environ["RINTENTO_CONFIG"] = old_config_path


@pytest.fixture(autouse=True, scope="session")
def start_server(server_path: pathlib.Path | None, init_config) -> bool:
    if server_path is None:
        yield False
    else:
        service = entity.server.Server(server_path, pathlib.Path("/tmp/rintento-stdout.txt"))
        assert service.start() is True, \
            f"Unable to start server: {server_path}"
        yield service.active()
        service.stop()
