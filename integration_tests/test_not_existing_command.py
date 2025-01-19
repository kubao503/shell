from fixtures import shell_container, TestContainer

def test_not_existing_command(shell_container: TestContainer):
    result = shell_container.run_uxsh(r"notExistingCommand")
    assert result.exit_code == 127

def test_not_existing_command_end_pipeline(shell_container: TestContainer):
    result = shell_container.run_uxsh(r"ls | notExistingCommand")
    assert result.exit_code == 127

def test_not_existing_command_inside_pipeline(shell_container: TestContainer):
    result = shell_container.run_uxsh(r"ls | notExistingCommand | echo it_is_working")
    assert result.exit_code == 0
    assert result.output.decode("utf-8").endswith('it_is_working\n');

def test_not_existing_command_with_redirections_in(shell_container: TestContainer):
    shell_container.run_uxsh(r"echo -e apple\napple\nBanana\napple > file.txt")
    result = shell_container.run_uxsh(r"notExistingCommand < file.txt")
    assert result.exit_code == 1

def test_not_existing_command_with_redirections_out(shell_container: TestContainer):
    result = shell_container.run_uxsh(r"notExistingCommand > file.txt")
    assert result.exit_code == 127
    assert shell_container.read_file('file.txt') == b''
