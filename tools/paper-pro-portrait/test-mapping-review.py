#!/usr/bin/env python3
"""Synthetic evidence only; no recorded notebook/device data."""
from copy import deepcopy
import importlib.util
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

spec = importlib.util.spec_from_file_location("mapping_review", Path(__file__).with_name("check-mapping.py"))
checker = importlib.util.module_from_spec(spec)
spec.loader.exec_module(checker)


def fixture(scale=1, ordinal=1, origin=(17, -9)):
    insert = ["insertImageFileAsSceneItem(QUrl,QPointF)"]
    return {"ok": True, "diagnosticOnly": True, "model": "ferrari",
            "release": "3.28.0.172", "qt": "6.10.3", "scanTruncated": False,
            "documentLoaded": True, "windows": [{"width": 1620, "height": 2160, "dpr": 1}],
            "candidates": [
                {"kind": "document-controller", "controllerOrdinal": ordinal,
                 "templateStatus": "unvalidated-ferrari-resource",
                 "insertImageFileAsSceneItem": insert},
                {"kind": "visible-view", "controllerOrdinal": ordinal,
                 "width": 1500, "height": 2000,
                 "viewToScene": ["viewToScene(QVariant)"],
                 "insertImageFileAsSceneItem": insert,
                 "mappingStatus": "valid", "mappingFinite": True,
                 "viewport": {"width": 1500, "height": 2000, "windowWidth": 1620,
                              "windowHeight": 2160, "devicePixelRatio": 1,
                              "points": [list(origin), [origin[0] + scale * 100, origin[1]],
                                         [origin[0], origin[1] + scale * 100]]}}]}


