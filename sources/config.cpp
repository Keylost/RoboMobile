#include "config.hpp"

const uint16_t nameslenght = 20;
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
	int sbx,sby,sex,sey;
	int lbx,lby,lex,ley;
	FILE *fp;
	char buf[180];	
	fp = fopen("../configs/config.conf","r");
	if(fp==NULL)
	{
		printf("[E]: Can't open config file. Default config loaded.\n");
		return;
	}
	
	int pnum =0;
	
	snprintf(names[pnum].name,180,"arduino_port");
	names[pnum].type = STRING_VAL;
	names[pnum].value = arduino_port;
	pnum++;
	
	snprintf(names[pnum].name,180,"capture_width");
	names[pnum].type = INT_VAL;
	names[pnum].value = &(capture_width);
	pnum++;
	
	snprintf(names[pnum].name,180,"capture_height");
	names[pnum].type = INT_VAL;
	names[pnum].value = &(capture_height);
	pnum++;
	
	snprintf(names[pnum].name,180,"image_quality");
	names[pnum].type = INT_VAL;
	names[pnum].value = &(image_quality);
	pnum++;
	
	snprintf(names[pnum].name,180,"robot_center");
	names[pnum].type = INT_VAL;
	names[pnum].value = &(robot_center);
	pnum++;
	
	snprintf(names[pnum].name,180,"sign_bx");
	names[pnum].type = INT_VAL;
	names[pnum].value = &(sbx);
	pnum++;
	
	snprintf(names[pnum].name,180,"sign_by");
	names[pnum].type = INT_VAL;
	names[pnum].value = &(sby);
	pnum++;
	
	snprintf(names[pnum].name,180,"sign_ey");
	names[pnum].type = INT_VAL;
	names[pnum].value = &(sey);
	pnum++;
	
	snprintf(names[pnum].name,180,"sign_ex");
	names[pnum].type = INT_VAL;
	names[pnum].value = &(sex);
	pnum++;
	
	snprintf(names[pnum].name,180,"line_bx");
	names[pnum].type = INT_VAL;
	names[pnum].value = &(lbx);
	pnum++;
	
	snprintf(names[pnum].name,180,"line_by");
	names[pnum].type = INT_VAL;
	names[pnum].value = &(lby);
	pnum++;
	
	snprintf(names[pnum].name,180,"line_ey");
	names[pnum].type = INT_VAL;
	names[pnum].value = &(ley);
	pnum++;
	
	snprintf(names[pnum].name,180,"line_ex");
	names[pnum].type = INT_VAL;
	names[pnum].value = &(lex);
	pnum++;
	
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
	
	signarea = Rect( Point( capture_width*(sbx/100.0), capture_height*(sby/100.0) ), Point( capture_width*(sex/100.0), capture_height*(sey/100.0) ) );
	linearea = Rect( Point( capture_width*(lbx/100.0), capture_height*(lby/100.0) ), Point( capture_width*(lex/100.0), capture_height*(ley/100.0) ) );
	
	fclose(fp);
}

void System::engine_get(Engine &destination)
{
	pthread_mutex_lock(&(engine_mutex));
	memcpy(&destination,&engine,sizeof(Engine));
	pthread_mutex_unlock(&(engine_mutex));
}

void System::engine_set(Engine &source)
{
	pthread_mutex_lock(&(engine_mutex));
	engine.speed = source.speed;
	engine.angle = source.angle;
	engine.direction = source.direction;
	pthread_mutex_unlock(&(engine_mutex));
}

void System::engine_set_realSpeed(uint32_t realSpeed)
{
	pthread_mutex_lock(&(engine_mutex));
	engine.real_speed = realSpeed;
	pthread_mutex_unlock(&(engine_mutex));
}

void System::signs_get(vector<sign_data> &destination)
{
	pthread_mutex_lock(&(signs_mutex));	
	destination = Signs;
	pthread_mutex_unlock(&(signs_mutex));
}

void System::signs_set(vector<sign_data> &source)
{
	pthread_mutex_lock(&(signs_mutex));
	Signs = source;
	pthread_mutex_unlock(&(signs_mutex));
}
