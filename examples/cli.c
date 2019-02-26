/*
 * Copyright 2019 Iguazio.io Systems Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License") with
 * an addition restriction as set forth herein. You may not use this
 * file except in compliance with the License. You may obtain a copy of
 * the License at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 * In addition, you may not use the software for any purposes that are
 * illegal under applicable law, and the grant of the foregoing license
 * under the Apache 2.0 license is conditioned upon your compliance with
 * such restriction.
 */

#include "icli.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct my_context {
    int something;
};

static enum icli_ret cli_list_jobs(char *argv[], int argc, void *context)
{
    icli_printf("Jobs: %d\n", 123);

    return ICLI_OK;
}

static int cli_show_containers(void)
{
    icli_printf("Container: %d\n", 1);
    icli_printf("Container: %d\n", 2);
    icli_printf("Container: %d\n", 3);
    icli_printf("Container: %d\n", 4);
    return 0;
}

static int cli_show_services(void)
{
    icli_printf("Service: %d\n", 1);
    icli_printf("Service: %d\n", 2);
    return 0;
}

static enum icli_ret cli_containers_list(char *argv[], int argc, void *context)
{
    struct my_context *self = context;

    self->something = 1;

    icli_printf("args:\n\n");
    for (int i = 0; i < argc; ++i)
        icli_printf("arg %d: %s\n", i, argv[i]);

    int ret = cli_show_containers();
    if (ret) {
        icli_err_printf("Error in cli_show_containers:%d\n", ret);
        return ICLI_ERR;
    }

    return ICLI_OK;
}

static enum icli_ret cli_interface(char *argv[], int argc, void *context)
{
    icli_printf("Set interface %s\n", argv[0]);

    return ICLI_OK;
}

static enum icli_ret cli_show(char *argv[], int argc, void *context)
{
    struct my_context *self = context;
    int ret;

    self->something = 2;

    if (strcmp(argv[0], "containers") == 0) {
        ret = cli_show_containers();
        if (ret) {
            icli_err_printf("Error in cli_show_containers:%d\n", ret);
            return ICLI_ERR;
        }
    } else if (strcmp(argv[0], "services") == 0) {
        ret = cli_show_services();
        if (ret) {
            icli_err_printf("Error in cli_show_services:%d\n", ret);
            return ICLI_ERR;
        }
    }
    return ICLI_OK;
}

int main(int argc, char *argv[])
{
    int res;
    int ret = EXIT_SUCCESS;
    struct my_context context = {};

    struct icli_params params = {.user_data = &context,
                                 .history_size = 10,
                                 .app_name = "example_cli",
                                 .prompt = "my_cli"};

    res = icli_init(&params);
    if (res) {
        fprintf(stderr, "Unable to init icli\n");
        return EXIT_FAILURE;
    }

    struct icli_command *containers, *services, *jobs, *interface;
    struct icli_command_params param = {.name = "containers", .help = "Containers"};

    struct icli_arg_val show_first_arg[] = {{.val = "containers"}, {.val = "services"}, {.val = NULL}};
    struct icli_arg_val *show_args[] = {show_first_arg};

    res = icli_register_command(&param, &containers);
    if (res) {
        fprintf(stderr, "Unable to register command: %s\n", param.name);
        ret = EXIT_FAILURE;
        goto out;
    }

    memset(&param, 0, sizeof(param));
    param.parent = containers;
    param.help = "List containers";
    param.name = "list";
    param.command = cli_containers_list;

    res = icli_register_command(&param, NULL);
    if (res) {
        fprintf(stderr, "Unable to register command: %s\n", param.name);
        ret = EXIT_FAILURE;
        goto out;
    }

    memset(&param, 0, sizeof(param));
    param.help = "Print info";
    param.name = "show";
    param.command = cli_show;
    param.argc = 1;
    param.argv = show_args;

    res = icli_register_command(&param, NULL);
    if (res) {
        fprintf(stderr, "Unable to register command: %s\n", param.name);
        ret = EXIT_FAILURE;
        goto out;
    }

    memset(&param, 0, sizeof(param));
    param.help = "Set interface";
    param.name = "interface";
    param.short_name = "intf";
    param.command = cli_interface;
    param.argc = 1;

    res = icli_register_command(&param, &interface);
    if (res) {
        fprintf(stderr, "Unable to register command: %s\n", param.name);
        ret = EXIT_FAILURE;
        goto out;
    }

    memset(&param, 0, sizeof(param));
    param.parent = interface;
    param.help = "IPs";
    param.name = "ip";

    res = icli_register_command(&param, NULL);
    if (res) {
        fprintf(stderr, "Unable to register command: %s\n", param.name);
        ret = EXIT_FAILURE;
        goto out;
    }

    memset(&param, 0, sizeof(param));
    param.help = "Services";
    param.name = "services";
    param.short_name = "svc";

    res = icli_register_command(&param, &services);
    if (res) {
        fprintf(stderr, "Unable to register command: %s\n", param.name);
        ret = EXIT_FAILURE;
        goto out;
    }

    memset(&param, 0, sizeof(param));
    param.parent = services;
    param.help = "Jobs";
    param.name = "jobs";

    res = icli_register_command(&param, &jobs);
    if (res) {
        fprintf(stderr, "Unable to register command: %s\n", param.name);
        ret = EXIT_FAILURE;
        goto out;
    }

    memset(&param, 0, sizeof(param));
    param.parent = jobs;
    param.help = "List jobs";
    param.name = "list";
    param.command = cli_list_jobs;

    res = icli_register_command(&param, NULL);
    if (res) {
        fprintf(stderr, "Unable to register command: %s\n", param.name);
        ret = EXIT_FAILURE;
        goto out;
    }

    icli_commands_to_dot("cli.dot");

    icli_run();

out:
    icli_cleanup();

    return ret;
}
