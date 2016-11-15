#ifndef _COMMAND_H_
#define _COMMAND_H_

typedef struct {
	unsigned int num_cmds;
	char** cmds;
}Commands_t;

bool parse_user_input (const char* input, Commands_t** cmd);
void destroy_commands(Commands_t** cmd);

#endif
