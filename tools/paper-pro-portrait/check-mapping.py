#!/usr/bin/env python3
"""Offline Ferrari numeric-probe review. Never grants runtime admission."""
import argparse
import hashlib
import json
import math
from pathlib import Path
import sys


class Rejected(ValueError):
    pass


def require(condition, reason):
    if not condition:
        raise Rejected(reason)


def number(value):
    require(type(value) in (int, float), "non-numeric-metadata")
    try:
        require(math.isfinite(value), "non-finite-metadata")
    except OverflowError:
        raise Rejected("non-finite-metadata") from None
    return value


def positive(value):
    require(number(value) > 0, "non-positive-metadata")
    return value


def unique_object(pairs):
    result = {}
    for key, value in pairs:
        require(key not in result, "duplicate-json-field")
        result[key] = value
    return result


def parse(raw):
    require(len(raw) <= 1024 * 1024, "response-too-large")
    def invalid_constant(_):
        raise Rejected("non-finite-json")
    try:
        return json.loads(raw, object_pairs_hook=unique_object,
                          parse_constant=invalid_constant)
    except (UnicodeError, json.JSONDecodeError, RecursionError):
        raise Rejected("invalid-json") from None
    except ValueError as error:
        if isinstance(error, Rejected):
            raise
        raise Rejected("invalid-json-number") from None


def snapshot(response):
    require(type(response) is dict, "response-not-object")
    require(response.get("ok") is True and response.get("diagnosticOnly") is True,
            "not-successful-diagnostic")
    require((response.get("model"), response.get("release"), response.get("qt")) ==
            ("ferrari", "3.28.0.172", "6.10.3"), "unsupported-runtime")
    require(response.get("scanTruncated") is False, "incomplete-scan")
    require(response.get("documentLoaded") is True, "no-document")
    windows = response.get("windows")
    require(type(windows) is list and len(windows) == 1, "ambiguous-window")
    window = windows[0]
    require(type(window) is dict, "invalid-window")
    for field in ("width", "height", "dpr"):
        positive(window.get(field))
    require((window["width"], window["height"], window["dpr"]) == (1620, 2160, 1),
            "unexpected-portrait-window")
    candidates = response.get("candidates")
    require(type(candidates) is list and len(candidates) == 2,
            "ambiguous-candidates")
    require(all(type(item) is dict for item in candidates), "invalid-candidate")
    controllers = [c for c in candidates if c.get("kind") == "document-controller"]
    views = [c for c in candidates if c.get("kind") == "visible-view"]
    require(len(controllers) == len(views) == 1, "ambiguous-candidates")
    controller, view = controllers[0], views[0]
    ordinal = controller.get("controllerOrdinal")
    require(type(ordinal) is int and ordinal > 0 and
            type(view.get("controllerOrdinal")) is int and
            view["controllerOrdinal"] == ordinal, "unassociated-controller")
    require(controller.get("templateStatus") == "unvalidated-ferrari-resource",
            "unexpected-template-status")
    require(view.get("viewToScene") == ["viewToScene(QVariant)"],
            "unsupported-mapping-abi")
    for candidate in (controller, view):
        require(candidate.get("insertImageFileAsSceneItem") ==
                ["insertImageFileAsSceneItem(QUrl,QPointF)"], "unsupported-insertion-abi")
    require(view.get("mappingStatus") == "valid" and view.get("mappingFinite") is True,
            "invalid-mapping")
    viewport = view.get("viewport")
    require(type(viewport) is dict and set(viewport) == {
        "width", "height", "windowWidth", "windowHeight", "devicePixelRatio", "points"
    }, "incomplete-viewport")
    for field in ("width", "height", "windowWidth", "windowHeight", "devicePixelRatio"):
        positive(viewport[field])
    positive(view.get("width"))
    positive(view.get("height"))
    require((viewport["width"], viewport["height"]) == (view["width"], view["height"]),
            "view-size-mismatch")
    require((viewport["windowWidth"], viewport["windowHeight"], viewport["devicePixelRatio"]) ==
            (window["width"], window["height"], window["dpr"]), "viewport-window-mismatch")
    points = viewport["points"]
    require(type(points) is list and len(points) == 3, "invalid-points")
    for point in points:
        require(type(point) is list and len(point) == 2, "invalid-points")
        for value in point:
            number(value)
    p0, p1, p2 = points
    dx, dy = number(p1[0] - p0[0]), number(p2[1] - p0[1])
    # Fixed before collecting evidence: cross-axis bound matches readonly-mapping.h;
    # uniform-scale bound matches geometry.h. Neither relaxes exact restoration.
    require(abs(number(p1[1] - p0[1])) <= 0.001 and
            abs(number(p2[0] - p0[0])) <= 0.001 and dx > 0 and dy > 0,
            "non-positive-axis-aligned-mapping")
    determinant = number(dx * dy - (p1[1] - p0[1]) * (p2[0] - p0[0]))
    require(determinant > 0.000001, "degenerate-mapping")
    sx, sy = dx / 100, dy / 100
    positive(sx)
    positive(sy)
    require(abs(sx - sy) <= 1e-9 * min(sx, sy), "nonuniform-mapping")
    return viewport, (sx, sy)


def review(baseline, zoomed, restored, owner_confirmed_normal=False):
    require(owner_confirmed_normal is True, "normal-zoom-not-owner-confirmed")
    normal, normal_scale = snapshot(baseline)
    _, zoom_scale = snapshot(zoomed)
    final, _ = snapshot(restored)
    require(zoom_scale != normal_scale, "zoom-scale-unchanged")
    require(final == normal, "normal-viewport-not-exactly-restored")
    # Controller ordinals are deliberately NOT compared between responses.
    return {"ok": True, "result": "numeric-metadata-consistent",
            "runtimeAdmission": False, "physicalCalibration": False,
            "templateValidated": False, "pageFreshnessEstablished": False}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("baseline", type=Path)
    parser.add_argument("zoomed", type=Path)
    parser.add_argument("restored", type=Path)
    parser.add_argument("--owner-confirmed-normal", action="store_true",
                        help="record actual owner confirmation; never infer from numbers")
    args = parser.parse_args()
    try:
        raw = []
        for path in (args.baseline, args.zoomed, args.restored):
            with path.open("rb") as stream:
                raw.append(stream.read(1024 * 1024 + 1))
        result = review(*(parse(data) for data in raw), args.owner_confirmed_normal)
        result["responseSha256"] = [hashlib.sha256(data).hexdigest() for data in raw]
    except (Rejected, OSError) as error:
        # Never echo source responses, paths, OS exception details or provider bodies.
        reason = str(error) if isinstance(error, Rejected) else "input-unreadable"
        print(json.dumps({"ok": False, "reason": reason, "runtimeAdmission": False}))
        return 1
    print(json.dumps(result, sort_keys=True))
    return 0


if __name__ == "__main__":
    sys.exit(main())
