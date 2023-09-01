#ifndef REMOTE_BT_H
#define REMOTE_BT_H

#include "libssh/libssh.h"
#include "types.h"

ssh_session start_ssh_connection();
int run_remote_command(ssh_session remote_session, char *command, u8_array *out_stdout_data);

#endif

