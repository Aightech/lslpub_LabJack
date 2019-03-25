/**
 * Name: t7_modbus_tcp_stream_example.c
 * Desc: Demonstrates low-level spontaneous streaming on a T7 using TCP and
 *       Modbus. Single threaded.
 *       General stream mode documentation can be found here:
 *       http://labjack.com/support/datasheets/t7/communication/stream-mode
 *
 *       Note to Visual Studios users:
 *       This program uses Ctrl+C to stop streaming. In Visual Studios this
 *       causes a first chance exception that stalls the program with a window.
 *       Click the Continue option to let the program finish as it handles
 *       Ctrl+C. Keep in mind this demonstration is keeping track of time,
 *       so the delay from the exception window leads to inaccurate time and
 *       rate calculations. To prevent this, disable the "Control-C" exception
 *       in the Debug->Exceptions menu, or run the built executable outside
 *       Visual Studios.
 * Auth: LabJack Corp.
**/

#ifdef WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

#include <vector>
#include <iostream>
#include <lsl_cpp.h>

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "tcp.h" //For TCP functions for communicating with a T7.
#include "calibration.h" //For reading the calibration constants from a T7 and applying them on stream data.
#include "stream.h" //Provides the stream related functions. These functions handle the Modbus calls. 


int gQuit = 0;

void streamExample(const char* IP_ADDR);

int	main(int argc, const char* argv[])
{
	const char DEFAULT_IP_ADDR[] = "192.168.1.207"; //Set your IP Addresses here, or set it using the first argument when running the program.
	if(argc > 1)
		streamExample(argv[1]);
	else
		streamExample(DEFAULT_IP_ADDR);
	return 0;
}

//Returns the current time (in seconds).
//Linux/Mac OS X: Time since the Epoch.
//Windows: Time since system was started up to 49.7 days.
double getTimeSec()
{
#ifdef WIN32
	return GetTickCount()/1000.0;
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec + (tv.tv_usec / 1000000.0);
#endif
}

//Handling function for Ctrl+C. This will tell the stream read loop to stop.
void quitHandler(int sig)
{
	gQuit = 1;
}

void setQuitHandler()
{
#ifdef WIN32
	signal(SIGINT, quitHandler);
#else
    struct sigaction sigHandler;
	sigHandler.sa_handler = quitHandler;
	sigemptyset(&sigHandler.sa_mask);
	sigHandler.sa_flags = SA_RESETHAND;
    sigaction(SIGINT, &sigHandler, NULL);
#endif
}

void deleteQuitHandler()
{
#ifdef WIN32
	signal(SIGINT, SIG_DFL);
#endif
}

