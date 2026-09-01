#!/usr/bin/env python3

from __future__ import annotations

import subprocess
import tempfile
import unittest
from pathlib import Path


CHECKER = Path(__file__).with_name("check-architecture.sh").resolve()


class CheckArchitectureTest(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name)
        for relative in (
            "src/inet/common",
            "src/inet/linklayer/common",
            "src/inet/linklayer/ethernet",
            "src/inet/applications/example",
        ):
            (self.root / relative).mkdir(parents=True, exist_ok=True)

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def write(self, relative: str, text: str) -> None:
        path = self.root / relative
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text, encoding="utf-8")

    def check(self, scope: str) -> subprocess.CompletedProcess[str]:
        return subprocess.run(
            ["bash", str(CHECKER), scope],
            cwd=self.root,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )

    def test_non_common_scope_does_not_apply_domain_rule(self) -> None:
        self.write(
            "src/inet/linklayer/ethernet/Ethernet.cc",
            '#include "inet/linklayer/common/MacAddress.h"\n',
        )
        result = self.check("src/inet/linklayer/ethernet")
        self.assertEqual(0, result.returncode, result.stdout + result.stderr)
        self.assertIn("AR-ORG-DOMAINS: N/A", result.stdout)

    def test_common_scope_reports_upward_dependency(self) -> None:
        self.write(
            "src/inet/common/Bad.cc",
            '#include "inet/linklayer/ethernet/Ethernet.h"\n',
        )
        result = self.check("src/inet/common")
        self.assertEqual(1, result.returncode, result.stdout + result.stderr)
        self.assertIn("AR-ORG-DOMAINS", result.stdout)
        self.assertIn("Bad.cc", result.stdout)

    def test_application_scope_reports_transport_implementation(self) -> None:
        self.write(
            "src/inet/applications/example/Bad.cc",
            '#include "inet/transportlayer/tcp/Tcp.h"\n',
        )
        result = self.check("src/inet/applications/example")
        self.assertEqual(1, result.returncode, result.stdout + result.stderr)
        self.assertIn("AR-COM-SOCKETS", result.stdout)
        self.assertIn("Bad.cc", result.stdout)

    def test_determinism_scan_applies_to_requested_scope(self) -> None:
        self.write(
            "src/inet/linklayer/ethernet/Bad.cc",
            "auto seed = std::random_device{}();\n",
        )
        result = self.check("src/inet/linklayer/ethernet")
        self.assertEqual(1, result.returncode, result.stdout + result.stderr)
        self.assertIn("AR-QUAL-DETERMINISM", result.stdout)
        self.assertIn("Bad.cc", result.stdout)

    def test_determinism_scan_does_not_exclude_named_directories(self) -> None:
        paths = (
            "src/inet/visualizer/BadVisualizer.cc",
            "src/inet/thirdparty/BadThirdParty.cc",
            "src/inet/applications/external/BadExternal.cc",
        )
        for path in paths:
            self.write(path, "auto seed = std::random_device{}();\n")

        result = self.check("src/inet")

        self.assertEqual(1, result.returncode, result.stdout + result.stderr)
        for path in paths:
            self.assertIn(Path(path).name, result.stdout)

    def test_wall_clock_time_with_output_pointer_is_reported(self) -> None:
        self.write(
            "src/inet/linklayer/ethernet/Bad.cc",
            "std::time_t now; auto value = std::time(&now);\n",
        )
        result = self.check("src/inet/linklayer/ethernet")
        self.assertEqual(1, result.returncode, result.stdout + result.stderr)
        self.assertIn("AR-QUAL-DETERMINISM", result.stdout)
        self.assertIn("Bad.cc", result.stdout)

    def test_determinism_scan_ignores_comments_and_literals(self) -> None:
        self.write(
            "src/inet/linklayer/ethernet/Prose.cc",
            """// simulation time (s), not a call to time(&now)
/* std::time(&now) and std::chrono::system_clock are examples. */
const char *message = "time(nullptr) std::random_device std::chrono::";
const char *raw = R"doc(rand() and time(&now))doc";
""",
        )
        result = self.check("src/inet/linklayer/ethernet")
        self.assertEqual(0, result.returncode, result.stdout + result.stderr)

    def test_noncanonical_scope_components_are_rejected(self) -> None:
        for scope in (
            "src/inet/linklayer/../applications",
            "src/inet/./applications",
            "src/inet//applications",
        ):
            with self.subTest(scope=scope):
                result = self.check(scope)
                self.assertEqual(2, result.returncode, result.stdout + result.stderr)
                self.assertIn("normalized", result.stderr)


if __name__ == "__main__":
    unittest.main()
