#include "main.hpp"

/*
 * Сорость в реальных единицах полученная с arduino
 */
uint16_t real_speed =0;

/*
 * struct Queue. Очередь, куда помещаются изображения полученные с видеокамеры.
 */
pthread_mutex_t queue_lock = PTHREAD_MUTEX_INITIALIZER;
struct Queue
{
	Mat image;
	unsigned int img_id=0;
}queue;

/*
 * struct Queue_engine. Очередь из одного элемента, куда помещаюся структуры Engine, хранящие
 * параметры движения робота.
 * Поле Engine engine содержит структуру сгенерированную автоматически потоками распознования
 * Поле Engine manual содержит структуру заполняемую пользователем. 
 */
pthread_mutex_t queue_engine_lock = PTHREAD_MUTEX_INITIALIZER;
struct Queue_engine
{
	Engine engine;
	Engine manual;
	unsigned int id=0;
}queue_engine;

/*
 * Функция main_thr() представляет собой основной поток приложения.
 * @System &sys. Файл содержащий конфигурацию системы, в соответствии с которой она будет работать.
 */
void main_thr(System &sys)
{
	/*Создает поток распознования изображений*/
	pthread_t recognize_thr;
	pthread_create(&recognize_thr, NULL, recognize_fnc, &sys);
	
	/*Создает поток взаимодействия с клиентским приложением*/
	pthread_t server_thr;
	pthread_create(&server_thr, NULL, server_fnc, &sys);
	
	/*Создает поток записи видео с камеры в видеофайл*/
	pthread_t videomaker_thr;
	pthread_create(&videomaker_thr, NULL, videomaker_fnc, &sys);
	
	/*Создает поток взаимодействия с микроконтроллером*/
	pthread_t arduino_thr;
	pthread_create(&arduino_thr, NULL, arduino_fnc, &sys);
	
	/*Создает поток захвата изображений с видеокамеры*/
	pthread_t capture_thr;
	pthread_create(&capture_thr, NULL, capture_fnc, &sys);
	
	/*Не завершать программу. Ожидать завершения потоков.*/
	pthread_join(capture_thr,NULL);
	pthread_join(recognize_thr,NULL);
	pthread_join(server_thr,NULL);
	pthread_join(arduino_thr,NULL);
	pthread_join(videomaker_thr,NULL);
}

/*
 * Функция videomaker_fnc() реализует поток записи видео с камеры робота в файл.
 * @ptr - указатель на структуру System.
 * Начинает запись видео, если поле videomaker объекта ptr истина
 */
void* videomaker_fnc(void *ptr)
{
	System *syst = (System *)ptr;
	unsigned int frame_id=0;
	if(!syst->videomaker)
	{
		return NULL;
	}
	else
	{
		LOG("[I]: VideoMaker started.");
	}
	
	Mat frame;
	VideoMaker clip;
	if(!clip.init(*syst))
	{
		LOG("[E]: Videomaker: Can't open output stream."); //chmod or codecs problem???
		exit(11);
	}
	
	while(1)
    {
		if (frame_id!=queue.img_id)
		{
			pthread_mutex_lock(&(queue_lock));
			frame_id = queue.img_id;      	       	
			queue.image.copyTo(frame);
			pthread_mutex_unlock(&(queue_lock));
			
			clip.write(frame);			
		}
		else usleep(1000);
	}
	clip.deinit();	
}

/*
 * Функция server_fnc() реализует поток взаимодействия с клиентским приложением.
 * @ptr - указатель на структуру System.
 * Устанавливает соединение с клиентским приложением и начинает процесс передачи телеметрии и приема управляющих команд.
 */
void* server_fnc(void *ptr)
{
	Mat frame;
	Engine eng;
	unsigned int frame_id=0;
	System *syst = (System *)ptr;
	Server srv(*syst); //need correct start check 
	
	/* Запускает поток приема управляющих команд */
	srv.receiver(&queue_engine.manual);
	Telemetry tl;
	
	/*Основной цикл потока*/
	while(1)
	{
		if (frame_id!=queue.img_id)
		{
			pthread_mutex_lock(&(queue_lock));   
			frame_id = queue.img_id;    	       	
			queue.image.copyTo(frame);
			pthread_mutex_unlock(&(queue_lock));
			
			pthread_mutex_lock(&(queue_engine_lock));
			memcpy(&eng,&queue_engine.engine,sizeof(Engine));
			pthread_mutex_unlock(&(queue_engine_lock));
			
			tl.speed = eng.speed;
			tl.direction = eng.direction;
			tl.angle = eng.angle;
			tl.real_speed = real_speed;
			memcpy(&tl.mysign,&eng.mysign,sizeof(sign_data));
			memcpy(&tl.myline,&eng.myline,sizeof(line_data));
			
			srv.send(frame,&tl);			
		}
		else usleep(100);
	}
	
	return NULL;
}

/*
 * Функция arduino_fnc() реализует поток взаимодействия с микроконтроллером.
 * @ptr - указатель на структуру System.
 * Устанавливает соединение с микроконтроллером и начинает процесс передачи параметров движения.
 */
