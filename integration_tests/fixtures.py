import pytest
from testcontainers.core.container import DockerContainer
from testcontainers.core.image import DockerImage

image = DockerImage(path='..').build()

@pytest.fixture
def shell_container(request):
    container = DockerContainer(str(image)).with_command('tail -f')
    container.start()
    def stop_container():
        container.stop()
    request.addfinalizer(stop_container)
    return container.get_wrapped_container()