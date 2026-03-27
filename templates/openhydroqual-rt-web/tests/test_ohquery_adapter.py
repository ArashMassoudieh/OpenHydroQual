"""!Unit tests for OHQuery adapter execution paths."""

from types import SimpleNamespace

from apps.worker import ohquery_adapter


def test_adapter_runs_openhydroqual_cli(monkeypatch) -> None:
    """!Verify adapter can execute OpenHydroQual CLI command when configured."""
    monkeypatch.setattr(ohquery_adapter, "OPENHYDROQUAL_CMD", "OpenHydroQualCLI")

    def _fake_run(command, capture_output, text, timeout, check):  # noqa: ANN001
        assert "OpenHydroQualCLI" in command[0]
        assert "--quiet" in command
        assert "/tmp/starter_generated.ohq" in command
        assert capture_output is True
        assert text is True
        assert check is False
        assert timeout == ohquery_adapter.OHQUERY_TIMEOUT_SECONDS
        return SimpleNamespace(returncode=0, stdout="ok", stderr="")

    monkeypatch.setattr(ohquery_adapter.subprocess, "run", _fake_run)
    result = ohquery_adapter.run_ohquery_calculation(
        {
            "script_path": "/tmp/starter_generated.ohq",
            "cli_args": ["--quiet"],
        }
    )

    assert result["engine"] == "OpenHydroQualCLI"
    assert result["returncode"] == 0
