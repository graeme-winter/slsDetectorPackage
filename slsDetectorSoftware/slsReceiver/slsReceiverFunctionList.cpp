#ifdef SLS_RECEIVER_FUNCTION_LIST
/********************************************//**
 * @file slsReceiverFunctionList.cpp
 * @short does all the functions for a receiver, set/get parameters, start/stop etc.
 ***********************************************/


#include "slsReceiverFunctionList.h"

#include <signal.h>  		// SIGINT
#include <sys/stat.h> 		// stat
#include <sys/socket.h>		// socket(), bind(), listen(), accept(), shut down
#include <arpa/inet.h>		// sock_addr_in, htonl, INADDR_ANY
#include <stdlib.h>			// exit()
#include <iomanip>			//set precision
#include <sys/mman.h>		//munmap


#include <string.h>
#include <iostream>
using namespace std;

FILE* slsReceiverFunctionList::sfilefd(NULL);
int slsReceiverFunctionList::receiver_threads_running(0);

slsReceiverFunctionList::slsReceiverFunctionList(detectorType det,bool moenchwithGotthardTest):
						myDetectorType(det),
						maxFramesPerFile(MAX_FRAMES_PER_FILE),
						enableFileWrite(1),
						fileIndex(0),
						frameIndexNeeded(0),
						framesCaught(0),
						acqStarted(false),
						measurementStarted(false),
						startFrameIndex(0),
						frameIndex(0),
						totalFramesCaught(0),
						startAcquisitionIndex(0),
						acquisitionIndex(0),
						framesInFile(0),
						prevframenum(0),
						listening_thread_running(0),
						writing_thread_running(0),
						status(IDLE),
						latestData(NULL),
						udpSocket(NULL),
						server_port(DEFAULT_UDP_PORTNO),
						fifo(NULL),
						fifofree(NULL),
						fifosize(GOTTHARD_FIFO_SIZE),
						shortFrame(-1),
						bufferSize(GOTTHARD_BUFFER_SIZE),
						packetsPerFrame(GOTTHARD_PACKETS_PER_FRAME),
						guiDataReady(0),
						guiData(NULL),
						guiFileName(NULL),
						currframenum(0),
						nFrameToGui(0),
						startAcquisitionCallBack(NULL),
						pStartAcquisition(NULL),
						acquisitionFinishedCallBack(NULL),
						pAcquisitionFinished(NULL),
						rawDataReadyCallBack(NULL),
						pRawDataReady(NULL),
						withGotthard(moenchwithGotthardTest),
						frameIndexMask(GOTTHARD_FRAME_INDEX_MASK),
						frameIndexOffset(GOTTHARD_FRAME_INDEX_OFFSET)

{
	int aligned_frame_size = GOTTHARD_ALIGNED_FRAME_SIZE;

	if(myDetectorType == MOENCH){
		aligned_frame_size = MOENCH_ALIGNED_FRAME_SIZE;
		fifosize = MOENCH_FIFO_SIZE;
		maxFramesPerFile = MOENCH_MAX_FRAMES_PER_FILE;
		bufferSize = MOENCH_BUFFER_SIZE;
		packetsPerFrame = MOENCH_PACKETS_PER_FRAME;
		if(!withGotthard){
			frameIndexMask = MOENCH_FRAME_INDEX_MASK;
			frameIndexOffset = MOENCH_FRAME_INDEX_OFFSET;
		}
	}

	strcpy(savefilename,"");
	strcpy(filePath,"");
	strcpy(fileName,"run");
	guiFileName = new char[MAX_STR_LENGTH];
	strcpy(guiFileName,"");
	eth = new char[MAX_STR_LENGTH];
	strcpy(eth,"");

	latestData = new char[bufferSize];
	fifofree = new CircularFifo<char>(fifosize);
	fifo = new CircularFifo<char>(fifosize);



	mem0=(char*)malloc(aligned_frame_size*fifosize);
	if (mem0==NULL) {
		cout<<"++++++++++++++++++++++ COULD NOT ALLOCATE MEMORY!!!!!!!+++++++++++++++++++++" << endl;
	}
	buffer=mem0;
	while (buffer<(mem0+aligned_frame_size*(fifosize-1))) {
		fifofree->push(buffer);
		buffer+=aligned_frame_size;
	}

	if(withGotthard)
		cout << "Testing MOENCH Receiver with GOTTHARD Detector" << endl;


	pthread_mutex_init(&status_mutex,NULL);
}



slsReceiverFunctionList::~slsReceiverFunctionList(){
	if(latestData) delete [] latestData;
	if(fifofree) delete [] fifofree;
	if(fifo) delete [] fifo;
	if(guiFileName) delete [] guiFileName;
	if(eth) delete [] eth;
	if(mem0) free(mem0);
}




