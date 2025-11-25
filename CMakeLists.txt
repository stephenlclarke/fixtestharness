# -----------------------------------------------------------------------------
# FIX Protocol Test Harness - CMake configuration and build scripts
# -----------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.16)
project(FixTestHarness LANGUAGES CXX)

# --- GCC-15 toolchain defaults (borrowed from ScopeTimer) ---------------------
if(NOT DEFINED ENV{CC} AND NOT CMAKE_C_COMPILER)
  find_program(_GCC15 gcc-15 PATHS /opt/homebrew/bin /usr/local/bin /usr/bin)
  if(_GCC15)
    set(ENV{CC} "${_GCC15}")
    message(STATUS "CC not set; defaulting to: $ENV{CC}")
  endif()
endif()
if(NOT DEFINED ENV{CXX} AND NOT CMAKE_CXX_COMPILER)
  find_program(_GPP15 g++-15 PATHS /opt/homebrew/bin /usr/local/bin /usr/bin)
  if(_GPP15)
    set(ENV{CXX} "${_GPP15}")
    message(STATUS "CXX not set; defaulting to: $ENV{CXX}")
  endif()
endif()
if(NOT DEFINED ENV{GCOV})
  find_program(_GCOV15 gcov-15 PATHS /opt/homebrew/bin /usr/local/bin /usr/bin)
  if(_GCOV15)
    set(ENV{GCOV} "${_GCOV15}")
    message(STATUS "GCOV not set; defaulting to: $ENV{GCOV}")
  endif()
endif()

if(NOT CMAKE_C_COMPILER AND DEFINED ENV{CC})
  set(CMAKE_C_COMPILER "$ENV{CC}" CACHE FILEPATH "C compiler" FORCE)
endif()
if(NOT CMAKE_CXX_COMPILER AND DEFINED ENV{CXX})
  set(CMAKE_CXX_COMPILER "$ENV{CXX}" CACHE FILEPATH "C++ compiler" FORCE)
endif()

# --- Basics ------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Threads)

# --- Library -----------------------------------------------------------------
add_library(fixharness
  src/FixMessage.cpp
  src/FixLogProcessor.cpp
  src/TestSender.cpp
  src/TestReceiver.cpp
)
target_include_directories(fixharness PUBLIC ${CMAKE_SOURCE_DIR}/include)
if(Threads_FOUND)
  target_link_libraries(fixharness PUBLIC Threads::Threads)
endif()

# --- Applications ------------------------------------------------------------
add_executable(fix_harness apps/harness_main.cpp)
target_link_libraries(fix_harness PRIVATE fixharness)

add_executable(test_sender apps/test_sender_main.cpp)
target_link_libraries(test_sender PRIVATE fixharness)

add_executable(test_receiver apps/test_receiver_main.cpp)
target_link_libraries(test_receiver PRIVATE fixharness)

# --- Unit tests --------------------------------------------------------------
add_executable(fix_tests test/FixHarnessTest.cpp)
target_link_libraries(fix_tests PRIVATE fixharness)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(fix_tests PRIVATE --coverage -O0 -g)
  target_link_options(fix_tests PRIVATE --coverage)
endif()

include(CTest)
enable_testing()
add_test(NAME run_fix_tests COMMAND fix_tests)

# --- Coverage (GCC + gcovr) --------------------------------------------------
option(ENABLE_COVERAGE "Enable GCC gcov coverage flags" ON)
if(ENABLE_COVERAGE)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    foreach(tgt fix_harness test_sender test_receiver fix_tests)
      if(TARGET ${tgt})
        target_compile_options(${tgt} PRIVATE --coverage -O0 -g)
        target_link_options(${tgt} PRIVATE --coverage)
      endif()
    endforeach()
  else()
    message(WARNING "ENABLE_COVERAGE=ON but compiler is ${CMAKE_CXX_COMPILER_ID}. Set CC/CXX to GCC to generate gcov data.")
  endif()
endif()

find_program(GCOVR_EXECUTABLE NAMES gcovr REQUIRED)
if(DEFINED ENV{GCOV} AND NOT "$ENV{GCOV}" STREQUAL "")
  set(GCOV_EXE "$ENV{GCOV}")
  message(STATUS "Using GCOV from environment: ${GCOV_EXE}")
endif()
set(_GCC_MAJOR "")
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if(CMAKE_CXX_COMPILER_VERSION)
    string(REGEX MATCH "^[0-9]+" _GCC_MAJOR "${CMAKE_CXX_COMPILER_VERSION}")
  endif()
endif()
if(NOT GCOV_EXE)
  if(_GCC_MAJOR)
    find_program(GCOV_EXE NAMES gcov-${_GCC_MAJOR} gcov REQUIRED)
  else()
    find_program(GCOV_EXE NAMES gcov REQUIRED)
  endif()
