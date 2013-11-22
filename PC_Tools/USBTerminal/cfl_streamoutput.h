/**************************************************************************************************

	File: cfl_streamoutput.h

	Klasse zum Umleiten eines Strems auf die FLTK-Oberfläche.
	z.B: cout umleinten: std::cout.rdbuf(instanznamen);


**************************************************************************************************/
#ifndef CFl_STREAMOUTPUT_H
#define CFl_STREAMOUTPUT_H

#include <FL/Fl_Input.H>
#include <streambuf>

class CFl_StreamOutput : public Fl_Input, public std::streambuf
{
	enum{BUFFERSIZE=64 };
	char* m_buffer;
	void writeToOuput();
protected:
	int_type overflow(int_type);
	int_type sync();

public:
	CFl_StreamOutput (int X,int Y,int W,int H, const char *l = 0);

	~CFl_StreamOutput()
	{
		delete[] m_buffer;
	}
};

#endif
