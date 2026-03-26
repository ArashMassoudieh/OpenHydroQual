# openhydroqual-rt-web (starter scaffold)

Reference scaffold for a real-time orchestration repo around OpenHydroQual/OHQuery.

## Included
- FastAPI app with:
  - `POST /v1/simulations`
  - `POST /v1/simulations/{job_id}/start`
  - `POST /v1/simulations/{job_id}/complete`
  - `GET /v1/simulations/{job_id}`
  - `GET /v1/simulations/{job_id}/results`
- Idempotency support via `X-Idempotency-Key` header on create endpoint
- Celery worker stub for queued simulation runs
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

## Local API smoke flow
```bash
# create
curl -s -X POST http://localhost:8000/v1/simulations \
  -H 'Content-Type: application/json' \
  -H 'X-Idempotency-Key: demo-1' \
  -d '{
    "site_id":"la-00123",
    "facility_type":"drywell",
    "time_window":{"start_utc":"2026-03-26T00:00:00Z","end_utc":"2026-03-27T00:00:00Z"},
    "forcing_ref":{"dataset_id":"noaa-hourly","version":"2026-03-26T00:15:00Z"},
    "parameters_ref":{"profile_id":"drywell-default-v1"},
    "request_contract":"simulation_request.v1"
  }'

# mark started + completed
curl -s -X POST http://localhost:8000/v1/simulations/<JOB_ID>/start
curl -s -X POST http://localhost:8000/v1/simulations/<JOB_ID>/complete \
  -H 'Content-Type: application/json' \
  -d '{"peak_depth_m":0.12,"infiltrated_volume_m3":8.5,"overflow":false}'
```
