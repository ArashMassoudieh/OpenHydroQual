# Real-time Web Repo Bootstrap (Execution Pack)

This file complements `realtime-web-repo-proposal.md` with concrete implementation artifacts you can copy into a new repository on day 1.


## 0) Export this scaffold to a standalone repository
From the OpenHydroQual repository root:

```bash
./scripts/export-openhydroqual-rt-web.sh   --init-git   --remote https://github.com/<your-org>/openhydroqual-rt-web.git   ../openhydroqual-rt-web
```

To also push immediately:

```bash
./scripts/export-openhydroqual-rt-web.sh   --init-git   --remote https://github.com/<your-org>/openhydroqual-rt-web.git   --push   ../openhydroqual-rt-web
```

## 1) API contract (v1)

### POST `/v1/simulations`
Create a simulation job.

Request body:
```json
{
  "site_id": "la-00123",
  "facility_type": "drywell",
  "time_window": {
    "start_utc": "2026-03-26T00:00:00Z",
    "end_utc": "2026-03-27T00:00:00Z"
  },
  "forcing_ref": {
    "dataset_id": "noaa-hourly",
    "version": "2026-03-26T00:15:00Z"
  },
  "parameters_ref": {
    "profile_id": "drywell-default-v1"
  },
  "request_contract": "simulation_request.v1"
}
```

Response:
```json
{
  "job_id": "sim_01HV...",
  "status": "queued",
  "submitted_at": "2026-03-26T00:16:00Z"
}
```

### GET `/v1/simulations/{job_id}`
Returns status and metadata.

### GET `/v1/simulations/{job_id}/results`
Returns normalized result payload (hydraulics + quality + derived KPIs).

## 2) Worker job envelope

Queue payload (canonical):
```json
{
  "job_id": "sim_01HV...",
  "site_id": "la-00123",
  "facility_type": "drywell",
  "run_mode": "scheduled_hourly",
  "forcing_ref": "forcing_9c2f",
  "parameter_ref": "params_31a9",
  "ohq_adapter_contract": "simulation_request.v1",
  "requested_at": "2026-03-26T00:16:00Z"
}
```

## 3) Database DDL starter (PostgreSQL)

```sql
create table site (
  site_id text primary key,
  county text not null,
  facility_type text not null,
  latitude double precision not null,
  longitude double precision not null,
  metadata jsonb not null default '{}'::jsonb,
  created_at timestamptz not null default now()
);

create table forcing_series (
  forcing_id text primary key,
  dataset_id text not null,
  version text not null,
  source_timestamp timestamptz not null,
  retrieved_at timestamptz not null,
  payload jsonb not null,
  created_at timestamptz not null default now()
);

create table simulation_run (
  job_id text primary key,
  site_id text not null references site(site_id),
  status text not null,
  request_contract text not null,
  result_contract text,
  model_version text,
  forcing_id text references forcing_series(forcing_id),
  parameter_profile text,
  queued_at timestamptz not null,
  started_at timestamptz,
  finished_at timestamptz,
  error_message text,
  provenance jsonb not null default '{}'::jsonb
);
```

## 4) OHQ adapter acceptance checklist

- [ ] Adapter can execute one drywell request against existing OHQuery HTTP endpoint.
- [ ] Adapter can map OHQuery output to `simulation_result.v1`.
- [ ] Adapter stores model hash/version in run provenance.
- [ ] Adapter retry logic handles transient failures (3 retries with exponential backoff).
- [ ] Conformance test fixture verifies deterministic output within tolerance.

## 5) Conformance test pattern

Use a frozen input fixture:
1. fixed site config
2. fixed forcing payload
3. fixed parameter profile
4. expected summary metrics (peak depth, total infiltrated volume, overflow flag)

Pass criteria:
- all required result fields present
- numerical metrics within configured tolerance
- runtime below target threshold for single run

## 6) First sprint ticket list (ready to import)

1. `API-001` Create FastAPI skeleton and health endpoint.
2. `API-002` Implement `POST /v1/simulations` + idempotency key support.
3. `WRK-001` Add Celery worker and Redis queue wiring.
4. `WRK-002` Implement OHQ adapter call path to `/calculate`.
5. `DB-001` Add migrations for `site`, `forcing_series`, `simulation_run`.
6. `ING-001` Build NOAA ingestion task and validation rules.
7. `WEB-001` Add dashboard map and “latest run status” panel.
8. `OBS-001` Add Prometheus metrics (`run_duration`, `run_failures_total`).
9. `ALT-001` Add overflow alert rule for next 6h horizon.
10. `QA-001` Add conformance fixture + CI gate.

## 7) Definition of done for MVP

- Hourly scheduled runs active for selected pilot sites.
- On-demand run endpoint works end-to-end.
- Dashboard shows latest results and one alert badge.
- Provenance is queryable for every run.
- Staging runbook exists with restart/backfill instructions.
