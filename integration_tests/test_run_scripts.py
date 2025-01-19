from fixtures import shell_container, TestContainer

def test_run_script(shell_container: TestContainer):
    result = shell_container.run_uxsh(r"echo 'cd /tmp; pwd' > change_directory.sh; . change_directory.sh")
    assert result.exit_code == 0
    assert result.output.decode("utf-8") == '/tmp\n'

def test_run_script_pipeline_in(shell_container: TestContainer):
    result = shell_container.run_uxsh(r"echo 'echo -e orange\nplum\napple | grep a | grep o' > program.sh | . program.sh")
    assert result.exit_code == 0
    assert result.output.decode("utf-8") == 'orange\n'

def test_run_script_pipeline_in_out(shell_container: TestContainer):
    result = shell_container.run_uxsh(r"echo 'echo -e orange\nplum\napple | grep a' > program.sh | . program.sh | grep o")
    assert result.exit_code == 0
    assert result.output.decode("utf-8") == 'orange\n'

def test_run_script_and_subshell(shell_container: TestContainer):
    result = shell_container.run_uxsh(r"echo 'echo -e orange\nplum\napple | grep a' > program.sh | . program.sh | (grep o)")
    assert result.exit_code == 0
    assert result.output.decode("utf-8") == 'orange\n'
