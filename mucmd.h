
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

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO: logging.

#define MUCMD_PROMPT ">>> "

typedef int (*mucmd_handler)(int argc, char *argv[]);

struct mucmd_command {
  char *name;
  mucmd_handler handler;
  char *desc;
};

struct mucmd_allocator {
  void *(*alloc)(void *ctx, size_t const size);
  void *(*realloc)(void *ctx, void *ptr, size_t const size);
  void (*free)(void *ctx, void *obj);
  void *ctx;
};

struct mucmd {
  struct mucmd_allocator allocator;
  struct mucmd_command *cmds;
  size_t cmds_number;
};

/* ============================================ Public functions declaration */

#define INLINE static inline

INLINE void mucmd_init(struct mucmd *mucmd, struct mucmd_allocator allocator);

INLINE void mucmd_deinit(struct mucmd *mucmd);

INLINE int mucmd_set_commands(struct mucmd *mucmd,
                              struct mucmd_command const *cmds);

INLINE int mucmd_run(struct mucmd *mucmd);

#define MUCMD_IMPLEMENTATION
#ifdef MUCMD_IMPLEMENTATION

#define MUCMD_ALLOC(mucmd, size)                                               \
  mucmd->allocator.alloc(mucmd->allocator.ctx, size)

#define MUCMD_REALLOC(mucmd, ptr, size)                                        \
  mucmd->allocator.realloc(mucmd->allocator.ctx, ptr, size)

#define MUCMD_FREE(mucmd, ptr) mucmd->allocator.free(mucmd->allocator.ctx, ptr)

struct mucmd_parsed_command {
  char *name;
  int argc;
  char **argv;
  struct mucmd_allocator allocator;
};

static void mucmd_parsed_command_destroy(struct mucmd_parsed_command *command) {
  if (command == NULL) {
    return;
  }
  if (command->argc > 0) {
    MUCMD_FREE(command, command->argv);
  }
  MUCMD_FREE(command, command);
  command = NULL;
  return;
}

static struct mucmd_parsed_command *
mucmd_parsed_command_create(char *input, struct mucmd_allocator allocator) {
  static char const *parsing_delim = " \n";
  size_t capacity = 16;
  struct mucmd_parsed_command *command =
      (struct mucmd_parsed_command *)allocator.alloc(
          allocator.ctx, sizeof(struct mucmd_parsed_command));
  if (command == NULL) {
    goto fail;
  }
  command->allocator = allocator;
  command->name = strtok(input, parsing_delim);
  if (command->name == NULL) {
    goto fail;
  }
  command->argc = 0;
  command->argv =
      (char **)allocator.alloc(allocator.ctx, capacity * sizeof(char *));
  if (command->argv == NULL) {
    goto fail;
  }
  while (true) {
    char *arg = strtok(NULL, parsing_delim);
    if (arg == NULL) {
      MUCMD_REALLOC(command, command->argv, command->argc * sizeof(char *));
      break;
    }
    command->argv[command->argc] = arg;
    command->argc++;
    if (command->argc == capacity) {
      capacity <<= 1;
      MUCMD_REALLOC(command, command->argv, capacity * sizeof(char *));
    }
  }
  return command;
fail:
  mucmd_parsed_command_destroy(command);
  return NULL;
}

static bool mucmd_exit(char *input) {
  bool exit = false;
  if (strcmp(input, "exit\n") == 0) {
    exit = true;
  }
  return exit;
}

static bool mucmd_info(char *input) {
  bool info = false;
  if (strcmp(input, "info\n") == 0) {
    info = true;
  }
  return info;
}

static void mucmd_print_info(struct mucmd const *mucmd) {
  if (mucmd->cmds_number == 0) {
    fprintf(stdout, "you have no registered commands\n");
    return;
  }
  for (size_t i = 0; i < mucmd->cmds_number; i++) {
    char *desc = mucmd->cmds[i].desc;
    char *name = mucmd->cmds[i].name;
    if (desc) {
      fprintf(stdout, "%s\t%s\n", name, desc);
    } else {
      fprintf(stdout, "%s\n", name);
    }
  }
  return;
}

void mucmd_init(struct mucmd *mucmd, struct mucmd_allocator allocator) {
  memset(mucmd, 0, sizeof(struct mucmd));
  mucmd->allocator = allocator;
  return;
}

void mucmd_deinit(struct mucmd *mucmd) {
  if (mucmd == NULL) {
    return;
  }
  if (mucmd->cmds && mucmd->allocator.free) {
    MUCMD_FREE(mucmd, mucmd->cmds);
  }
  mucmd->cmds_number = 0;
  return;
}

int mucmd_set_commands(struct mucmd *mucmd, struct mucmd_command const *cmds) {
  int res = 0;
  if (mucmd->cmds != NULL) {
    MUCMD_FREE(mucmd, mucmd->cmds);
    mucmd->cmds_number = 0;
  }
  size_t cmds_number = 0;
  while (true) {
    if (cmds[cmds_number].handler == NULL) {
      break;
    }
    cmds_number++;
  }
  mucmd->cmds = (struct mucmd_command *)MUCMD_ALLOC(
      mucmd, cmds_number * sizeof(struct mucmd_command));
  if (mucmd->cmds == NULL) {
    res = -1;
    goto exit;
  }
  mucmd->cmds_number = cmds_number;
  memcpy(mucmd->cmds, cmds, mucmd->cmds_number * sizeof(struct mucmd_command));
exit:
  return res;
}

int mucmd_run(struct mucmd *mucmd) {
  fprintf(stdout, "mucmd interpreter.\nEnter \"info\" for commands "
                  "list.\nEnter \"exit\" to close.\n\n");
  int res = 0;
  char *line = NULL;
  while (true) {
    if (line) {
      free(line);
      line = NULL;
    }
    fprintf(stdout, "%s", MUCMD_PROMPT);
    size_t s;
    getline(&line, &s, stdin);
    if (mucmd_exit(line)) {
      goto exit;
    }
    if (mucmd_info(line)) {
      mucmd_print_info(mucmd);
      continue;
    }
    struct mucmd_parsed_command *cmd_parsed =
        mucmd_parsed_command_create(line, mucmd->allocator);
    if (cmd_parsed == NULL) {
      fprintf(stderr, "[mucmd:WARN] cmd parsing failed\n");
      continue;
    }
    struct mucmd_command *c = NULL;
    for (size_t i = 0; i < mucmd->cmds_number; i++) {
      if (strcmp(cmd_parsed->name, mucmd->cmds[i].name) == 0) {
        c = &mucmd->cmds[i];
        break;
      }
    }
    if (c == NULL) {
      fprintf(stderr, "[mucmd:WARN] cmd unknown\n");
      mucmd_parsed_command_destroy(cmd_parsed);
      continue;
    }
    c->handler(cmd_parsed->argc, cmd_parsed->argv);
    mucmd_parsed_command_destroy(cmd_parsed);
  }
exit:
  if (line) {
    free(line);
  }
  return res;
}

#endif
