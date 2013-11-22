#include "clibusbwrapper.h"
#include <iostream>

using namespace std;
int  CLibusbWrapper::getStringAscii(int index, int langid, char *buf, int buflen)
{
	char    buffer[256];
	int     rval, i;

    if((rval = usb_control_msg(m_handle, USB_ENDPOINT_IN, USB_REQ_GET_DESCRIPTOR, (USB_DT_STRING << 8) + index, langid, buffer, sizeof(buffer), 1000)) < 0)
        return rval;
    if(buffer[1] != USB_DT_STRING)
        return 0;
    if((unsigned char)buffer[0] < rval)
        rval = (unsigned char)buffer[0];
    rval /= 2;
    /* lossy conversion to ISO Latin1 */
    for(i=1;i<rval;i++){
        if(i > buflen)  /* destination buffer overflow */
            break;
        buf[i-1] = buffer[2 * i];
        if(buffer[2 * i + 1] != 0)  /* outside of ISO Latin1 range */
            buf[i-1] = '?';
    }
    buf[i-1] = 0;
    return i-1;
}

int CLibusbWrapper::checkConnection()
{
	if(!m_matched)
		connect();
	if(m_handle)
		return 1;
	else
		return 0;

}

void CLibusbWrapper::checkErrors(int code)
{
	if(code < 0)
	{
		m_errors++;
		if( m_errors >= 5)
		{
			usb_close(m_handle);
            m_handle = NULL;
        }
    }
    else
    	m_errors=0;
}


CLibusbWrapper::CLibusbWrapper(int vendor, int product, string vendorName, string productName)
	:m_vendor(vendor), m_product(product), m_vendorName(vendorName), m_productName(productName)
{
	usb_init();
	connect();
}

void CLibusbWrapper::connect()
{
	usb_find_busses();
    usb_find_devices();


	m_bus= usb_get_busses();
	m_handle= NULL;

    while(m_bus && !m_handle)
    {
        m_dev= m_bus->devices;
        while(m_dev && !m_handle)
        {
            //cout << "v: " << m_dev->descriptor.idVendor << "\t p: " << m_dev->descriptor.idProduct << endl;
            if(m_dev->descriptor.idVendor == m_vendor && m_dev->descriptor.idProduct == m_product)
            {
            	m_handle = usb_open(m_dev); /* we need to open the device in order to query strings */
                if(!m_handle)
                    cerr << "Warning: cannot open USB device: " << usb_strerror() << "\n";
                else
                {
                	char    str[256];
                	int     len;
                	len = getStringAscii(m_dev->descriptor.iManufacturer, 0x0409, str, sizeof(str));
                	if(len < 0)
                	{
                		cerr <<  "Warning: cannot query manufacturer for device: " << usb_strerror() << "\n";
                	}
                	else if (m_vendorName.size() == 0 || m_vendorName == str)
                	{
                		len = getStringAscii(m_dev->descriptor.iProduct, 0x0409, str, sizeof(str));
                		if(len < 0)
                		{
                             cerr << "Warning: cannot query product for device: "<< usb_strerror() << "\n";
                        }
                        else if (m_productName.size() == 0 || m_productName == str)
                        {
                        	continue;
                        }
                    }
                 	usb_close(m_handle);
                	m_handle = NULL;
                 }

             }
             m_dev= m_dev->next;
        }
        m_bus= m_bus->next;
    }
	if(!m_handle)
	{
		cerr << "Error: Could not find USB device with:\n\tvid: " << m_vendor << "\n\tpid: " << m_product;
		if( m_vendorName.size() != 0)
			cerr << "\n\tvendor name: " << m_vendorName;
		if( m_productName.size() != 0)
			cerr << "\n\tproduct name: " << m_productName;
		cerr << "\n";
	}
	m_matched=1;
	m_errors=0;
}

/*void CLibusbWrapper::getStatus()
{
	char buffer[255];
    int i;
    int nBytes;
    nBytes = usb_control_msg(m_handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, 1, 0, 0, (char *)buffer, sizeof(buffer), 5000);
    if(nBytes < 2){
        if(nBytes < 0)
            fprintf(stderr, "USB error: %s\n", usb_strerror());
        fprintf(stderr, "only %d bytes status received\n", nBytes);
        exit(1);
    }
    for(i=0;i<8;i++){
        int isOn = buffer[0] & (1 << i);
        int isInv = buffer[1] & (1 << i);
        printf("port %d: %s%s\n", i, isOn ? "on" : "off", isInv ? (isOn ? " / pulse off" : " / pulse on") : "");
    }
}*/

int CLibusbWrapper::sendData(int command, char* buffer, int bufferSize)
{
	if(!checkConnection())
		return -1;

	int ret= usb_control_msg(m_handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT, 0x10, command, 0, buffer, bufferSize, 5000);
	if(ret < 0)
		cerr << "USB Sendefehler" << endl;
	checkErrors(ret);
	return ret;
}

int CLibusbWrapper::receiveData(int command, char* buffer, int bufferSize)
{
	if(!checkConnection())
		return -1;

	int ret= usb_control_msg(m_handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, 0x10, command, 0, buffer, bufferSize, 5000);
	if(ret < 0)
	{
		cerr << "USB Empfangsfehler" << endl;
	}
	checkErrors(ret);
	return ret;
}

