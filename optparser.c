#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <argp.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "optparser.h"

error_t client_parser(int key, char *arg, struct argp_state *state) {
	struct client_arguments *args = state->input;
	error_t ret = 0;
	switch(key) {
	case 'a':
		strncpy(args->ip_address, arg, 16);
		struct sockaddr_in addr;
		int result = inet_pton(AF_INET, args->ip_address, &addr.sin_addr.s_addr);
		if (result < 1) {
			argp_error(state, "Invalid address");
		}
		break;
	case 'p':
		args->port = atoi(arg);
		// Validate port is an int and falls within the valid range
		if (args->port == 0 || args->port < 1 || args->port > 65535) {
			argp_error(state, "Invalid option for a port, must be a number");
		}
		break;
	case 'n':
		args->num_req = atoi(arg);
		if (args->num_req < 0) {
			argp_error(state, "Invalid number of requests");
		}
		break;
	case 't':
		args->timeout = atoi(arg);
		if (args->timeout < 0) {
			argp_error(state, "Invalid timeout");
		}
		break;
	default:
		ret = ARGP_ERR_UNKNOWN;
		break;
	}
	return ret;
}

struct client_arguments client_parseopt(int argc, char *argv[]) {
	struct argp_option options[] = {
		{ "addr", 'a', "addr", 0, "The IP address the server is listening at", 0},
		{ "port", 'p', "port", 0, "The port that is being used at the server", 0},
		{ "numReq", 'n', "numReq", 0, "The number of timerequests to send to the server", 0},
		{ "timeout", 't', "timeout", 0, "The time in seconds that the client will wait to receive a timeresponse", 0},
		{0}
	};

	struct argp argp_settings = { options, client_parser, 0, 0, 0, 0, 0 };

	struct client_arguments args;
	bzero(&args, sizeof(args));

	if (argp_parse(&argp_settings, argc, argv, 0, NULL, &args) != 0) {
		printf("Got error in parse\n");
	}

	/* If they don't pass in all required settings, you should detect
	 * this and return a non-zero value from main */
	printf("Got %s on port %d with n=%d timeout=%d\n",
	       args.ip_address, args.port, args.num_req, args.timeout);
 
    return(args);
}

error_t server_parser(int key, char *arg, struct argp_state *state) {
	struct server_arguments *args = state->input;
	error_t ret = 0;
	switch(key) {
	case 'p':
		args->port = atoi(arg);
        // Validate port is an int and falls within the valid range
		if (args->port == 0 || args->port < 0 || args->port > 65535) {
			argp_error(state, "Invalid option for a port, must be a number");
		}
		break;
	case 'd':
        args->percent = atoi(arg);
		// Validate percent is an int and falls within the valid range
		if (args->percent < 0 || args->percent > 100) {
			argp_error(state, "Percent must be in range (0,100) inclusive");
		}
		break;
	default:
		ret = ARGP_ERR_UNKNOWN;
		break;
	}
	return ret;
}

struct server_arguments server_parseopt(int argc, char *argv[]) {
	struct server_arguments args;

	/* bzero ensures that "default" parameters are all zeroed out */
	bzero(&args, sizeof(args));

	struct argp_option options[] = {
		{ "port", 'p', "port", 0, "The port to be used for the server" ,0},
		{ "percent", 'd', "percent", 0, "The percent chance the server drops any given UDP payload", 0},
		{0}
	};
	struct argp argp_settings = { options, server_parser, 0, 0, 0, 0, 0 };
	if (argp_parse(&argp_settings, argc, argv, 0, NULL, &args) != 0) {
		printf("Got an error condition when parsing\n");
	}

	// Print args for sanity
	printf("Got port %d and percent %d\n", args.port, args.percent);

    return(args);
	// free(args.salt); When do I free? Leave to OS?
}
