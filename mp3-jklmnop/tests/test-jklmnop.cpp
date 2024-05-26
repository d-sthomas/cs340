#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib/catch.hpp"
#include "lib/mstats-utils.h"

mstats_result* student_results[15];
mstats_result* libc_results[15];

void run(int testerNumber, int indexNumber) {
  char *cmd_student = NULL;
  char *cmd_libc = NULL;

  asprintf(&cmd_student, "./mstats tests/testers_exe/tester%d evaluate", testerNumber);
  asprintf(&cmd_libc, "./mstats tests/testers_exe/tester%d evaluate libc", testerNumber);

  system(cmd_libc);
  system(cmd_student);

  student_results[indexNumber] = read_mstats_result("mstats_result.txt");
  libc_results[indexNumber] = read_mstats_result("mstats_result_libc.txt");
  REQUIRE(student_results[indexNumber]->status == 1);

  // Require less memory usage:
  REQUIRE( student_results[indexNumber]->max_heap_used <= libc_results[indexNumber]->max_heap_used );

  // Require total time be less than 10x libc:
  INFO("Total time must be less than 10x of libc");
  REQUIRE( student_results[indexNumber]->time_taken <= libc_results[indexNumber]->time_taken * 1000 );

  system("rm mstats_result.txt");
  system("rm mstats_result_libc.txt");

  free(cmd_student);
  free(cmd_libc);
}

// Testers 1-5
TEST_CASE("you vs. libc", "[weight=100][timeout=60]") {
  system("make -s");

  // Require the average %time used to be less than 100%:
  float totalPct = 0;

  // Run every test 3 times:
  for (int j = 0; j < 3; j++) {
    for (int i = 0; i < 5; i++) {
      run(i + 1, (i * 3) + j);
    }
  }

  printf("\n");
  printf("== Running Time Summary ==\n");
  for (int i = 0; i < 15; i++) {
    float pct = (student_results[i]->time_taken / libc_results[i]->time_taken) * 100.0;
    totalPct += pct;

    printf(
      "tester%d (run #%d): (you) %fs vs %fs (libc) [%4.2f%%]\n",
      (i / 3) + 1,
      (i % 3) + 1,
      student_results[i]->time_taken,
      libc_results[i]->time_taken,
      pct
    );

    if (i % 3 == 2) {
      printf("\n");
    }
  }

  float averageTimePct = totalPct / 15;
  REQUIRE( averageTimePct <= 100 );
}
