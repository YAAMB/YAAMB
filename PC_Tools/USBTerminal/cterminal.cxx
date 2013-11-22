#include "cterminal.h"
#include <FL/Fl.H>
#include <iostream>
void CTerminal::send(char data)
{
	m_sendFunc(data);
	position(size());
    readonly(0);
}

int CTerminal::handle(int event)
{
    int key= Fl::event_key();
    if (event == FL_KEYBOARD)
    {
        int del;
        if (Fl::compose(del))
        {
            if (del || Fl::event_length())
            {
            	send(Fl::event_text()[0]);
            }
            if(m_echoOn) Fl_Input::handle(event);
            return 1;
        }
        else if(key == FL_Enter || key == FL_KP_Enter)
        {
	       	send('\n');
	       	if(m_echoOn) return Fl_Input::handle(event);
	       	return 1;
        }

    }
    readonly(1);
    int temp= Fl_Input::handle(event);
    readonly(position() != size());
    return temp;
}

