#include "cfl_streamoutput.h"
#include <cstdio>

void CFl_StreamOutput::writeToOuput()
{
	if (pbase() != pptr())
	{
		position(size());
		insert(pbase(),pptr() - pbase());
		setp(pbase(), epptr());
	}
}

std::streambuf::int_type CFl_StreamOutput::sync()
{
	writeToOuput();
	return 0;
}

std::streambuf::int_type CFl_StreamOutput::overflow(int_type c)
{
	writeToOuput();
	if (c != EOF)
	{
	  sputc(c);
	}
	return 0;
}

CFl_StreamOutput::CFl_StreamOutput (int X,int Y,int W,int H, const char *l)
	: Fl_Input(X, Y, W, H, l)
{
	type(FL_MULTILINE_INPUT | FL_INPUT_READONLY);

 	m_buffer= new char[BUFFERSIZE];
 	setp(m_buffer, m_buffer + BUFFERSIZE);
 	setg(0, 0, 0);
}


