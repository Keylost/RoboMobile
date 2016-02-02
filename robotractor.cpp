#include "CLP.hpp" //commandline parser
#include "config.hpp" //gets system configuration from conf file
#include "main.hpp"

/*
 * RoboMobile - это многопоточное приложение предназначеннное для управления робототехническим средством
 * на основании данных полученных с датчиков
 * Функция main() настраивает систему в зависимости от параметров, хранящихся в файле конфигурации и переданных
 * в командной стрроке и затем вызывает функцию, запускающую систему
 */
int main(int argc, char **argv)
{
	System syst;	
	syst.init(); //Загрузить конфигурацию из файла config.conf
	
	CLP::parse(argc,argv,syst); //get command line parameters
	
	main_thr(syst);
    
    return 0; //this make us happy:)
}
