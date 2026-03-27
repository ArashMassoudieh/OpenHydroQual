import os

from celery import Celery


def get_celery_client() -> Celery:
    """!Create and return a Celery client instance."""
    broker_url = os.getenv("BROKER_URL", "redis://localhost:6379/0")
    result_backend = os.getenv("RESULT_BACKEND", "redis://localhost:6379/1")
    return Celery("ohq_rt_api", broker=broker_url, backend=result_backend)


def enqueue_run(job_payload: dict) -> str:
    """!Enqueue a simulation run task and return task id."""
    celery = get_celery_client()
    async_result = celery.send_task("run_simulation", kwargs={"job_payload": job_payload})
    return async_result.id
