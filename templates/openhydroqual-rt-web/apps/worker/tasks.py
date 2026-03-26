from celery import Celery


celery_app = Celery(
    "ohq_rt_worker",
    broker="redis://localhost:6379/0",
    backend="redis://localhost:6379/1",
)


@celery_app.task(name="run_simulation")
def run_simulation(job_payload: dict) -> dict:
    """Stub task: replace with OHQuery HTTP /calculate adapter call."""
    return {
        "job_id": job_payload.get("job_id"),
        "status": "completed",
        "result_contract": "simulation_result.v1",
        "metrics": {
            "peak_depth_m": 0.0,
            "infiltrated_volume_m3": 0.0,
            "overflow": False,
        },
    }
