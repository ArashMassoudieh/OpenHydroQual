from datetime import datetime, timezone
from threading import Lock
from uuid import uuid4

from fastapi import FastAPI, Header, HTTPException
from pydantic import BaseModel


app = FastAPI(title="OHQ Real-time API", version="0.2.0")

JOBS: dict[str, dict] = {}
IDEMPOTENCY_INDEX: dict[str, str] = {}
LOCK = Lock()


class TimeWindow(BaseModel):
    start_utc: datetime
    end_utc: datetime


class RefPayload(BaseModel):
    dataset_id: str | None = None
    version: str | None = None
    profile_id: str | None = None


class SimulationRequest(BaseModel):
    site_id: str
    facility_type: str
    time_window: TimeWindow
    forcing_ref: RefPayload
    parameters_ref: RefPayload
    request_contract: str = "simulation_request.v1"


class CompletionPayload(BaseModel):
    peak_depth_m: float
    infiltrated_volume_m3: float
    overflow: bool


@app.get("/health")
def health() -> dict:
    return {"status": "ok", "service": "api", "version": "0.2.0"}


@app.post("/v1/simulations")
def create_simulation(
    payload: SimulationRequest,
    x_idempotency_key: str | None = Header(default=None),
) -> dict:
    now = datetime.now(timezone.utc).isoformat()

    with LOCK:
        if x_idempotency_key and x_idempotency_key in IDEMPOTENCY_INDEX:
            existing_job = IDEMPOTENCY_INDEX[x_idempotency_key]
            job = JOBS[existing_job]
            return {
                "job_id": existing_job,
                "status": job["status"],
                "submitted_at": job["submitted_at"],
                "idempotent_replay": True,
            }

        job_id = f"sim_{uuid4().hex[:12]}"
        JOBS[job_id] = {
            "job_id": job_id,
            "status": "queued",
            "submitted_at": now,
            "payload": payload.model_dump(mode="json"),
            "events": [{"at": now, "status": "queued"}],
        }
        if x_idempotency_key:
            IDEMPOTENCY_INDEX[x_idempotency_key] = job_id

    return {"job_id": job_id, "status": "queued", "submitted_at": now}


@app.post("/v1/simulations/{job_id}/start")
def mark_started(job_id: str) -> dict:
    now = datetime.now(timezone.utc).isoformat()
    with LOCK:
        job = JOBS.get(job_id)
        if not job:
            raise HTTPException(status_code=404, detail="job not found")
        job["status"] = "running"
        job["started_at"] = now
        job["events"].append({"at": now, "status": "running"})
    return {"job_id": job_id, "status": "running"}


@app.post("/v1/simulations/{job_id}/complete")
def mark_completed(job_id: str, result: CompletionPayload) -> dict:
    now = datetime.now(timezone.utc).isoformat()
    with LOCK:
        job = JOBS.get(job_id)
        if not job:
            raise HTTPException(status_code=404, detail="job not found")
        job["status"] = "completed"
        job["finished_at"] = now
        job["events"].append({"at": now, "status": "completed"})
        job["result"] = {
            "job_id": job_id,
            "status": "completed",
            "result_contract": "simulation_result.v1",
            "metrics": result.model_dump(),
        }
    return {"job_id": job_id, "status": "completed"}


@app.get("/v1/simulations/{job_id}")
def get_simulation(job_id: str) -> dict:
    job = JOBS.get(job_id)
    if not job:
        raise HTTPException(status_code=404, detail="job not found")
    return {
        "job_id": job["job_id"],
        "status": job["status"],
        "submitted_at": job["submitted_at"],
        "started_at": job.get("started_at"),
        "finished_at": job.get("finished_at"),
        "events": job.get("events", []),
    }


@app.get("/v1/simulations/{job_id}/results")
def get_simulation_results(job_id: str) -> dict:
    job = JOBS.get(job_id)
    if not job:
        raise HTTPException(status_code=404, detail="job not found")
    if job["status"] != "completed":
        return {
            "job_id": job_id,
            "status": job["status"],
            "message": "results not ready",
        }
    return job["result"]