endif()

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/cmake")
file(WRITE "${CMAKE_BINARY_DIR}/cmake/CleanGcda.cmake" "
file(GLOB_RECURSE gcda \"${CMAKE_BINARY_DIR}/*.gcda\")
if(gcda)
  message(STATUS \"Removing stale .gcda files to avoid stamp mismatch\")
  file(REMOVE ${gcda})
endif()
")

add_custom_target(coverage DEPENDS fix_tests
  COMMENT "Run tests once and generate gcovr reports (SonarQube XML + HTML)"
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/coverage
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/cmake/CleanGcda.cmake
  COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
  COMMAND ${GCOVR_EXECUTABLE}
          --gcov-executable ${GCOV_EXE}
          --root ${CMAKE_SOURCE_DIR}
          --object-directory ${CMAKE_BINARY_DIR}
          --sonarqube ${CMAKE_BINARY_DIR}/sonarqube-coverage.xml
          --html-details ${CMAKE_BINARY_DIR}/coverage/index.html
          --print-summary
          --delete
          --gcov-ignore-errors=no_working_dir_found
          --exclude '.*_deps/.*'
          --exclude '.*bw-output/.*'
          --exclude '.*CMakeFiles/.*'
          --exclude '.*build-wrapper-dump.json'
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  VERBATIM
)

# --- Sonar build-wrapper capture ---------------------------------------------
option(ENABLE_SONAR "Enable Sonar build-wrapper target" ON)
set(SONAR_BW_OUT ${CMAKE_BINARY_DIR}/bw-output CACHE PATH "build-wrapper output directory")
set(SONAR_BUILD_WRAPPER "" CACHE FILEPATH "Path to build-wrapper executable (optional; auto-detect if empty)")
set(SONAR_TOOLS_DIR ${CMAKE_BINARY_DIR}/tools)

function(_detect_or_install_build_wrapper OUT_VAR)
  if(SONAR_BUILD_WRAPPER)
    if(EXISTS "${SONAR_BUILD_WRAPPER}")
      set(${OUT_VAR} "${SONAR_BUILD_WRAPPER}" PARENT_SCOPE)
      return()
    endif()
  endif()
  if(DEFINED ENV{SONAR_BUILD_WRAPPER} AND EXISTS "$ENV{SONAR_BUILD_WRAPPER}")
    set(${OUT_VAR} "$ENV{SONAR_BUILD_WRAPPER}" PARENT_SCOPE)
    return()
  endif()
  if(APPLE)
    find_program(_BW_EXE NAMES build-wrapper-macosx-arm64 build-wrapper-macosx-x86)
  elseif(WIN32)
    find_program(_BW_EXE NAMES build-wrapper-win-x86-64.exe)
  else()
    find_program(_BW_EXE NAMES build-wrapper-linux-x86-64)
  endif()
  if(_BW_EXE)
    set(${OUT_VAR} "${_BW_EXE}" PARENT_SCOPE)
    return()
  endif()

  file(MAKE_DIRECTORY "${SONAR_TOOLS_DIR}")
  if(APPLE)
    set(_BW_ZIP_URL "https://sonarcloud.io/static/cpp/build-wrapper-macosx-arm64.zip")
    set(_BW_DIR "${SONAR_TOOLS_DIR}/build-wrapper-macosx-arm64")
    set(_BW_EXE_PATH "${_BW_DIR}/build-wrapper-macosx-arm64")
    set(_BW_FALLBACK_URL "https://sonarcloud.io/static/cpp/build-wrapper-macosx-x86.zip")
    set(_BW_FALLBACK_DIR "${SONAR_TOOLS_DIR}/build-wrapper-macosx-x86")
    set(_BW_FALLBACK_EXE "${_BW_FALLBACK_DIR}/build-wrapper-macosx-x86")
  elseif(WIN32)
    set(_BW_ZIP_URL "https://sonarcloud.io/static/cpp/build-wrapper-win-x86.zip")
    set(_BW_DIR "${SONAR_TOOLS_DIR}/build-wrapper-win-x86")
    set(_BW_EXE_PATH "${_BW_DIR}/build-wrapper-win-x86-64.exe")
  else()
    set(_BW_ZIP_URL "https://sonarcloud.io/static/cpp/build-wrapper-linux-x86-64.zip")
    set(_BW_DIR "${SONAR_TOOLS_DIR}/build-wrapper-linux-x86-64")
    set(_BW_EXE_PATH "${_BW_DIR}/build-wrapper-linux-x86-64")
  endif()

  set(_BW_ZIP_FILE "${SONAR_TOOLS_DIR}/build-wrapper.zip")
  message(STATUS "Downloading Sonar build-wrapper from: ${_BW_ZIP_URL}")
  file(DOWNLOAD "${_BW_ZIP_URL}" "${_BW_ZIP_FILE}" STATUS _dl_stat)
  list(GET _dl_stat 0 _dl_code)
  if(NOT _dl_code EQUAL 0)
    if(APPLE)
      message(STATUS "Arm64 build-wrapper download failed; trying x86 fallback: ${_BW_FALLBACK_URL}")
      file(DOWNLOAD "${_BW_FALLBACK_URL}" "${_BW_ZIP_FILE}" STATUS _dl_stat2)
      list(GET _dl_stat2 0 _dl_code2)
      if(NOT _dl_code2 EQUAL 0)
        message(WARNING "Failed to download build-wrapper (arm64 & x86). Configure SONAR_BUILD_WRAPPER manually.")
        set(${OUT_VAR} "" PARENT_SCOPE)
        return()
      endif()
      set(_BW_DIR "${_BW_FALLBACK_DIR}")
      set(_BW_EXE_PATH "${_BW_FALLBACK_EXE}")
    else()
      message(WARNING "Failed to download build-wrapper. Configure SONAR_BUILD_WRAPPER manually.")
      set(${OUT_VAR} "" PARENT_SCOPE)
      return()
    endif()
  endif()

  file(MAKE_DIRECTORY "${_BW_DIR}")
  execute_process(COMMAND ${CMAKE_COMMAND} -E tar xf "${_BW_ZIP_FILE}"
                  WORKING_DIRECTORY "${SONAR_TOOLS_DIR}"
                  RESULT_VARIABLE _unzip_res)
  if(NOT _unzip_res EQUAL 0)
    message(WARNING "Failed to extract build-wrapper archive at ${_BW_ZIP_FILE}.")
    set(${OUT_VAR} "" PARENT_SCOPE)
    return()
  endif()

  if(NOT EXISTS "${_BW_EXE_PATH}" AND WIN32)
    file(GLOB_RECURSE _BW_WIN_EXE "${_BW_DIR}/build-wrapper-win-x86-64.exe")
    if(_BW_WIN_EXE)
      list(GET _BW_WIN_EXE 0 _BW_EXE_PATH)
    endif()
  endif()

  if(EXISTS "${_BW_EXE_PATH}")
    message(STATUS "Installed build-wrapper locally: ${_BW_EXE_PATH}")
    set(${OUT_VAR} "${_BW_EXE_PATH}" PARENT_SCOPE)
  else()
    message(WARNING "build-wrapper executable not found after extraction: ${_BW_EXE_PATH}")
    set(${OUT_VAR} "" PARENT_SCOPE)
  endif()
endfunction()

if(ENABLE_SONAR)
  _detect_or_install_build_wrapper(SONAR_BUILD_WRAPPER_EXE)
  if(SONAR_BUILD_WRAPPER_EXE)
    message(STATUS "Using Sonar build-wrapper: ${SONAR_BUILD_WRAPPER_EXE}")
    add_custom_target(sonar_bw
      COMMENT "Capture clean build with Sonar build-wrapper -> ${SONAR_BW_OUT}"
      COMMAND ${CMAKE_COMMAND} -E make_directory ${SONAR_BW_OUT}
      COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target clean
      COMMAND ${SONAR_BUILD_WRAPPER_EXE} --out-dir ${SONAR_BW_OUT}
              ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --clean-first -j
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
              ${CMAKE_BINARY_DIR}/compile_commands.json
              ${SONAR_BW_OUT}/compile_commands.json
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      VERBATIM
    )
  else()
    message(STATUS "Sonar build-wrapper not available (download failed). 'sonar_bw' will print a helpful error.")
    add_custom_target(sonar_bw
      COMMENT "ERROR: Sonar build-wrapper missing — set SONAR_BUILD_WRAPPER to skip auto-download and use a custom path"
      COMMAND ${CMAKE_COMMAND} -E echo "[sonar_bw] ERROR: Sonar build-wrapper not found."
      COMMAND ${CMAKE_COMMAND} -E false
      VERBATIM
    )
  endif()
endif()

# --- Sonar scanner -----------------------------------------------------------
function(_detect_or_install_scanner OUT_VAR)
  find_program(_SC_EXE NAMES sonar-scanner sonar-scanner.bat)
  if(_SC_EXE)
    set(${OUT_VAR} "${_SC_EXE}" PARENT_SCOPE)
    return()
  endif()

  set(_URL_LINUX_AARCH64  "https://binaries.sonarsource.com/Distribution/sonar-scanner-cli/sonar-scanner-cli-7.2.0.5079-linux-aarch64.zip")
  set(_URL_LINUX_X64      "https://binaries.sonarsource.com/Distribution/sonar-scanner-cli/sonar-scanner-cli-7.2.0.5079-linux-x64.zip")
  set(_URL_MAC_AARCH64    "https://binaries.sonarsource.com/Distribution/sonar-scanner-cli/sonar-scanner-cli-7.2.0.5079-macosx-aarch64.zip")
  set(_URL_MAC_X64        "https://binaries.sonarsource.com/Distribution/sonar-scanner-cli/sonar-scanner-cli-7.2.0.5079-macosx-x64.zip")
  set(_URL_WIN_X64        "https://binaries.sonarsource.com/Distribution/sonar-scanner-cli/sonar-scanner-cli-7.2.0.5079-windows-x64.zip")

  set(_SC_URLS)
  if(WIN32)
    list(APPEND _SC_URLS ${_URL_WIN_X64})
  elseif(APPLE)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "(arm64|aarch64)")
      list(APPEND _SC_URLS ${_URL_MAC_AARCH64} ${_URL_MAC_X64})
    else()
      list(APPEND _SC_URLS ${_URL_MAC_X64} ${_URL_MAC_AARCH64})
    endif()
  else()
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "(arm64|aarch64)")
      list(APPEND _SC_URLS ${_URL_LINUX_AARCH64} ${_URL_LINUX_X64})
    else()
      list(APPEND _SC_URLS ${_URL_LINUX_X64} ${_URL_LINUX_AARCH64})
    endif()
  endif()

  file(MAKE_DIRECTORY "${SONAR_TOOLS_DIR}")
  set(_SC_ZIP_FILE "${SONAR_TOOLS_DIR}/sonar-scanner.zip")
  set(_DOWNLOADED_ZIP "")
  foreach(_URL IN LISTS _SC_URLS)
    message(STATUS "Attempting to download sonar-scanner from: ${_URL}")
    file(DOWNLOAD "${_URL}" "${_SC_ZIP_FILE}" STATUS _sc_dl TLS_VERIFY ON)
    list(GET _sc_dl 0 _sc_code)
    if(_sc_code EQUAL 0)
      set(_DOWNLOADED_ZIP "${_SC_ZIP_FILE}")
      set(_SC_URL_USED "${_URL}")
      break()
    endif()
  endforeach()

  if(NOT _DOWNLOADED_ZIP)
    message(WARNING "Failed to download sonar-scanner for this platform. Install sonar-scanner in PATH or set SONAR_SCANNER_EXECUTABLE manually.")
    set(${OUT_VAR} "" PARENT_SCOPE)
    return()
  endif()

  execute_process(COMMAND ${CMAKE_COMMAND} -E tar xf "${_DOWNLOADED_ZIP}"
                  WORKING_DIRECTORY "${SONAR_TOOLS_DIR}"
                  RESULT_VARIABLE _sc_unzip)
  if(NOT _sc_unzip EQUAL 0)
    message(WARNING "Failed to extract sonar-scanner archive at ${_DOWNLOADED_ZIP}")
    set(${OUT_VAR} "" PARENT_SCOPE)
    return()
  endif()

  file(GLOB_RECURSE _SC_BIN_CAND
       "${SONAR_TOOLS_DIR}/sonar-scanner*/bin/sonar-scanner"
       "${SONAR_TOOLS_DIR}/sonar-scanner*/bin/sonar-scanner.bat")
  if(_SC_BIN_CAND)
    list(GET _SC_BIN_CAND 0 _SC_EXE_FOUND)
    message(STATUS "Installed sonar-scanner locally from ${_SC_URL_USED}: ${_SC_EXE_FOUND}")
    set(${OUT_VAR} "${_SC_EXE_FOUND}" PARENT_SCOPE)
  else()
    message(WARNING "sonar-scanner executable not found after extraction in ${SONAR_TOOLS_DIR}")
    set(${OUT_VAR} "" PARENT_SCOPE)
  endif()
endfunction()

set(_scan_deps coverage)
if(TARGET sonar_bw)
  list(APPEND _scan_deps sonar_bw)
endif()

_detect_or_install_scanner(SONAR_SCANNER_EXECUTABLE)
if(SONAR_SCANNER_EXECUTABLE)
  add_custom_target(sonar_scan
    COMMENT "Run sonar build-wrapper, generate coverage, then run sonar-scanner"
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target sonar_bw
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target coverage
    COMMAND ${SONAR_SCANNER_EXECUTABLE}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    VERBATIM
  )
else()
  message(STATUS "sonar-scanner not available (download failed). 'sonar_scan' will print a helpful error.")
  add_custom_target(sonar_scan
    COMMENT "ERROR: sonar-scanner missing — install it or set PATH"
    COMMAND ${CMAKE_COMMAND} -E echo "[sonar_scan] ERROR: sonar-scanner not found."
    COMMAND ${CMAKE_COMMAND} -E false
    VERBATIM
  )
endif()
