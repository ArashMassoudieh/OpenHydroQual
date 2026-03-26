from apps.worker.tasks import run_simulation


def test_worker_mock_mode() -> None:
    result = run_simulation({"job_id": "sim_1", "payload": {"project_id": "la-drywell-pilot"}})
    assert result["status"] == "completed"
    assert result["adapter"]["engine"] == "OHQuery"
