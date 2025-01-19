from fixtures import shell_container, TestContainer

def test_assign_variable(shell_container: TestContainer):
    result = shell_container.run_uxsh(r'var=l ; echo -e plum\napple\norange | grep $var')
    assert result.exit_code == 0
    assert result.output.decode("utf-8") == 'plum\napple\n'

def test_not_existing_variable(shell_container: TestContainer):
    result = shell_container.run_uxsh(r'echo $notExistingVariable')
    assert result.exit_code == 0
    assert result.output.decode("utf-8") == '\n'

def test_export_variable(shell_container: TestContainer):
    result = shell_container.run_uxsh(r"VAR=5; export VAR; bash -c 'echo $VAR'")
    assert result.exit_code == 0
    assert result.output.decode("utf-8") == '5\n'

def test_single_quotes(shell_container: TestContainer):
    result = shell_container.run_uxsh(r"VAR=5; echo '$VAR'")
    assert result.exit_code == 0
    assert result.output.decode("utf-8") == '$VAR\n'