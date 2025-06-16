
import os
import difflib
import subprocess

DEFAULT_RUST_EXE_PATH = ".\\target\\debug\\palladium"

def failure(error: str) -> None:
    print(f"! Regression failed [failure]: {error}")
    

def error(error: str) -> None:
    print(f"!! Regression failed [error]: {error}")


def critical(error: str) -> None:
    print(f"!!! Regression failed [critical]: {error}")
    exit(1)


def load_test_cases(directory: str) -> dict:
    if not os.path.exists(directory):
        critical(f"Could not load files from directory {directory}")

    tests = {}

    for test_directory in os.listdir(directory):
        if not os.path.isdir(os.path.join(directory, test_directory)):
            continue

        input_files = []
        expected_files = []
        for file in os.listdir(os.path.join(directory, test_directory)):
            if file.endswith(".pd"):
                input_files.append(os.path.join(directory, test_directory, file))
            elif file.endswith(".txt"):
                expected_files.append(os.path.join(directory, test_directory, file))
        
        tests[test_directory] = {
            "input_files": input_files,
            "expected_files": expected_files
        }

    return tests


def run_test_cases(cases: dict) -> None:
    print()
    tests_run = 0
    failures = 0

    for test in cases:
        for i, input_file in enumerate(cases[test]["input_files"]):
            expected_file = cases[test]["expected_files"][i]
            print(f"Running test {test}")
            output_file = expected_file.replace(".txt", "")
            if os.path.exists(output_file):
                os.remove(output_file)
            subprocess.run(f"cmd /C {DEFAULT_RUST_EXE_PATH} regression\\{input_file}", stdout=subprocess.PIPE, cwd="..")
            if not os.path.exists(output_file):
                failures += 1
                error(f"Could not find output file {output_file} (Did palladium compile?)")
                continue

            with open(output_file, "r") as generated, open(expected_file, 'r') as expected:
                expected_lines = expected.readlines()
                generated_lines = generated.readlines()

                diff = difflib.unified_diff(
                    expected_lines,
                    generated_lines,
                    fromfile=expected_file,
                    tofile=output_file,
                    lineterm=''
                )

                tests_run += 1

                lines = list(diff)

                if len(lines) == 0:
                    print(f"SUCCESS: Test {test} passed")
                    continue

                failures += 1
                failure(f"Test {test} failed, diff below:")
                for line in lines:
                    print(line)
    
    print(f"\n[SUMMARY]:\n\tRan: {tests_run} tests, successes: {max(0, tests_run - failures)}, failures: {failures}")
    print(f"\t- Success rate: {max(0, tests_run - failures) / tests_run * 100}%")



def main() -> None:
    tests = load_test_cases("tests")
    run_test_cases(tests)


if __name__ == "__main__":
    main()