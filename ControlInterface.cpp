#include "stdafx.h"
#include "ControlInterface.h"

ControlInterface::ControlInterface()
{
	dwNumBytesToSend = 0;;
	numDevs = 0;
	BaudRate = 0;
	dwNumBytesRead = 0;
	is_open = false;
}


ControlInterface::~ControlInterface()
{
	if (is_open)
	{
		this->closeDevice();
	}
}


FT_STATUS ControlInterface::openDevice(int index)
{
	//Index of the device to open. Indices are 0 based.
	ftStatus = FT_Open(index, &ftHandle);
	if (ftStatus == FT_OK)
	{
		// FT_Open OK, use ftHandle to access device
		printf("\n\n\tConnection to the chip opened successfully\n");
		is_open = true;
		return ftStatus;
	}
	else
	{
		// FT_Open failed
		printf("\terror in opening connection,Chip not connected or loose cable\n");
		return ftStatus;
	}

}


FT_STATUS ControlInterface::openDeviceBySerial(char* serialNumber)
{
	ftStatus = FT_OpenEx(serialNumber, FT_OPEN_BY_SERIAL_NUMBER, &ftHandle);
	if (ftStatus == FT_OK) 
	{
		// success - device with serial number  is open
		is_open = true;
		return ftStatus;
	}
	else 
	{
		// failure
		return ftStatus;
	}
}


FT_STATUS ControlInterface::setBaudRate(ULONG BaudRate)
{
	ftStatus = FT_SetBaudRate(ftHandle, BaudRate); //Setting up the BaudRate
	if (ftStatus == FT_OK)
	{
		// Baud Rate OK
		printf("2) - Baud Rate set at %d bps\n", BaudRate);
		this->BaudRate = BaudRate;
		return ftStatus;
	}
	else
	{
		// Baud Rate failed
		printf("E2) - Error in setting BaudRate \n");
		return ftStatus;
	}
}


FT_STATUS ControlInterface::setFlowControl(USHORT FlowControl)
{
	ftStatus=FT_SetFlowControl(ftHandle, FlowControl, NULL, NULL);
	if (ftStatus == FT_OK)
	{
		printf("4) - Flow Control = None \n");
		return ftStatus;
	}
	else
	{
		printf("E4) - Error in setting Flow Control \n");
		return ftStatus;
	}
}

FT_STATUS ControlInterface::setDataCharacteristic(UCHAR WordLength,UCHAR StopBits,UCHAR Parity)
{
	ftStatus = FT_SetDataCharacteristics(ftHandle, WordLength, StopBits, Parity);
	if (ftStatus == FT_OK)
	{
		printf("3) - Format-> 8 DataBits,No Parity,1 Stop Bit (8N1)\n");
		return ftStatus;
	}
	else
	{
		printf("E3) - Error in setting Data Format \n");
		return ftStatus;
	}	
}


FT_STATUS ControlInterface::setRTS()
{
	ftStatus = FT_SetRts(ftHandle);
	if (ftStatus == FT_OK)
	{
		// FT_SetRts OK
		return ftStatus;
	}
	else 
	{
		// FT_SetRts failed
		return ftStatus;
	}
}

FT_STATUS ControlInterface::setClrRTS()
{
	ftStatus = FT_ClrRts(ftHandle);
	if (ftStatus == FT_OK)
	{
		// FT_ClrRts OK
		return ftStatus;
	}
	else 
	{
		// FT_ClrRts failed
		return ftStatus;
	}
}


FT_STATUS ControlInterface::setTimeout(ULONG readTimeOut, ULONG writeTimeOut)
{
	// Set read timeout of readTimeOut, write timeout of writeTimeOut
	ftStatus = FT_SetTimeouts(ftHandle, readTimeOut, writeTimeOut);
	if (ftStatus == FT_OK)
	{
		// FT_SetTimeouts OK
		return ftStatus;
	}
	else
	{
		// FT_SetTimeouts failed
		return ftStatus;
	}
}

