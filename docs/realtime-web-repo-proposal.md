# Proposal: Real-time Web Simulation Repo for OpenHydroQual (LA County GI)

## 1) Recommended repository goal
Build a **separate web-oriented repo** that runs near-real-time simulations for LA County green-infrastructure (GI) assets (drywell, bioretention, bioswale, etc.) while reusing OpenHydroQual as the computational engine.

Suggested name: `openhydroqual-rt-web`.

## 2) Keep this split of responsibilities
- **OpenHydroQual (existing repo):** model core, numerical solver, calibration logic, physics/chemistry routines.
- **New repo:** ingestion, orchestration, API, UI, scenario management, scheduling, monitoring, and deployments.

This separation avoids coupling solver development to web-product velocity.

## 3) Architecture suggestion (practical and scalable)
- **Engine Adapter Service**
  - Wrap OHQ model runs behind a clean interface:
    - `run_simulation(site_id, start, end, forcing_inputs, params)`
    - `get_status(job_id)`
    - `get_results(job_id)`
  - Start with local process execution; later swap to container jobs.
- **Data Ingestion Service**
  - Pull weather and forecast data (hourly cadence).
  - Validate and map source variables into model forcing schema.
  - Version raw + processed payloads for replayability.
- **Simulation Orchestrator**
  - Trigger runs per site/event/schedule.
  - Use queue-based jobs (retry, backoff, dead-letter queues).
  - Record provenance (which forcing version + parameter set generated each run).
- **Time-series Store + Relational DB**
  - Time-series (forcing + outputs) and relational metadata (sites, structures, configs, alerts).
- **Web API + Dashboard**
  - Site map, current status, hydrograph/water-quality plots, uncertainty bands.
  - Scenario controls for “what-if” storms and controls/operations.
- **Alerting Layer**
  - Threshold-based alerts (overflow risk, infiltration exceedance, pollutant export risk).

## 4) Suggested minimal stack
- **Backend API:** FastAPI (Python) or Node/TypeScript.
- **Workers:** Celery/RQ (Python) or BullMQ (Node).
- **Queue/Broker:** Redis (simple) or RabbitMQ (more robust).
- **Databases:** PostgreSQL + TimescaleDB extension.
- **Frontend:** React + map/chart libraries.
- **Infra:** Docker Compose first, Kubernetes later.
- **Observability:** Prometheus + Grafana + structured logs.

## 5) LA County domain model you should lock early
Define canonical entities before coding too much:
- `Site` (location, drainage area, soil group, facility type)
- `Facility` (drywell/bioretention/bioswale parameters)
- `ForcingSeries` (rainfall, ET, temperature, inflow assumptions)
- `SimulationConfig` (time step, solver options, calibration profile)
- `SimulationRun` (status, provenance, performance metrics)
- `SimulationResult` (hydraulics, quality, KPIs)
- `AlertRule` and `AlertEvent`

This prevents schema churn when adding more jurisdictions.

## 6) Real-time workflow (target state)
1. Ingest latest observed weather + short-term forecast.
2. Transform to OHQ forcing format.
3. Trigger run per active facility/site.
4. Persist results + QC flags.
5. Refresh dashboard and emit alerts.
6. Archive run artifacts for replay/audit.

Target SLA example: full county portfolio refresh every 15–30 minutes for short-horizon forecasts.

## 7) Integration pattern with OpenHydroQual
- Prefer a **stable adapter boundary** instead of direct deep coupling.
- Keep model input/output contracts explicit (JSON schema + version).
- Store every run’s model version hash and parameter set for reproducibility.
- Add a lightweight benchmark suite to detect runtime or numerical regressions when OHQ changes.

## 8) Delivery roadmap (fast but safe)
- **Phase 0 (2–3 weeks):** one facility type (drywell), one weather provider, one watershed subset.
- **Phase 1 (4–6 weeks):** add bioretention + bioswale; introduce scheduler, queue retries, observability.
- **Phase 2:** uncertainty/scenario mode, automated calibration refresh, user roles/permissions.
- **Phase 3:** countywide scaling, multi-tenant support for other agencies.

## 9) Governance and reliability checklist
- Input data quality gates (missing values, units, time-zone normalization).
- Deterministic reruns from archived inputs.
- Backfill mode for outages.
- Cost controls (concurrency caps, spot/batch options).
- Security baseline (secrets manager, authN/authZ, audit logs).

## 10) Repo bootstrap layout (recommended)
```
openhydroqual-rt-web/
  apps/
    api/
    worker/
    web/
  packages/
    ohq-adapter/
    data-contracts/
    shared-utils/
  infra/
    docker/
    k8s/
    terraform/
  docs/
    architecture/
    adr/
    runbooks/
  tests/
    integration/
    performance/
```

## 11) First concrete milestone
Before building full UI, deliver:
- one endpoint to run a site simulation on demand,
- one scheduled hourly run,
- one dashboard view with latest hydrograph,
- one alert rule (e.g., overflow risk in next 6 hours).

That gives stakeholders visible value while validating the core pipeline.
