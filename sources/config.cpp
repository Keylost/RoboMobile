#include "config.hpp"

const uint16_t nameslenght = 10;
config names[nameslenght];
uint16_t nnum = 0;

/*
 * Функция ifcomment() проверяет является ли строка содержащаяся в buf комментарием
 */
bool ifcomment(char *buf)
{
	if(buf[0]=='#') return true;
	else return false;
}

/*
 * Функция parsename() проверяет содержится ли в buf имя параметра. Если да, то возвращает
 * "истина" и записывает в переменную nnum номер параметра в массиве имен.
 */
bool parsename(char *buf)
{
	uint16_t i = 0;
	while(i<nameslenght && strcmp(buf,names[i].name)!=0)
	{
		i++;
	}
	if(i==nameslenght)
	{		
		return false;
	}
	else
	{
		nnum =i;
		return true;
	}
}

/*
 * Функция parsesign() проверяет является ли символ содержащийся в buf знаком присваивания
 */
bool parsesign(char *buf)
{
	if(buf[0]=='=')
	{
		return true;
	}
	else
	{
		return false;
	}
}

/*
 * Функция parsevalue() считывает значения параметра конфигурации из файла конфигурации, описанного в структуре fp
 */
bool parsevalue(FILE *fp)
{
	switch(names[nnum].type)
	{
		case INT_VAL:
		{
			fscanf(fp,"%d",(int *)names[nnum].value);
			break;
		}
		case STRING_VAL:
		{
			fscanf(fp,"%s",(char *)names[nnum].value);
			break;
		}
	}
	return true;
}

/*
 * Функция goto_newline() отвечает за переход парсера на новую строку конфигурационного файла
 */
void goto_newline(FILE *fp)
{
	char c='0';
	while (c!='\n') 
	{
		fscanf(fp,"%c",&c);
	}
}

/*
 * Функция init() отвечает за инициализацию структуры System и парсинг конфигурационного файла.
 */
void System::init()
{
	
	FILE *fp;
	char buf[180];	
	fp = fopen("../configs/config.conf","r");
	if(fp==NULL)
	{
		printf("[E]: Can't open config file. Default config loaded.\n");
		return;
	}
	
	snprintf(names[0].name,180,"arduino_port");
	names[0].type = STRING_VAL;
	names[0].value = arduino_port;
	
	snprintf(names[1].name,180,"capture_width");
	names[1].type = INT_VAL;
	names[1].value = &(capture_width);
	
	snprintf(names[2].name,180,"capture_height");
	names[2].type = INT_VAL;
	names[2].value = &(capture_height);
	
	snprintf(names[3].name,180,"image_quality");
	names[3].type = INT_VAL;
	names[3].value = &(image_quality);
	
	snprintf(names[4].name,180,"robot_center");
	names[4].type = INT_VAL;
	names[4].value = &(robot_center);
	
	
	while(fscanf(fp,"%s",buf)!=EOF)
	{
		if(ifcomment(buf))
		{
			goto_newline(fp);
			continue;
		}
		if(!parsename(buf))
		{
			printf("[W] Config file parsing error: unknown parameter\n");
			break;
		}
		fscanf(fp,"%s",buf);
		if(!parsesign(buf))
		{
			printf("[W] Config file parsing error: unexpected symbol\n");
			break;
		}		
		if(!parsevalue(fp))
		{
			printf("[W] Config file parsing error: unexpected value\n");
			break;
		}
		
		goto_newline(fp);
	}
	fclose(fp);
}
