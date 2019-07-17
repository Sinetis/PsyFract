#ifndef _MENU_H_
#define _MENU_H_

#include "stdafx.h"



class MenuItem
{
	char* caption;
	void (*select)(void);
	
public:
	MenuItem();
	~MenuItem();

	void Select(); // ����� �������� ����
	void SetSelectFunction(void (*func)(void));

private:

};

class Menu: MenuItem
{
	std::list<MenuItem*> menuItems; 

public:
	Menu();
	~Menu();

	void Back(); // ��������� � ����������� ���������
	void Exit(); // ����� �� ���������
};

#endif