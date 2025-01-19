from fixtures import shell_container, TestContainer

def test_not_existing_command(shell_container: TestContainer):
    result = shell_container.run_uxsh(r"notExistingCommand")
    assert result.exit_code == 0
    assert result.output.decode("utf-8") == ''

def test_not_existing_command_end_pipeline(shell_container: TestContainer):
    result = shell_container.run_uxsh(r"ls | notExistingCommand")
    assert result.exit_code == 0
    assert result.output.decode("utf-8") == ''

def test_not_existing_command_inside_pipeline(shell_container: TestContainer):
    result = shell_container.run_uxsh(r"ls | notExistingCommand | echo it_is_working")
    assert result.exit_code == 0
    assert result.output.decode("utf-8") == 'it_is_working\n'

def test_not_existing_command_with_redirections_in(shell_container: TestContainer):
    result = shell_container.run_uxsh(r"echo -e apple\napple\nBanana\napple > new_file.txt | blablabla < new_file.txt")
    assert result.exit_code == 0
    assert result.output.decode("utf-8") == ''
    # assert shell_container.read_file('empyt_file.txt') == b''

def test_not_existing_command_with_redirections_out(shell_container: TestContainer):
    result = shell_container.run_uxsh(r"blablabla > new_file.txt")
    assert result.exit_code == 0
    assert result.output.decode("utf-8") == ''
    assert shell_container.read_file('new_file.txt') == b''

# def test_not_existing_command_with_redirections_in_out_pipeline(shell_container: TestContainer):
#     result = shell_container.run_uxsh(r"ls | notExistingCommand > new_file.txt | notExistingCommand < new_file.txt")
#     assert result.exit_code == 0
#     assert result.output.decode("utf-8") == ''
#     assert shell_container.read_file('new_file.txt') == b''