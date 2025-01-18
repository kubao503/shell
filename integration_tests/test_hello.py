from fixtures import shell_container, TestContainer
import pytest

def test_hello(shell_container: TestContainer):
    result = shell_container.run_uxsh('echo hello')
    assert result.exit_code == 0
    assert result.output.decode("utf-8") == 'hello\n'

def test_file(shell_container: TestContainer):
    result = shell_container.run_uxsh('echo hello > /hello.txt')
    assert result.exit_code == 0
    assert shell_container.read_file('/hello.txt') == b'hello\n'

def test_long_pipeline(shell_container: TestContainer):
    result = shell_container.run_uxsh(r'echo -e apple\nplum\nbanana\napple\nplum\norange\nplum\napple\nbanana\napple | sort | uniq | grep -v apple | head -n 2 | tee output.txt')
    assert result.exit_code == 0
    assert result.output.decode("utf-8") == 'banana\norange\n'
    assert shell_container.read_file('output.txt') == b'banana\norange\n'

def test_assign_variable(shell_container: TestContainer):
    result = shell_container.run_uxsh(r'var=l ; echo -e plum\napple\norange | grep $var')
    assert result.exit_code == 0
    assert result.output.decode("utf-8") == 'plum\napple\n'

def test_not_existing_variable(shell_container: TestContainer):
    result = shell_container.run_uxsh(r'echo $notExistingVariable')
    assert result.exit_code == 0
    assert result.output.decode("utf-8") == '\n'
