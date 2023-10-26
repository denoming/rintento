import pathlib
import pytest
import entity.server


def pytest_addoption(parser: pytest.Parser):
    parser.addoption("--host", action="store", type=str, default="127.0.0.1",
                     help="The server hostname with operable service")
    parser.addoption("--port", action="store", type=int,
                     help="The server port with operable service", default=8080)
    parser.addoption("--exec_path", action="store",
                     help="The path to server executable file")


@pytest.fixture(scope="session")
def exec_path(pytestconfig: pytest.Config) -> pathlib.Path | None:
    option = pytestconfig.getoption("exec_path")
    return pathlib.Path(option) if option is not None else None


@pytest.fixture(scope="session")
def server_host(pytestconfig: pytest.Config) -> str:
    return pytestconfig.getoption("host")


@pytest.fixture(scope="session")
def server_port(pytestconfig: pytest.Config) -> int:
    return pytestconfig.getoption("port")


@pytest.fixture(autouse=True, scope="session")
def start_server(exec_path: pathlib.Path | None) -> bool:
    if exec_path is not None:
        service = entity.server.Server(exec_path)
        assert service.start() is True, \
            f"Unable to start server: {exec_path}"
        yield service.active()
        service.stop()
