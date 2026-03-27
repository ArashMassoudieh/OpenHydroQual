"""!Adapter helpers for invoking OHQuery calculation endpoints."""

import os
import shlex
import subprocess
from typing import Any

import requests


OHQUERY_BASE_URL = os.getenv("OHQUERY_BASE_URL", "http://localhost:8080")
OHQUERY_TIMEOUT_SECONDS = float(os.getenv("OHQUERY_TIMEOUT_SECONDS", "30"))
OPENHYDROQUAL_CMD = os.getenv("OPENHYDROQUAL_CMD", "").strip()


def run_ohquery_calculation(parameters: dict[str, Any]) -> dict[str, Any]:
    """!Call OHQuery /calculate endpoint and return JSON output.

    Expected OHQuery service behavior is based on existing terminal/OHQuery server implementation.
    """
    if OPENHYDROQUAL_CMD:
        command = shlex.split(OPENHYDROQUAL_CMD)
        cli_args = parameters.get("cli_args", [])
        if isinstance(cli_args, list):
            command.extend(str(arg) for arg in cli_args)
        script_path = (
            parameters.get("script_path")
            or parameters.get("ohq_script_path")
            or parameters.get("ohq_file")
        )
        if script_path:
            command.append(str(script_path))
        run = subprocess.run(
            command,
            capture_output=True,
            text=True,
            timeout=OHQUERY_TIMEOUT_SECONDS,
            check=False,
        )
        if run.returncode != 0:
            raise RuntimeError(
                f"OpenHydroQual command failed (exit={run.returncode}): {run.stderr.strip() or run.stdout.strip()}"
            )
        return {
            "engine": "OpenHydroQualCLI",
            "command": command,
            "returncode": run.returncode,
            "stdout": run.stdout,
            "stderr": run.stderr,
        }

    response = requests.post(
        f"{OHQUERY_BASE_URL.rstrip('/')}/calculate",
        json=parameters,
        timeout=OHQUERY_TIMEOUT_SECONDS,
    )
    response.raise_for_status()

    try:
        return response.json()
    except ValueError:
        return {"raw_response": response.text}
