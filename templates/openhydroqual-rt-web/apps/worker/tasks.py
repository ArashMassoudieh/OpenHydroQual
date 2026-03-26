import os

from celery import Celery


BROKER_URL = os.getenv("BROKER_URL", "redis://localhost:6379/0")
RESULT_BACKEND = os.getenv("RESULT_BACKEND", "redis://localhost:6379/1")
OHQUERY_BASE_URL = os.getenv("OHQUERY_BASE_URL", "http://localhost:8080")

celery_app = Celery(
    "ohq_rt_worker",
    broker=BROKER_URL,
    backend=RESULT_BACKEND,
)


@celery_app.task(name="run_simulation")
def run_simulation(job_payload: dict) -> dict:
    """Stub task: replace with OHQuery HTTP /calculate adapter call using OHQUERY_BASE_URL."""
    return {
        "job_id": job_payload.get("job_id"),
        "status": "completed",
        "result_contract": "simulation_result.v1",
        "adapter": {
            "engine": "OHQuery",
            "base_url": OHQUERY_BASE_URL,
        },
        "metrics": {
            "peak_depth_m": 0.0,
            "infiltrated_volume_m3": 0.0,
            "overflow": False,
        },
    }
