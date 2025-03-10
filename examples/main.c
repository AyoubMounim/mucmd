
/*
    Copyright (C) 2025 Ayoub Mounim

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "mucmd.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

// declare commands
static int hello_handler(int argc, char *argv[]);
static int sum_handler(int argc, char *argv[]);

static struct mucmd_command const cmds[] = {
    {"hello", hello_handler, "prints greetings"},
    {"sum", sum_handler, "the sum of two numbers"},
    {0},
};

// define allocator
static void *alloc_func(void *ctx, size_t const size) {
  (void)ctx;
  return malloc(size);
}

static void *realloc_func(void *ctx, void *ptr, size_t const size) {
  (void)ctx;
  return realloc(ptr, size);
}

static void free_func(void *ctx, void *ptr) {
  (void)ctx;
  free(ptr);
  return;
}

static struct mucmd_allocator allocator = {
    .alloc = alloc_func,
    .realloc = realloc_func,
    .free = free_func,
    .ctx = NULL,
};

int main(int argc, char *argv[]) {
  struct mucmd cmd;
  mucmd_init(&cmd, allocator);
  // register commands
  mucmd_set_commands(&cmd, cmds);

  printf("starting cmd interpreter...\n\n");
  mucmd_run(&cmd);
  printf("\nbye\n");

  return 0;
}

// define commands
static int hello_handler(int argc, char *argv[]) {
  if (argc != 1) {
    printf("expected 1 args, got %d\n", argc);
    return -1;
  }
  printf("Hello There!\nGeneral %s\n", argv[0]);
  return 0;
}

static int sum_handler(int argc, char *argv[]) {
  if (argc != 2) {
    printf("expected 2 args, got %d\n", argc);
    return -1;
  }
  float a = strtof(argv[0], NULL);
  float b = strtof(argv[1], NULL);
  printf("result is %f\n", a + b);
  return 0;
}
