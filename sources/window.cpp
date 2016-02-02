#include "window.hpp"

/*
 * Конструктор класса Window
 * Создает окно c именем main
 */
Window::Window()
{
	namedWindow("Main");
}
/*
 * Деструктор класса Window
 * Уничтожает все окна созданные приложением
 */
Window::~Window()
{
	destroyAllWindows();
}

/*
 * Функция showimg отвечает за показ изображения @image в окне приложения,
 * а также отслеживает нажатие клавиши ESC и завершает выполнение приложения 
 * при нажатии на эту клавишу
 */
void Window::showimg(const Mat& image)
{
	imshow("Main",image);
	if((int)waitKey(1)==27) 
	{
		LOG("[I]: ESC -> exit");
		exit(0);		
	}
}
