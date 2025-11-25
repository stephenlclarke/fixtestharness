# FIX Protocol Test Harness

[![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=stephenlclarke_fixtestharness&metric=alert_status)](https://sonarcloud.io/summary/new_code?id=stephenlclarke_fixtestharness)
[![Bugs](https://sonarcloud.io/api/project_badges/measure?project=stephenlclarke_fixtestharness&metric=bugs)](https://sonarcloud.io/summary/new_code?id=stephenlclarke_fixtestharness)
[![Code Smells](https://sonarcloud.io/api/project_badges/measure?project=stephenlclarke_fixtestharness&metric=code_smells)](https://sonarcloud.io/summary/new_code?id=stephenlclarke_fixtestharness)
[![Coverage](https://sonarcloud.io/api/project_badges/measure?project=stephenlclarke_fixtestharness&metric=coverage)](https://sonarcloud.io/summary/new_code?id=stephenlclarke_fixtestharness)
[![Duplicated Lines (%)](https://sonarcloud.io/api/project_badges/measure?project=stephenlclarke_fixtestharness&metric=duplicated_lines_density)](https://sonarcloud.io/summary/new_code?id=stephenlclarke_fixtestharness)
[![Lines of Code](https://sonarcloud.io/api/project_badges/measure?project=stephenlclarke_fixtestharness&metric=ncloc)](https://sonarcloud.io/summary/new_code?id=stephenlclarke_fixtestharness)
[![Reliability Rating](https://sonarcloud.io/api/project_badges/measure?project=stephenlclarke_fixtestharness&metric=reliability_rating)](https://sonarcloud.io/summary/new_code?id=stephenlclarke_fixtestharness)
[![Security Rating](https://sonarcloud.io/api/project_badges/measure?project=stephenlclarke_fixtestharness&metric=security_rating)](https://sonarcloud.io/summary/new_code?id=stephenlclarke_fixtestharness)
[![Technical Debt](https://sonarcloud.io/api/project_badges/measure?project=stephenlclarke_fixtestharness&metric=sqale_index)](https://sonarcloud.io/summary/new_code?id=stephenlclarke_fixtestharness)
[![Maintainability Rating](https://sonarcloud.io/api/project_badges/measure?project=stephenlclarke_fixtestharness&metric=sqale_rating)](https://sonarcloud.io/summary/new_code?id=stephenlclarke_fixtestharness)
[![Vulnerabilities](https://sonarcloud.io/api/project_badges/measure?project=stephenlclarke_fixtestharness&metric=vulnerabilities)](https://sonarcloud.io/summary/new_code?id=stephenlclarke_fixtestharness)

Minimal FIX message harness built with GNU C++17, CMake, gcov/gcovr coverage,
and SonarQube targets borrowed from the ScopeTimer project.

## Components

- `fix_harness`: reads a FIX message log, extracts a Sender/Target CompID session, and writes `orig_tx.fix` (sent) and `orig_rx.fix` (received).
- `test_receiver`: loads `orig_rx.fix`, listens on a TCP port, and replies with those messages in order.
- `test_sender`: loads `orig_tx.fix`, sends each message to the receiver, reads responses, and validates them against `orig_rx.fix` (ignores timestamp/date fields such as 52/60/75 and derived checksum tag 10).
- `fix_tests`: lightweight unit/integration tests.

## Build

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## Usage

```bash
# Extract session messages from a log
./build/fix_harness path/to/fix.log SENDER TARGET output_dir
# Start the receiver (responds using orig_rx.fix)
./build/test_receiver output_dir/orig_rx.fix 9000
# In another shell, run the sender (validates responses)
./build/test_sender output_dir/orig_tx.fix output_dir/orig_rx.fix 127.0.0.1 9000
```

## Coverage and SonarQube

- `cmake --build build --target coverage` generates gcovr reports in `build/coverage` and Sonar XML at `build/sonarqube-coverage.xml`.
- `cmake --build build --target sonar_bw` captures a clean build with Sonar build-wrapper in `build/bw-output`.
- `cmake --build build --target sonar_scan` runs build-wrapper, coverage, and `sonar-scanner` (config in `sonar-project.properties`).

## Example MacOS build

```bash
❯ CC=/opt/homebrew/bin/gcc-15 CXX=/opt/homebrew/bin/g++-15 GCOV=/opt/homebrew/bin/gcov-15 \
cmake -S . -B build && cmake --build build && \
cmake --build build --target coverage && \
cmake --build build --target sonar_bw && \
cmake --build build --target sonar_scan

CMake Warning at CMakeLists.txt:91 (message):
  ENABLE_COVERAGE=ON but compiler is AppleClang.  Set CC/CXX to GCC to
  generate gcov data.


-- Using GCOV from environment: /opt/homebrew/bin/gcov-15
-- Downloading Sonar build-wrapper from: https://sonarcloud.io/static/cpp/build-wrapper-macosx-arm64.zip
-- Arm64 build-wrapper download failed; trying x86 fallback: https://sonarcloud.io/static/cpp/build-wrapper-macosx-x86.zip
-- Installed build-wrapper locally: /Users/sclarke/github/fixtestharness/build/tools/build-wrapper-macosx-x86/build-wrapper-macosx-x86
-- Using Sonar build-wrapper: /Users/sclarke/github/fixtestharness/build/tools/build-wrapper-macosx-x86/build-wrapper-macosx-x86
-- Configuring done (1.1s)
-- Generating done (0.1s)
-- Build files have been written to: /Users/sclarke/github/fixtestharness/build
[ 38%] Built target fixharness
[ 53%] Built target fix_harness
[ 69%] Built target test_sender
[ 84%] Built target test_receiver
[100%] Built target fix_tests
[ 62%] Built target fixharness
[ 87%] Built target fix_tests
[100%] Run tests once and generate gcovr reports (SonarQube XML + HTML)
Test project /Users/sclarke/github/fixtestharness/build
    Start 1: run_fix_tests
1/1 Test #1: run_fix_tests ....................   Passed    0.06 sec

100% tests passed, 0 tests failed out of 1

Total Test time (real) =   0.06 sec
(INFO) Reading coverage data...
(INFO) Writing coverage report...
lines: 0.0% (0 out of 0)
functions: 0.0% (0 out of 0)
branches: 0.0% (0 out of 0)
[100%] Built target coverage
[100%] Capture clean build with Sonar build-wrapper -> /Users/sclarke/github/fixtestharness/build/bw-output
[  7%] Building CXX object CMakeFiles/fixharness.dir/src/FixMessage.cpp.o
[ 15%] Building CXX object CMakeFiles/fixharness.dir/src/FixLogProcessor.cpp.o
[ 23%] Building CXX object CMakeFiles/fixharness.dir/src/TestReceiver.cpp.o
[ 30%] Building CXX object CMakeFiles/fixharness.dir/src/TestSender.cpp.o
[ 38%] Linking CXX static library libfixharness.a
[ 38%] Built target fixharness
[ 61%] Building CXX object CMakeFiles/test_sender.dir/apps/test_sender_main.cpp.o
[ 61%] Building CXX object CMakeFiles/test_receiver.dir/apps/test_receiver_main.cpp.o
[ 61%] Building CXX object CMakeFiles/fix_tests.dir/test/FixHarnessTest.cpp.o
[ 69%] Building CXX object CMakeFiles/fix_harness.dir/apps/harness_main.cpp.o
[ 76%] Linking CXX executable test_receiver
[ 84%] Linking CXX executable test_sender
[ 92%] Linking CXX executable fix_harness
[ 92%] Built target test_sender
[ 92%] Built target test_receiver
[ 92%] Built target fix_harness
[100%] Linking CXX executable fix_tests
[100%] Built target fix_tests
Built target sonar_bw
[100%] Run sonar build-wrapper, generate coverage, then run sonar-scanner
[100%] Capture clean build with Sonar build-wrapper -> /Users/sclarke/github/fixtestharness/build/bw-output
[  7%] Building CXX object CMakeFiles/fixharness.dir/src/FixMessage.cpp.o
[ 30%] Building CXX object CMakeFiles/fixharness.dir/src/FixLogProcessor.cpp.o
[ 30%] Building CXX object CMakeFiles/fixharness.dir/src/TestSender.cpp.o
[ 30%] Building CXX object CMakeFiles/fixharness.dir/src/TestReceiver.cpp.o
[ 38%] Linking CXX static library libfixharness.a
[ 38%] Built target fixharness
[ 46%] Building CXX object CMakeFiles/fix_tests.dir/test/FixHarnessTest.cpp.o
[ 61%] Building CXX object CMakeFiles/fix_harness.dir/apps/harness_main.cpp.o
[ 61%] Building CXX object CMakeFiles/test_receiver.dir/apps/test_receiver_main.cpp.o
[ 69%] Building CXX object CMakeFiles/test_sender.dir/apps/test_sender_main.cpp.o
[ 76%] Linking CXX executable test_receiver
[ 84%] Linking CXX executable test_sender
[ 92%] Linking CXX executable fix_harness
[ 92%] Built target test_receiver
[ 92%] Built target test_sender
[ 92%] Built target fix_harness
[100%] Linking CXX executable fix_tests
[100%] Built target fix_tests
Built target sonar_bw
[ 62%] Built target fixharness
[ 87%] Built target fix_tests
[100%] Run tests once and generate gcovr reports (SonarQube XML + HTML)
Test project /Users/sclarke/github/fixtestharness/build
    Start 1: run_fix_tests
1/1 Test #1: run_fix_tests ....................   Passed    0.31 sec

100% tests passed, 0 tests failed out of 1

Total Test time (real) =   0.31 sec
(INFO) Reading coverage data...
(INFO) Writing coverage report...
lines: 0.0% (0 out of 0)
functions: 0.0% (0 out of 0)
branches: 0.0% (0 out of 0)
[100%] Built target coverage
19:39:35.991 INFO  Scanner configuration file: /opt/homebrew/Cellar/sonar-scanner/7.3.0.5189/libexec/conf/sonar-scanner.properties
19:39:35.993 INFO  Project root configuration file: /Users/sclarke/github/fixtestharness/sonar-project.properties
19:39:36.002 INFO  SonarScanner CLI 7.3.0.5189
19:39:36.004 INFO  Mac OS X 26.1 aarch64
19:39:37.601 INFO  Communicating with SonarQube Cloud
19:39:37.602 INFO  JRE provisioning: os[macos], arch[arm64]
19:39:39.328 INFO  Starting SonarScanner Engine...
19:39:39.329 INFO  Java 17.0.11 Eclipse Adoptium (64-bit)
19:39:41.150 INFO  Load global settings
19:39:41.825 INFO  Load global settings (done) | time=675ms
19:39:41.867 INFO  Server id: 1BD809FA-AWHW8ct9-T_TB3XqouNu
19:39:41.957 INFO  Loading required plugins
19:39:41.957 INFO  Load plugins index
19:39:42.150 INFO  Load plugins index (done) | time=193ms
19:39:42.150 INFO  Load/download plugins
19:39:42.197 INFO  Load/download plugins (done) | time=47ms
19:39:42.428 INFO  Loaded core extensions: sca, a3s, architecture
19:39:42.558 INFO  Process project properties
19:39:42.567 INFO  Project key: fixtestharness
19:39:42.567 INFO  Base dir: /Users/sclarke/github/fixtestharness
19:39:42.567 INFO  Working dir: /Users/sclarke/github/fixtestharness/.scannerwork
19:39:42.575 INFO  Load project branches
19:39:42.886 INFO  Load project branches (done) | time=311ms
19:39:42.888 INFO  Load project settings for component key: 'fixtestharness'
19:39:43.278 INFO  Load project settings for component key: 'fixtestharness' (done) | time=387ms
19:39:43.984 INFO  Check ALM binding of project 'fixtestharness'
19:39:44.171 INFO  Detected project binding: NOT_BOUND
19:39:44.171 INFO  Check ALM binding of project 'fixtestharness' (done) | time=188ms
19:39:44.174 INFO  Load project pull requests
19:39:44.434 INFO  Load project pull requests (done) | time=260ms
19:39:44.437 INFO  Load branch configuration
19:39:44.438 INFO  Load branch configuration (done) | time=1ms
19:39:44.445 INFO  Load quality profiles
19:39:44.930 INFO  Load quality profiles (done) | time=485ms
19:39:44.947 INFO  Create analysis
19:39:45.285 INFO  Create analysis (done) | time=337ms
19:39:45.383 INFO  Load active rules
19:39:47.159 INFO  Load active rules (done) | time=1775ms
19:39:47.359 INFO  Organization key: stephenlclarke
19:39:47.368 INFO  Preprocessing files...
19:39:47.463 INFO  2 languages detected in 18 preprocessed files (done) | time=95ms
19:39:47.463 INFO  294 files ignored because of inclusion/exclusion patterns
19:39:47.463 INFO  87 files ignored because of scm ignore settings
19:39:47.521 INFO  Loading plugins for detected languages
19:39:47.521 INFO  Load/download plugins
19:39:47.523 INFO  Load/download plugins (done) | time=2ms
19:39:47.551 INFO  Load project repositories
19:39:47.834 INFO  Load project repositories (done) | time=282ms
19:39:47.839 INFO  Indexing files...
19:39:47.839 INFO  Project configuration:
19:39:47.839 INFO    Excluded sources: **/build/**, **/_deps/**, **/bw-output/**, **/compile_commands.json, **/build-wrapper-dump.json, **/test/**, **/*Test.cpp, **/*Tests.cpp
19:39:47.839 INFO    Included tests: **/test/**, **/*Test.cpp, **/*Tests.cpp
19:39:47.843 INFO  18 files indexed (done) | time=5ms
19:39:47.847 INFO  Quality profile for cpp: Sonar way
19:39:47.847 INFO  Quality profile for json: Sonar way
19:39:47.847 INFO  ------------- Run sensors on module FixTestHarness
19:39:47.869 INFO  Load metrics repository
19:39:48.078 INFO  Load metrics repository (done) | time=208ms
19:39:48.082 INFO  Sensor cache enabled
19:39:48.084 INFO  Load sensor cache
19:39:48.696 INFO  Load sensor cache (62 KB) | time=611ms
19:39:48.848 INFO  Sensor JaCoCo XML Report Importer [jacoco]
19:39:48.849 INFO  'sonar.coverage.jacoco.xmlReportPaths' is not defined. Using default locations: target/site/jacoco/jacoco.xml,target/site/jacoco-it/jacoco.xml,build/reports/jacoco/test/jacocoTestReport.xml
19:39:48.849 INFO  No report imported, no coverage information will be imported by JaCoCo XML Report Importer
19:39:48.849 INFO  Sensor JaCoCo XML Report Importer [jacoco] (done) | time=1ms
19:39:48.849 INFO  Sensor IaC CloudFormation Sensor [iac]
19:39:48.857 INFO  There are no files to be analyzed for the CloudFormation language
19:39:48.857 INFO  Sensor IaC CloudFormation Sensor [iac] (done) | time=8ms
19:39:48.857 INFO  Sensor IaC cfn-lint report Sensor [iac]
19:39:48.857 INFO  Sensor IaC cfn-lint report Sensor [iac] (done) | time=0ms
19:39:48.858 INFO  Sensor IaC Docker Sensor [iac]
19:39:48.858 INFO  There are no files to be analyzed for the Docker language
19:39:48.858 INFO  Sensor IaC Docker Sensor [iac] (done) | time=1ms
19:39:48.858 INFO  Sensor IaC hadolint report Sensor [iac]
19:39:48.858 INFO  Sensor IaC hadolint report Sensor [iac] (done) | time=0ms
19:39:48.858 INFO  Sensor IaC Azure Resource Manager Sensor [iac]
19:39:48.859 INFO  There are no files to be analyzed for the Azure Resource Manager language
19:39:48.859 INFO  Sensor IaC Azure Resource Manager Sensor [iac] (done) | time=0ms
19:39:48.859 INFO  Sensor Java Config Sensor [iac]
19:39:48.860 INFO  There are no files to be analyzed for the Java language
19:39:48.860 INFO  Sensor Java Config Sensor [iac] (done) | time=1ms
19:39:48.860 INFO  Sensor IaC GitHub Actions Sensor [iac]
19:39:48.860 INFO  There are no files to be analyzed for the GitHub Actions language
19:39:48.860 INFO  Sensor IaC GitHub Actions Sensor [iac] (done) | time=0ms
19:39:48.860 INFO  Sensor IaC Shell Sensor [iac]
19:39:48.860 INFO  There are no files to be analyzed for the Shell language
19:39:48.860 INFO  Sensor IaC Shell Sensor [iac] (done) | time=0ms
19:39:48.860 INFO  Sensor IaC JSON Sensor [iac]
19:39:48.861 INFO  Sensor for language "JSON" is enabled by a feature flag. You can disable it by setting "sonar.json.activate" to false.
19:39:48.865 INFO  1 source file to be analyzed
19:39:48.886 INFO  1/1 source file has been analyzed
19:39:48.886 INFO  Sensor IaC JSON Sensor [iac] (done) | time=26ms
19:39:48.887 INFO  Sensor Generic Coverage Report
19:39:48.887 INFO  Parsing /Users/sclarke/github/fixtestharness/build/sonarqube-coverage.xml
19:39:48.905 INFO  Imported coverage data for 0 files
19:39:48.905 INFO  Sensor Generic Coverage Report (done) | time=19ms
19:39:48.905 INFO  Sensor EnterpriseTextAndSecretsSensor [textenterprise]
19:39:48.915 INFO  Available processors: 12
19:39:48.915 INFO  Using 12 threads for analysis.
19:39:49.102 INFO  Start fetching files for the text and secrets analysis
19:39:49.110 INFO  Using Git CLI to retrieve untracked files
19:39:49.125 INFO  Retrieving language associated files and files included via "sonar.text.inclusions" that are tracked by git
19:39:49.126 INFO  Starting the text and secrets analysis
19:39:49.126 INFO  14 source files to be analyzed for the text and secrets analysis
19:39:49.153 INFO  14/14 source files have been analyzed for the text and secrets analysis
19:39:49.154 INFO  Start fetching files for the binary file analysis
19:39:49.154 INFO  There are no files to be analyzed for the binary file analysis
19:39:49.154 INFO  1 file is ignored because it is untracked by git
19:39:49.155 INFO  Sensor EnterpriseTextAndSecretsSensor [textenterprise] (done) | time=250ms
19:39:49.155 INFO  Sensor CFamily [cpp]
19:39:49.178 INFO  CFamily plugin version: 6.74.0.91793 (b828a9364d6778e337590775598d969f96b9514c)
19:39:49.178 INFO  CFamily analysis configuration mode: Compile-Commands
19:39:49.179 INFO  Incremental symbolic execution is enabled
19:39:49.179 INFO  Use incremental symbolic execution's path length implementation
19:39:49.180 INFO  Using compile commands: /Users/sclarke/github/fixtestharness/build/bw-output/compile_commands.json
19:39:49.181 INFO  Available processors: 12
19:39:49.181 INFO  Using 12 threads for analysis.
19:39:50.643 INFO  Loading cache from: server
19:39:51.768 INFO  Detected compilers: c++=8
19:39:51.768 INFO  Number of compilation units in the provided Compilation Database: 8
19:39:51.769 INFO  Number of skipped unsupported compilation units: 0
19:39:51.769 INFO  Number of skipped non-indexed compilation units: 1
19:39:51.769 INFO  Number of skipped duplicate compilation units: 0
19:39:51.769 INFO  Number of remaining compilation units to be analyzed: 7
19:39:51.769 INFO  Detected standards: c++17=7
19:39:51.769 INFO  Starting the analysis of 7 CFamily compilation units
19:39:51.899 INFO  [1/7] Cache hit for: /Users/sclarke/github/fixtestharness/apps/harness_main.cpp
19:39:51.925 INFO  [2/7] Cache hit for: /Users/sclarke/github/fixtestharness/apps/test_receiver_main.cpp
19:39:51.940 INFO  [3/7] Cache hit for: /Users/sclarke/github/fixtestharness/apps/test_sender_main.cpp
19:39:51.946 INFO  [4/7] Cache hit for: /Users/sclarke/github/fixtestharness/src/FixLogProcessor.cpp
19:39:51.954 INFO  [5/7] Cache hit for: /Users/sclarke/github/fixtestharness/src/FixMessage.cpp
19:39:51.964 INFO  [6/7] Cache hit for: /Users/sclarke/github/fixtestharness/src/TestReceiver.cpp
19:39:51.970 INFO  [7/7] Cache hit for: /Users/sclarke/github/fixtestharness/src/TestSender.cpp
19:39:51.982 INFO  11/11 files marked as unchanged
19:39:51.983 INFO  1/1 test files marked as unchanged
19:39:51.983 INFO  Analysis cache: 7/7 hits, 235658 bytes
19:39:51.983 INFO  Incremental symbolic execution cache: 0 hits out of 0 total analyzed targets (hit rate 0%)
19:39:51.983 INFO  CFamily analysis done in: 2805ms
19:39:51.983 INFO  7 compilation units were fully analyzed
19:39:51.983 INFO  0 compilation units were partially analyzed: 0 stopped on Config, 0 stopped on Parsing, 0 stopped on AST, 0 stopped on Symbolic Execution, and 0 stopped on Symbolic Execution with Parsing Error
19:39:51.983 INFO  54 rules with 'symbolic-execution' tag raised 0 out of 27 issues
19:39:51.984 INFO  Performance statistics per analysis stage (Total, Average, Percentage) across 7 compilation units:
annotationStage: 53ms, 7ms, 0.7%
astRulesStage: 63ms, 9ms, 0.83%
configStage: 48ms, 6ms, 0.64%
parsingStage: 6721ms, 960ms, 88.94%
symbolicExecutionStage: 672ms, 96ms, 8.89%
19:39:51.984 INFO  PCH: unique=0 use=0 (forceInclude=0,throughHeader=0,firstInclude=0) out of 7 (forceInclude=0,throughHeader=0)
19:39:51.984 INFO  Z3 refutation rate: 0 out of 0
19:39:51.984 INFO  Percentage of files indexed with CFamily languages: 92.31% (C: 0, C++: 12, ObjC: 0, AnyLang: 13)
19:39:51.985 INFO  C and C++ analysis quality score: 100/100
19:39:51.985 INFO  Analysis measures statistics:
100% of classes were parsed successfully (0 out of 6 have parsing errors)
100% of functions were parsed successfully (0 out of 30 have parsing errors)
100% of statements were parsed successfully (0 out of 266 have parsing errors)
100% of the project includes directives were resolved (0 out of 50 were not resolved)
0 external includes directives were not resolved
0 modules imports were not resolved
0 header units were not resolved
19:39:51.987 INFO  Incremental symbolic execution internal implementation statistics (across analyzed compilation units):
Successful cache reads, writes: 0, 0 for 0 target compilation unit(s) (success rates 0%, 0%)
Unsupported cases in USR conversion decoding, encoding: 0, 0
Unsupported cases in issue-relocation decoding, encoding: 0, 0
Unsupported cases in abs. line no. decoding, encoding: 0, 0
Top-level declarations dropped decoding, encoding: 0, 0
Replayed path diagnostics: 0
Recorded path diagnostics: 0
Overwritten path diagnostic targets: 0
19:39:51.988 INFO  Sensor CFamily [cpp] (done) | time=2833ms
19:39:51.991 INFO  ------------- Run sensors on project
19:39:52.050 INFO  Sensor Zero Coverage Sensor
19:39:52.054 INFO  Sensor Zero Coverage Sensor (done) | time=4ms
19:39:52.127 INFO  ------------- Gather SCA dependencies on project
19:39:52.127 INFO  Checking if SCA is enabled for organization stephenlclarke
19:39:52.879 INFO  Dependency analysis skipped
19:39:52.885 INFO  CPD Executor Calculating CPD for 11 files
19:39:52.899 INFO  CPD Executor CPD calculation finished (done) | time=14ms
19:39:52.900 INFO  SCM Publisher SCM provider for this project is: git
19:39:52.901 INFO  SCM Publisher 13 source files to be analyzed
19:39:52.984 INFO  SCM Publisher 0/13 source files have been analyzed (done) | time=82ms
19:39:52.984 WARN  Missing blame information for the following files:
19:39:52.984 WARN    * CMakePresets.json
19:39:52.984 WARN    * apps/test_sender_main.cpp
19:39:52.984 WARN    * apps/harness_main.cpp
19:39:52.984 WARN    * src/FixMessage.cpp
19:39:52.984 WARN    * src/FixLogProcessor.cpp
19:39:52.984 WARN    * apps/test_receiver_main.cpp
19:39:52.984 WARN    * test/FixHarnessTest.cpp
19:39:52.984 WARN    * src/TestReceiver.cpp
19:39:52.984 WARN    * include/fix/FixMessage.hpp
19:39:52.984 WARN    * include/fix/TestSender.hpp
19:39:52.985 WARN    * src/TestSender.cpp
19:39:52.985 WARN    * include/fix/FixLogProcessor.hpp
19:39:52.985 WARN    * include/fix/TestReceiver.hpp
19:39:52.985 WARN  This may lead to missing/broken features in SonarCloud
19:39:53.051 INFO  Analysis report generated in 64ms, dir size=369 KB
19:39:53.080 INFO  Analysis report compressed in 28ms, zip size=91 KB
19:39:53.653 INFO  Analysis report uploaded in 572ms
19:39:53.654 INFO  ANALYSIS SUCCESSFUL, you can find the results at: https://sonarcloud.io/dashboard?id=fixtestharness
19:39:53.655 INFO  Note that you will be able to access the updated dashboard once the server has processed the submitted analysis report
19:39:53.655 INFO  More about the report processing at https://sonarcloud.io/api/ce/task?id=AZq8h5WZJ6H2gophs7Nc
19:39:53.655 INFO  ------------- Upload SCA dependency files
19:39:54.188 INFO  Sensor cache published successfully
19:39:54.231 INFO  Analysis total time: 11.803 s
19:39:54.232 INFO  SonarScanner Engine completed successfully
19:39:54.647 INFO  EXECUTION SUCCESS
19:39:54.649 INFO  Total time: 18.658s
Built target sonar_scan
```