LogicState ControlInterface::digitalRead(PinMap Pin)
{
	this->purgeBuffers(FT_PURGE_TX);
	dwNumBytesToSend = 0;
	byOutputBuffer[dwNumBytesToSend++] = 0x81;// Get data bits - returns state of pins,  either input or output on low byte of MPSSE
	this->write(dwNumBytesToSend);
	dwNumBytesToSend = 0; // Reset output buffer pointer
	Sleep(10); // Wait for data to be transmitted and status  to be returned by the device driver - see latency timer above

			   // Check the receive buffer - there should be one byte
	ftStatus = FT_GetQueueStatus(ftHandle, &dwNumBytesToRead);// Get the number of bytes in the  FT2232H receive buffer
	clock_t begin = clock();
	clock_t end = clock();
	while (!(dwNumBytesToRead > 0))
	{
		FT_GetQueueStatus(ftHandle, &dwNumBytesToRead);
		if (dwNumBytesToRead > 0)
		{
			break;
		}
		else
		{
			end = clock();
			if (double(end - begin) / CLOCKS_PER_SEC > 1000)
			{
				return UNKNOWN;
			}
		}
	}

	ftStatus |= FT_Read(ftHandle, &byInputBuffer, dwNumBytesToRead, &dwNumBytesRead);
	if ((ftStatus != FT_OK) & (dwNumBytesRead != 1))
	{
		printf("Error - GPIO cannot be read\n");
		return UNKNOWN;
	}
	return (LogicState)((byInputBuffer[0] & (0x01 << Pin)) >> Pin);
}


FT_STATUS ControlInterface::digitalWrite(PinMap Pin, LogicState state)
{
	/*
	prepare the message and send it 
	*/
	this->purgeBuffers(FT_PURGE_TX);
	dwNumBytesToSend = 0;
	byOutputBuffer[dwNumBytesToSend++] = 0x81;// Get data bits - returns state of pins,  either input or output on low byte of MPSSE
	this->write(dwNumBytesToSend);
	dwNumBytesToSend = 0; // Reset output buffer pointer
	Sleep(10); // Wait for data to be transmitted and status  to be returned by the device driver - see latency timer above

	// Check the receive buffer - there should be one byte
	ftStatus = FT_GetQueueStatus(ftHandle, &dwNumBytesToRead);// Get the number of bytes in the  FT2232H receive buffer
	clock_t begin = clock();
	clock_t end = clock();
	while (!(dwNumBytesToRead > 0))
	{
		FT_GetQueueStatus(ftHandle, &dwNumBytesToRead);
		if (dwNumBytesToRead > 0)
		{
			break;
		}
		else
		{
			end = clock();
			if (double(end - begin) / CLOCKS_PER_SEC > 1000)
			{
				return FT_FAILED_TO_WRITE_DEVICE;
			}
		}
	}
	
	ftStatus |= FT_Read(ftHandle, &byInputBuffer, dwNumBytesToRead, &dwNumBytesRead);
	if ((ftStatus != FT_OK) & (dwNumBytesRead != 1))
	{
		printf("Error - GPIO cannot be read\n");
		return FT_FAILED_TO_WRITE_DEVICE;
	}
	printf("The GPIO low-byte = 0x%X\n", byInputBuffer[0]);//8 bits
	//Read From GPIO byte and change specific bit to manipulate GPIO 
	byOutputBuffer[dwNumBytesToSend++] = 0x80;//opcode to 
	// Set/reset data bits low-byte of MPSSE port
	if (state == LOW)
	{
		byOutputBuffer[dwNumBytesToSend++] = byInputBuffer[0] & (~(0x01 << Pin));//Clearing a bit
	}
	else
	{
		byOutputBuffer[dwNumBytesToSend++] = byInputBuffer[0] | 0x01<< Pin;//Setting a bit
	}
	byOutputBuffer[dwNumBytesToSend++] = 0xFF;	// Direction config is still needed for each GPIO write
	ftStatus=this->write(dwNumBytesToSend);
	dwNumBytesToSend = 0;
	Sleep(10);
	return ftStatus;
	
}

FT_STATUS ControlInterface::write(DWORD size)
{
	DWORD NumOfBytesWritten = 0;
	ftStatus = FT_Write(ftHandle,              // Handle to the chip
		                &byOutputBuffer,       // Variable Containing the byte to be Txed
		                size,                  // sizeof(byOutputBuffer) = char
		                &NumOfBytesWritten     // No of Bytes written to the port
	);

	if ((ftStatus == FT_OK)&&(NumOfBytesWritten==size))
	{
		// Error checking for FT_Write()
		printf("\t'%c' written to the serial port at %d bps\n", byOutputBuffer[1], BaudRate);
		return ftStatus;
	}

	else
	{
		printf("\tError in writing to port\n");
		return ftStatus;
	}
}


FT_STATUS ControlInterface::setBitMode()
{
	ftStatus |= FT_SetBitMode(ftHandle, 0x0, 0x00);
	//Reset controller
	ftStatus |= FT_SetBitMode(ftHandle, 0x0, 0x02);
	//Enable MPSSE mode
	//Enable the MPSSE controller. Pin directions are set later through the MPSSE commands.
	return ftStatus;
}

