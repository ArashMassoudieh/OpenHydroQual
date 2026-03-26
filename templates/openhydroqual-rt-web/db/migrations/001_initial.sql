create table if not exists site (
  site_id text primary key,
  county text not null,
  facility_type text not null,
  latitude double precision not null,
  longitude double precision not null,
  metadata jsonb not null default '{}'::jsonb,
  created_at timestamptz not null default now()
);

create table if not exists forcing_series (
  forcing_id text primary key,
  dataset_id text not null,
  version text not null,
  source_timestamp timestamptz not null,
  retrieved_at timestamptz not null,
  payload jsonb not null,
  created_at timestamptz not null default now()
);

create table if not exists simulation_run (
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
