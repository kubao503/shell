from fixtures import shell_container, TestContainer

def test_subshell(shell_container: TestContainer):
    result = shell_container.run_uxsh(r"(echo -e orange\nplum | grep p)")
    assert result.exit_code == 0
    assert result.output.decode("utf-8") == 'plum\n'

def test_subshell_pipeline_out(shell_container: TestContainer):
    result = shell_container.run_uxsh(r"(echo -e orange\nplum\napple | grep a) | grep o")
    assert result.exit_code == 0
    assert result.output.decode("utf-8") == 'orange\n'

def test_subshell_pipeline_in(shell_container: TestContainer):
    result = shell_container.run_uxsh(r"echo -e orange\nplum\napple | (grep o | grep a)")
    assert result.exit_code == 0
    assert result.output.decode("utf-8") == 'orange\n'

def test_subshell_pipeline_in_out(shell_container: TestContainer):
    result = shell_container.run_uxsh(r"echo -e orange\nplum\napple | (grep o) | grep a")
    assert result.exit_code == 0
    assert result.output.decode("utf-8") == 'orange\n'

def test_subshell_pipeline_redirection_out(shell_container: TestContainer):
    result = shell_container.run_uxsh(r"echo -e orange\nplum\napple | (grep o | grep a > file.txt)")
    assert result.exit_code == 0
    assert shell_container.read_file('file.txt') == b'orange\n'

def test_subshell_pipeline_redirection_in(shell_container: TestContainer):
    result = shell_container.run_uxsh(r"echo -e orange\nplum > file.txt; (grep l < file.txt)")
    assert result.exit_code == 0
    assert result.output.decode("utf-8") == 'plum\n'

def test_subshell_pipeline_redirection_in_out(shell_container: TestContainer):
    result = shell_container.run_uxsh(r"echo -e orange\nplum > file.txt; (grep l < file.txt > result.txt)")
    assert result.exit_code == 0
    assert shell_container.read_file('result.txt') == b'plum\n'

def test_subshell_double(shell_container: TestContainer):
    result = shell_container.run_uxsh(r"(echo -e orange\nplum\napple) | (grep o) | grep a")
    assert result.exit_code == 0
    assert result.output.decode("utf-8") == 'orange\n'
