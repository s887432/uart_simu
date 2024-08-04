#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <pthread.h>

#include "uartFunc.h"
#include "bleProtocol.h"

#define RECEIVE_BUFFER_SIZE	512

//#define ENABLE_CMD_PROC

typedef struct __UART_PROC_STR__
{
	int uartID;
}UartProcStr, *pUartProcStr;

void *receive_proc(void *param)
{
	unsigned char recBuff[RECEIVE_BUFFER_SIZE];
	bool KeepGoing = true;
	int fdUart = ((pUartProcStr)param)->uartID;
	int nread;
	
	while (KeepGoing)
	{
		while((nread = read(fdUart, recBuff, 512))>0)
		{
			printf("Received %d bytes\r\n", nread);
			
			// for debug test
			for(int i=0; i<nread; i++)
			{
				if( recBuff[i] >= 0x20 && recBuff[i] < 128 )
				{
					printf("(%c), ", recBuff[i]);
				} else
				{
					printf(", ");
				}
			}
			printf("\r\n");
		}		
	}
	
	return NULL;
}

void printMenu()
{
	std::cout << std::endl << std::endl << "*****************************************************" << std::endl;
	std::cout << "Select command" << std::endl;
	std::cout << "d: disconnect" << std::endl;
	std::cout << "c: connect" << std::endl;
	std::cout << "1: Turn Upper Left" << std::endl;
	std::cout << "2: Turn Upper Left" << std::endl;
	std::cout << "3: Turn Left" << std::endl;
	std::cout << "4: Turn Right" << std::endl;
	std::cout << "5: Turn Lower Left" << std::endl;
	std::cout << "6: Turn Lower Right" << std::endl;
	std::cout << "7: U Turn Left" << std::endl;
	std::cout << "8: U Turn Right" << std::endl;
	std::cout << "9: Arrived" << std::endl;
	std::cout << "x: Exit" << std::endl;
	std::cout << "-----------------------------------------------------" << std::endl;
}

int main(int argc, char **argv)
{
	bool KeepGoing = true;
	int fdUart;	
	int nread;
	unsigned char recBuff[RECEIVE_BUFFER_SIZE];
	unsigned char cmdBuff[RECEIVE_BUFFER_SIZE];		
	char receiveBuffer[RECEIVE_BUFFER_SIZE];
	int receiveIndex = 0;
	int cmdSize;
	NaviStr naviInfo;
	BLE_COMMANDS cmd;
	char input_cmd;
	
	UartProcStr uartParam;
	pthread_t threadReceiveID;
	
	bool bleConnected = false;
	
	if( argc != 2 )
	{
		printf("USAGE: uart_transmit UART_PORT\r\n");
		return -1;
	}

	fdUart = uartOpen(argv[1]);
	uartSetSpeed(fdUart, 115200);

	if (uartSetParity(fdUart,8,1,'N') == -1)
	{
		printf("Set Parity Error\n");
		return -1;
	}
	else
	{
		printf("%s connected\r\n", argv[1]);
	}
	
	
	// ********************************************************************************
	// data receive process
	// ********************************************************************************    
	if( pthread_create(&threadReceiveID, NULL, &receive_proc, &uartParam) != 0 )
	{
		printf("Create receive process thread fail\r\n");
	}
	
	pthread_detach(threadReceiveID);
	pthread_join(threadReceiveID, NULL);
	// ********************************************************************************
	
	bleConnect(fdUart);
	bleConnected = true;
	
	while (KeepGoing)
	{
		printMenu();
		std::cin >> input_cmd;
		
		switch (input_cmd)
		{
			case 'd':
			case 'D':
				if( bleConnected )
				{
					std::cout << "Sending disconnect command" << std::endl;
					bleDisconnect(fdUart);
					bleConnected = false;
				}
				break;
			case 'C':
			case 'c':
				if( !bleConnected )
				{
					bleConnect(fdUart);
					bleConnected = true;
				}
				break;
			case '1':
				std::cout << "Sending Turn Upper Left command" << std::endl;
				bleSend(fdUart, COMMAND_NAVI_DIRECTION, NAVIDIR_UPPER_LEFT);
				break;
			case '2':
				std::cout << "Sending Turn Upper Right command" << std::endl;
				bleSend(fdUart, COMMAND_NAVI_DIRECTION, NAVIDIR_UPPER_RIGHT);
				break;
			case '3':
				std::cout << "Sending Turn Left command" << std::endl;
				bleSend(fdUart, COMMAND_NAVI_DIRECTION, NAVIDIR_LEFT);
				break;
			case '4':
				std::cout << "Sending Turn Right command" << std::endl;
				bleSend(fdUart, COMMAND_NAVI_DIRECTION, NAVIDIR_RIGHT);
				break;
			case '5':
				std::cout << "Sending Turn Lower LEft command" << std::endl;
				bleSend(fdUart, COMMAND_NAVI_DIRECTION, NAVIDIR_LOWER_LEFT);
				break;
			case '6':
				std::cout << "Sending Turn Lower Right command" << std::endl;
				bleSend(fdUart, COMMAND_NAVI_DIRECTION, NAVIDIR_LOWER_RIGHT);
				break;
			case '7':
				std::cout << "Sending U Turn Left command" << std::endl;
				bleSend(fdUart, COMMAND_NAVI_DIRECTION, NAVIDIR_LEFT_U_TURN);
				break;
			case '8':
				std::cout << "Sending U Turn Right command" << std::endl;
				bleSend(fdUart, COMMAND_NAVI_DIRECTION, NAVIDIR_RIGHT_U_TURN);
				break;
			case '9':
				std::cout << "Sending Arrive command" << std::endl;
				bleSend(fdUart, COMMAND_NAVI_DIRECTION, NAVIDIR_ARRIVED);
				break;
			case 'x':
			case 'X':
				bleDisconnect(fdUart);
				KeepGoing = false;
				break;
			default:
				std::cout << "Unknown command" << std::endl;
		}
	}

	close(fdUart);
	
	return 0;
}

// end of file
