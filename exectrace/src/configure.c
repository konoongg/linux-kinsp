#include "typedef_libc.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "allocator/lk_allocator.h"
#include "collections/lk_string.h"
#include "logger/lk_logger.h"

#include "configure.h"

typedef struct config {
	lk_string output_path;
	lk_string log_level;
	int target_pid;
	et_mode mode;
} config;

static config *g_config = NULL;

extern void
et_cfg_usage(FILE *out, const char *prog)
{
	fprintf(out,
		"Usage: %s [options]\n"
		"\n"
		"  -o, --out FILE       Output log file (default: %s)\n"
		"  -l, --log-file FILE  Same as -o, --out\n"
		"  -L, --log-level LVL  Log level: debug, info, warn, error, fatal\n"
		"                       (default: debug)\n"
		"  -p, --pid PID        Only trace exec calls from this PID (0 = all)\n"
		"  -h, --help           Show this help\n",
		prog, ET_DEFAULT_OUTPUT);
}

static void
config_free(config *cfg)
{
	lk_free(cfg->output_path.mem.data);
	lk_free(cfg->log_level.mem.data);
	free(cfg);
}

static lk_status
config_new(config **out)
{
	config *cfg = calloc(1, sizeof(*cfg));
	if (!cfg)
		return lk_status_create(LK_MEM_ALLOC_FAIL);

	cfg->mode = ET_MODE_NORMAL;
	cfg->output_path = lk_str_create(ET_DEFAULT_OUTPUT);
	if (cfg->output_path.mem.data == NULL) {
		free(cfg);
		return lk_status_create(LK_MEM_ALLOC_FAIL);
	}

	*out = cfg;
	return lk_status_create(LK_OK);
}

extern lk_status
et_cfg_init(int argc, char **argv)
{
	static const lk_option long_opts[] = {
		{ "out",       required_argument, NULL, 'o' },
		{ "log-file",  required_argument, NULL, 'l' },
		{ "log-level", required_argument, NULL, 'L' },
		{ "pid",       required_argument, NULL, 'p' },
		{ "help",      no_argument,       NULL, 'h' },
		{ NULL, 0, NULL, 0 },
	};

	config *cfg = NULL;
	lk_string new_path;
	lk_string new_level;
	lk_status st;
	int opt;

	assert(g_config == NULL);

	st = config_new(&cfg);
	if (!lk_status_is_ok(st))
		return st;

	while ((opt = getopt_long(argc, argv, "o:l:L:p:h", long_opts, NULL)) != -1) {
		switch (opt) {
		case 'o':
		case 'l':
			new_path = lk_str_create(optarg);
			if (new_path.mem.data == NULL) {
				config_free(cfg);
				return lk_status_create(LK_MEM_ALLOC_FAIL);
			}
			lk_free(cfg->output_path.mem.data);
			cfg->output_path = new_path;
			break;
		case 'L':
			new_level = lk_str_create(optarg);
			if (new_level.mem.data == NULL) {
				config_free(cfg);
				return lk_status_create(LK_MEM_ALLOC_FAIL);
			}
			lk_free(cfg->log_level.mem.data);
			cfg->log_level = new_level;
			break;
		case 'p':
			cfg->target_pid = atoi(optarg);
			if (cfg->target_pid <= 0) {
				lk_string msg = lk_str_create("Invalid PID: ");
				if (msg.mem.data == NULL) {
					config_free(cfg);
					return lk_status_create(LK_MEM_ALLOC_FAIL);
				}
				msg = lk_str_add_back(msg, optarg);
				config_free(cfg);
				if (msg.mem.data == NULL)
					return lk_status_create(LK_MEM_ALLOC_FAIL);
				st = lk_status_create_msg(LK_INVALID_ARGUMENT,
							   msg.mem.data);
				lk_free(msg.mem.data);
				return st;
			}
			break;
		case 'h':
			cfg->mode = ET_MODE_HELP;
			g_config = cfg;
			return lk_status_create(LK_OK);
		default:
			et_cfg_usage_err(argv[0]);
			config_free(cfg);
			return lk_status_create(LK_INVALID_ARGUMENT);
		}
	}

	if (optind < argc) {
		lk_string msg = lk_str_create("Unexpected argument: ");
		if (msg.mem.data == NULL) {
			config_free(cfg);
			return lk_status_create(LK_MEM_ALLOC_FAIL);
		}
		msg = lk_str_add_back(msg, argv[optind]);
		et_cfg_usage_err(argv[0]);
		config_free(cfg);
		if (msg.mem.data == NULL)
			return lk_status_create(LK_MEM_ALLOC_FAIL);
		st = lk_status_create_msg(LK_INVALID_ARGUMENT, msg.mem.data);
		lk_free(msg.mem.data);
		return st;
	}

	g_config = cfg;
	return lk_status_create(LK_OK);
}

extern void
et_cfg_deinit(void)
{
	if (g_config) {
		config_free(g_config);
		g_config = NULL;
	}
}

extern const char *
et_cfg_output_path(void)
{
	assert(g_config != NULL);
	return g_config->output_path.mem.data;
}

extern lk_log_level
et_cfg_log_level(void)
{
	assert(g_config != NULL);

	if (g_config->log_level.mem.data == NULL)
		return LK_LOG_LEVEL_DEBUG;

	return lk_logger_level_from_str(g_config->log_level.mem.data);
}

extern int
et_cfg_target_pid(void)
{
	assert(g_config != NULL);
	return g_config->target_pid;
}

extern et_mode
et_cfg_mode(void)
{
	assert(g_config != NULL);
	return g_config->mode;
}
