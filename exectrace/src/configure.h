#ifndef ET_CONFIGURE_H
#define ET_CONFIGURE_H

#include <stdio.h>

#include "logger/lk_logger.h"
#include "markers.h"
#include "object/lk_status.h"

#define ET_DEFAULT_OUTPUT LK_LOG_DEFAULT_FILE

typedef enum {
	ET_MODE_NORMAL,
	ET_MODE_HELP,
} et_mode;

extern void et_cfg_usage(FILE *out, const char *prog);

#define et_cfg_usage_err(prog)  et_cfg_usage(stderr, (prog))
#define et_cfg_usage_help(prog) et_cfg_usage(stdout, (prog))

SINGLTON extern lk_status et_cfg_init(int argc, char **argv);

SINGLTON extern void et_cfg_deinit(void);

SINGLTON extern const char *et_cfg_output_path(void);

SINGLTON extern lk_log_level et_cfg_log_level(void);

SINGLTON extern int et_cfg_target_pid(void);

SINGLTON extern et_mode et_cfg_mode(void);

#endif
