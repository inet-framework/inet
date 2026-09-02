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

    def test_spaced_std_determinism_sources_are_reported(self) -> None:
        self.write(
            "src/inet/linklayer/ethernet/SpacedChrono.cc",
            "auto now = std\n    :: chrono :: system_clock :: now();\n",
        )
        self.write(
            "src/inet/linklayer/ethernet/SpacedRandom.cc",
            "auto seed = std /* legal whitespace */ :: random_device{}();\n",
        )

        result = self.check("src/inet/linklayer/ethernet")

        self.assertEqual(1, result.returncode, result.stdout + result.stderr)
        self.assertIn("SpacedChrono.cc", result.stdout)
        self.assertIn("SpacedRandom.cc", result.stdout)

    def test_deterministic_chrono_types_and_operations_are_ignored(self) -> None:
        self.write(
            "src/inet/linklayer/ethernet/DeterministicChrono.cc",
            """using Duration = std::chrono::duration<double>;
using Clock = std::chrono::system_clock;
using TimePoint = Clock::time_point;
auto duration = Duration{1};
auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
auto epoch = TimePoint{};
auto count = Duration{}.count();
auto unrelated = OtherClock{}.now();
auto member = clockObject.now();
auto wrapped = makeClock(Clock{}).now();
auto wrappedGrouped = makeClock((Clock{})).now();
""",
        )

        result = self.check("src/inet/linklayer/ethernet")

        self.assertEqual(0, result.returncode, result.stdout + result.stderr)

    def test_imported_and_aliased_random_devices_are_reported_at_the_use(self) -> None:
        sources = {
            "UsingRandom.cc": "using std::random_device;\nauto seed = random_device{}();\n",
            "UsingNamespaceRandom.cc": "using namespace std;\nauto seed = random_device{}();\n",
            "NamespaceAliasRandom.cc": "namespace standard = std;\nauto seed = standard::random_device{}();\n",
            "TypeAliasRandom.cc": "using RandomDevice = std::random_device;\nauto seed = RandomDevice{}();\n",
            "AliasChainRandom.cc": """namespace standard = std;
using RandomDevice = standard::random_device;
using EntropySource = RandomDevice;
auto seed = EntropySource{}();
""",
        }
        for name, source in sources.items():
            self.write(f"src/inet/linklayer/ethernet/{name}", source)

        result = self.check("src/inet/linklayer/ethernet")

        self.assertEqual(1, result.returncode, result.stdout + result.stderr)
        for name, source in sources.items():
            with self.subTest(name=name):
                use_line = len(source.splitlines())
                self.assertIn(f"{name}:{use_line}:", result.stdout)

    def test_direct_chrono_clock_object_reads_are_reported(self) -> None:
        self.write(
            "src/inet/linklayer/ethernet/DirectClockObjects.cc",
            """auto first = std::chrono::system_clock{}.now();
auto second = std::chrono::steady_clock().now();
auto third = (std::chrono::high_resolution_clock{}).now();
auto fourth = ((std::chrono::system_clock())).now();
consume((std::chrono::steady_clock{}).now());
auto sixth = (::std::chrono::system_clock{}).now();
""",
        )

        result = self.check("src/inet/linklayer/ethernet")

        self.assertEqual(1, result.returncode, result.stdout + result.stderr)
        for line in range(1, 7):
            self.assertIn(f"DirectClockObjects.cc:{line}:", result.stdout)

    def test_aliased_chrono_clock_object_reads_are_reported(self) -> None:
        self.write(
            "src/inet/linklayer/ethernet/AliasedClockObjects.cc",
            """using BraceClock = std::chrono::system_clock;
using ParenClock = std::chrono::steady_clock;
namespace chrono = std::chrono;
using GroupedClock = chrono::high_resolution_clock;
auto first = BraceClock{}.now();
auto second = ParenClock().now();
auto third = (GroupedClock{}).now();
auto fourth = ((BraceClock())).now();
consume((ParenClock()).now());
""",
        )

        result = self.check("src/inet/linklayer/ethernet")

        self.assertEqual(1, result.returncode, result.stdout + result.stderr)
        for line in range(5, 10):
            self.assertIn(f"AliasedClockObjects.cc:{line}:", result.stdout)

    def test_chrono_clock_object_reads_in_unevaluated_operands_are_ignored(self) -> None:
        self.write(
            "src/inet/linklayer/ethernet/UnevaluatedClockObjects.cc",
            """using Clock = std::chrono::system_clock;
decltype(Clock{}.now()) first;
constexpr auto second = sizeof(std::chrono::steady_clock().now());
constexpr bool third = noexcept((Clock{}).now());
decltype(inspect(((std::chrono::high_resolution_clock())).now())) fourth;
constexpr auto fifth = sizeof((Clock()).now());
constexpr auto sixth = sizeof Clock{}.now();
""",
        )

        result = self.check("src/inet/linklayer/ethernet")

        self.assertEqual(0, result.returncode, result.stdout + result.stderr)

    def test_chrono_clock_object_reads_in_evaluated_nested_expressions_are_reported(self) -> None:
        self.write(
            "src/inet/linklayer/ethernet/EvaluatedClockObjects.cc",
            """using Clock = std::chrono::system_clock;
consume((Clock{}).now());
auto second = sizeof(int) ? Clock{}.now() : fallback();
auto third = noexcept(fallback()) ? std::chrono::steady_clock{}.now() : fallback();
auto fourth = decltype(existing)(Clock().now());
""",
        )

        result = self.check("src/inet/linklayer/ethernet")

        self.assertEqual(1, result.returncode, result.stdout + result.stderr)
        for line in range(2, 6):
            self.assertIn(f"EvaluatedClockObjects.cc:{line}:", result.stdout)

    def test_static_chrono_clock_reads_in_unevaluated_operands_are_ignored(self) -> None:
        self.write(
            "src/inet/linklayer/ethernet/UnevaluatedStaticClocks.cc",
            """using Clock = std::chrono::system_clock;
namespace chrono = std::chrono;
decltype(std::chrono::system_clock::now()) first;
constexpr bool second = noexcept(Clock::now());
constexpr auto third = sizeof(chrono::steady_clock::now());
constexpr auto fourth = sizeof Clock::now();
""",
        )

        result = self.check("src/inet/linklayer/ethernet")

        self.assertEqual(0, result.returncode, result.stdout + result.stderr)

    def test_chrono_clock_reads_in_requires_expressions_are_ignored(self) -> None:
        self.write(
            "src/inet/linklayer/ethernet/RequiresClockReads.cc",
            """using Clock = std::chrono::system_clock;
namespace chrono = std::chrono;
static_assert(requires {
    std::chrono::steady_clock{}.now();
    Clock::now();
});
template <typename T>
concept HasClockReads = requires(T value) {
    chrono::high_resolution_clock().now();
    Clock{}.now();
};
""",
        )

        result = self.check("src/inet/linklayer/ethernet")

        self.assertEqual(0, result.returncode, result.stdout + result.stderr)

    def test_chrono_clock_reads_after_requires_expressions_are_reported(self) -> None:
        self.write(
            "src/inet/linklayer/ethernet/EvaluatedAfterRequires.cc",
            """using Clock = std::chrono::system_clock;
constexpr bool objectSupported = requires { Clock{}.now(); };
auto objectRead = Clock{}.now();
constexpr bool staticSupported = requires { Clock::now(); };
auto staticRead = Clock::now();
""",
        )

        result = self.check("src/inet/linklayer/ethernet")

        self.assertEqual(1, result.returncode, result.stdout + result.stderr)
        for line in (3, 5):
            self.assertIn(f"EvaluatedAfterRequires.cc:{line}:", result.stdout)
        for line in (2, 4):
            self.assertNotIn(f"EvaluatedAfterRequires.cc:{line}:", result.stdout)

    def test_chrono_clock_reads_in_deferred_lambda_bodies_are_reported(self) -> None:
        self.write(
            "src/inet/linklayer/ethernet/DeferredLambda.cc",
            """using Clock = std::chrono::system_clock;
using Reader = decltype([] { return Clock{}.now(); });
using ExplicitReader = decltype([]() -> Clock::time_point { return Clock::now(); });
using SizeReader = decltype([] { return sizeof(Clock{}.now()); });
auto reader = Reader{};
auto explicitReader = ExplicitReader{};
auto sizeReader = SizeReader{};
auto value = reader();
auto explicitValue = explicitReader();
auto size = sizeReader();
""",
        )

        result = self.check("src/inet/linklayer/ethernet")

        self.assertEqual(1, result.returncode, result.stdout + result.stderr)
        self.assertIn("DeferredLambda.cc:2:", result.stdout)
        self.assertIn("DeferredLambda.cc:3:", result.stdout)
        self.assertNotIn("DeferredLambda.cc:4:", result.stdout)

    def test_chrono_clock_reads_in_constrained_function_bodies_are_reported(self) -> None:
        self.write(
            "src/inet/linklayer/ethernet/ConstrainedFunction.cc",
            """using Clock = std::chrono::system_clock;
template <typename T>
void readClock() requires (sizeof(T) > 0) {
    auto value = Clock{}.now();
}
""",
        )

        result = self.check("src/inet/linklayer/ethernet")

        self.assertEqual(1, result.returncode, result.stdout + result.stderr)
        self.assertIn("ConstrainedFunction.cc:4:", result.stdout)

    def test_random_device_imports_and_aliases_are_forbidden_without_a_draw(self) -> None:
        self.write(
            "src/inet/linklayer/ethernet/ImportedRandom.cc",
            """using std::random_device;
using RandomDevice = std::random_device;
typedef std::random_device EntropySource;
""",
        )

        result = self.check("src/inet/linklayer/ethernet")

        self.assertEqual(1, result.returncode, result.stdout + result.stderr)
        for line in (1, 2, 3):
            self.assertIn(f"ImportedRandom.cc:{line}:", result.stdout)

    def test_direct_imported_and_aliased_chrono_clock_reads_are_reported(self) -> None:
        sources = {
            "DirectClocks.cc": """auto wall = std::chrono::system_clock::now();
auto monotonic = std::chrono::steady_clock::now();
auto highResolution = std::chrono::high_resolution_clock::now();
""",
            "UsingClock.cc": "using std::chrono::system_clock;\nauto now = system_clock::now();\n",
            "UsingNamespaceClock.cc": "using namespace std::chrono;\nauto now = system_clock::now();\n",
            "NamespaceAliasClock.cc": "namespace chrono = std::chrono;\nauto now = chrono::system_clock::now();\n",
            "TypeAliasClock.cc": "using Clock = std::chrono::steady_clock;\nauto now = Clock::now();\n",
            "TypedefClock.cc": "typedef std::chrono::high_resolution_clock Clock;\nauto now = Clock::now();\n",
            "AliasChainClock.cc": """namespace standard = std;
namespace chrono = standard::chrono;
using Clock = chrono::system_clock;
using WallClock = Clock;
auto now = WallClock::now();
""",
        }
        for name, source in sources.items():
            self.write(f"src/inet/linklayer/ethernet/{name}", source)

        result = self.check("src/inet/linklayer/ethernet")

        self.assertEqual(1, result.returncode, result.stdout + result.stderr)
        for name, source in sources.items():
            with self.subTest(name=name):
                use_line = len(source.splitlines())
                self.assertIn(f"{name}:{use_line}:", result.stdout)

    def test_global_and_std_rand_time_calls_are_reported(self) -> None:
        self.write(
            "src/inet/linklayer/ethernet/BadCalls.cc",
            "auto first = :: rand();\nauto second = std :: time(nullptr);\n",
        )

        result = self.check("src/inet/linklayer/ethernet")

        self.assertEqual(1, result.returncode, result.stdout + result.stderr)
        self.assertIn("BadCalls.cc", result.stdout)

    def test_rand_and_time_calls_in_unevaluated_operands_are_ignored(self) -> None:
        self.write(
            "src/inet/linklayer/ethernet/UnevaluatedCalls.cc",
            """using RandomResult = decltype(::rand());
constexpr auto timeSize = sizeof(std::time(nullptr));
constexpr bool randomIsNoexcept = noexcept(rand());
static_assert(requires {
    std::rand();
    ::time(nullptr);
});
""",
        )

        result = self.check("src/inet/linklayer/ethernet")

        self.assertEqual(0, result.returncode, result.stdout + result.stderr)

    def test_rand_and_time_calls_after_requires_expressions_are_reported(self) -> None:
        self.write(
            "src/inet/linklayer/ethernet/EvaluatedCalls.cc",
            """static_assert(requires { rand(); std::time(nullptr); });
auto randomValue = rand();
auto wallTime = std::time(nullptr);
""",
        )

        result = self.check("src/inet/linklayer/ethernet")

        self.assertEqual(1, result.returncode, result.stdout + result.stderr)
        self.assertNotIn("EvaluatedCalls.cc:1:", result.stdout)
        self.assertIn("EvaluatedCalls.cc:2:", result.stdout)
        self.assertIn("EvaluatedCalls.cc:3:", result.stdout)

    def test_member_and_unrelated_qualified_calls_are_ignored(self) -> None:
        self.write(
            "src/inet/linklayer/ethernet/Deterministic.cc",
            """auto first = object.time();
auto second = pointer->time();
auto third = other :: time();
auto fourth = object.rand();
auto fifth = pointer -> rand();
auto sixth = other::rand();
auto seventh = other::std::chrono::steady_clock::now();
auto eighth = object.now();
auto ninth = other::std::chrono::steady_clock{}.now();
""",
        )

        result = self.check("src/inet/linklayer/ethernet")

        self.assertEqual(0, result.returncode, result.stdout + result.stderr)

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
