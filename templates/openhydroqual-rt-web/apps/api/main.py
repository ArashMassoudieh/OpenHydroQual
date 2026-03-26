from datetime import datetime, timezone
import json
import os
from pathlib import Path
from threading import Lock
from uuid import uuid4

from fastapi import FastAPI, Header, HTTPException, Response
from pydantic import BaseModel

from .queue import enqueue_run


app = FastAPI(title="OHQ Real-time API", version="0.2.0")

JOBS: dict[str, dict] = {}
IDEMPOTENCY_INDEX: dict[str, str] = {}
LOCK = Lock()
PROJECTS: dict[str, dict] = {}
SITES: dict[str, dict] = {}
STATE_FILE = Path(os.getenv("STATE_FILE", "./ohq_rt_state.json"))
ENABLE_FILE_STATE = os.getenv("ENABLE_FILE_STATE", "false").lower() == "true"
METRICS: dict[str, int] = {
    "jobs_created_total": 0,
    "jobs_completed_total": 0,
    "projects_created_total": 0,
    "sites_created_total": 0,
    "jobs_failed_total": 0,
    "jobs_cancelled_total": 0,
}


def _load_state() -> None:
    if not ENABLE_FILE_STATE or not STATE_FILE.exists():
        return
    try:
        payload = json.loads(STATE_FILE.read_text())
        JOBS.update(payload.get("jobs", {}))
        IDEMPOTENCY_INDEX.update(payload.get("idempotency_index", {}))
        PROJECTS.update(payload.get("projects", {}))
        SITES.update(payload.get("sites", {}))
    except Exception:
        # Best-effort bootstrap for scaffold; production should log and alert.
        return


def _persist_state() -> None:
    if not ENABLE_FILE_STATE:
        return
    STATE_FILE.write_text(
        json.dumps(
            {
                "jobs": JOBS,
                "idempotency_index": IDEMPOTENCY_INDEX,
                "projects": PROJECTS,
                "sites": SITES,
            }
        )
    )


_load_state()


class TimeWindow(BaseModel):
    start_utc: datetime
    end_utc: datetime


class RefPayload(BaseModel):
    dataset_id: str | None = None
    version: str | None = None
    profile_id: str | None = None


class SimulationRequest(BaseModel):
    project_id: str
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


class ProjectCreate(BaseModel):
    project_id: str
    name: str


class SiteCreate(BaseModel):
    site_id: str
    facility_type: str
    latitude: float
    longitude: float
    metadata: dict = {}


class WorkerResultPayload(BaseModel):
    status: str = "completed"
    result_contract: str = "simulation_result.v1"
    metrics: dict
    adapter: dict | None = None


@app.get("/health")
def health() -> dict:
    return {"status": "ok", "service": "api", "version": "0.2.0"}






@app.get("/metrics")
def metrics() -> Response:
    lines = [f"{k} {v}" for k, v in METRICS.items()]
    body = "\n".join(lines) + "\n"
    return Response(content=body, media_type="text/plain; version=0.0.4")

@app.post("/v1/projects")
def create_project(payload: ProjectCreate) -> dict:
    with LOCK:
        if payload.project_id in PROJECTS:
            raise HTTPException(status_code=409, detail="project already exists")
        PROJECTS[payload.project_id] = payload.model_dump()
        METRICS["projects_created_total"] += 1
        _persist_state()
    return PROJECTS[payload.project_id]


@app.post("/v1/projects/{project_id}/sites")
def create_project_site(project_id: str, payload: SiteCreate) -> dict:
    with LOCK:
        if project_id not in PROJECTS:
            raise HTTPException(status_code=404, detail="project not found")
        key = f"{project_id}:{payload.site_id}"
        if key in SITES:
            raise HTTPException(status_code=409, detail="site already exists")
        SITES[key] = {
            "project_id": project_id,
            **payload.model_dump(),
        }
        METRICS["sites_created_total"] += 1
        _persist_state()
    return SITES[key]


@app.get("/v1/projects/{project_id}/sites")
def list_project_sites(project_id: str, limit: int = 100, offset: int = 0) -> dict:
    with LOCK:
        sites = [s for s in SITES.values() if s["project_id"] == project_id]
    sliced = sites[offset: offset + limit]
    return {"project_id": project_id, "count": len(sites), "returned": len(sliced), "sites": sliced}



