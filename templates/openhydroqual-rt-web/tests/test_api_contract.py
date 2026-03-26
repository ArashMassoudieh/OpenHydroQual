from fastapi.testclient import TestClient

from apps.api.main import app


def test_simulation_lifecycle() -> None:
    client = TestClient(app)

    create_project = client.post(
        "/v1/projects",
        json={"project_id": "la-drywell-pilot", "name": "LA Drywell Pilot"},
    )
    assert create_project.status_code == 200

    create_site = client.post(
        "/v1/projects/la-drywell-pilot/sites",
        json={
            "site_id": "la-00123",
            "facility_type": "drywell",
            "latitude": 34.05,
            "longitude": -118.24,
            "metadata": {"county": "LA"},
        },
    )
    assert create_site.status_code == 200



    trigger = client.post("/v1/projects/la-drywell-pilot/simulate")
    assert trigger.status_code == 200
    assert trigger.json()["queued_jobs"] >= 1

    create = client.post(
        "/v1/simulations",
        json={
            "project_id": "la-drywell-pilot",
            "site_id": "la-00123",
            "facility_type": "drywell",
            "time_window": {
                "start_utc": "2026-03-26T00:00:00Z",
                "end_utc": "2026-03-27T00:00:00Z",
            },
            "forcing_ref": {"dataset_id": "noaa-hourly", "version": "v1"},
            "parameters_ref": {"profile_id": "drywell-default-v1"},
            "request_contract": "simulation_request.v1",
        },
        headers={"X-Idempotency-Key": "test-key-1"},
    )
    assert create.status_code == 200
    job_id = create.json()["job_id"]

    replay = client.post(
        "/v1/simulations",
        json={
            "project_id": "la-drywell-pilot",
            "site_id": "la-00123",
            "facility_type": "drywell",
            "time_window": {
                "start_utc": "2026-03-26T00:00:00Z",
                "end_utc": "2026-03-27T00:00:00Z",
            },
            "forcing_ref": {"dataset_id": "noaa-hourly", "version": "v1"},
            "parameters_ref": {"profile_id": "drywell-default-v1"},
            "request_contract": "simulation_request.v1",
        },
        headers={"X-Idempotency-Key": "test-key-1"},
    )
    assert replay.status_code == 200
    assert replay.json()["job_id"] == job_id

    started = client.post(f"/v1/simulations/{job_id}/start")
    assert started.status_code == 200

    completed = client.post(
        f"/v1/simulations/{job_id}/complete",
        json={"peak_depth_m": 0.1, "infiltrated_volume_m3": 8.0, "overflow": False},
    )
    assert completed.status_code == 200


    worker_result = client.post(
        f"/v1/internal/simulations/{job_id}/result",
        json={
            "status": "completed",
            "result_contract": "simulation_result.v1",
            "metrics": {"peak_depth_m": 0.11, "infiltrated_volume_m3": 7.9, "overflow": False},
            "adapter": {"engine": "OHQuery", "mock_mode": True},
        },
    )
    assert worker_result.status_code == 200

    result = client.get(f"/v1/simulations/{job_id}/results")
    assert result.status_code == 200
    assert result.json()["result_contract"] == "simulation_result.v1"


    project_jobs = client.get("/v1/projects/la-drywell-pilot/simulations", params={"status": "completed", "limit": 10, "offset": 0})
    assert project_jobs.status_code == 200
    assert project_jobs.json()["count"] >= 1
    assert "returned" in project_jobs.json()

    sites = client.get("/v1/projects/la-drywell-pilot/sites", params={"limit": 10, "offset": 0})
    assert sites.status_code == 200
    assert sites.json()["count"] >= 1
    assert "returned" in sites.json()




    failed_job = client.post(
        "/v1/simulations",
        json={
            "project_id": "la-drywell-pilot",
            "site_id": "la-00123",
            "facility_type": "drywell",
            "time_window": {"start_utc": "2026-03-26T00:00:00Z", "end_utc": "2026-03-27T00:00:00Z"},
            "forcing_ref": {"dataset_id": "noaa-hourly", "version": "v1"},
            "parameters_ref": {"profile_id": "drywell-default-v1"},
            "request_contract": "simulation_request.v1",
        },
    )
    assert failed_job.status_code == 200
    failed_id = failed_job.json()["job_id"]

    failed = client.post(f"/v1/simulations/{failed_id}/fail", params={"error_message": "engine timeout"})
    assert failed.status_code == 200

    metrics = client.get("/metrics")
    assert metrics.status_code == 200
    assert "jobs_created_total" in metrics.text
    assert "jobs_failed_total" in metrics.text
