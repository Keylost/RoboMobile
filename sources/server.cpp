#include "server.hpp"
#include "OpenCVCompatibylityDefs.hpp"

/*
 * Функция send_data() отвечает за отправку данных из @src размера @size байт с использованием сокета @socket
 */
bool send_data(void *src,int socket,size_t size)
{
	int bytes = 0;
	size_t i = 0;
	for (i = 0; i < size; i += bytes) 
	{
		if ((bytes = send(socket, ((uint8_t *)src)+i, size-i, MSG_NOSIGNAL)) <= 0) 
		{
			LOG("Sending data error");
			return false;
		}
	}
	return true;
}

/*
 * Функция get_data() отвечает за прием данных из @dst размера @size байт с использованием сокета @socket
 */
bool get_data(void *dst, int socket, size_t size)
{
	size_t i=0;
	int bytes =0;
	for (i = 0; i < size; i += bytes)
	{
		if ((bytes = recv(socket, (char *)dst+i, size-i, 0)) == -1)
		{
			return false;
		}
	}
	return true;
}

/*
 * Функция receiv() реализует поток приема данных с клиентского приложения 
 * @ptr - указатель на объект класса Server
 */
void *receiv(void *ptr)
{
	Server *pt  = (Server *)ptr;
	LOG("receiver started");
	while(true)
	{
		if(pt->isconnected)
		{
			//if(get_data(&(pt->engine->power),pt->newsockfd,1))
			{
				LOG("[I]: Receiver got data");
			}
		}
		else usleep(300000); //300 ms
	}
	return NULL;
}


/*
 * Конструктор класса Server
 * Отвечает за инициализацию объекта класса Server
 * @syst - структура содержащая параметры конфигурации приложения
 */
Server::Server(System &syst)
{
	sys = &syst;
	isconnected=false;
	portno = 1111; //server port number
	start();
}

/*
 * Включить опцию @optName для сокета @sockfd
 */
void sockOptEnable(int sockfd, int optName)
{
	int32_t optval = 1;
	size_t optlen = sizeof(optval);
	if(setsockopt(sockfd, SOL_SOCKET, optName, (char *)&optval, optlen) < 0)
	{
		perror("sockOptEnable()");
		close(sockfd);
		exit(EXIT_FAILURE);
	}
}

/*
 * Функция start() устанавливает соединение с клиентским приложением. 
 */
void Server::start()
{
	//open sock and wait for client
	if(isconnected) return;
	LOG("Server started");
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;    
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		LOG("ERROR opening socket");
	}
	
	sockOptEnable(sockfd,SO_KEEPALIVE);
	sockOptEnable(sockfd,SO_REUSEADDR);

    bzero((char *) &serv_addr, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	
	while(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))<0)
    {
		LOG("ERROR binding socket");
		sleep(1); //wait 1 sec befor next try
	}

	listen(sockfd,5);
	clilen = sizeof(cli_addr);
	LOG("waiting for client");
	newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	LOG("client connected");
	isconnected=true;
	
	uint16_t width = (uint16_t)sys->capture_width;
	uint16_t height = (uint16_t)sys->capture_height;
	send_data(&width, newsockfd, 2);
	send_data(&height, newsockfd, 2);
		 
	send_data(&sys->signarea,newsockfd,sizeof(Rect));
	send_data(&sys->linearea,newsockfd,sizeof(Rect));
	return;
}

/*
 * Функция send() отправляет данные на клиентское приложение.
 * @type - тип отправляемых данных. 
 * @dataSize - размер отправляемых данных.
 * @ptr - указатель на начало отправляемых данных.
 */
void Server::send(dataType type, uint32_t dataSize, void *ptr)
{
	bool status = true;
	uint32_t tp = type;
	if(!send_data((void *)&tp,newsockfd,sizeof(uint32_t)))
	{
		status = false;
	}

	if(!send_data((void *)&dataSize,newsockfd,sizeof(uint32_t)))
	{
		status = false;
	}
	if(!send_data(ptr,newsockfd,(size_t)dataSize))
	{
		status = false;
	}
	
	
	if(status)
	{
		return;
	}
	else
	{
		LOG("[I]: Client disconnected");
		isconnected = false;
		stop();
		start();
		return;
	}
}

/*
 * Функция stop() отвечает за разрыв соединения с клиентским приложением. 
 */
void Server::stop()
{
	close(newsockfd);
	close(sockfd);
}

/*
 * Функция receiver() отвечает за создание потока приема данных от клиентского приложения
 */
void Server::receiver(Engine *eng)
{
	engine = eng;
	pthread_t receiver_thr;
	pthread_create(&receiver_thr, NULL, receiv, this);
	pthread_detach(receiver_thr);	
}


/*
 * Функция server_fnc() реализует поток взаимодействия с клиентским приложением.
 * @ptr - указатель на структуру System.
 * Устанавливает соединение с клиентским приложением и начинает процесс передачи телеметрии и приема управляющих команд.
 */
void* server_fnc(void *ptr)
{
	Engine eng; //локальная копия структуры Engine
	System &syst = *((System *)ptr);
	vector<int> parameters; //вектор параметров сжатия изображения(jpeg 20%)
	vector<uchar> buffer; //вектор содержащий сжатое изображение
	parameters = vector<int>(2);
	parameters[0] = CV_IMWRITE_JPEG_QUALITY; //jpeg
	parameters[1] = syst.image_quality; //0-100 quality
	
	vector<sign_data> Signs;
	
	Object<Mat> *curObj = NULL;
	Queue<Mat> &queue = syst.queue;
	
	Object<line_data> *curLineData = NULL;
	Queue<line_data> &qline = syst.qline;
	
	Server srv(syst);
	
	/* Запускает поток приема управляющих команд */
	//srv.receiver(&queue_engine.manual);
	
	/*Основной цикл потока*/
	while(1)
	{
			curObj = queue.waitForNewObject(curObj);
			curLineData = qline.waitForNewObject(curLineData);
			Mat &frame = *(curObj->obj);
			imencode(".jpg", frame, buffer, parameters);
			syst.engine_get(eng);
			
			srv.send(Image_t,(uint32_t)buffer.size(),(void *)(&buffer[0]));
			srv.send(Line_t,sizeof(line_data),(void *)(curLineData->obj));
			srv.send(Engine_t,sizeof(Engine),(void *)(&eng));
			
			
			syst.signs_get(Signs);
			for(unsigned i=0;i<Signs.size();i++)
			{
				srv.send(Sing_t,sizeof(sign_data),(void *)(&Signs[i]));
			}
			
			curObj->free();
			curLineData->free();
	}
	
	return NULL;
}
