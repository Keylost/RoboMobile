#pragma once
#include <stdio.h>
#include <getopt.h> //command line parsing
#include <stdint.h>
#include "config.hpp"
#include <stdlib.h>


class CLP
{
	public:
	static void usage(const char *progname);
	static void parse(int argc, char **argv,System &syst);
};
