/**************************************************************************************************

	File: clibusbwraper.h

	Wrapperklasse für das senden und empfangen von Daten über USB.


**************************************************************************************************/
#ifndef CLIBUSBWRAPPER_H
#define CLIBUSBWRAPPER_H

#include <usb.h>
#include <string>
using namespace std;

class CLibusbWrapper
{
	struct usb_bus* m_bus;
	struct usb_device* m_dev;
	usb_dev_handle* m_handle;
	int m_vendor;
	int m_product;
	string m_vendorName;
	string m_productName;
	bool m_matched;
	int m_errors;

	int  getStringAscii(int index, int langid, char *buf, int buflen);
	int checkConnection();
	void checkErrors(int code);
	void connect();

public:
	CLibusbWrapper(int vendor, int product, string vendorName= string(), string productName= string());
	//void getStatus();
	int sendData(int command, char* buffer, int bufferSize);
	int receiveData(int command, char* buffer, int bufferSize);

	template<class T>
	int sendDataT(int command, T value)
	{
		return sendData(command, (char*)&value, sizeof (T));
	}
	int receiveData(int command, short& value)
	{
		return receiveData(command, (char*)&value, sizeof (short));
	}

	int getVendorID()	{ return m_vendor; }
	int getProductID()	{ return m_product; }
	const string& getVendorName()	{ return m_vendorName; }
	const string& getProductName()	{ return m_productName; }

	void setVendorID(int v)	{ m_vendor= v; m_matched= 0; }
	void setProductID(int p){ m_product= p; m_matched= 0; }
	void setVendorName(const string& vn)	{ m_vendorName= vn; m_matched= 0;}
	void setProductName(const string& pn)	{ m_productName= pn; m_matched= 0;}


	void pubConnect()	{ if(!m_handle) connect(); }
	void disConnect()	{ if(m_handle) { usb_close(m_handle); m_handle = NULL; } }
	int connected()		{ return m_handle ? 1 : 0; }
};

#endif
