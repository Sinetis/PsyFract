#include "stdafx.h"
#include "Menu.h"

MenuItem::MenuItem()
{}

MenuItem::~MenuItem()
{}

void MenuItem::Select()
{
	if (select != NULL)
		return;
	select();
}

void MenuItem::SetSelectFunction(void (*func)())
{
	select = func;
}