int slsReceiverFunctionList::setEnableFileWrite(int i){
	if(i!=-1)
		enableFileWrite=i;
	return enableFileWrite;
}


void slsReceiverFunctionList::setEthernetInterface(char* c){
	strcpy(eth,c);
}



uint32_t slsReceiverFunctionList::getFrameIndex(){
	if(!framesCaught)
		frameIndex=0;
	else if(frameIndexOffset)
		frameIndex = currframenum - startFrameIndex; //moench
	else
		frameIndex = (currframenum - startFrameIndex)/packetsPerFrame;//moench with gotthard, gotthard
	return frameIndex;
}



uint32_t slsReceiverFunctionList::getAcquisitionIndex(){
	if(!totalFramesCaught)
		acquisitionIndex=0;
	else if(frameIndexOffset)
		acquisitionIndex = currframenum - startAcquisitionIndex;  //moench
	else
		acquisitionIndex = (currframenum - startAcquisitionIndex)/packetsPerFrame; //moench with gotthard, gotthard
	return acquisitionIndex;
}



char* slsReceiverFunctionList::setFileName(char c[]){
	if(strlen(c))
		strcpy(fileName,c);

	return getFileName();
}



char* slsReceiverFunctionList::setFilePath(char c[]){
	if(strlen(c)){
		//check if filepath exists
		struct stat st;
		if(stat(c,&st) == 0)
			strcpy(filePath,c);
		else{
			strcpy(filePath,"");
			cout<<"FilePath does not exist:"<<filePath<<endl;
		}
	}
	return getFilePath();
}



int slsReceiverFunctionList::setFileIndex(int i){
	if(i>=0)
		fileIndex = i;
	return getFileIndex();
}


void slsReceiverFunctionList::resetTotalFramesCaught(){
	acqStarted = false;
	startAcquisitionIndex = 0;
	totalFramesCaught = 0;

}



int slsReceiverFunctionList::startReceiver(char message[]){
#ifdef VERBOSE
	cout << "Starting Receiver" << endl;
#endif
	cout << endl;

	int err = 0;
	if(!receiver_threads_running){
#ifdef VERBOSE
		cout << "Starting new acquisition threadddd ...." << endl;
#endif
		//change status
		while(1){
			if(!pthread_mutex_trylock(&(status_mutex))){
				status = IDLE;
				listening_thread_running = 0;
				writing_thread_running = 0;
				receiver_threads_running = 1;
				pthread_mutex_unlock(&(status_mutex));
				break;
			}
		}

		// creating listening thread----------
		err = pthread_create(&listening_thread, NULL,startListeningThread, (void*) this);
		if(err){
			//change status
			while(1){
				if(!pthread_mutex_trylock(&(status_mutex))){
					status = IDLE;
					listening_thread_running = 0;
					receiver_threads_running = 0;
					pthread_mutex_unlock(&(status_mutex));
					break;
				}
			}
			sprintf(message,"Cant create listening thread. Status:%d\n",status);
			cout << endl << message << endl;
			return FAIL;
		}
		//wait till udp socket created
		while(!listening_thread_running);
		if(listening_thread_running!=1){
			strcpy(message,"Could not create UDP Socket.\n");
			return FAIL;
		}
#ifdef VERBOSE
				cout << "Listening thread created successfully." << endl;
#endif

		// creating writing thread----------
		err = 0;
		err = pthread_create(&writing_thread, NULL,startWritingThread, (void*) this);
		if(err){
			//change status
			while(1){
				if(!pthread_mutex_trylock(&(status_mutex))){
					status = IDLE;
					writing_thread_running = 0;
					receiver_threads_running = 0;
					pthread_mutex_unlock(&(status_mutex));
					break;
				}
			}
			//stop listening thread
			pthread_join(listening_thread,NULL);
			sprintf(message,"Cant create writing thread. Status:%d\n",status);
			cout << endl << message << endl;
			return FAIL;
		}
		//wait till file is created
		while(!writing_thread_running);
		if(writing_thread_running!=1){
			sprintf(message,"Could not create file %s.\n",savefilename);
			return FAIL;
		}
#ifdef VERBOSE
		cout << "Writing thread created successfully." << endl;
#endif


		//change status----------
		while(1){
			if(!pthread_mutex_trylock(&(status_mutex))){
				status = RUNNING;
				pthread_mutex_unlock(&(status_mutex));
				break;
			}
		}
		cout << "Threads created successfully." << endl;




		struct sched_param tcp_param, listen_param, write_param;
		int policy= SCHED_RR;

		tcp_param.sched_priority = 50;
		listen_param.sched_priority = 99;
		write_param.sched_priority = 90;


		if (pthread_setschedparam(listening_thread, policy, &listen_param) == EPERM)
			cout << "WARNING: Could not prioritize threads. You need to be super user for that." << endl;
		if (pthread_setschedparam(writing_thread, policy, &write_param) == EPERM)
			cout << "WARNING: Could not prioritize threads. You need to be super user for that." << endl;
		if (pthread_setschedparam(pthread_self(),5 , &tcp_param) == EPERM)
			cout << "WARNING: Could not prioritize threads. You need to be super user for that." << endl;


		//pthread_getschedparam(pthread_self(),&policy,&tcp_param);
		//cout << "current priority of main tcp thread is " << tcp_param.sched_priority << endl;

	}


	return OK;
}





