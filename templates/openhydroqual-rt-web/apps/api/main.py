from datetime import datetime, timezone
from uuid import uuid4

from fastapi import FastAPI, HTTPException
from pydantic import BaseModel


app = FastAPI(title="OHQ Real-time API", version="0.1.0")

JOBS: dict[str, dict] = {}


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


@app.get("/health")
def health() -> dict:
    return {"status": "ok", "service": "api"}


@app.post("/v1/simulations")
def create_simulation(payload: SimulationRequest) -> dict:
    job_id = f"sim_{uuid4().hex[:12]}"
    now = datetime.now(timezone.utc).isoformat()
    JOBS[job_id] = {
        "job_id": job_id,
        "status": "queued",
        "submitted_at": now,
        "payload": payload.model_dump(mode="json"),
    }
    return {"job_id": job_id, "status": "queued", "submitted_at": now}


@app.get("/v1/simulations/{job_id}")
def get_simulation(job_id: str) -> dict:
    job = JOBS.get(job_id)
    if not job:
        raise HTTPException(status_code=404, detail="job not found")
    return {k: v for k, v in job.items() if k != "payload"}


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
    return job.get("result", {"job_id": job_id, "status": "completed", "result": {}})
