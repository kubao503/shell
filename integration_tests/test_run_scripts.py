from fixtures import shell_container, TestContainer

def test_run_script(shell_container: TestContainer):
    result = shell_container.run_uxsh(r"echo 'cd /tmp; pwd' > change_directory.sh; . change_directory.sh")
    assert result.exit_code == 0
    assert result.output.decode("utf-8") == '/tmp\n'
