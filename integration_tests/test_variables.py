from fixtures import shell_container, TestContainer

def test_assign_variable(shell_container: TestContainer):
    result = shell_container.run_uxsh(r'VAR=l ; echo -e plum\napple\norange | grep $VAR')
    assert result.exit_code == 0
    assert result.output.decode("utf-8") == 'plum\napple\n'

def test_not_existing_variable(shell_container: TestContainer):
    result = shell_container.run_uxsh(r'echo $notExistingVariable')
    assert result.exit_code == 0
    assert result.output.decode("utf-8") == '\n'

def test_variable_out_of_scope_bash(shell_container: TestContainer):
    result = shell_container.run_uxsh("VAR=5; bash -c 'echo $VAR'")
    assert result.exit_code == 0
    assert result.output.decode("utf-8") == '\n'

def test_export_variable_bash_run(shell_container: TestContainer):
    result = shell_container.run_uxsh(r"VAR=5; export VAR; bash -c 'echo $VAR'")
    assert result.exit_code == 0
    assert result.output.decode("utf-8") == '5\n'

def test_export_variable_subshell(shell_container: TestContainer):
    result = shell_container.run_uxsh(r"VAR=5; (echo $VAR)")
    assert result.exit_code == 0
    assert result.output.decode("utf-8") == '5\n'

def test_single_quotes(shell_container: TestContainer):
    result = shell_container.run_uxsh(r"VAR=5; echo '$VAR'")
    assert result.exit_code == 0
    assert result.output.decode("utf-8") == '$VAR\n'

# def test_assign_variable(shell_container: TestContainer):
#     result = shell_container.run_uxsh(r'VAR=l | echo -e plum\napple\norange | grep $VAR')
#     assert result.exit_code == 0
#     assert result.output.decode("utf-8") == 'plum\napple\n'
