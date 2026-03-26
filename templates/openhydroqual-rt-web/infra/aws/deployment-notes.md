# AWS Deployment Notes (openhydroqual-rt-web)

## Reference topology
1. API service (FastAPI) on ECS Fargate behind ALB.
2. Worker service (Celery) on ECS Fargate in private subnets.
3. Redis broker/backend on ElastiCache.
4. PostgreSQL on RDS.
5. OHQuery service deployed as an internal ECS service or EC2 service.
6. CloudWatch logs + metrics, alarms for queue depth and run failures.

## Security baseline
- Use IAM task roles (no static cloud credentials in containers).
- Keep OHQuery and Redis private; only API exposed publicly.
- Put secrets in Secrets Manager (DB password, any API tokens).
- Enforce TLS at ALB and internal service-to-service where possible.

## Scaling hints
- Scale worker count on queue depth and task latency.
- Scale API on request count + latency percentiles.
- Use per-site concurrency controls to avoid model execution storms.

## Reliability hints
- Use dead-letter queue or failure table for exhausted retries.
- Persist run provenance (`model_version`, forcing version, parameter profile).
- Add backfill job path for ingestion outages.
