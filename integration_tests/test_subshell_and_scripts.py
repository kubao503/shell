from fixtures import shell_container, TestContainer

def test_dot_construction(shell_container: TestContainer):
    result = shell_container.run_uxsh(r"echo 'cd /tmp; pwd' > change_directory.sh; . change_directory.sh")
    assert result.exit_code == 0
    assert result.output.decode("utf-8") == '/tmp\n'

def test_dot_construction_pipeline(shell_container: TestContainer):
    result = shell_container.run_uxsh(r"echo 'echo -e orange\nplum\napple | grep a' > program.sh | . program.sh | grep o")
    assert result.exit_code == 0
    assert result.output.decode("utf-8") == 'orange\n'

def test_braces_construction(shell_container: TestContainer):
    result = shell_container.run_uxsh(r"(echo -e orange\nplum | grep p)")
    assert result.exit_code == 0
    assert result.output.decode("utf-8") == 'plum\n'

def test_braces_construction_pipeline(shell_container: TestContainer):
    result = shell_container.run_uxsh(r"(echo -e orange\nplum\napple | grep a) | grep o")
    assert result.exit_code == 0
    assert result.output.decode("utf-8") == 'orange\n'