FT_STATUS ControlInterface::resetPort_D()
{
	//reset  D0-D7 pins
	dwNumBytesToSend = 0;
	byOutputBuffer[dwNumBytesToSend++] = 0x80;
	byOutputBuffer[dwNumBytesToSend++] = 0X00;
	byOutputBuffer[dwNumBytesToSend++] = 0xFF;
	ftStatus=this->write(dwNumBytesToSend);
	dwNumBytesToSend = 0;
	return ftStatus;
}
LONG ControlInterface::getComPortNumber()
{
	LONG lComPortNumber;
	ftStatus = FT_GetComPortNumber(ftHandle, &lComPortNumber);
	if (ftStatus == FT_OK)
	{
		if (lComPortNumber == -1)
		{
			// No COM port assigned 
			return lComPortNumber;
		}
		else 
		{
			// COM port assigned with number held in lComPortNumber
			return lComPortNumber;	
		}
	}
	else
	{
		// FT_GetComPortNumber FAILED!
		return -2;
	}
}

FT_STATUS ControlInterface::resetDevice()
{
	ftStatus = FT_ResetPort(ftHandle);
	if (ftStatus == FT_OK) 
	{
		// Port has been reset
		return ftStatus;
	}
	else 
	{
		// FT_ResetPort FAILED!
		return ftStatus;
	}
}


DWORD ControlInterface::getQueueStatus() 
{
	DWORD RxBytes;
	FT_GetQueueStatus(ftHandle, &RxBytes);
	return RxBytes;
}

FT_STATUS ControlInterface::purgeBuffers(DWORD dwMask)
{
	ftStatus = FT_Purge(ftHandle, dwMask); // Purge RX,TX or both of them
	if (ftStatus == FT_OK) 
	{
		// FT_Purge OK
		return ftStatus;
	}
	else 
	{
		// FT_Purge failed
		return ftStatus;
	}
}

FT_STATUS ControlInterface::resetPort()
{
	ftStatus = FT_ResetPort(ftHandle);
	return ftStatus;
}

void ControlInterface::rescanDevices()
{
	ftStatus = FT_Rescan();
	if (ftStatus != FT_OK)
	{
		// FT_Rescan OK
	}
	else
	{
		// FT_Rescan failed!
	}
}

void ControlInterface::closeDevice()
{
	//reset D0-D7 pins
	this->resetPort_D();
	FT_SetBitMode(ftHandle, 0x0, 0x00);// Reset MPSSE
	FT_Close(ftHandle); //Close the handle to the chip
	is_open = false;
}


void ControlInterface::listDevices()
{
	numDevs = 0;
	CreateDeviceInfoList(numDevs);
	if (numDevs > 0) {
		FT_DEVICE_LIST_INFO_NODE *devInfo;
		// allocate storage for list based on numDevs
		devInfo =(FT_DEVICE_LIST_INFO_NODE*)malloc(sizeof(FT_DEVICE_LIST_INFO_NODE)*numDevs);
		// get the device information list
		ftStatus = FT_GetDeviceInfoList(devInfo, &numDevs);
		if (ftStatus == FT_OK) {
			for (int i = 0; i < numDevs; i++) 
			{
				printf("Dev %d:\n", i);
				printf(" Flags=0x%x\n", devInfo[i].Flags);
				printf(" Type=0x%x\n", devInfo[i].Type);
				printf(" ID=0x%x\n", devInfo[i].ID);
				printf(" LocId=0x%x\n", devInfo[i].LocId);
				printf(" SerialNumber=%s\n", devInfo[i].SerialNumber);
				printf(" Description=%s\n", devInfo[i].Description);
				printf(" ftHandle=0x%x\n", devInfo[i].ftHandle);
			}
		}
	}
}

FT_STATUS ControlInterface::getDeviceInfo(DWORD devIndex, FT_DEVICE_LIST_INFO_NODE &deviceInfo)
{
	numDevs = 0;
	CreateDeviceInfoList(numDevs);
	if (numDevs > 0) 
	{
		// get information for device 0
		ftStatus = FT_GetDeviceInfoDetail(devIndex, &deviceInfo.Flags, &deviceInfo.Type, &deviceInfo.ID, &deviceInfo.LocId, &deviceInfo.SerialNumber,
			&deviceInfo.Description, &deviceInfo.ftHandle);
		if (ftStatus == FT_OK)
		{
			return ftStatus;
		}
		else
		{
			return ftStatus;
		}
	}
}


void ControlInterface::CreateDeviceInfoList(DWORD &numOfDev)
{
	// create the device information list
	ftStatus = FT_CreateDeviceInfoList(&numDevs);
	if (ftStatus == FT_OK)
	{
		numOfDev = numDevs;
		printf("%d FTDI devices found \- the count includes individual ports on a single chip\n", numOfDev);
	}
	else 
	{
		// FT_CreateDeviceInfoList failed
	}
}



bool ControlInterface::isOpen()
{
	return is_open;
}