/**************************************************************************************************

	File: cterminal.h

	Abgeleitete Klasse von Fl_Input, die das Eingegebene Zeichen nicht direkt ausgibt
	sondern über einen Callback versendet.


**************************************************************************************************/


#ifndef CTERMINAL_H
#define CTERMINAL_H

#include <FL/Fl_Input.H>
typedef void (*sendData)(char data);

class CTerminal : public Fl_Input
{
	sendData m_sendFunc;
	bool m_echoOn;

	void send(char);

public:

	CTerminal(int X,int Y,int W,int H, const char *l = 0)
		: Fl_Input(X, Y, W, H, l), m_sendFunc(0), m_echoOn(0) {type(FL_MULTILINE_INPUT);}
	int handle(int);

	void getData(char data)		//Ein Zeichen auf der Oberfläche ausgeben
	{
		position(size());
		insert(&data,1);
    	readonly(0);
	}
	int echoOn() {return m_echoOn; }
	void echoOn(bool f) { m_echoOn= f; }
	void setSendFunc(sendData func) { m_sendFunc= func; }
};

#endif
