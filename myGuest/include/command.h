#ifndef __USER_COMMAND_H
#define __USER_COMMAND_H

#define COMMAND_INIT_TABLE __command_init_table
#define COMMAND_INIT_TABLE_END __command_init_table_end

struct run_params {
	char command[64];
	int argc;
	char argv[16][64];
	int busy;
	int ready;
	int vmid;
	int cpu;
};

struct command {
	char cmd[64];
	int (*handler)(int argc, char *argv[], void *priv);
	void *priv;
};

struct command_info {
	int in_used;
	const struct command *command;
};

struct commands {
	struct command_info *p_commands;
	int total;
	int avail;
};

struct command_entry {
	int (*init)(void);
};

#define for_each_command(entry, commands, n)                                \
	for (entry = (struct command_info *)commands; n > 0; entry++, n--)

#define APP_COMMAND_REGISTER(name, init_fn)                           \
	static const struct command_entry __attribute__((used))   \
		__command_entry_##name                                \
		__attribute__((section(".command_init_table"))) = {   \
			.init = init_fn,                              \
		}

void walk_and_print_command(void);
int register_command(const struct command *command);
int command_init(void);
int do_command(struct run_params *params);

#endif
