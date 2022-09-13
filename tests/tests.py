########################################################################################
# File       : tests.py
# Author     : Devin Arena
# Description: Handles automated test cases for Palladium.
# Since      : 7/4/2022
########################################################################################

from cmath import exp
import sys
import glob
import os
import subprocess


##
# Runs a specific test case, comparing the output of the palladium file to the contents of the expected result file.
#
# @param case The name of the test case to run.
#
def run_test(case):
    # Run the test case.
    output = subprocess.check_output([
        os.path.join("..", "bin", "palladium.exe"),
        os.path.join("cases", case + ".pd")
    ])
    # Compare the output to the expected result.
    expected = open(os.path.join("cases", case + ".expected.txt"), "r")
    passed = False
    if output.decode("utf-8").replace("\r", "") == expected.read():
        passed = True
        print(f"Test [{case}] passed.")
    else:
        print(f"Test [{case}] failed.")
    expected.close()

    return passed


##
# Uses glob to find all .pd test cases in the cases/ directory.
# Runs each of these test cases.
#
def run_all_tests():
    passed = 0
    failed = 0
    for case in glob.glob("cases/*.pd"):
        case = os.path.splitext(os.path.os.path.basename(case))[0]
        print("\nRunning test case: " + case)
        if run_test(case):
            passed += 1
        else:
            failed += 1

    print(f"\nPassed: {passed}")
    print(f"Failed: {failed}")


##
# Uses glob to find all .pd test cases in the cases/ directory.
# Runs each of these test cases and appends their output to an output file.
#
def compile_all_tests():
    counter = 0
    for case in glob.glob("cases/*.pd"):
        case = os.path.splitext(os.path.os.path.basename(case))[0]
        print("\nCompiling test case: " + case)
        with open(os.path.join("cases", case + ".expected.txt"), "w") as f:
            output = subprocess.check_output([
                os.path.join("..", "bin", "palladium.exe"),
                os.path.join("cases", case + ".pd")
            ])
            f.write(output.decode("utf-8").replace("\r", ""))
        counter += 1

    print(f"\nSuccessfully compiled {counter} test cases")


##
# Main function.
#
def main(argv):
    os.chdir(os.path.dirname(os.path.abspath(__file__)))
    if argv[0] == "all":
        print("Running all tests.")
        run_all_tests()
    elif argv[0] == "compile":
        print("Compiling all tests.")
        compile_all_tests()
    else:
        case = argv[0]
        if os.path.exists(os.path.join("cases", case + ".pd")):
            print("Running test case: " + case)
            run_test(case)
        else:
            print("Test case not found.")
            sys.exit(1)


if __name__ == "__main__":
    if len(sys.argv) <= 1:
        print("Palladium automated testing client.")
        print("Developed by Devin Arena.")
        print("Usage: python3 tests.py [all|test_name|compile]")
        print("\tall: Runs all test cases.")
        print(
            "\tcompile: Compiles all test cases (runs .pd files and places their output into cases/)."
        )
        print("\ttest_name: Runs a specific test case.")
        sys.exit(1)
    main(sys.argv[1:])