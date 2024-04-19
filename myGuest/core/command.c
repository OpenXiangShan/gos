#include "command.h"
#include "string.h"
#include "print.h"
#include "mm.h"

static struct commands _commands;

static void command_setup(void)
{
	extern unsigned long COMMAND_INIT_TABLE, COMMAND_INIT_TABLE_END;
	int nr =
	    (struct command_entry *)&COMMAND_INIT_TABLE_END -
	    (struct command_entry *)&COMMAND_INIT_TABLE;
	struct command_entry *entry;
	struct command_entry *head =
	    (struct command_entry *)&COMMAND_INIT_TABLE;

	for (entry = head; nr; entry++, nr--)
		entry->init();
}

int command_init(void)
{
	struct command_info *p_commands = (struct command_info *)mm_alloc(4096);
	if (!p_commands)
		return -1;

	memset((char *)p_commands, 0, 4096);
	_commands.p_commands = p_commands;
	_commands.total = 4096;
	_commands.avail = 0;

	command_setup();

	return 0;
}

int register_command(const struct command *command)
{
	struct command_info *cmd, *new;
	int remain = _commands.total;
	int new_size;

retry:
	cmd = _commands.p_commands;
	remain = _commands.total;
	while (remain > sizeof(struct command_info)) {
		if (!cmd->in_used)
			goto found;
		cmd++;
		remain -= sizeof(struct command_info);
	}

	new_size = _commands.total + 4096;
	new = mm_alloc(new_size);
	if (!new)
		return -1;
	memset((char *)new, 0, new_size);
	memcpy((char *)new, (char *)_commands.p_commands, _commands.total);
	mm_free(_commands.p_commands, _commands.total);
	_commands.p_commands = new;
	_commands.total = new_size;

	goto retry;

found:
	cmd->in_used = 1;
	cmd->command = command;
	_commands.avail++;

	return 0;
}

void walk_and_print_command()
{
	struct command_info *entry;
	int nr = _commands.avail;

	for_each_command(entry, _commands.p_commands, nr) {
		if (!entry->in_used)
			continue;
		print("%s\n", entry->command->cmd);
	}
}

int do_command(struct run_params *params)
{
	int nr = _commands.avail, argc = 0;
	struct command_info *entry;
	char *argv[16];
	char *command;

	command = params->command;
	argc = params->argc;

	for (int i = 0; i < argc; i++)
		argv[i] = params->argv[i];

	for_each_command(entry, _commands.p_commands, nr) {
		if (!entry->in_used)
			continue;

		if (!strncmp(entry->command->cmd, command, 64)) {
			if (entry->command->handler) {
				entry->command->handler(argc, argv,
							entry->command->priv);
			}
		}
	}

	return 0;
}
