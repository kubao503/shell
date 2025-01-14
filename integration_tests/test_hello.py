from fixtures import shell_container, TestContainer


def test_hello(shell_container: TestContainer):
    result = shell_container.run_uxsh('echo hello')
    assert result.exit_code == 0
    assert result.output.decode("utf-8") == 'hello\n'

def test_file(shell_container: TestContainer):
    result = shell_container.run_uxsh('echo hello > /hello.txt')
    assert result.exit_code == 0
    assert shell_container.read_file('/hello.txt') == b'hello\n'