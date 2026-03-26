.PHONY: venv install test test-api test-worker

venv:
	python -m venv .venv

install:
	. .venv/bin/activate && pip install -r requirements-dev.txt

test:
	. .venv/bin/activate && pytest -q tests

test-api:
	. .venv/bin/activate && pytest -q tests/test_api_contract.py

test-worker:
	. .venv/bin/activate && pytest -q tests/test_worker_task.py
