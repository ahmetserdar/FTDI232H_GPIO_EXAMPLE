/*
LED BLINKING DEMO BY USING ADAFRUIT FT232H
STEPS:
	*Confirm device existence and open handle
	*Configure FTDI Port For MPSSE Use
	*Configure the FTDI MPSSE	*Serial Communications	*Close handle
*/
#include "stdafx.h"
#include "ControlInterface.h"
int main()
{
	ControlInterface ft232;
	DWORD DeviceNumber = 0;
	FT_STATUS ftStatus;
	ft232.CreateDeviceInfoList(DeviceNumber);
	if (DeviceNumber< 1)
	{
		printf("There are no FTDI devices installed\n");
		getchar();
		return 1; // Exit with error
	}

	printf("%d FTDI devices found \- the count includes individual ports on a single chip\n", DeviceNumber);


	printf("\nAssume first device has the MPSSE and open it...\n");
	if (ft232.openDevice()!=FT_OK)
	{
		getchar();
		return 1;
	}
	printf("\nConfiguring port for MPSSE use...\n");

	//Configure FTDI Port For MPSSE Use
	if (ft232.getQueueStatus() > 0)
	{
		//Purge USB receive buffer first by reading out all old data from FT2232H receive buffer or call function to purge RX buffer
		ft232.purgeBuffers(FT_PURGE_TX);
		//ft232.purgeBuffers(FT_PURGE_RX|FT_PURGE_TX) or purge both buffer
	}
	ft232.setBaudRate(FT_BAUD_115200);
	ftStatus= ft232.setFlowControl(FT_FLOW_NONE);
	if (ftStatus != FT_OK)
	{
		printf("Error in setFlowControl the MPSSE %d\n", ftStatus);
		ft232.closeDevice();
		getchar();
		return 1; // Exit with error
	}
	ftStatus=ft232.setTimeout(2000, 2000);
	if (ftStatus != FT_OK)
	{
		printf("Error in setTimeout the MPSSE %d\n", ftStatus);
		ft232.closeDevice();
		getchar();
		return 1; // Exit with error
	}
	ftStatus=ft232.setBitMode();
	if (ftStatus != FT_OK)
	{
		printf("Error in setBitMode the MPSSE %d\n", ftStatus);
		ft232.closeDevice();
		getchar();
		return 1; // Exit with error
	}
	ftStatus = ft232.resetPort_D();	if (ftStatus != FT_OK)
	{
		printf("Error in resetPort_D the MPSSE %d\n", ftStatus);
		ft232.closeDevice();
		getchar();
		return 1; // Exit with error
	}
	Sleep(100); // Wait for all the USB stuff to complete and work
	//At this point, the MPSSE is ready for commands.
	while (true)
	{
		printf("LOW\n");
		ft232.digitalWrite(D0, LOW);
		Sleep(500);
		ft232.digitalWrite(D0, HIGH);
		printf("HIGH\n");
		Sleep(500);
	}


	getchar();

    return 0;
}

