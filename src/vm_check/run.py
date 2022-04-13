# Josh A. Bosley 
# April 12 2022
# This script compiles and executes all files within the directory
# and ensure that they have the expected return code

import re
import glob
import sys
import os
import subprocess
import threading
import time

if len(sys.argv) < 2:
    print("Expected path to skiff binary")
    exit(1)

parallel = False
skiff = sys.argv[1]

print("\n<< VM CHECKS >>\n")
check_directory = os.getcwd()
print("CWD : ", check_directory)

# Directories with checks
# - They are placed in this order as reliance begins to build on
#   previous checks working
test_directories = [
  check_directory + "/movs",
  check_directory + "/asserts",
  check_directory + "/branching",
  check_directory + "/arithmetic",
  check_directory + "/memory",
]

def time_to_ms_str(t):
  return str(round(t * 1000, 4)) + "ms"

def display_result(result_item):
    out = "-" * 10
    out += "\n\n"
    out += "Test File    : " + result_item["name"] + "\n"
    out += "Assembler    : "

    if not result_item["assemble_result"]["success"]:
      out += "[FAILED] after " + time_to_ms_str(result_item["assemble_result"]["time"]) + "\n"
      out += "\n"
      out += "\t---- output ----\n"
      out += result_item["assemble_result"]["output"]
      out += "\t----------------\n"
      print(out)
      exit(1)
    else:
      out += "[PASSED] after " + time_to_ms_str(result_item["assemble_result"]["time"]) + "\n"

    out += "VM Execution : "

    if not result_item["execution_result"]["success"]:
      out += "[FAILED] after " + time_to_ms_str(result_item["execution_result"]["time"])  + "\n"
      out += "\n"
      out += "\t---- output ----\n"
      out += result_item["execution_result"]["output"]
      out += "\t----------------\n"
      print(out)
      exit(1)
    else:
      out += "[PASSED] after " + time_to_ms_str(result_item["execution_result"]["time"]) + "\n"

    print(out)

def test_item(id, expected_result, item):
  results = {}

  bin_name = "task_" + str(id) + "_test.bin"

  assemble_start = time.time()
  assemble_result = subprocess.run([skiff, "-a", item, "-o", bin_name, "-l", "trace"], stdout=subprocess.PIPE)
  assemble_end = time.time()
  assemble_status = True

  decoded = assemble_result.stdout.decode("utf-8")
  if "No resulting binary." in decoded or "Error (line:" in decoded:
    assemble_status = False

  results["name"] = item

  results["assemble_result"] = {
    "time": assemble_end - assemble_start,
    "success": assemble_status,
    "output": decoded
  }
  if not assemble_status:
    return results

  execute_start = time.time()
  execute_result = subprocess.run([skiff, bin_name, "-l", "trace"], stdout=subprocess.PIPE)
  execute_end = time.time()

  results["execution_result"] = {
    "time": execute_end - execute_start,
    "expected_code": int(expected_result),
    "actual_code": execute_result.returncode,
    "success": execute_result.returncode == int(expected_result),
    "output": execute_result.stdout.decode("utf-8")
  }

  return results

def retrieve_objects_from(directory):
  os.chdir(directory)
  items_in_dir = glob.glob("*.asm")
  results = []
  for item in items_in_dir:
    result = {}
    result["expected_code"] = item.split("_")[0]
    result["path"] = directory + "/" + item
    results.append(result)
  return results

def build_exec_list(dirs):
  exec_list = []
  for dir in dirs:
    print("Scanning directory : ", dir)
    exec_list.append(retrieve_objects_from(dir))
  print("")
  return exec_list

def task(id, jobs):
  results = []
  for item in jobs:
    results.append(test_item(id, item["expected_code"], item["path"]))
  for item in results:
    display_result(item)

def linear_run():
  exec_list = build_exec_list(test_directories)
  os.chdir(check_directory)
  for item in exec_list:
    task(0, item)


print("\n<< LINEAR >>\n")
run_time_start = time.time()
linear_run()
run_time_end = time.time()

print("-" * 10)
print("\nChecks complete after ", round(run_time_end - run_time_start, 4), " seconds\n")

exit(0)