import argparse
import re

def calculate_pass_rate(summary_log_path):
    with open(summary_log_path, 'r') as file:
        lines = file.readlines()

    results = []
    total_files = 0
    pass_files = 0
    total_effective_cases = 0
    total_pass_cases = 0

    for line in lines:
        parts = line.strip().split(',')
        if len(parts) < 2:
            test_name = parts[0]
            results.append((test_name, 0, 0, 0))
            total_files += 1
            continue

        test_name = parts[0]
        result = parts[1]

        ran_match = re.search(r'Ran (\d+) tests?', result)
        skipped_match = re.search(r'skipped=(\d+)', result)
        failures_match = re.search(r'failures=(\d+)', result)
        errors_match = re.search(r'errors=(\d+)', result)

        ran = int(ran_match.group(1)) if ran_match else 0
        skipped = int(skipped_match.group(1)) if skipped_match else 0
        failures = int(failures_match.group(1)) if failures_match else 0
        errors = int(errors_match.group(1)) if errors_match else 0

        # Check for additional occurrences of failures and errors
        if len(parts) > 2:
            for additional_result in parts:
                additional_failures_match = re.search(r'failures=(\d+)', additional_result)
                additional_errors_match = re.search(r'errors=(\d+)', additional_result)
                failures = max(failures, int(additional_failures_match.group(1)) if additional_failures_match else 0)
                errors = max(errors, int(additional_errors_match.group(1)) if additional_errors_match else 0)

        effective_case = ran - skipped
        pass_case = effective_case - failures - errors
        if pass_case > 0:
            pass_rate = pass_case / effective_case
        else:
            pass_rate = 0.0
        if effective_case <= 0:
            pass_rate = 1.0

        results.append((test_name, pass_rate, pass_case, effective_case))

        total_files += 1
        if pass_rate == 1.0:
            pass_files += 1
        total_effective_cases += effective_case
        total_pass_cases += pass_case

    overall_pass_rate = pass_files / total_files if total_files > 0 else 0.0
    sub_test_pass_rate = total_pass_cases / total_effective_cases if total_effective_cases > 0 else 0.0

    print(f"Overall pass rate: {overall_pass_rate:.2%}, {pass_files}/{total_files}")
    print(f"Sub-test pass rate: {sub_test_pass_rate:.2%}, {total_pass_cases}/{total_effective_cases}")
    return results, overall_pass_rate, sub_test_pass_rate

def print_pass_rates(results, overall_pass_rate, sub_test_pass_rate):
    for test_name, pass_rate, pass_case, effective_case in results:
        print(f"{test_name}: {pass_rate:.2%}, {pass_case}/{effective_case}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Calculate pass rates from summary log")
    parser.add_argument("--summary_log_path", required=True, help="Path to the summary log file")
    args = parser.parse_args()

    results, overall_pass_rate, sub_test_pass_rate = calculate_pass_rate(args.summary_log_path)
    print_pass_rates(results, overall_pass_rate, sub_test_pass_rate)
