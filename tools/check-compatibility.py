#!/usr/bin/env python3
"""Offline release gate. A successful compile is NOT runtime validation."""
import argparse
import json
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("target", choices=("rm2", "ferrari", "chiappa"))
    parser.add_argument("release")
    parser.add_argument("--architecture", required=True)
    args = parser.parse_args()
    manifest = json.loads((Path(__file__).resolve().parents[1] / "compatibility/targets.json").read_text())
    target = manifest["targets"][args.target]
    supported = args.architecture == target["architecture"] and args.release in target["install_releases"]
    print(json.dumps({"target": args.target, "release": args.release,
                      "architecture": args.architecture, "install_allowed": supported,
                      "status": target["status"]}, sort_keys=True))
    return 0 if supported else 1


if __name__ == "__main__":
    raise SystemExit(main())
