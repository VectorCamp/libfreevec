/***************************************************************************
 *   Copyright (C) 2005-2007 by CODEX                                      *
 *   Konstantinos Margaritis <markos@codex.gr>                             *
 *                                                                         *
 *   This code is distributed under the GPLv3 license                       *
 *   See http://www.gnu.org/licenses/gpl-3.0.html                           *
 ***************************************************************************/

#include "config.h"

void handle_cmdlineargs(struct bench_conf *conf, int argc, char *argv[]);
void run_threaded_test(void *(*bench_func)(), struct bench_conf *conf);
void show_conf_details(struct bench_conf *conf);
void db_repeat_until_done(struct bench_conf *conf, char *cmd);