void streamExample(const char* IP_ADDR)
{
	//Time related
	double startTime = 0;
	double endTime = 0;
	double lastPrint = 0;

	//IP address and port settings
	const int CR_PORT = 502; //Command/response TCP port (most operations)
	const int SP_PORT = 702; //Spontaneous stream TCP port

	//Sockets
	TCP_SOCKET crSock = 0; //Command/Response socket
	TCP_SOCKET arSock = 0; //Spontaneous stream socket
	
	//Calibration constants
	DeviceCalibration devCal;

	//Stream config. settings. Except for NUM_ADDRESSES, configured later.
	enum {NUM_ADDRESSES = 2};
	float scanRate = 0;
	unsigned int numAddresses = 0;
	unsigned int samplesPerPacket = 0;
	float settling = 0;
	unsigned int resolutionIndex = 0;
	unsigned int bufferSizeBytes = 0;
	unsigned int autoTarget = 0;
	unsigned int numScans = 0;
	unsigned int scanListAddresses[NUM_ADDRESSES] = {0};
	unsigned short nChanList[NUM_ADDRESSES] = {0};
	float rangeList[NUM_ADDRESSES] = {0.0};
	unsigned int gainList[NUM_ADDRESSES]; //Based off rangeList

	//Stream read returns
	unsigned short backlog = 0;
	unsigned short status = 0;
	unsigned short additionalInfo = 0;

	//Stream read loop variables
	unsigned int i = 0, j = 0;
	unsigned int addrIndex = 0;
	float volts = 0.0f;
	unsigned char *rawData;
	int printStream = 0;
	int printStreamStart = 0;
	const double printStreamTimeSec = 1.0; //How often to print to the terminal in seconds.
	double scanTotal = 0;
	double numScansSkipped = 0;

	printf("Connecting to %s ...\n", IP_ADDR);

	//Open sockets
	
	arSock = openTCP(IP_ADDR, SP_PORT);
	crSock = openTCP(IP_ADDR, CR_PORT);
	printf("Connecting to %s ...\n", IP_ADDR);
	if(crSock == INVALID_SOCKET || arSock == INVALID_SOCKET)
		goto END;

	setCommTimeoutTCP(crSock, 5); //Set command/response port timeouts to 5 seconds

	printf("Connected.\n");

	//Get device calibration
	printf("Reading	calibration constants.\n");
	getCalibration(crSock, &devCal);

	//Configure stream
	scanRate = 1000.0f; //Scans per second. Samples per second = scanRate * numAddresses
	numAddresses = NUM_ADDRESSES;
	samplesPerPacket = STREAM_MAX_SAMPLES_PER_PACKET_TCP;  //Max is 512. For better throughput set this to high values.
	settling = 10.0; //10 microseconds
	resolutionIndex = 0; //Default
	bufferSizeBytes = 0; //Default
	autoTarget = STREAM_TARGET_ETHERNET; //Stream target is Ethernet.
	numScans = 0; //0 = Run continuously.

	//Using a loop to add Modbus addresses for AIN0 - AIN(NUM_ADDRESSES-1) to the
	//stream scan and configure the analog input settings.
	for(i = 0; i < numAddresses; i++)
		{
			scanListAddresses[i] = i*2; //AIN(i) (Modbus address i*2)
			nChanList[i] = 199; //Negative channel is 199 (single ended)
			rangeList[i] = 10.0; //0.0 = +/-10V, 10.0 = +/-10V, 1.0 = +/-1V, 0.1 = +/-0.1V, or 0.01 = +/-0.01V.
			gainList[i] = 0; //gain index 0 = +/-10V 
		}
	
	//Does the same as above with default settings NUM_ADDRESSES = 2 without a loop.
	//scanListAddresses[0] = 0; //First sample is AIN0 (Modbus address 0).
	//scanListAddresses[1] = 2; //Second sample is AIN1 (Modbus address 2).
	//nChanList[0] = 199; //First sample's negative channel is 199 (single ended).
	//nChanList[1] = 199; //Second sample's negative channel is 199 (single	ended).
	//rangeList[0] = 10.0; //First sample's range is +/-10V.
	//rangeList[1] = 10.0; //Second sample's range is +/-10V.
	//gainList[0] = 0; //First sample's gain index is 0 (+/-10V) 
	//gainList[1] = 0; //Second sample's gain index is 0 (+/-10V)

	//Call not neccessary for non analog input addresses. Demonstration assumes all
	//addresses are analog input.
	printf("Configuring analog inputs.\n");
	if(ainConfig(crSock, numAddresses, scanListAddresses, nChanList, rangeList) != 0)
		goto END;

	printf("Configuring stream settings.\n");
	if(streamConfig(crSock, scanRate, numAddresses, samplesPerPacket, settling, resolutionIndex, bufferSizeBytes, autoTarget, numScans, scanListAddresses) != 0)
		{
			printf("streamConfig failed - Stop stream just in case.\n");
			streamStop(crSock);
			goto END;
		}

	//Read back stream settings
	printf("Reading stream configuration.\n");
	if(readStreamConfig(crSock, &scanRate, &numAddresses, &samplesPerPacket, &settling, &resolutionIndex, &bufferSizeBytes, &autoTarget, &numScans) != 0)
		goto END;
	if(numAddresses != NUM_ADDRESSES)
		{
			printf("Modbus addresses were not set correctly.\n");
			goto END;
		}

	printf("Reading stream scan list.\n");
	if(readStreamAddressesConfig(crSock, numAddresses, scanListAddresses) != 0)
		goto END;

	printf("Reading analog inputs configuration.\n");
	if(readAinConfig(crSock, numAddresses, scanListAddresses, nChanList, rangeList) != 0)
		goto END;

	printf("Stream Configuration:\n");
	printf("  Scan Rate (Hz) = %.3f, Samples Per Packet = %u, # Samples Per Scan = %u\n", scanRate, samplesPerPacket, numAddresses);
	printf("  Settling (us) = %.3f, Resolution Index = %u, Buffer Size Bytes = %u\n", settling, resolutionIndex, bufferSizeBytes);
	printf("  Auto Target = %u, Number of Scans = %u\n", autoTarget, numScans);
	
	printf("  Scan List Addresses = ");
	for(i = 0; i < numAddresses; i++)
		printf("%u ", scanListAddresses[i]);
	printf("\n  Negative Channels = ");
	for(i = 0; i < numAddresses; i++)
		printf("%u ", nChanList[i]);
	printf("\n  Ranges = ");
	for(i = 0; i < numAddresses; i++)
		printf("%.3f ", rangeList[i]);
	printf("\n");
	printf("Press Enter key to start streaming.\nPress Ctrl+C to stop streaming.\n");
	getchar();

	//Set signal handling for Ctrl+C
	setQuitHandler();

	//Set spontaneous stream port timeouts to expected time per packet + 2 seconds.
	setCommTimeoutTCP(arSock, (int)(samplesPerPacket/(scanRate*numAddresses))+2);

	printf("Starting stream.\n");
	if(streamStart(crSock) != 0)
		{
			printf("Stopping stream\n");
			streamStop(crSock);
			goto END;
		}

	startTime = getTimeSec();
	lastPrint = startTime;

	printf("Reading streaming data.\n");
	rawData = (unsigned char *)malloc(samplesPerPacket*STREAM_BYTES_PER_SAMPLE);

	//Stream read loop. If encountering stream buffer overflows in your own code,
	//move your stream read loop to its own dedicated thread and perform
	//operations on stream data in different threads.
	try {
	        lsl::stream_info info("LabJack", "labJackSamples", numAddresses, lsl::IRREGULAR_RATE,lsl::cf_float32);
		lsl::stream_outlet outlet(info);
		std::vector<std::vector<float>> chunk;
		
		while(!gQuit)
			{
				backlog = 0;
				status = 0;
				additionalInfo = 0;

				if(spontaneousStreamRead(arSock, samplesPerPacket, &backlog, &status, &additionalInfo, rawData) != 0)
					{
						if(gQuit)
							break; //Stream read error due to interrupt (Ctrl+C). Expected and stopping loop.
						goto STOP_STREAM;
					}
				backlog = backlog / (numAddresses*STREAM_BYTES_PER_SAMPLE); //Scan backlog
		
				//Check status
				if(status == STREAM_STATUS_SCAN_OVERLAP)
					{
						//Stream scan overlap occured. This usually indicates the scan rate
						//is too fast for the stream configuration.
						//Stopping the stream.
						printf("\nReceived stream status error 2942 - STREAM_SCAN_OVERLAP. Stopping stream.\n");
						gQuit = 1;
						continue;
					}
				else if(status == STREAM_STATUS_AUTO_RECOVER_END_OVERFLOW)
					{
						//During auto recovery the skipped samples counter (16-bit) overflowed.
						//Stopping the stream because of unknown amount of skipped samples.
						printf("\nReceived stream status error 2943 - STREAM_AUTO_RECOVER_END_OVERFLOW. Stopping stream.\n");
						printf("Scan Backlog = %u\n", backlog);
						gQuit = 1;
						continue;
					}
				else if(status == STREAM_STATUS_AUTO_RECOVER_ACTIVE)
					{
						//Stream buffer overload occured. In auto recovery mode. Continue
						//reading existing samples from the T7's stream buffer which is still valid.
						printf("\nReceived stream status 2940 -	STREAM_AUTO_RECOVER_ACTIVE.\n");
						printf("Scan Backlog = %u\n", backlog);
					}
				else if(status == STREAM_STATUS_AUTO_RECOVER_END)
					{
						//Auto recover mode has ended. The number of skipped scans are reported
						//and new samples are coming in.
						numScansSkipped += (double)additionalInfo; //# skipped scans
						printf("\nReceived stream status 2941 - STREAM_AUTO_RECOVER_END. %u scans were skipped.\n", additionalInfo);
						printf("Scan Backlog = %u\n", backlog);
					}
				else if(status == STREAM_STATUS_BURST_COMPLETE)
					{
						//Stream burst has completed. Status used when numScans
						//(Address 4020 - STREAM_NUM_SCANS) is configured to a non-zero value.
						printf("Stream burst has completed\n");
						gQuit = 1;
					}
				else if(status != 0)
					{
						printf("\nReceived stream status %u\n", status);
					}
				
			
				chunk.clear();
				std::vector<float> s;
				int k=-1;
				//Convert to voltage and display readings
				for(j = 0; j < samplesPerPacket; j++)
					{
						if(rawData[j*STREAM_BYTES_PER_SAMPLE] == 0xFF && rawData[j*STREAM_BYTES_PER_SAMPLE+1] == 0xFF)
							{
								//Dummy value to indicate where the missing scan/samples would be.
								//numAddresses samples will	be 0xFFFF, and then new data.
								printf("Dummy sample detected, addr. index = %d\n",	addrIndex);
								if(addrIndex != 0)
									{
										printf("\nReceived dummy sample (0xFFFF) in the middle of a scan. Incomplete scans shouldn't happen.\n");
										printf("Scan sample index = %d\n", addrIndex);
									}
								continue;
							}
						ainBinToVolts(&devCal, &rawData[j*STREAM_BYTES_PER_SAMPLE], gainList[addrIndex], &volts);
						if(j%numAddresses==0)
							{
								k++;
								std::vector<float> s;
								chunk.push_back(s);
							}
						chunk[k].push_back(volts);
						//Print to terminal
						if(printStream)
							{
								if(addrIndex == 0 && !printStreamStart)
									{
										//Start printing (Stream info and the current scan)
										printf("\nScan # %.00f: ", scanTotal+1);
										printStreamStart = 1;
									}
								if(printStreamStart)
									printf("%f ", volts);
								if(addrIndex == (numAddresses - 1) && printStreamStart)
									{
										//Stop printing
										printStream = 0;
										printStreamStart = 0;
										//Backlog is in bytes
										printf("\nScan Backlog = %u, Status = %u, Additional Info. = %u\n", backlog, status, additionalInfo);
									}
							}

						//The current scan's address index.
						addrIndex++;
						if(addrIndex >=	numAddresses)
							{
								addrIndex = 0;
								scanTotal++;
							}
					}
				// send it
				outlet.push_chunk(chunk);
				

				if((getTimeSec() - lastPrint) > printStreamTimeSec)
					{
						//Initiate terminal printing
						printStream = 1;
						printStreamStart = 0;
						lastPrint = getTimeSec();
					}
			}

	} catch (std::exception& e) { std::cerr << "[ERROR] Got an exception: " << e.what() << std::endl; }
	
	

	endTime = getTimeSec();
	printf("\nStopped stream reading.\n\n");

	if(addrIndex > 0)
		scanTotal += ((double)addrIndex/(double)numAddresses); //Add uncounted samples to scan total

	printf("Configured Scan Rate = %.00f\n", scanRate);
	printf("# Scans = %.03f\n", scanTotal);
	printf("# Scans skipped = %.00f (%.00f samples)\n", numScansSkipped, numScansSkipped*numAddresses);
	printf("Time taken = %f sec.\n", (endTime-startTime));
	printf("Timed Scan Rate = %0.03f\n", (scanTotal/(endTime-startTime)));
	printf("Timed Sample Rate = %0.03f\n", ((scanTotal*numAddresses)/(endTime-startTime)));

 STOP_STREAM:
	free(rawData);
	printf("Stopping stream\n");
	if(streamStop(crSock))
		goto END;
	printf("Stream stopped\n");
 END:
	deleteQuitHandler();

	//Close sockets
	closeTCP(crSock);
	closeTCP(arSock);

	printf("Press enter to exit.\n");
	while(getchar() != '\n') {}

	return;
}