@app.post("/v1/projects/{project_id}/simulate")
def trigger_project_simulations(project_id: str) -> dict:
    with LOCK:
        if project_id not in PROJECTS:
            raise HTTPException(status_code=404, detail="project not found")
        project_sites = [s for s in SITES.values() if s["project_id"] == project_id]

    created = []
    now = datetime.now(timezone.utc).isoformat()
    for site in project_sites:
        job_id = f"sim_{uuid4().hex[:12]}"
        payload = {
            "project_id": project_id,
            "site_id": site["site_id"],
            "facility_type": site["facility_type"],
            "time_window": {"start_utc": now, "end_utc": now},
            "forcing_ref": {"dataset_id": "scheduled", "version": now},
            "parameters_ref": {"profile_id": "default"},
            "request_contract": "simulation_request.v1",
        }

        with LOCK:
            JOBS[job_id] = {
                "job_id": job_id,
                "status": "queued",
                "submitted_at": now,
                "payload": payload,
                "events": [{"at": now, "status": "queued"}],
            }
            METRICS["jobs_created_total"] += 1
            _persist_state()

        if os.getenv("ASYNC_EXECUTION", "false").lower() == "true":
            task_id = enqueue_run({"job_id": job_id, "payload": payload})
            JOBS[job_id]["queue_task_id"] = task_id

        created.append(job_id)

    return {"project_id": project_id, "queued_jobs": len(created), "job_ids": created}

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

        if payload.project_id not in PROJECTS:
            raise HTTPException(status_code=404, detail="unknown project_id")
        site_key = f"{payload.project_id}:{payload.site_id}"
        if site_key not in SITES:
            raise HTTPException(status_code=404, detail="unknown site_id for project")
        if SITES[site_key]["facility_type"] != payload.facility_type:
            raise HTTPException(status_code=400, detail="facility_type mismatch for site")

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
        METRICS["jobs_created_total"] += 1
        _persist_state()

    if os.getenv("ASYNC_EXECUTION", "false").lower() == "true":
        task_id = enqueue_run({"job_id": job_id, "payload": JOBS[job_id]["payload"]})
        JOBS[job_id]["queue_task_id"] = task_id

    return {"job_id": job_id, "status": "queued", "submitted_at": now, "queue_task_id": JOBS[job_id].get("queue_task_id")}


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
        _persist_state()
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
        METRICS["jobs_completed_total"] += 1
        _persist_state()
    return {"job_id": job_id, "status": "completed"}










@app.post("/v1/simulations/{job_id}/cancel")
def cancel_simulation(job_id: str, reason: str | None = None) -> dict:
    now = datetime.now(timezone.utc).isoformat()
    with LOCK:
        job = JOBS.get(job_id)
        if not job:
            raise HTTPException(status_code=404, detail="job not found")
        if job.get("status") in {"completed", "failed"}:
            raise HTTPException(status_code=409, detail="job already finalized")
        job["status"] = "cancelled"
        job["finished_at"] = now
        job["cancel_reason"] = reason or "cancelled by user"
        job["events"].append({"at": now, "status": "cancelled"})
        METRICS["jobs_cancelled_total"] += 1
        _persist_state()
    return {"job_id": job_id, "status": "cancelled"}

@app.post("/v1/simulations/{job_id}/fail")
def mark_failed(job_id: str, error_message: str | None = None) -> dict:
    now = datetime.now(timezone.utc).isoformat()
    with LOCK:
        job = JOBS.get(job_id)
        if not job:
            raise HTTPException(status_code=404, detail="job not found")
        job["status"] = "failed"
        job["finished_at"] = now
        job["error_message"] = error_message or "unknown error"
        job["events"].append({"at": now, "status": "failed"})
        METRICS["jobs_failed_total"] += 1
        _persist_state()
    return {"job_id": job_id, "status": "failed"}

@app.post("/v1/internal/simulations/{job_id}/result")
def post_worker_result(job_id: str, payload: WorkerResultPayload, x_internal_token: str | None = Header(default=None)) -> dict:
    configured = os.getenv("INTERNAL_API_TOKEN")
    if configured and x_internal_token != configured:
        raise HTTPException(status_code=403, detail="forbidden")

    now = datetime.now(timezone.utc).isoformat()
    with LOCK:
        job = JOBS.get(job_id)
        if not job:
            raise HTTPException(status_code=404, detail="job not found")
        job["status"] = payload.status
        job["finished_at"] = now
        job["events"].append({"at": now, "status": payload.status})
        job["result"] = {
            "job_id": job_id,
            "status": payload.status,
            "result_contract": payload.result_contract,
            "metrics": payload.metrics,
            "adapter": payload.adapter or {},
        }
        if payload.status == "completed":
            METRICS["jobs_completed_total"] += 1
        elif payload.status == "failed":
            METRICS["jobs_failed_total"] += 1
        _persist_state()
    return {"job_id": job_id, "status": payload.status}

@app.get("/v1/projects/{project_id}/simulations")
def list_project_simulations(project_id: str, status: str | None = None, limit: int = 100, offset: int = 0) -> dict:
    jobs = [
        {
            "job_id": job["job_id"],
            "status": job["status"],
            "project_id": job["payload"]["project_id"],
            "site_id": job["payload"]["site_id"],
            "submitted_at": job["submitted_at"],
        }
        for job in JOBS.values()
        if job["payload"]["project_id"] == project_id
    ]
    if status is not None:
        jobs = [j for j in jobs if j["status"] == status]
    sliced = jobs[offset: offset + limit]
    return {"project_id": project_id, "count": len(jobs), "returned": len(sliced), "jobs": sliced}

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
