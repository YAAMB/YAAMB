# data file for the Fltk User Interface Designer (fluid)
version 1.0108 
header_name {.h} 
code_name {.cxx}
decl {\#include <iostream>} {public
} 

decl {\#include <sstream>} {public
} 

decl {\#include <iomanip>} {public
} 

decl {\#include "clibusbwrapper.h"} {public
} 

decl {\#include "cterminal.h"} {public
} 

decl {\#include <FL/Fl_PNG_Image.H>} {public
} 

decl {\#include <FL/Fl_Int_Input.H>} {public
} 

decl {\#include "cfl_streamoutput.h"} {public
} 

decl {Fl_PNG_Image* g_activeImage;} {} 

decl {Fl_PNG_Image* g_inactiveImage;} {} 

decl {CLibusbWrapper* board;} {} 

Function {make_window()} {open
} {
  Fl_Window {} {
    label {AVR-USB Terminal V0.1} open
    xywh {470 55 445 395} type Double box DOWN_BOX selection_color 48 resizable hotspot size_range {300 250 0 0} visible
  } {
    Fl_Input term {
      xywh {10 33 425 235} when 0 resizable
      class CTerminal
    }
    Fl_Menu_Bar bar {open
      xywh {15 5 420 25}
    } {
      MenuItem {} {
        callback {if(board->connected())
   board->disConnect();
else
   board->pubConnect();}
        xywh {0 0 100 20} value 1
        code0 {g_activeImage = new Fl_PNG_Image("usb_mount.png");}
        code1 {g_inactiveImage = new Fl_PNG_Image("usb_unmount.png");}
        code2 {Fl_Menu_Item* a = &menu_bar[0];}
        code3 {a->image(g_inactiveImage);}
      }
      MenuItem {} {
        label Echo
        callback {Fl_Menu_Item* a = &menu_bar[1];
term->echoOn(a->value());}
        xywh {5 5 100 20}
        code0 {o->flags |= FL_MENU_TOGGLE;}
      }
      MenuItem {} {
        label Configuration
        callback {std::stringstream stream;
stream << "0x" << hex << board->getVendorID(); 
vid->value((stream.str()).c_str());

stream.str("");
stream << "0x" << hex << board->getProductID();
pid->value((stream.str()).c_str());

iMan->value(board->getVendorName().c_str());
iPro->value(board->getProductName().c_str());




configWin->show();}
        xywh {15 15 100 20}
      }
      MenuItem {} {
        label Clear
        callback {outDisplay->value("");
term->value("");}
        xywh {0 0 100 20}
      }
    }
    Fl_Input outDisplay {
      xywh {10 275 425 110}
      code0 {std::cout.rdbuf(o);}
      code1 {std::cerr.rdbuf(o);}
      class CFl_StreamOutput
    }
  }
} 

Function {make_configWin()} {open
} {
  Fl_Window configWin {
    label {USB Configuration} open
    xywh {495 186 275 130} type Double hide
  } {
    Fl_Input vid {
      label idVendor
      xywh {105 10 80 25}
      class Fl_Int_Input
    }
    Fl_Input pid {
      label idProduct
      xywh {105 40 80 25}
      class Fl_Int_Input
    }
    Fl_Input iMan {
      label iManufacturer
      xywh {105 70 165 25}
    }
    Fl_Input iPro {
      label iProduct
      xywh {105 100 165 25}
    }
    Fl_Button {} {
      label OK
      callback {std::stringstream stream;
int dummy;
if(vid->index(1) == 'x')
{
   stream << (vid->value()+2);
   stream >> hex >> dummy;
}
else
{   
   stream << vid->value();
   stream >> dummy;
}
board->setVendorID(dummy); 
std::stringstream stream2;
dummy=0;
if(pid->index(1) == 'x')
{
   stream2 << (pid->value()+2);
   stream2 >> hex >> dummy;
}
else
{   
   stream2 << pid->value();
   stream2 >> dummy;
}   
board->setProductID(dummy); 

board->setVendorName(iMan->value());
board->setProductName(iPro->value());

configWin->hide();}
      xywh {200 10 65 25}
    }
    Fl_Button {} {
      label Cancel
      callback {configWin->hide();}
      xywh {200 40 65 25}
    }
  }
} 

Function {sendUSB(char data)} {open
} {
  code {board->sendDataT(0, data);} {}
} 

Function {} {open
} {
  code {board= new CLibusbWrapper (0x16C0, 0x05DC, "www.obdev.at", "MK_Board");
make_configWin();
make_window()->show(argc, argv);
term->setSendFunc(sendUSB);
int merker=0;			//Flankenmerker für den Verbindungsstatus 
while (Fl::check())
{
    char temp;
    //Daten pollen und auf dem Terminal ausgeben
    if(board->receiveData(0, &temp, 1) > 0 && temp)
    	term->getData(temp);
    
    //Abbildung auf dem ersten Menuepunkt dem Verbindungsstatus anpassen
    Fl_Menu_Item* a = &menu_bar[0];    
    if(board->connected() && !merker)
    {
   	merker =1;
   	a->set();
   	a->image(g_activeImage);
    	bar->redraw();
    }
    else if(!board->connected() && merker) 
    {
       merker = 0;
       a->image(g_inactiveImage);
       bar->redraw();
    }	
    
}} {selected
  }
} 
