import os

from celery import Celery

from .ohquery_adapter import run_ohquery_calculation


BROKER_URL = os.getenv("BROKER_URL", "redis://localhost:6379/0")
RESULT_BACKEND = os.getenv("RESULT_BACKEND", "redis://localhost:6379/1")
OHQUERY_BASE_URL = os.getenv("OHQUERY_BASE_URL", "http://localhost:8080")
MOCK_OHQUERY = os.getenv("MOCK_OHQUERY", "true").lower() == "true"

celery_app = Celery(
    "ohq_rt_worker",
    broker=BROKER_URL,
    backend=RESULT_BACKEND,
)


@celery_app.task(name="run_simulation")
def run_simulation(job_payload: dict) -> dict:
    """Run a simulation via OHQuery adapter (or mock mode for local smoke)."""
    parameters = job_payload.get("payload", {})

    if MOCK_OHQUERY:
        adapter_result = {"mock": True, "message": "MOCK_OHQUERY enabled"}
        metrics = {
            "peak_depth_m": 0.0,
            "infiltrated_volume_m3": 0.0,
            "overflow": False,
        }
    else:
        adapter_result = run_ohquery_calculation(parameters)
        metrics = {
            "peak_depth_m": float(adapter_result.get("peak_depth_m", 0.0)),
            "infiltrated_volume_m3": float(adapter_result.get("infiltrated_volume_m3", 0.0)),
            "overflow": bool(adapter_result.get("overflow", False)),
        }

    return {
        "job_id": job_payload.get("job_id"),
        "status": "completed",
        "result_contract": "simulation_result.v1",
        "adapter": {
            "engine": "OHQuery",
            "base_url": OHQUERY_BASE_URL,
            "mock_mode": MOCK_OHQUERY,
            "raw": adapter_result,
        },
        "metrics": metrics,
    }
