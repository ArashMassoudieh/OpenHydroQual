# openhydroqual-rt-web (starter scaffold)

Reference scaffold for a real-time orchestration repo around OpenHydroQual/OHQuery.

## Included
- FastAPI app with:
  - `POST /v1/projects`
  - `POST /v1/projects/{project_id}/sites`
  - `GET /v1/projects/{project_id}/export` (supports `include_jobs=true`)
  - `POST /v1/projects/import` (supports `jobs` in payload)
  - `POST /v1/projects/{project_id}/clone`
  - `DELETE /v1/projects/{project_id}` (supports `force=true`)
  - `DELETE /v1/projects/{project_id}/sites/{site_id}` (supports `force=true`)
  - `GET /v1/projects/{project_id}/sites` (supports `limit`, `offset`)
  - `POST /v1/simulations`
  - `POST /v1/projects/{project_id}/simulate` (queue all project sites)
  - `GET /v1/projects/{project_id}/stats`
  - `GET /v1/projects/{project_id}/simulations` (supports `status`, `limit`, `offset`)
  - `POST /v1/simulations/{job_id}/start`
  - `POST /v1/simulations/{job_id}/complete`
  - `POST /v1/simulations/{job_id}/cancel`
  - `POST /v1/simulations/{job_id}/fail`
  - `GET /v1/simulations/{job_id}`
  - `GET /v1/simulations/{job_id}/events`
  - `GET /v1/simulations/{job_id}/results`
  - `POST /v1/internal/simulations/{job_id}/result` (worker callback)
  - `GET /metrics` (Prometheus-style counters)
- Idempotency support via `X-Idempotency-Key` header on create endpoint
- `POST /v1/simulations` validates project/site existence and facility type match
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
# create project + site
curl -s -X POST http://localhost:8000/v1/projects \
  -H 'Content-Type: application/json' \
  -d '{"project_id":"la-drywell-pilot","name":"LA Drywell Pilot"}'

curl -s -X POST http://localhost:8000/v1/projects/la-drywell-pilot/sites \
  -H 'Content-Type: application/json' \
  -d '{"site_id":"la-00123","facility_type":"drywell","latitude":34.05,"longitude":-118.24,"metadata":{"county":"LA"}}'

# create
curl -s -X POST http://localhost:8000/v1/simulations \
  -H 'Content-Type: application/json' \
  -H 'X-Idempotency-Key: demo-1' \
  -d '{
    "project_id":"la-drywell-pilot",
    "site_id":"la-00123",
    "facility_type":"drywell",
    "time_window":{"start_utc":"2026-03-26T00:00:00Z","end_utc":"2026-03-27T00:00:00Z"},
    "forcing_ref":{"dataset_id":"noaa-hourly","version":"2026-03-26T00:15:00Z"},
    "parameters_ref":{"profile_id":"drywell-default-v1"},
    "request_contract":"simulation_request.v1"
  }'

# trigger queued runs for all sites in a project
curl -s -X POST http://localhost:8000/v1/projects/la-drywell-pilot/simulate

# mark started + completed
curl -s -X POST http://localhost:8000/v1/simulations/<JOB_ID>/start
curl -s -X POST http://localhost:8000/v1/simulations/<JOB_ID>/complete \
  -H 'Content-Type: application/json' \
  -d '{"peak_depth_m":0.12,"infiltrated_volume_m3":8.5,"overflow":false}'
```

## AWS deployment notes
- Use **RDS PostgreSQL** for relational storage.
- Use **ElastiCache Redis** (or Amazon MQ/SQS adapter if preferred) for broker/backend.
- Run API/worker containers on **ECS Fargate** (or EKS) with private networking.
- Expose API via **ALB** + HTTPS; keep OHQuery service internal behind private ALB or service discovery.
- Set environment variables from `.env.example` using AWS Secrets Manager / SSM Parameter Store.

Minimum environment variables in AWS:
- `ASYNC_EXECUTION=true`
- `BROKER_URL` (ElastiCache endpoint)
- `RESULT_BACKEND` (ElastiCache endpoint)
- `OHQUERY_BASE_URL` (internal OHQuery service URL)



## OHQuery integration mode
- Default local mode uses `MOCK_OHQUERY=true` (no external engine call).
- Set `MOCK_OHQUERY=false` and `OHQUERY_BASE_URL=http://<ohquery-host>:8080` to call real OHQuery `POST /calculate`.


## Testing
```bash
cd templates/openhydroqual-rt-web
make venv
make install
make test
```

A GitHub Actions workflow is included at `.github/workflows/openhydroqual-rt-web-ci.yml` to run scaffold tests automatically.


## Optional local state persistence
- By default API state is in-memory only.
- Set `ENABLE_FILE_STATE=true` to persist projects/sites/jobs in `STATE_FILE` (JSON file) across restarts.


## Worker callback endpoint
Workers can push normalized results back to API using `POST /v1/internal/simulations/{job_id}/result`.
Set `INTERNAL_API_TOKEN` and pass it in `X-Internal-Token` for simple protection.
Accepted callback statuses are `completed` and `failed`.

Example callback payload:

```json
{
  "status": "completed",
  "result_contract": "simulation_result.v1",
  "generated_at_utc": "2026-03-27T00:00:00+00:00",
  "adapter": {
    "engine": "OHQuery",
    "mock": true,
    "mock_mode": true,
    "raw": { "mock": true }
  },
  "metrics": {
    "peak_depth_m": 0.11,
    "infiltrated_volume_m3": 7.9,
    "overflow": false
  }
}
```