class ReviewTests(unittest.TestCase):
    def test_success_is_not_admission_and_ordinals_are_response_local(self):
        result = checker.review(fixture(1, 1), fixture(2, 8), fixture(1, 23), True)
        self.assertEqual(result["result"], "numeric-metadata-consistent")
        for key in ("runtimeAdmission", "physicalCalibration", "templateValidated",
                    "pageFreshnessEstablished"):
            self.assertIs(result[key], False)

    def test_candidate_traversal_order_is_not_identity(self):
        response = fixture()
        response["candidates"].reverse()
        checker.review(response, fixture(2), fixture(), True)

    def test_owner_confirmation_is_required(self):
        with self.assertRaisesRegex(checker.Rejected, "not-owner-confirmed"):
            checker.review(fixture(), fixture(2), fixture())

    def test_invalid_evidence_stops_each_phase(self):
        mutations = [
            lambda r: r.update(ok=False),
            lambda r: r.update(diagnosticOnly=1),
            lambda r: r.update(model="chiappa"),
            lambda r: r.update(release="3.28.0.173"),
            lambda r: r.update(qt="6.10.4"),
            lambda r: r.update(scanTruncated=True),
            lambda r: r.pop("scanTruncated"),
            lambda r: r.update(documentLoaded=False),
            lambda r: r["windows"].append(deepcopy(r["windows"][0])),
            lambda r: r["windows"][0].update(width=2160, height=1620),
            lambda r: r["windows"][0].update(dpr=True),
            lambda r: r["candidates"].append(deepcopy(r["candidates"][1])),
            lambda r: r["candidates"][0].update(controllerOrdinal=0),
            lambda r: r["candidates"][1].update(controllerOrdinal=True),
            lambda r: r["candidates"][1].update(controllerOrdinal=42),
            lambda r: r["candidates"][0].update(templateStatus="blank"),
            lambda r: r["candidates"][1].update(viewToScene=["viewToScene(QPointF)"]),
            lambda r: r["candidates"][0].update(insertImageFileAsSceneItem=[]),
            lambda r: r["candidates"][1].update(insertImageFileAsSceneItem=[]),
            lambda r: r["candidates"][1].update(mappingFinite=None),
            lambda r: r["candidates"][1].update(mappingStatus="invocation-failed"),
            lambda r: r["candidates"][1].pop("viewport"),
            lambda r: r["candidates"][1].update(width=1499),
            lambda r: r["candidates"][1]["viewport"].update(width=0),
            lambda r: r["candidates"][1]["viewport"].update(devicePixelRatio=2),
            lambda r: r["candidates"][1]["viewport"].update(windowWidth=954),
            lambda r: r["candidates"][1]["viewport"].update(extraNumeric=10),
            lambda r: r["candidates"][1]["viewport"].update(points=[[0, 0], [100, 0]]),
            lambda r: r["candidates"][1]["viewport"].update(points=[[0, 0], [-100, 0], [0, 100]]),
            lambda r: r["candidates"][1]["viewport"].update(points=[[0, 0], [100, 0], [0, 101]]),
            lambda r: r["candidates"][1]["viewport"].update(points=[[0, 0], [100, 1], [0, 100]]),
            lambda r: r["candidates"][1]["viewport"].update(points=[[0, 0], [0, 100], [100, 0]]),
            lambda r: r["candidates"][1]["viewport"].update(points=[[0, 0], [0, 0], [0, 0]]),
            lambda r: r["candidates"][1]["viewport"].update(points=[[0, 0], [float("inf"), 0], [0, 100]]),
            lambda r: r["candidates"][1]["viewport"].update(points=[[0, 0], [True, 0], [0, 100]]),
            lambda r: r["candidates"][1]["viewport"].update(points=[[0, 0], [10**400, 0], [0, 100]]),
            lambda r: r["candidates"][1]["viewport"].update(points=[[-1e308, 0], [1e308, 0], [-1e308, 100]]),
            lambda r: r["candidates"][1]["viewport"].update(points=[[0, 0], [1e-5, 0], [0, 1e-5]]),
            lambda r: r["candidates"][1]["viewport"].update(points=[[0, 0], [1e308, 0], [0, 1e308]]),
        ]
        for index, mutation in enumerate(mutations):
            for phase in range(3):
                with self.subTest(mutation=index, phase=phase):
                    responses = [fixture(), fixture(2), fixture()]
                    mutation(responses[phase])
                    with self.assertRaises(checker.Rejected):
                        checker.review(*responses, True)

    def test_pan_is_not_zoom(self):
        with self.assertRaisesRegex(checker.Rejected, "zoom-scale-unchanged"):
            checker.review(fixture(), fixture(origin=(100, 200)), fixture(), True)

    def test_exact_restoration_not_a_posthoc_tolerance(self):
        responses = [fixture(), fixture(2), fixture()]
        responses[2]["candidates"][1]["viewport"]["points"][0][0] += 1e-12
        with self.assertRaisesRegex(checker.Rejected, "not-exactly-restored"):
            checker.review(*responses, True)

    def test_restored_dimensions_also_must_match(self):
        restored = fixture()
        restored["candidates"][1]["width"] = 1499
        restored["candidates"][1]["viewport"]["width"] = 1499
        with self.assertRaisesRegex(checker.Rejected, "not-exactly-restored"):
            checker.review(fixture(), fixture(2), restored, True)

    def test_strict_json(self):
        for raw in (b'{"ok":true,"ok":false}', b'{"value":NaN}', b'{"v":Infinity}',
                    b'[]trailing', b'\xff', b'[' * 2000, b' ' * (1024 * 1024 + 1)):
            with self.subTest(raw_length=len(raw)), self.assertRaises(checker.Rejected):
                checker.parse(raw)

    def test_cli_success_failure_and_private_output(self):
        script = str(Path(__file__).with_name("check-mapping.py"))
        with tempfile.TemporaryDirectory() as directory:
            paths = [Path(directory) / name for name in ("normal.json", "zoom.json", "restored.json")]
            for path, response in zip(paths, [fixture(), fixture(2), fixture()]):
                path.write_text(json.dumps(response))
            command = [sys.executable, script, *map(str, paths), "--owner-confirmed-normal"]
            success = subprocess.run(command, capture_output=True, text=True)
            self.assertEqual(success.returncode, 0, success.stderr)
            result = json.loads(success.stdout)
            self.assertEqual(len(result["responseSha256"]), 3)
            self.assertNotIn("controllerOrdinal", success.stdout)
            paths[0].write_text('{"private-marker": "DO-NOT-ECHO", "ok": false}')
            failed = subprocess.run(command, capture_output=True, text=True)
            self.assertEqual(failed.returncode, 1)
            self.assertNotIn("DO-NOT-ECHO", failed.stdout + failed.stderr)
            paths[0].unlink()
            missing = subprocess.run(command, capture_output=True, text=True)
            self.assertEqual(missing.returncode, 1)
            self.assertNotIn(directory, missing.stdout + missing.stderr)
            self.assertEqual(json.loads(missing.stdout)["reason"], "input-unreadable")


if __name__ == "__main__":
    unittest.main()
