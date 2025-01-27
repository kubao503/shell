import tarfile
from io import BytesIO

import pytest
from docker.models.containers import Container
from testcontainers.core.container import DockerContainer
from testcontainers.core.image import DockerImage

image = DockerImage(path='..').build()

class TestContainer:
    __test__ = False
    def __init__(self, container):
        self.container: Container = container

    def run(self, *args, **kwargs):
        return self.container.exec_run(*args, **kwargs)

    def run_uxsh(self, cmd: str):
        return self.run(['uxsh', '-c', cmd])

    def read_file(self, path: str) -> bytes:
        try:
            bits, _ = self.container.get_archive(path)
        except Exception:
            raise Exception('Cannot load file')
        with tarfile.open(fileobj=BytesIO(b''.join(bits))) as tar:
            file_info = tar.getmember(path.lstrip('/'))
            extracted_file = tar.extractfile(file_info)
            return extracted_file.read()

@pytest.fixture
def shell_container(request):
    container = (DockerContainer(str(image))
                 .with_command('tail -f'))
    container.start()
    def stop_container():
        container.stop()
    request.addfinalizer(stop_container)
    return TestContainer(container.get_wrapped_container())