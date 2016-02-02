#include "CLP.hpp"

/*
 * Функция usage()
 * Выводит справку по параметрам командной строки программы RoboMobile
 */
void CLP::usage(const char *progname)
{
	printf(
		"Usage: %s [OPTIONS]\n"
		"Options:\n"
		"\t-v - capture video in file. Example: %s -v myvideo1.avi \n"
		"\t-m - input mode: CAMERA, VIDEO. Example: %s -m VIDEO \n"
		"\t-h - display this help and exit\n",
		progname,progname,progname);
}

/*
 * Функция parse()
 * Производит разбор параметров командной строки программы RoboMobile
 * и заполняет структуру syst в соответствии с полученными данными
 */
void CLP::parse(int argc, char **argv,System &syst)
{
	int32_t opt;
	int32_t option_index = 0;
	static struct option long_options[] = 
	{
		{"help",   no_argument,       0, 'h'},
		{"video", required_argument, 0, 'v'},
		{"mode", required_argument, 0, 'm'},
		{0, 0, 0, 0}
	};
	while((opt = getopt_long(argc, argv, "hv:m:", long_options, &option_index)) != -1) 
	{
		switch (opt) 
		{
		case 'h':
			usage(argv[0]);
			exit(0);
		case 'v':
			strcpy(syst.videoname,optarg);
			syst.videomaker = true;
			break;
		case 'm':
			if(strcmp(optarg,"CAMERA")==0) syst.MODE = CAMERA;
			else if(strcmp(optarg,"VIDEO")==0) syst.MODE = VIDEO;
			else 
			{
				printf("[W]: Incorrect mode. CAMERA will be used.\n");
			}
			break;
		default:
			usage(argv[0]);			
			exit(1);
		}
	}
}
