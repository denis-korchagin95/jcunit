#include <string.h>

#include "headers/options.h"
#include "headers/errors.h"
#include "headers/application.h"


void parse_options(int argc, char * argv[], struct application_context * application_context)
{
    int i;
    char * arg;
    for(i = 1; i < argc; ++i) {
        arg = argv[i];
        if (strncmp("--version", arg, sizeof("--version") - 1) == 0) {
            application_context->options |= OPTION_SHOW_VERSION;
            continue;
        }
        if (strncmp("--run-mode=", arg, sizeof("--run-mode=") - 1) == 0) {
            const char * run_mode = arg + sizeof("--run-mode=") - 1;
            if (strcmp(run_mode, "detail") == 0) {
                application_context->run_mode = RUN_MODE_DETAIL;
                continue;
            }
            if (strcmp(run_mode, "passthrough") == 0) {
                application_context->run_mode = RUN_MODE_PASSTHROUGH;
                continue;
            }
            jcunit_fatal_error("The unknown run mode '%s'!", run_mode);
        }
        if (strncmp("--help", arg, sizeof("--help") - 1) == 0) {
            application_context->options |= OPTION_SHOW_HELP;
            continue;
        }
        if (strncmp("--", arg, 2) == 0) {
            jcunit_fatal_error("The unknown option: %s!", arg);
        }
    }
}
