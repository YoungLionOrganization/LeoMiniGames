#!/usr/bin/env python3
"""Compatibility entry point; use validate_release_contract.py for new CI."""
from validate_release_contract import main
if __name__ == "__main__":
    raise SystemExit(main())
