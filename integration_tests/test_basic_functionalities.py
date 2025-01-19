from fixtures import shell_container, TestContainer

def test_hello(shell_container: TestContainer):
    result = shell_container.run_uxsh(r'echo hello')
    assert result.exit_code == 0
    assert result.output.decode("utf-8") == 'hello\n'

def test_file(shell_container: TestContainer):
    result = shell_container.run_uxsh(r'echo hello > /file.txt')
    assert result.exit_code == 0
    assert shell_container.read_file('/file.txt') == b'hello\n'

def test_multiple_arguments(shell_container: TestContainer):
    result = shell_container.run_uxsh(r'echo -e apple\napple\nBanana\napple | grep banana -i -n')
    assert result.exit_code == 0
    assert result.output.decode("utf-8") == '3:Banana\n'

def test_multiple_arguments_short(shell_container: TestContainer):
    result = shell_container.run_uxsh(r'echo -e apple\napple\nBanana\napple | grep banana -in')
    assert result.exit_code == 0
    assert result.output.decode("utf-8") == '3:Banana\n'

def test_long_pipeline(shell_container: TestContainer):
    result = shell_container.run_uxsh(r'echo -e apple\nplum\nbanana\napple\nplum\norange\nplum\napple\nbanana\napple | sort | uniq | grep -v apple | head -n 2 | tee output.txt')
    assert result.exit_code == 0
    assert result.output.decode("utf-8") == 'banana\norange\n'
    assert shell_container.read_file('output.txt') == b'banana\norange\n'

def test_builtin_change_directory(shell_container: TestContainer):
    result = shell_container.run_uxsh(r'cd /tmp; mkdir trial_folder; cd trial_folder; pwd')
    assert result.exit_code == 0
    assert result.output.decode("utf-8") == '/tmp/trial_folder\n'

def test_change_directory_no_args(shell_container: TestContainer):
    result = shell_container.run_uxsh(r'cd ; pwd')
    assert result.exit_code == 0
    assert result.output.decode("utf-8") == '/root\n'

def test_redirections_in_out(shell_container: TestContainer):
    result = shell_container.run_uxsh(r"echo -e orange\nplum > file.txt | echo orange | grep l < file.txt")
    assert result.exit_code == 0
    assert result.output.decode("utf-8") == 'plum\n'

def test_multiple_redirections_in_out(shell_container: TestContainer):
    result = shell_container.run_uxsh(r"echo -e orange\nplum > file.txt > file2.txt | echo orange | grep l < file.txt < file2.txt > grep_result.txt")
    assert result.exit_code == 0
    assert shell_container.read_file('grep_result.txt') == b'plum\n'
    assert shell_container.read_file('file.txt') == b''
    assert shell_container.read_file('file2.txt') == b'orange\nplum\n'