int slsReceiverFunctionList::stopReceiver(){
#ifdef VERBOSE
	cout << "Stopping Receiver" << endl;
#endif

	if(receiver_threads_running){
#ifdef VERBOSE
		cout << "Stopping new acquisition threadddd ...." << endl;
#endif
		//stop listening thread
		while(1){
			if(!pthread_mutex_trylock(&(status_mutex))){
				receiver_threads_running=0;
				pthread_mutex_unlock(&(status_mutex));
				break;
			}
		}
		if(udpSocket) udpSocket->ShutDownSocket();
		pthread_join(listening_thread,NULL);
		pthread_join(writing_thread,NULL);
	}
	//change status
	while(1){
		if(!pthread_mutex_trylock(&(status_mutex))){
			status = IDLE;
			pthread_mutex_unlock(&(status_mutex));
			break;
		}
	}
	cout << "Receiver Stopped.\nStatus:" << status << endl;
	return OK;
}



void* slsReceiverFunctionList::startListeningThread(void* this_pointer){
	((slsReceiverFunctionList*)this_pointer)->startListening();

	return this_pointer;
}










int slsReceiverFunctionList::startListening(){
#ifdef VERYVERBOSE
	cout << "In startListening()\n");
#endif
	int rc;

	measurementStarted = false;
	startFrameIndex = 0;

	// A do/while(FALSE) loop is used to make error cleanup easier.  The
	//  close() of each of the socket descriptors is only done once at the
	//  very end of the program.
	do {

		//to increase socket receiver buffer size and max length of input queue by changing kernel settings
		if(system("echo $((100*1024*1024)) > /proc/sys/net/core/rmem_max"))
			cout << "\nWARNING: Could not change socket receiver buffer size in file /proc/sys/net/core/rmem_max" << endl;
		else if(system("echo 250000 > /proc/sys/net/core/netdev_max_backlog"))
			cout << "\nWARNING: Could not change max length of input queue in file /proc/sys/net/core/netdev_max_backlog" << endl;

		/** permanent setting heiner
		net.core.rmem_max = 104857600 # 100MiB
		net.core.netdev_max_backlog = 250000
		sysctl -p
		// from the manual
		sysctl -w net.core.rmem_max=16777216
		sysctl -w net.core.netdev_max_backlog=250000
		*/

		//creating udp socket
		if (strchr(eth,'.')!=NULL) strcpy(eth,"");
		if(!strlen(eth)){
			cout<<"warning:eth is empty.listening to all"<<endl;
			if(udpSocket){
				delete udpSocket;
				udpSocket = NULL;
			}
			udpSocket = new genericSocket(server_port,genericSocket::UDP,bufferSize/packetsPerFrame,packetsPerFrame);
		}else{
			cout<<"eth:"<<eth<<endl;
			udpSocket = new genericSocket(server_port,genericSocket::UDP,bufferSize/packetsPerFrame,packetsPerFrame,eth);
		}
		if (udpSocket->getErrorStatus()){
#ifdef VERBOSE
			std::cout<< "Could not create UDP socket "<< server_port  << std::endl;
#endif
			while(1){
				if(!pthread_mutex_trylock(&(status_mutex))){
					listening_thread_running = -1;
					pthread_mutex_unlock(&(status_mutex));
					break;
				}
			}
			break;
		}



		while (receiver_threads_running) {
			while(1){
				if(!pthread_mutex_trylock(&(status_mutex))){
					listening_thread_running = 1;
					pthread_mutex_unlock(&(status_mutex));
					break;
				}
			}
			if (!fifofree->isEmpty()) {
				fifofree->pop(buffer);


				//receiver 2 half frames / 1 short frame / 40 moench frames
				rc = udpSocket->ReceiveDataOnly(buffer,bufferSize);
				if( rc < 0)
					cerr << "recvfrom() failed" << endl;

				//start for each scan
				if(!measurementStarted){
					if(!frameIndexOffset)
					  startFrameIndex = ((uint32_t)(*((uint32_t*)buffer)));
					else
					  startFrameIndex = ((((uint32_t)(*((uint32_t*)buffer))) & (frameIndexMask)) >> frameIndexOffset);

					//cout<<"startFrameIndex:"<<startFrameIndex<<endl;
					prevframenum=startFrameIndex;
					measurementStarted = true;
				}

				//start of acquisition
				if(!acqStarted){
					startAcquisitionIndex=startFrameIndex;
					currframenum = startAcquisitionIndex;
					acqStarted = true;
					cout<<"startAcquisitionIndex:"<<startAcquisitionIndex<<endl;
				}



				//so that it doesnt write the last frame twice
				if(receiver_threads_running){
					//s.assign(buffer);
					if(fifo->isFull())
						;//cout<<"**********************FIFO FULLLLLLLL************************"<<endl;
					else{
						//cout<<"read index:"<<dec<<(int)(*(int*)buffer)<<endl;& (frameIndexMask)) >> frameIndexOffset;
						fifo->push(buffer);
					}



				}
			}
		}
	} while (receiver_threads_running);
	while(1){
		if(!pthread_mutex_trylock(&(status_mutex))){
			receiver_threads_running=0;
			status = IDLE;
			pthread_mutex_unlock(&(status_mutex));
			break;
		}
	}

	//Close down any open socket descriptors
	udpSocket->Disconnect();

