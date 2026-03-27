from datetime import datetime

from apps.worker.tasks import run_simulation


def test_worker_mock_mode() -> None:
    result = run_simulation({"job_id": "sim_1", "payload": {"project_id": "la-drywell-pilot"}})

    assert result["job_id"] == "sim_1"
    assert result["status"] == "completed"
    assert result["result_contract"] == "simulation_result.v1"
    assert result["adapter"]["engine"] == "OHQuery"
    assert result["adapter"]["mock"] is True
    assert result["adapter"]["mock_mode"] is True
    assert datetime.fromisoformat(result["generated_at_utc"])
    assert result["metrics"]["peak_depth_m"] == 0.0
    assert result["metrics"]["infiltrated_volume_m3"] == 0.0
    assert result["metrics"]["overflow"] is False
