#pragma once
//Created on 28/06/2021
#include <windows.h>
#include "ftd2xx.h"
#include "stdafx.h"
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <map>
#include<ctime>

enum PinMap
{
	D0 = 0,
	D1 = 1,
	LE_Att = 2,//Latch enable of Attenuator
	D1_Att=3,
	D2_Att=4,
	D3_Att=5,
	D4_Att=6,
	D5_Att=7
};

enum LogicState
{
	LOW=0,
	HIGH=1,
	UNKNOWN=2
};

class ControlInterface
{
public:
	ControlInterface();

	~ControlInterface();

	void CreateDeviceInfoList(DWORD &numOfDev);

	void listDevices();

	FT_STATUS getDeviceInfo(DWORD devIndex, FT_DEVICE_LIST_INFO_NODE &deviceInfo);

	//Open the device and return a handle which will be used for subsequent accesses. By default this method tries to open first device of the list.
	FT_STATUS openDevice(int index = 0);


	//Open the specified device and return a handle that will be used for subsequent accesses.
	//The device can be specified by its serial number
	FT_STATUS openDeviceBySerial(char* serialNumber);


	//Gets the number of bytes in the receive queue.
	DWORD getQueueStatus();

	//it is used to set Baud Rate while communicating device over UART protocol.
	//This function sets the baud rate for the device. 
	FT_STATUS setBaudRate(ULONG BaudRate);

	//Enables different chip modes.It is set to MPSSE mode by default
	FT_STATUS setBitMode();

	FT_STATUS setDataCharacteristic(UCHAR WordLength, UCHAR StopBits, UCHAR Parity);

	//This function sets the read and write timeouts for the device,if do not be set,timeout is infinity
	FT_STATUS setTimeout(ULONG readTimeOut,ULONG writeTimeOut);

	FT_STATUS digitalWrite(PinMap Pin, LogicState state);

	LogicState digitalRead(PinMap Pin);

	FT_STATUS setFlowControl(USHORT FlowControl);

	//This function sets the Request To Send (RTS) control signal.
	FT_STATUS setRTS();

	//This function clears the Request To Send(RTS) control signal.
	FT_STATUS setClrRTS();

	//Retrieves the COM port associated with a device.
	LONG getComPortNumber();

	FT_STATUS resetDevice();

	//This function purges receive and transmit buffers in the device.
	FT_STATUS purgeBuffers(DWORD dwMask);

	//Sends a reset command to the port
	FT_STATUS resetPort();

	//reset D0-D7 pins
	FT_STATUS resetPort_D();

	//This function can be of use when trying to recover devices programatically.
	void rescanDevices();

	//Close an open device.
	void closeDevice();

	bool isOpen();

private:
	FT_HANDLE ftHandle;
	FT_STATUS ftStatus;
	DWORD numDevs;
	ULONG BaudRate;
	BYTE byOutputBuffer[8];
	BYTE byInputBuffer[8];
	DWORD dwNumBytesToSend;
	DWORD dwNumBytesToRead;
	DWORD dwNumBytesRead;
	bool is_open;

	//Write data to the device.
	FT_STATUS write(DWORD size);
};