#ifdef VERBOSE
	cout << "receiver_threads_running:" << receiver_threads_running << endl;
#endif

	return 0;
}






void* slsReceiverFunctionList::startWritingThread(void* this_pointer){
	((slsReceiverFunctionList*)this_pointer)->startWriting();

	return this_pointer;
}




int slsReceiverFunctionList::startWriting(){
#ifdef VERYVERBOSE
	cout << "In startWriting()" <<endl;
#endif

	char *wbuf;
	int sleepnumber=0;
	int frameFactor=0;
	int packetloss=0;

	framesInFile=0;
	framesCaught=0;
	frameIndex=0;
	if(sfilefd) sfilefd=NULL;
	strcpy(savefilename,"");

	//reset this before each acq or you send old data
	guiData = NULL;
	guiDataReady=0;
	strcpy(guiFileName,"");

	cout << "Max Frames Per File:" << maxFramesPerFile << endl;
	if (rawDataReadyCallBack)
		cout << "Note: Data Write has been defined exernally" << endl;
	if(nFrameToGui)
		cout << "Sending every " << nFrameToGui << "th frame to gui" <<  endl;


	//by default, we read/write everything
	cbAction = DO_EVERYTHING;
	//acquisition start call back returns enable write
	if (startAcquisitionCallBack)
		cbAction=startAcquisitionCallBack(filePath,fileName,fileIndex,bufferSize,pStartAcquisition);
	if(cbAction < DO_EVERYTHING)
		cout << endl << "Note: Call back activated. Data saving must be taken care of by user in call back." << endl;
	else if(enableFileWrite==0)
		cout << endl << "Note: Data will not be saved" << endl;


	cout << "Ready!" << endl;
	while(receiver_threads_running || (!fifo->isEmpty())){

		//start a new file
		if ((framesInFile == maxFramesPerFile)  || (strlen(savefilename) == 0)){

			//create file name
			if(frameIndexNeeded==-1)	sprintf(savefilename, "%s/%s_%d.raw", filePath,fileName,fileIndex);
			else						sprintf(savefilename, "%s/%s_f%012d_%d.raw", filePath,fileName,framesCaught,fileIndex);

			if(enableFileWrite && cbAction > DO_NOTHING){

				if(sfilefd){
					fclose(sfilefd);
					sfilefd = NULL;
				}

				if (NULL == (sfilefd = fopen((const char *) (savefilename), "w"))){
					cout << "Error: Could not create file " << savefilename << endl;
					while(1){
						if(!pthread_mutex_trylock(&(status_mutex))){
							writing_thread_running = -1;
							pthread_mutex_unlock(&(status_mutex));
							break;
						}
					}
					break;
				}

				//setting buffer
				setvbuf(sfilefd,NULL,_IOFBF,BUF_SIZE);

				//printing packet losses and file names
				//if(prevframenum != 0)
				if(!framesCaught)
					cout << savefilename << endl;
				else{
					if(!frameIndexOffset)
						packetloss = ((currframenum-prevframenum-(packetsPerFrame*framesInFile))/(double)(packetsPerFrame*framesInFile))*100.000;
					else
						packetloss = ((currframenum-prevframenum-(framesInFile))/(double)(framesInFile))*100.000;
					cout << savefilename
							<< "\tpacket loss " << setw(4)<<fixed << setprecision(4)	<< packetloss
							<< "%\tframenum "
							<< currframenum //<< "\t\t p " << prevframenum
							<< "\tindex " << getFrameIndex()
							<< endl;
				}
			}

			while(1){
				if(!pthread_mutex_trylock(&(status_mutex))){
					writing_thread_running = 1;
					pthread_mutex_unlock(&(status_mutex));
					break;
				}
			}

			//if(prevframenum != 0){
			if(framesCaught){
				prevframenum = currframenum;
				framesInFile = 0;
			}
		}



		//pop fifo
		if(!fifo->isEmpty()){

			if(fifo->pop(wbuf)){
				framesCaught++;
				totalFramesCaught++;
				if(!frameIndexOffset)
					currframenum = (uint32_t)(*((uint32_t*)wbuf));
				else
					currframenum = (((uint32_t)(*((uint32_t*)wbuf))) & (frameIndexMask)) >> frameIndexOffset;

				//cout<<"**************curreframenm:"<<currframenum<<endl;

				//write data call back
				if (cbAction < DO_EVERYTHING) {
					rawDataReadyCallBack(currframenum, wbuf, bufferSize, sfilefd, guiData,pRawDataReady);
				}
				//default writing to file
				else if(enableFileWrite){
					if(sfilefd)
						fwrite(wbuf, 1, bufferSize, sfilefd);
					else{
						cout << "You do not have permissions to overwrite: " << savefilename << endl;
						usleep(50000);
					}
				}

				//does not read every frame
				if(!nFrameToGui){
					if(guiData){
						memcpy(latestData,wbuf,bufferSize);
						strcpy(guiFileName,savefilename);
						guiDataReady=1;
					}else
						guiDataReady=0;
				}


				//reads every nth frame
				else{
					if(frameFactor){
						frameFactor--;
					}else{
						frameFactor = nFrameToGui-1;
						//catch nth frame: gui ready to copy data
						while(guiData==NULL){
							if(!receiver_threads_running)
								break;
							usleep(10000);
							guiDataReady=0;
						}

						//copies gui data and sets/resets guiDataReady
						memcpy(latestData,wbuf,bufferSize);
						strcpy(guiFileName,savefilename);
						guiDataReady=1;

						//catch nth frame: wait for gui to take data
						while(guiData==latestData){
							if(!receiver_threads_running)
								break;
							usleep(100000);
						}
						guiDataReady=0;
					}
				}


				framesInFile++;
				fifofree->push(wbuf);
			}
		}
		else{//cout<<"************************fifo empty**********************************"<<endl;
			sleepnumber++;
			usleep(50000);
		}
	}

	while(1){
		if(!pthread_mutex_trylock(&(status_mutex))){
			receiver_threads_running=0;
			pthread_mutex_unlock(&(status_mutex));
			break;
		}
	}
	cout << "Total Frames Caught:"<< totalFramesCaught << endl;


	if(sfilefd){
#ifdef VERBOSE
	cout << "sfield:" << (int)sfilefd << endl;
#endif
		fclose(sfilefd);
		sfilefd = NULL;
	}

	//acquistion over call back
	if (acquisitionFinishedCallBack)
		acquisitionFinishedCallBack(totalFramesCaught, pAcquisitionFinished);

	return 0;
}








void slsReceiverFunctionList::readFrame(char* c,char** raw){

	//point to gui data
	if (guiData == NULL)
		guiData = latestData;

	//copy data and filename
	strcpy(c,guiFileName);

	//could not get gui data
	if(!guiDataReady){
		*raw = NULL;
	}
	//data ready, set guidata to receive new data
	else{
		*raw = guiData;
		guiData = NULL;
	}
}




int slsReceiverFunctionList::setShortFrame(int i){
	shortFrame=i;

	if(shortFrame!=-1){
		bufferSize = GOTTHARD_SHORT_BUFFER_SIZE;
		maxFramesPerFile = SHORT_MAX_FRAMES_PER_FILE;
		packetsPerFrame = GOTTHARD_SHORT_PACKETS_PER_FRAME;

	}else{
		bufferSize = GOTTHARD_BUFFER_SIZE;
		maxFramesPerFile = MAX_FRAMES_PER_FILE;
		packetsPerFrame = GOTTHARD_PACKETS_PER_FRAME;
	}

	return shortFrame;
}


#endif

