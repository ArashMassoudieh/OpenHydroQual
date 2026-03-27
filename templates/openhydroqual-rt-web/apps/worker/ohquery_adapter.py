import os
from typing import Any

import requests


OHQUERY_BASE_URL = os.getenv("OHQUERY_BASE_URL", "http://localhost:8080")
OHQUERY_TIMEOUT_SECONDS = float(os.getenv("OHQUERY_TIMEOUT_SECONDS", "30"))


def run_ohquery_calculation(parameters: dict[str, Any]) -> dict[str, Any]:
    """!Call OHQuery /calculate endpoint and return JSON output.

    Expected OHQuery service behavior is based on existing terminal/OHQuery server implementation.
    """
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
