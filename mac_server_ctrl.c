//------------------------------------------------------------------------------
/**
 * @file mac_server_ctrl.c
 * @author charles-park (charles.park@hardkernel.com)
 * @brief ODROID Mac Request server control application.
 * @version 0.1
 * @date 2022-12-09
 *
 * @package apt install python3 python3-pip
 *          python3 -m pip install aiohttp asyncio
 * 
 * @copyright Copyright (c) 2022
 *
 */
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <getopt.h>

#include "mac_server_ctrl.h"

//------------------------------------------------------------------------------
#define	MAC_SERVER_CTRL_PYTHON_FILE_NAME	"/root/m1-server/mac_server_ctrl/mac_server_ctrl.py"
#define	CMD_LINE_CHARS	128

//------------------------------------------------------------------------------
int set_erase_mac (char *rboard, char *rmac, char ctrl_server)
{
	FILE *fp;
	char cmd_line[CMD_LINE_CHARS];

	// python send command setup
	memset (cmd_line, 0, sizeof(cmd_line));
	sprintf(cmd_line, "python3 %s -e %s %s %s\n",
			MAC_SERVER_CTRL_PYTHON_FILE_NAME, rboard, rmac, ctrl_server ? "-f" : " ");

	if (NULL != (fp = popen(cmd_line, "r"))) {
		// read buffer init
		memset(cmd_line, 0, sizeof(cmd_line));
		while (fgets(cmd_line, 4096, fp)) {
			if (strstr(cmd_line, "success") != NULL) {
				pclose(fp);
				return 1;
			}
		}
	}
	pclose(fp);
	return 0;
}

//------------------------------------------------------------------------------
int get_mac_uuid (char *rboard, char rtype, char *rdata, char ctrl_server)
{
	FILE *fp;
	char cmd_line[CMD_LINE_CHARS];

	// python send command setup
	memset (cmd_line, 0, sizeof(cmd_line));
	sprintf(cmd_line, "python3 %s -r %s %s\n",
			MAC_SERVER_CTRL_PYTHON_FILE_NAME, rboard, ctrl_server ? "-f" : " ");

	if (NULL != (fp = popen(cmd_line, "r"))) {
		// read buffer init
		memset(cmd_line, 0, sizeof(cmd_line));
		while (fgets(cmd_line, 4096, fp)) {
			if (strstr(cmd_line, "success") != NULL) {
				char *p = strstr(cmd_line, "001e06");
				/* rtype == 1 is uuid */
				strncpy(rdata, rtype ? (p + 13) : (p),
						rtype ? MAC_SERVER_CTRL_TYPE_UUID_SIZE : MAC_SERVER_CTRL_TYPE_MAC_SIZE);
				pclose(fp);
				return 1;
			}
		}
	}
	pclose(fp);
	return 0;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#if defined(__DEBUG_APP__)
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static void print_usage(const char *prog)
{
	printf("Usage: %s [-bre]\n", prog);
	puts("  -b --board_name     request board name.\n"
	     "  -r --request        request from server. (mac or uuid)\n"
	     "  -e --erase          request erase mac.\n"
		 "  -f --factory        *** warning *** : factory server ctrl\n"
		 "  e.g) request mac from dev-server (mac address dosen't change) \n"
		 "       mac_server_test -b n2l -r mac\n"
		 "  e.g) request mac from factory-server (mac address change) \n"
		 "       mac_server_test -b n2l -r mac -f\n"
		 "  e.g) request uuid from dev-server (uuid dosen't change) \n"
		 "       mac_server_test -b n2l -r uuid\n"
		 "  e.g) request uuid from dev-server (uuid change) \n"
		 "       mac_server_test -b n2l -r uuid -f\n"
		 "  e.g) request erase mac from dev-server \n"
		 "       mac_server_test -b n2l -e 001e06123456\n"
		 "  e.g) request erase mac from factory-server \n"
		 "       mac_server_test -b n2l -e 001e06123456 -f\n"
	);
	exit(1);
}

//------------------------------------------------------------------------------
/* Warning */
/* 0 = dev-server control, 1 = factory-server control */
static char OPT_ACCESS_SERVER = MAC_SERVER_CTRL_DEV_SERVER;

//------------------------------------------------------------------------------
static char *OPT_REQUEST_BOARD = "None";
static char *OPT_ERASE_MAC = "000000000000";
static char OPT_REQUEST_MAC = 0;

static void parse_opts (int argc, char *argv[])
{
	while (1) {
		static const struct option lopts[] = {
			{ "board_name", 1, 0, 'b' },
			{ "request",	1, 0, 'r' },
			{ "erase",		1, 0, 'e' },
			{ "factory",	0, 0, 'f' },
			{ NULL, 0, 0, 0 },
		};
		int c;

		c = getopt_long(argc, argv, "b:r:e:f", lopts, NULL);

		if (c == -1)
			break;

		switch (c) {
		case 'b':
			OPT_REQUEST_BOARD = optarg;
			break;
		case 'r':
            if (!strncmp("mac", optarg, strlen("mac")))
				OPT_REQUEST_MAC = 1;
			else
				OPT_REQUEST_MAC = 0;	// read uuid
			break;
		case 'e':
			OPT_ERASE_MAC = optarg;
			break;
		case 'f':
			OPT_ACCESS_SERVER = MAC_SERVER_CTRL_FACTORY_SERVER;
			break;
		default:
			print_usage(argv[0]);
			break;
		}
	}
}

//------------------------------------------------------------------------------
int main(int argc, char **argv)
{
    parse_opts(argc, argv);

	if (!strncmp(OPT_REQUEST_BOARD, "None", strlen("None"))) {
		printf ("Request Board name is None.\n");
		return 0;
	}
	if (strstr(OPT_ERASE_MAC, "001e06") != NULL) {
		printf ("board name = %s, erase mac = %s, ret = %d\n",
			OPT_REQUEST_BOARD,
			OPT_ERASE_MAC,
			set_erase_mac (OPT_REQUEST_BOARD, OPT_ERASE_MAC, OPT_ACCESS_SERVER));
	} else {
		char rdata[MAC_SERVER_CTRL_TYPE_UUID_SIZE +1];
		int ret;

		memset(rdata, 0, sizeof(rdata));
		ret = get_mac_uuid (OPT_REQUEST_BOARD,
							OPT_REQUEST_MAC ? MAC_SERVER_CTRL_TYPE_MAC : MAC_SERVER_CTRL_TYPE_UUID,
							rdata,
							OPT_ACCESS_SERVER);
		if (strstr(rdata, "001e06") != NULL)
			printf ("board name = %s, request %s = %s, server = %s, ret = %d\n",
				OPT_REQUEST_BOARD,
				OPT_REQUEST_MAC ? "mac" : "uuid",
				rdata,
				OPT_ACCESS_SERVER ? "factory" : "dev",
				ret);
		else
			printf ("board name = %s, request %s = %s, ret = %d\n",
				OPT_REQUEST_BOARD,
				OPT_REQUEST_MAC ? "mac" : "uuid",
				"Request data error",
				ret);
	}
	return 0;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#endif	// #if defined(__DEBUG_APP__)
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