void* arduino_fnc(void *ptr)
{
	System *syst = (System *)ptr;
	unsigned int engine_id=10002;
	Engine eng;
	Arduino_ctrl controller(*syst);
	if(!controller.isconnected())
	{
		LOG("[W]: Arduino isn't attached. The robot willn't move.");
		return NULL;
	}
	
	while(1) //
	{    
		if (engine_id != queue_engine.id)
		{
			pthread_mutex_lock(&(queue_engine_lock));
			memcpy(&eng,&queue_engine.engine,sizeof(Engine));
			engine_id = queue_engine.id;
			pthread_mutex_unlock(&(queue_engine_lock));
			
			controller.send_command(&eng);
			int spd=controller.feedback();
			if(spd!=-1) real_speed = spd;
		}
		else usleep(1000);
	}
	//controller.deinit();
	return NULL;
}

/*
 * Функция capture_fnc() реализует поток захвата изображения с видеокамеры робота.
 * @ptr - указатель на структуру System.
 * Захватывает изображения с видеокамеры робота и заполняет очередь.
 * Может использовать видеофайл в качестве источника видеоданных вместо видеокамеры.
 */
void* capture_fnc(void *ptr)
{
	System *syst = (System *)ptr;
	switch (syst->MODE)
	{
		case CAMERA:
					capture_from_cam_fnc(*syst);
					break;
		case VIDEO:
					capture_from_vid_fnc(*syst);
					break;		
		default:
					capture_from_cam_fnc(*syst);
					break;		
	}
	return NULL;
}

/*
 * Функция capture_from_cam_fnc() реализует захват изображения с видеокамеры.
 * См. capture_fnc()
 */
void capture_from_cam_fnc(System &syst)
{
	VideoCapture cap(0); // open the default camera
    if(!cap.isOpened())  // check if we succeeded
    {
        LOG("[E:] Cant't open camera.");
    	exit(1);
    }
    cap.set(CV_CAP_PROP_FRAME_WIDTH,syst.capture_width);
    cap.set(CV_CAP_PROP_FRAME_HEIGHT,syst.capture_height);
    cap.set(CV_CAP_PROP_FPS,30);
    
    sleep(1); //waiting for init all threads
	
	while(1) // Capture frame from camera
    {    
        if (cap.grab())
        {
			pthread_mutex_lock(&(queue_lock));       	       	
			cap.retrieve(queue.image, 0); //0 - bgr
			queue.img_id++;
			pthread_mutex_unlock(&(queue_lock));				
		}
		else 
		{
			usleep(1000);
		}
	}
	return;
}

/*
 * Функция capture_from_vid_fnc реализует захват изображения из видеофайла.
 * См. capture_fnc()
 */
void capture_from_vid_fnc(System &syst)
{
	char filename[80];
	printf("Enter video file name: ");
	scanf("%s",filename);
	
	VideoCapture cap(filename); // open the videofile
    if(!cap.isOpened())  // check if we succeeded
    {
        LOG("[E:] Cant't open video file.");
    	exit(1);
    }
	
	while(1) // Capture frame from videofile
    {    
        if (cap.grab())
        {
			pthread_mutex_lock(&(queue_lock));
			queue.img_id++;
			cap.retrieve(queue.image, 0); //0 - bgr
			pthread_mutex_unlock(&(queue_lock));
			usleep(33000); //33 ms				
		}
		else 
		{			
			LOG("[I]: End of file.");
			exit(0);
		}
	}
	return;
}


/*
 * Функция recognize_fnc() реализует поток распознования объектов на изображении.
 * @ptr - указатель на структуру System.
 * Распознает интересующие объекты на изображении и вычисляет параметры движения робота в соответствии с полученными данными.
 */
void* recognize_fnc(void *ptr)
{
	System *syst = (System *)ptr;
	unsigned int frame_id=0;
	Mat frame;
	Recognition recognize(*syst);
	
	robotimer r_timer;
	
	while(1)
	{    
		if (frame_id!=queue.img_id)
		{       	       		
			r_timer.start();
			pthread_mutex_lock(&(queue_lock));			
			queue.image.copyTo(frame);
			frame_id = queue.img_id;
			pthread_mutex_unlock(&(queue_lock));
			
			recognize.recognize_line(frame);
			if(recognize.mysign.sign == sign_none) recognize.recognize_sign(frame);
			else recognize.handle_sign(frame);
			recognize.handle_line();
			
			if(!queue_engine.manual.power) recognize.engine.speed = 0;
			
			//memcpy(&engine.myline, &recognize.myline, sizeof(line_data));
			//memcpy(&engine.mysign, &recognize.mysign, sizeof(sign_data));
			
			pthread_mutex_lock(&(queue_engine_lock));
			memcpy(&queue_engine.engine,&recognize.engine,sizeof(Engine));
			queue_engine.id++;
			pthread_mutex_unlock(&(queue_engine_lock));
			
			r_timer.stop();
			printf("runtime: %ld ms\n",r_timer.get());			
		}
		else
		{
			usleep(300); //0.3ms
		}
	}
	return NULL;	
}
