#pragma once
//This Header file contains all of includes which are being used in main.c
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRTDBG_MAP_ALLOC
#define BUFFER_SIZE 1501
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#include <math.h>
#include <crtdbg.h>
#include <string.h>
#include <stdio.h>
#include <Windows.h>
#include <tchar.h>
#include <stdlib.h>
#include <stdbool.h>
SOCKET client_s;

// translates a string of 26 bytes (-uncoded) into hamming form (-coded) by adding 5 parity bits (log(26) = 5)  .
void Encoder(char* uncoded, char* coded) {
	char data[248] = { 0 }; //8*31 
	int i, j, skip = 0;

	for (i = 0; i < 248; i++) {   //inserting -1 for all the places that are used for parity bits
		if (i % 31 == 0 || i % 31 == 1 || i % 31 == 3 || i % 31 == 7 || i % 31 == 15) {
			data[i] = -1;
		}
	}

	for (i = 0; i < 26; i++) { //coding the word into bits format
		for (j = 0; j < 8; j++) {
			if (data[i * 8 + j + skip] == -1) skip++;
			if (data[i * 8 + j + skip] == -1) skip++;
			data[i * 8 + j + skip] = abs((char)((int)(uncoded[i] >> (7 - j))) % 2);

		}
	}

	for (i = 0; i < 8; i++) {//
		data[i * 31] = data[i * 31 + 2] ^ data[i * 31 + 4] ^ data[i * 31 + 6] ^ data[i * 31 + 8] ^ data[i * 31 + 10] ^ data[i * 31 + 12] ^ data[i * 31 + 14] ^ data[i * 31 + 16] ^ data[i * 31 + 18] ^ data[i * 31 + 20] ^ data[i * 31 + 22] ^ data[i * 31 + 24] ^ data[i * 31 + 26] ^ data[i * 31 + 28] ^ data[i * 31 + 30];
		data[(i * 31) + 1] = data[i * 31 + 2] ^ data[i * 31 + 5] ^ data[i * 31 + 6] ^ data[i * 31 + 9] ^ data[i * 31 + 10] ^ data[i * 31 + 13] ^ data[i * 31 + 14] ^ data[i * 31 + 17] ^ data[i * 31 + 18] ^ data[i * 31 + 21] ^ data[i * 31 + 22] ^ data[i * 31 + 25] ^ data[i * 31 + 26] ^ data[i * 31 + 29] ^ data[i * 31 + 30];
	// change	data[(i * 15) + 3] = data[i * 15 + 4] ^ data[i * 15 + 5] ^ data[i * 15 + 6] ^ data[i * 15 + 11] ^ data[i * 15 + 12] ^ data[i * 15 + 13] ^ data[i * 15 + 14];
	//change	data[(i * 15) + 7] = data[i * 15 + 8] ^ data[i * 15 + 9] ^ data[i * 15 + 10] ^ data[i * 15 + 11] ^ data[i * 15 + 12] ^ data[i * 15 + 13] ^ data[i * 15 + 14];
	//add one more parity 
	}

	for (i = 0; i < 15; i++) {//turn coded bits into 15 char string
		coded[i] = 0;
		for (j = 0; j < 8; j++) {
			coded[i] = coded[i] * 2;
			coded[i] += data[i * 8 + j];
		}

	}


}
int cleanupAll(char* recieved, int connected) {
	int to_return = 0;
	int result = WSACleanup();
	if (result != 0) {
		printf("WSACleanup failed: %d\n", result);
		to_return = 1;
	}
	closesocket(client_s);
	return to_return;
}

int main(int argc, char* argv[])
{
	//reading Arguments
	char IP[20], port[40];
	strcpy(IP, argv[1]);
	strcpy(port, argv[2]);

	// Infinite iteration for socket opening until recieving quit:
	while (1) {

		//socket creation and conecction:
		WSADATA wsa_data; 	// Initialize Winsock
		SOCKET client_s;
		int result;
		result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
		if (result != 0) {
			printf("WSAStartup failed: %d\n", result);
			return 1;
		}
		if ((client_s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) // Create and init socket in TCP protocol
		{
			printf("Could not create socket : %d", WSAGetLastError());
			return 1;
		}
		struct sockaddr_in client_addr; //Create a sockaddr_in object client_addr and set values.
		client_addr.sin_family = AF_INET;
		client_addr.sin_port = htons(atoi(port));			// port address
		client_addr.sin_addr.s_addr = inet_addr(IP);		// IP address
		int client_addr_len = sizeof(client_addr);
		//Try to connect to Server
		if (connect(client_s, (SOCKADDR*)&client_addr, sizeof(client_addr)) == SOCKET_ERROR) {
			printf("Failed to connect to server on %s:%s  %d", argv[1], argv[2], WSAGetLastError());
		//	return 1;
		}

		printf("enter a file name or quit if done:\n");
		char f_name[100];
		// Read a char type variable,
		// store in "f_name"
		scanf("%s", f_name);
		if (!strcmp(f_name, "quit"))
			return cleanupAll(f_name, 0);
		FILE* f = NULL;
		f = fopen(f_name, "rb");
		if (f == NULL)
		{
			printf("File error. Coudn't open file\n");// CHECKING IF OPENED SUCCSESSFULLY
			return -1;
		}
		printf("File opened successfully\n");
		
		// read File and send data:
		char bit_str[27] = { 0 }, bit_str_hamming[32] = { 0 }, buffer[BUFFER_SIZE] = { 0 };
		int packet_size = 0, bits_read = 0, bits_sent = 0;
			int i = 0;
		while (fread(bit_str, 1, 26, f))
		{
			bits_read += 26 * 8;
			Encoder(bit_str, bit_str_hamming);
			bits_sent += 31 * 8;
			strncpy(buffer + packet_size, bit_str_hamming, 31);
			packet_size += 31;
			//printf("%d\n", strlen(buffer));
			if (packet_size == BUFFER_SIZE - 1)
			{
				printf("sending packet num %d\n", i);
				i++;
				//send data
				if (send(client_s, buffer, strlen(buffer), 0) == SOCKET_ERROR)
				{
					printf("send() failed with error code : %d", WSAGetLastError());
					exit(EXIT_FAILURE);
				}
				for (int i = 0; i < BUFFER_SIZE; i++)
				{
					buffer[i] = 0;
				}
				packet_size = 0;
			}
			//printf("%d %s\n", (char*)coded_chr, buffer);
		}
		if (packet_size != BUFFER_SIZE - 1)
		{
			buffer[packet_size] = '\0';
			//sends remaining data after eof
			if (send(client_s, buffer, strlen(buffer), 0) == SOCKET_ERROR)
			{
				printf("sendto() failed with error code : %d", WSAGetLastError());
				exit(EXIT_FAILURE);
			}
			printf("sending packet num %d\n", i);
		}

		printf("Fils length: %d\nsent %d:\n", bits_read, bits_sent);

		closesocket(client_s);
		fclose(f);		
	}
	return 0;
}