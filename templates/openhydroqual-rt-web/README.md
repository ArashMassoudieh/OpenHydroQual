# openhydroqual-rt-web (starter scaffold)

Reference scaffold for a real-time orchestration repo around OpenHydroQual/OHQuery.

## Included
- FastAPI app with `POST /v1/simulations`, `GET /v1/simulations/{job_id}`, `GET /v1/simulations/{job_id}/results`
- Celery worker stub for executing queued simulation runs
- JSON Schemas for request/result contracts (`simulation_request.v1`, `simulation_result.v1`)
- SQL migration starter for `site`, `forcing_series`, `simulation_run`
- Docker Compose stack for API + worker + Redis + Postgres

## Quick start
```bash
cd templates/openhydroqual-rt-web
python -m venv .venv
source .venv/bin/activate
pip install -r apps/api/requirements.txt -r apps/worker/requirements.txt
uvicorn apps.api.main:app --reload --port 8000
```

In a second terminal:
```bash
cd templates/openhydroqual-rt-web
source .venv/bin/activate
celery -A apps.worker.tasks worker --loglevel=info
```
