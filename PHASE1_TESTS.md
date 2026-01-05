# Verification Tests for Fase 1 Upgrades

## Test 1: Invalid Date Rejection (31 February)
echo "[2026-02-31 10:30:15] [ERROR] Test" > /tmp/test_invalid_date.log
./log_analyzer --input /tmp/test_invalid_date.log --report /tmp/test_report.txt
# Expected: Should reject with BadTimestamp

## Test 2: Valid Leap Year Date (29 February 2024)
echo "[2024-02-29 10:30:15] [INFO] Leap year test" > /tmp/test_leap.log
./log_analyzer --input /tmp/test_leap.log --report /tmp/test_report.txt
# Expected: Should parse successfully

## Test 3: Invalid Leap Year (29 February 2023)
echo "[2023-02-29 10:30:15] [ERROR] Not a leap year" > /tmp/test_not_leap.log
./log_analyzer --input /tmp/test_not_leap.log --report /tmp/test_report.txt
# Expected: Should reject with BadTimestamp

## Test 4: Flexible Bracket Positions
echo "[2026-01-05 10:30:15]    [ERROR] Extra spaces" > /tmp/test_spaces.log
# Expected: Should still parse (robust find() based parsing)
