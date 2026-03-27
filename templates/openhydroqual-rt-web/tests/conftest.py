from pathlib import Path
import sys


# Ensure `apps.*` imports resolve regardless of where pytest is invoked from.
TEMPLATE_ROOT = Path(__file__).resolve().parents[1]
if str(TEMPLATE_ROOT) not in sys.path:
    sys.path.insert(0, str(TEMPLATE_ROOT))
