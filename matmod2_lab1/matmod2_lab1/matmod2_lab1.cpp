#include "Application.h"
#include "Application_2.h"
#include "Application_3.h"
#include <iostream>
#include <fstream>

int main()
{
	int task = 0;
	std::cout << "Task#";
	std::cin >> task;
	if (task == 1)
	{
		Application app(32,40,60);
	}
	else if (task == 2)
	{
		Application_2 app(256, 3, 30);
	}
	else if (task == 3)
	{
		Application_3 app(100, 8, 30);
		std::vector<unsigned> data;
		app.run(data);
	}
	else if (task == 4)
	{
		Application_3 app(100, 8, 30);
		app.m_smart = true;
		std::vector<unsigned> data;
		app.run(data);
	}
	//app.run();
	/*std::fstream fout("out_2.txt",'w');
	for (int i = 0; i < data.size(); i++)
	{
		fout << data[i] << " ";
	}
	fout.close();*/
}