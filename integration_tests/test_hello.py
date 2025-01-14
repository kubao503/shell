from docker.models.containers import Container
from fixtures import shell_container

def test_hello(shell_container: Container):
    result = shell_container.exec_run("uxsh -c echo hello")
    assert result.exit_code == 0
    assert result.output.decode("utf-8") == 'hello\n'