import pytest

host_default = "172.16.238.2:5020"


def pytest_addoption(parser):
    parser.addoption(
        "--host", action="store", default=host_default, help="Setup connection string"
    )


@pytest.fixture
def host(request):
    return request.config.getoption("--host")
