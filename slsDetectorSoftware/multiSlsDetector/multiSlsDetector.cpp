/*******************************************************************

Date:       $Date$
Revision:   $Rev$
Author:     $Author$
URL:        $URL$
ID:         $Id$

********************************************************************/



#include "multiSlsDetector.h"
#include "slsDetector.h"
#include "multiSlsDetectorCommand.h"
#include "multiSlsDetectorClient.h"
#include "postProcessingFuncs.h"
#include "usersFunctions.h"
#include  <sys/types.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>
#include  <iostream>
#include  <string>
using namespace std;


char ans[MAX_STR_LENGTH];

int multiSlsDetector::freeSharedMemory() {
  // Detach Memory address
  for (int id=0; id<thisMultiDetector->numberOfDetectors; id++) {
    if (detectors[id])
      detectors[id]->freeSharedMemory();
  }


  if (shmdt(thisMultiDetector) == -1) {
    perror("shmdt failed\n");
    return FAIL;
  }
  printf("Shared memory %d detached\n", shmId);
  // remove shared memory
  if (shmctl(shmId, IPC_RMID, 0) == -1) {
    perror("shmctl(IPC_RMID) failed\n");
    return FAIL;
  }
  printf("Shared memory %d deleted\n", shmId);
  return OK;

}



int multiSlsDetector::initSharedMemory(int id=0) {
  
  key_t     mem_key=DEFAULT_SHM_KEY+MAXDET+id;
  int       shm_id;
  int sz;



  sz=sizeof(sharedMultiSlsDetector);


#ifdef VERBOSE
  std::cout<<"multiSlsDetector: Size of shared memory is "<< sz << " - id " << mem_key << std::endl;
#endif
  shm_id = shmget(mem_key,sz,IPC_CREAT  | 0666); // allocate shared memory

  if (shm_id < 0) {
    std::cout<<"*** shmget error (server) ***"<< shm_id << std::endl;
    return shm_id;
  }
  
  /**
     thisMultiDetector pointer is set to the memory address of the shared memory
  */

  thisMultiDetector = (sharedMultiSlsDetector*) shmat(shm_id, NULL, 0);  /* attach */
  
  if (thisMultiDetector == (void*)-1) {
    std::cout<<"*** shmat error (server) ***" << std::endl;
    return shm_id;
  }
  /**
     shm_id returns -1 is shared memory initialization fails
  */ 

  return shm_id;

}




multiSlsDetector::multiSlsDetector(int id) :  slsDetectorUtils(), shmId(-1)
{
  while (shmId<0) {
    shmId=initSharedMemory(id);
    id++;
  }
  id--;

  if (thisMultiDetector->alreadyExisting==0) {


    thisMultiDetector->onlineFlag = ONLINE_FLAG;
    thisMultiDetector->numberOfDetectors=0;
    for (int id=0; id<MAXDET; id++) {
      thisMultiDetector->detectorIds[id]=-1;
      thisMultiDetector->offsetX[id]=0;
      thisMultiDetector->offsetY[id]=0;
    }
    thisMultiDetector->masterPosition=-1;
    thisMultiDetector->dataBytes=0;
    thisMultiDetector->numberOfChannels=0;

    thisMultiDetector->maxNumberOfChannels=0;

    /** set trimDsdir, calDir and filePath to default to home directory*/
    strcpy(thisMultiDetector->filePath,getenv("HOME"));
    /** set fileName to default to run*/
    strcpy(thisMultiDetector->fileName,"run");
    /** set fileIndex to default to 0*/
    thisMultiDetector->fileIndex=0;
    /** set frames per file to default to 1*/
    thisMultiDetector->framesPerFile=1;

    /** set progress Index to default to 0*/
    thisMultiDetector->progressIndex=0;
    /** set total number of frames to be acquired to default to 1*/
    thisMultiDetector->totalProgress=1;




    /** set correction mask to 0*/
    thisMultiDetector->correctionMask=1<<WRITE_FILE;
    /** set deat time*/
    thisMultiDetector->tDead=0;
    /** sets bad channel list file to none */
    strcpy(thisMultiDetector->badChanFile,"none");
    /** sets flat field correction directory */
    strcpy(thisMultiDetector->flatFieldDir,getenv("HOME"));
    /** sets flat field correction file */
    strcpy(thisMultiDetector->flatFieldFile,"none");
    /** set angular direction to 1*/
    thisMultiDetector->angDirection=1;
    /** set fine offset to 0*/
    thisMultiDetector->fineOffset=0;
    /** set global offset to 0*/
    thisMultiDetector->globalOffset=0;



    /** set threshold to -1*/
    thisMultiDetector->currentThresholdEV=-1;
    // /** set clockdivider to 1*/
    // thisMultiDetector->clkDiv=1;
    /** set number of positions to 0*/
    thisMultiDetector->numberOfPositions=0;
    /** sets angular conversion file to none */
    strcpy(thisMultiDetector->angConvFile,"none");
    /** set binsize*/
    thisMultiDetector->binSize=0.001;
    thisMultiDetector->stoppedFlag=0;
     
    thisMultiDetector->threadedProcessing=1;

    thisMultiDetector->actionMask=0;


    for (int ia=0; ia<MAX_ACTIONS; ia++) {
      //thisMultiDetector->actionMode[ia]=0;
      strcpy(thisMultiDetector->actionScript[ia],"none");
      strcpy(thisMultiDetector->actionParameter[ia],"none");
    }


    for (int iscan=0; iscan<MAX_SCAN_LEVELS; iscan++) {
       
      thisMultiDetector->scanMode[iscan]=0;
      strcpy(thisMultiDetector->scanScript[iscan],"none");
      strcpy(thisMultiDetector->scanParameter[iscan],"none");
      thisMultiDetector->nScanSteps[iscan]=0;
      thisMultiDetector->scanPrecision[iscan]=0;
    }





    thisMultiDetector->alreadyExisting=1;
  }

  
  //assigned before creating detector
  stoppedFlag=&thisMultiDetector->stoppedFlag;
  threadedProcessing=&thisMultiDetector->threadedProcessing;
  actionMask=&thisMultiDetector->actionMask;
  actionScript=thisMultiDetector->actionScript;		
  actionParameter=thisMultiDetector->actionParameter;      
  nScanSteps=thisMultiDetector->nScanSteps;
  scanMode=thisMultiDetector->scanMode;
  scanScript=thisMultiDetector->scanScript;
  scanParameter=thisMultiDetector->scanParameter;
  scanSteps=thisMultiDetector->scanSteps;
  scanPrecision=thisMultiDetector->scanPrecision;
  numberOfPositions=&thisMultiDetector->numberOfPositions;
  detPositions=thisMultiDetector->detPositions;
  angConvFile=thisMultiDetector->angConvFile;
  correctionMask=&thisMultiDetector->correctionMask;
  binSize=&thisMultiDetector->binSize;
  fineOffset=&thisMultiDetector->fineOffset;
  globalOffset=&thisMultiDetector->globalOffset;
  angDirection=&thisMultiDetector->angDirection;
  flatFieldDir=thisMultiDetector->flatFieldDir;
  flatFieldFile=thisMultiDetector->flatFieldFile;
  badChanFile=thisMultiDetector->badChanFile;
  timerValue=thisMultiDetector->timerValue;

  expTime=&timerValue[ACQUISITION_TIME];

  currentSettings=&thisMultiDetector->currentSettings;
  currentThresholdEV=&thisMultiDetector->currentThresholdEV;
  moveFlag=NULL;

  sampleDisplacement=thisMultiDetector->sampleDisplacement;

  filePath=thisMultiDetector->filePath;
  fileName=thisMultiDetector->fileName;
  fileIndex=&thisMultiDetector->fileIndex;
  framesPerFile=&thisMultiDetector->framesPerFile;


  for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
#ifdef VERBOSE
    cout << thisMultiDetector->detectorIds[i] << endl;
#endif
    detectors[i]=new slsDetector(thisMultiDetector->detectorIds[i], this);


    //  setAngularConversionPointer(detectors[i]->getAngularConversionPointer(),detectors[i]->getNModsPointer(),detectors[i]->getNChans()*detectors[i]->getNChips(), i);

  }
  for (int i=thisMultiDetector->numberOfDetectors; i<MAXDET; i++)
    detectors[i]=NULL;



  /** modifies the last PID accessing the detector system*/
  thisMultiDetector->lastPID=getpid();


  getNMods();
  getMaxMods();

}

multiSlsDetector::~multiSlsDetector() {
  //removeSlsDetector();

}

int multiSlsDetector::addSlsDetector(int id, int pos) {
  int j=thisMultiDetector->numberOfDetectors;
 

  if (slsDetector::exists(id)==0) {
    cout << "Detector " << id << " does not exist - You should first create it to determine type etc." << endl;
  }
  
#ifdef VERBOSE
  cout << "Adding detector " << id << " in position " << pos << endl;
#endif

  if (pos<0)
    pos=j;

  if (pos>j)
    pos=thisMultiDetector->numberOfDetectors;
  


  //check that it is not already in the list
  
  for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
    //check that it is not already in the list, in that case move to new position
    if (detectors[i]) {
      if (detectors[i]->getDetectorId()==id) { 
	cout << "Detector " << id << "already part of the multiDetector in position " << i << "!" << endl << "Remove it before adding it back in a new position!"<< endl;
	return -1;
      }
    }
  }
  


  if (pos!=thisMultiDetector->numberOfDetectors) {
    for (int ip=thisMultiDetector->numberOfDetectors-1; ip>=pos; ip--) {
#ifdef VERBOSE
      cout << "Moving detector " << thisMultiDetector->detectorIds[ip] << " from position " << ip << " to " << ip+1 << endl;
#endif
      thisMultiDetector->detectorIds[ip+1]=thisMultiDetector->detectorIds[ip];
      detectors[ip+1]=detectors[ip];
    }
  }
#ifdef VERBOSE
  cout << "Creating new detector " << pos << endl;
#endif

  detectors[pos]=new slsDetector(id, this);
  thisMultiDetector->detectorIds[pos]=detectors[pos]->getDetectorId();
  thisMultiDetector->numberOfDetectors++;



  thisMultiDetector->dataBytes+=detectors[pos]->getDataBytes();
 
  thisMultiDetector->numberOfChannels+=detectors[pos]->getTotalNumberOfChannels();
  thisMultiDetector->maxNumberOfChannels-=detectors[j]->getMaxNumberOfChannels();
  getMaxMods();
  getNMods();

#ifdef VERBOSE
  cout << "Detector added " << thisMultiDetector->numberOfDetectors<< endl;

  for (int ip=0; ip<thisMultiDetector->numberOfDetectors; ip++) {
    cout << "Detector " << thisMultiDetector->detectorIds[ip] << " position " << ip << " "  << detectors[ip]->getHostname() << endl;
  }
#endif

  return thisMultiDetector->numberOfDetectors;

}


string multiSlsDetector::setHostname(const char* name, int pos){

  // int id=0;
  string s;
  if (pos>=0) {
    addSlsDetector(name, pos);
    if (detectors[pos])
      return detectors[pos]->getHostname();
  } else {
    size_t p1=0;
    s=string(name);
    size_t p2=s.find('+',p1);
    char hn[1000];
    if (p2==string::npos) {
      strcpy(hn,s.c_str());
      addSlsDetector(hn, pos);
    } else {
      while (p2!=string::npos) {
	strcpy(hn,s.substr(p1,p2-p1).c_str());
	addSlsDetector(hn, pos);
	s=s.substr(p2+1);
	p2=s.find('+');
      }
    }
  }
#ifdef VERBOSE
  cout << "-----------------------------set online!" << endl;
#endif
  setOnline(ONLINE_FLAG);

  return getHostname(pos);
}

string multiSlsDetector::ssetDetectorsType(string name, int pos) {


  // int id=0;
  string s;
  if (pos>=0) {
    if (getDetectorType(name)!=GET_DETECTOR_TYPE)
      addSlsDetector(name.c_str(), pos);
  } else {
    removeSlsDetector(); //reset detector list!
    size_t p1=0;
    s=string(name);
    size_t p2=s.find('+',p1);
    char hn[1000];
    if (p2==string::npos) {
      strcpy(hn,s.c_str());
      addSlsDetector(hn, pos);
    } else {
      while (p2!=string::npos) {
	strcpy(hn,s.substr(p1,p2-p1).c_str());
	if (getDetectorType(hn)!=GET_DETECTOR_TYPE)
	  addSlsDetector(hn, pos);
	s=s.substr(p2+1);
	p2=s.find('+');
      }
    }
  }
  return sgetDetectorsType(pos);

}

string multiSlsDetector::getHostname(int pos) {
  
  string s=string("");
#ifdef VERBOSE
  cout << "returning hostname" << pos << endl;
#endif
  if (pos>=0) {
    if (detectors[pos])
      return detectors[pos]->getHostname();
  } else {
    for (int ip=0; ip<thisMultiDetector->numberOfDetectors; ip++) {
#ifdef VERBOSE
      cout << "detector " << ip << endl;
#endif
      if (detectors[ip]) {
	s+=detectors[ip]->getHostname();
	s+=string("+");
      }
      cout << s <<endl;
#ifdef VERBOSE
      cout << "hostname " << s << endl;
#endif
    }
  }
  return s;
  

}


slsDetectorDefs::detectorType multiSlsDetector::getDetectorsType(int pos) {

  detectorType s =GENERIC;
#ifdef VERBOSE
  cout << "returning type of detector with ID " << pos << endl;
#endif
  if (pos>=0) {
    if (detectors[pos])
      return detectors[pos]->getDetectorsType();
  } else if (detectors[0])
    return detectors[0]->getDetectorsType();
  return s;
}


string multiSlsDetector::sgetDetectorsType(int pos) {
  
  string s=string("");
#ifdef VERBOSE
  cout << "returning type" << pos << endl;
#endif
  if (pos>=0) {
    if (detectors[pos])
      return detectors[pos]->sgetDetectorsType();
  } else {
    for (int ip=0; ip<thisMultiDetector->numberOfDetectors; ip++) {
#ifdef VERBOSE
      cout << "detector " << ip << endl;
#endif
      if (detectors[ip]) {
	s+=detectors[ip]->sgetDetectorsType();
	s+=string("+");
      }
#ifdef VERBOSE
      cout << "type " << s << endl;
#endif
    }
  }
  return s;
  
}




int multiSlsDetector::getDetectorId(int pos) {
  
#ifdef VERBOSE
  cout << "Getting detector ID " << pos << endl;
#endif

  if (pos>=0) {
    if (detectors[pos])
      return detectors[pos]->getDetectorId();
  } 
  return -1;
}



int multiSlsDetector::setDetectorId(int ival, int pos){

  if (pos>=0) {
    addSlsDetector(ival, pos);
    if (detectors[pos])
      return detectors[pos]->getDetectorId();
  } else {
    return -1;
  }
  return -1;
 
}


int multiSlsDetector::addSlsDetector(const char *name, int pos) {
  

  detectorType t=getDetectorType(string(name));
  int online=0;
  slsDetector *s=NULL;
  int id;
#ifdef VERBOSE
  cout << "Adding detector "<<name << " in position " << pos << endl;
#endif


  if (t==GENERIC) {
    for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
      if (detectors[i]) {
	if (detectors[i]->getHostname()==string(name)) {
	  cout << "Detector " << name << "already part of the multiDetector in position " << i << "!" << endl<<  "Remove it before adding it back in a new position!"<< endl;
	  return -1;	
	}
      }
    }
   
    //checking that the detector doesn't already exists
    
    for (id=0; id<MAXDET; id++) {
      if (slsDetector::exists(id)>0) {
#ifdef VERBOSE
	cout << "Detector " << id << " already exists" << endl;
#endif
	s=new slsDetector(id, this);
	if (s->getHostname()==string(name))
	  break;
	delete s;
	s=NULL;
	//id++;
      }
    }

    if (s==NULL) {
      t=slsDetector::getDetectorType(name, DEFAULT_PORTNO);
      if (t==GENERIC) {
	cout << "Detector " << name << "does not exist in shared memory and could not connect to it to determine the type (which is not specified)!" << endl;
	return -1;
      }
#ifdef VERBOSE
      else
	cout << "Detector type is " << t << endl;
#endif  
      online=1;
    }
  } 
#ifdef VERBOSE
  else
    cout << "Adding detector by type " << getDetectorType(t) << endl;
#endif  



  if (s==NULL) {
    for (id=0; id<MAXDET; id++) {
      if (slsDetector::exists(id)==0) {
	break;
      }
    }
    
#ifdef VERBOSE
    cout << "Creating detector " << id << " of type "  << getDetectorType(t) << endl;
#endif
    s=new slsDetector(t, id, this);
    if (online) {
      s->setTCPSocket(name);
      setOnline(ONLINE_FLAG);
    }
    delete s;
  }
#ifdef VERBOSE
  cout << "Adding it to the multi detector structure" << endl;
#endif
  return addSlsDetector(id, pos);


}


int multiSlsDetector::addSlsDetector(detectorType t, int pos) {
  
  int id;

  if (t==GENERIC) {
    return -1;
  }

  for (id=0; id<MAXDET; id++) {
    if (slsDetector::exists(id)==0) {
      break;
    }
  }
    
#ifdef VERBOSE
  cout << "Creating detector " << id << " of type "  << getDetectorType(t) << endl;
#endif
  slsDetector *s=new slsDetector(t, id, this);
  s=NULL;
#ifdef VERBOSE
  cout << "Adding it to the multi detector structure" << endl;
#endif

  return addSlsDetector(id, pos);

}








int multiSlsDetector::getDetectorOffset(int pos, int &ox, int &oy) {
  ox=-1;
  oy=-1;
  int ret=FAIL;
  if (pos>=0 && pos<thisMultiDetector->numberOfDetectors) {
    if (detectors[pos]) {
      ox=thisMultiDetector->offsetX[pos];
      oy=thisMultiDetector->offsetY[pos];
      ret=OK;
    }
  }
  return ret;
}

int multiSlsDetector::setDetectorOffset(int pos, int ox, int oy) {
 
 
  int ret=FAIL;
 
  if (pos>=0 && pos<thisMultiDetector->numberOfDetectors) {
    if (detectors[pos]) {
      if (ox!=-1)
	thisMultiDetector->offsetX[pos]=ox;
      if (oy!=-1) 
	thisMultiDetector->offsetY[pos]=oy;
      ret=OK;
    }
  }
  return ret;
}



int multiSlsDetector::removeSlsDetector(char *name){
  for (int id=0; id<thisMultiDetector->numberOfDetectors; id++) {	
    if (detectors[id]) {						
      if (detectors[id]->getHostname()==string(name)) {			
	removeSlsDetector(id);						
      }									
    }									
  }									
  return thisMultiDetector->numberOfDetectors;
};




int multiSlsDetector::removeSlsDetector(int pos) {
  int j;
  
#ifdef VERBOSE
  cout << "Removing detector in position " << pos << endl;
#endif

  int mi=0, ma=thisMultiDetector->numberOfDetectors, single=0;

  if (pos>=0) {
    mi=pos;
    ma=pos+1;
    single=1;
  }

  //   if (pos<0 )
  //     pos=thisMultiDetector->numberOfDetectors-1;

  if (pos>=thisMultiDetector->numberOfDetectors)
    return thisMultiDetector->numberOfDetectors;

  //j=pos;

  for (j=mi; j<ma; j++) {
    
    if (detectors[j]) {

      thisMultiDetector->dataBytes-=detectors[j]->getDataBytes();
      thisMultiDetector->numberOfChannels-=detectors[j]->getTotalNumberOfChannels();
      thisMultiDetector->maxNumberOfChannels-=detectors[j]->getMaxNumberOfChannels();

      delete detectors[j];
      thisMultiDetector->numberOfDetectors--;
    
    

      if (single) {
	for (int i=j+1; i<thisMultiDetector->numberOfDetectors+1; i++) {
	  detectors[i-1]=detectors[i];
	  thisMultiDetector->detectorIds[i-1]=thisMultiDetector->detectorIds[i];
	}
	detectors[thisMultiDetector->numberOfDetectors]=NULL;
	thisMultiDetector->detectorIds[thisMultiDetector->numberOfDetectors]=-1;
      }
    }
  }

  return thisMultiDetector->numberOfDetectors;
}







 
int multiSlsDetector::setMaster(int i) {

  int ret=-1, slave=0;
  masterFlags f;
#ifdef VERBOSE
  cout << "settin master in position " << i << endl;
#endif
  if (i>=0 && i<thisMultiDetector->numberOfDetectors) {
    if (detectors[i]) {
#ifdef VERBOSE
      cout << "detector position " << i << " ";
#endif
      thisMultiDetector->masterPosition=i;
      detectors[i]->setMaster(IS_MASTER);
    }
    for (int id=0; id<thisMultiDetector->numberOfDetectors; id++) {
      if (i!=id) {
	if (detectors[id]) {
#ifdef VERBOSE
	  cout << "detector position " << id << " ";
#endif
	  detectors[id]->setMaster(IS_SLAVE);
	}
      }
    }

  } else if (i==-2) {
    for (int id=0; id<thisMultiDetector->numberOfDetectors; id++) {
      if (detectors[id]) {
#ifdef VERBOSE
	cout << "detector position " << id << " ";
#endif
	detectors[id]->setMaster(NO_MASTER);
      }
    }
    
  }

  // check return value

  for (int id=0; id<thisMultiDetector->numberOfDetectors; id++) {
    if (detectors[id]) {
#ifdef VERBOSE
      cout << "detector position " << id << " ";
#endif
      f=detectors[id]->setMaster(GET_MASTER);
      switch (f) {
      case NO_MASTER:
	if (ret!=-1)
	  ret=-2;
	break;
      case IS_MASTER:
	if (ret==-1)
	  ret=id;
	else
	  ret=-2;
	break;
      case IS_SLAVE:
	slave=1;
	break;
      default:
	ret=-2;
      }
    }
  }
  if (slave>0 && ret<0)
    ret=-2;
  
  if (ret<0)
    ret=-1;
  
  thisMultiDetector->masterPosition=ret;

  return thisMultiDetector->masterPosition;
}

//   enum synchronyzationMode {
//     GET_SYNCHRONIZATION_MODE=-1, /**< the multidetector will return its synchronization mode */
//     NONE, /**< all detectors are independent (no cabling) */
//     MASTER_GATES, /**< the master gates the other detectors */
//     MASTER_TRIGGERS, /**< the master triggers the other detectors */
//     SLAVE_STARTS_WHEN_MASTER_STOPS /**< the slave acquires when the master finishes, to avoid deadtime */
//   }
  
/** 
    Sets/gets the synchronization mode of the various detectors
    \param sync syncronization mode
    \returns current syncronization mode
*/
slsDetectorDefs::synchronizationMode multiSlsDetector::setSynchronization(synchronizationMode sync) {

  
  synchronizationMode ret=GET_SYNCHRONIZATION_MODE, ret1=GET_SYNCHRONIZATION_MODE;
  
  for (int id=0; id<thisMultiDetector->numberOfDetectors; id++) {
    if (detectors[id]) {
      ret1=detectors[id]->setSynchronization(sync);
      if (id==0)
	ret=ret1;
      else if (ret!=ret1)
	ret=GET_SYNCHRONIZATION_MODE;

    }
  }
  
  thisMultiDetector->syncMode=ret;

  return thisMultiDetector->syncMode;
  
}



























int multiSlsDetector::setOnline(int off) {
  // int retdet;

  if (off!=GET_ONLINE_FLAG) {
    thisMultiDetector->onlineFlag=off;
    for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
      if (detectors[i]){
        detectors[i]->setOnline(off);
	    if(detectors[i]->getErrorMask())
		  setErrorMask(getErrorMask()|(1<<i));
      }
    }
  }
  return thisMultiDetector->onlineFlag;

};



string multiSlsDetector::checkOnline() {
  string retval1 = "",retval;
  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      retval=detectors[idet]->checkOnline();
      if(!retval.empty()){
        retval1.append(retval);
    	retval1.append("+");
      }
    }
  }
  return retval1;
};





int multiSlsDetector::exists() {
  return thisMultiDetector->alreadyExisting;
}




// Initialization functions



  

int multiSlsDetector::getThresholdEnergy(int pos) {
 

  int i, posmin, posmax;
  int ret1=-100, ret;

  if (pos<0) {
    posmin=0;
    posmax=thisMultiDetector->numberOfDetectors;
  } else {
    posmin=pos;
    posmax=pos+1;
  }

  for (i=posmin; i<posmax; i++) {
    if (detectors[i]) {
      ret=detectors[i]->getThresholdEnergy();
      if (ret1==-100)
	ret1=ret;
      else if (ret<(ret1-200) || ret>(ret1+200))
	ret1=FAIL;
      
    }
   
  }
  thisMultiDetector->currentThresholdEV=ret1;
  return ret1;


}  


int multiSlsDetector::setThresholdEnergy(int e_eV, int pos, detectorSettings isettings) {

  int i, posmin, posmax;
  int ret1=-100, ret;

  if (pos<0) {
    posmin=0;
    posmax=thisMultiDetector->numberOfDetectors;
  } else {
    posmin=pos;
    posmax=pos+1;
  }

  for (i=posmin; i<posmax; i++) {
    if (detectors[i]) {
      ret=detectors[i]->setThresholdEnergy(e_eV,-1,isettings);
#ifdef VERBOSE
      cout << "detetcor " << i << " threshold " << ret << endl;
#endif
      if (ret1==-100)
	ret1=ret;
      else if (ret<(ret1-200) || ret>(ret1+200))
	ret1=FAIL;
      
#ifdef VERBOSE
      cout << "return value " << ret1 << endl;
#endif
    }
   
  }
  thisMultiDetector->currentThresholdEV=ret1;
  return ret1;

}
 
slsDetectorDefs::detectorSettings multiSlsDetector::getSettings(int pos) {

  int i, posmin, posmax;
  detectorSettings ret1=GET_SETTINGS, ret;

  if (pos<0) {
    posmin=0;
    posmax=thisMultiDetector->numberOfDetectors;
  } else {
    posmin=pos;
    posmax=pos+1;
  }

  for (i=posmin; i<posmax; i++) {
    if (detectors[i]) {
      ret=detectors[i]->getSettings();
      if (ret1==GET_SETTINGS)
	ret1=ret;
      else if (ret!=ret1)
	ret1=GET_SETTINGS;
      
    }
   
  }
  thisMultiDetector->currentSettings=ret1;
  return ret1;
}

slsDetectorDefs::detectorSettings multiSlsDetector::setSettings(detectorSettings isettings, int pos) {


  int i, posmin, posmax;
  detectorSettings ret1=GET_SETTINGS, ret;

  if (pos<0) {
    posmin=0;
    posmax=thisMultiDetector->numberOfDetectors;
  } else {
    posmin=pos;
    posmax=pos+1;
  }

  for (i=posmin; i<posmax; i++) {
    if (detectors[i]) {
      ret=detectors[i]->setSettings(isettings);
      if (ret1==GET_SETTINGS)
	ret1=ret;
      else if (ret!=ret1)
	ret1=GET_SETTINGS;
      
    }
   
  }
  thisMultiDetector->currentSettings=ret1;
  return ret1;

}



int multiSlsDetector::getChanRegs(double* retval,bool fromDetector){
  //nChansDet and currentNumChans is because of varying channel size per detector
  int n = thisMultiDetector->numberOfChannels,nChansDet,currentNumChans=0;
  double retval1[n];

  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      nChansDet = detectors[idet]->getChanRegs(retval1,fromDetector);

      memcpy(retval + (currentNumChans * sizeof(double)), retval1 , nChansDet*sizeof(double));
      currentNumChans += nChansDet;
    }
  }
  return n;
}



















/* Communication to server */



// Acquisition functions
/* change these funcs accepting also ok/fail */

int multiSlsDetector::startAcquisition(){
 
  int i=0;
  int ret=OK, ret1=OK;
  for (i=0; i<thisMultiDetector->numberOfDetectors; i++) {
    if (i!=thisMultiDetector->masterPosition)
      if (detectors[i]) {
	ret=detectors[i]->startAcquisition();
	if (ret!=OK)
	  ret1=FAIL;
      }
  }
  i=thisMultiDetector->masterPosition;
  if (thisMultiDetector->masterPosition>=0) {
    if (detectors[i]) {
      ret=detectors[i]->startAcquisition();
      if (ret!=OK)
	ret1=FAIL;
    }
  }
  return ret1;
     
};




int multiSlsDetector::stopAcquisition(){

  int i=0;
  int ret=OK, ret1=OK;  
 
  i=thisMultiDetector->masterPosition;
  if (thisMultiDetector->masterPosition>=0) {
    if (detectors[i]) {
      ret=detectors[i]->stopAcquisition();
      if (ret!=OK)
	ret1=FAIL;
    }
  }
  for (i=0; i<thisMultiDetector->numberOfDetectors; i++) {
    if (detectors[i]) {
      ret=detectors[i]->stopAcquisition();
      if (ret!=OK)
	ret1=FAIL;
    }
  }
  *stoppedFlag=1;
  return ret1;


};

int multiSlsDetector::startReadOut(){

  int i=0;
  int ret=OK, ret1=OK;
  i=thisMultiDetector->masterPosition;
  if (i>=0) {
    if (detectors[i]) {
      ret=detectors[i]->startReadOut();
      if (ret!=OK)
	ret1=FAIL;
    }
  }
  for (i=0; i<thisMultiDetector->numberOfDetectors; i++) {
    if (detectors[i]) {
      ret=detectors[i]->startReadOut();
      if (ret!=OK)
	ret1=FAIL;
    }
  }

  return ret1;
  

};



int* multiSlsDetector::getDataFromDetector() {

  int nel=thisMultiDetector->dataBytes/sizeof(int);
  int n;
  int* retval=new int[nel];
  int *retdet, *p=retval;
  int nodata=1, nodatadet=-1;;
 

  for (int id=0; id<thisMultiDetector->numberOfDetectors; id++) {
    if (detectors[id]) {
      retdet=detectors[id]->getDataFromDetector(p);
      n=detectors[id]->getDataBytes();
      if (retdet) {
	nodata=0;
#ifdef VERBOSE
	cout << "Detector " << id << " returned " << n << " bytes " << endl;
#endif
      } else {
	nodatadet=id;
#ifdef VERBOSE
	cout << "Detector " << id << " does not have data left " << endl;
#endif
	break;
      }      
      p+=n/sizeof(int);
    }
  }
  if (nodatadet>=0) {
    for (int id=0; id<thisMultiDetector->numberOfDetectors; id++) {
      if (id!=nodatadet) {
	if (detectors[id]) {
#ifdef VERBOSE
	  cout << "Stopping detector "<< id << endl;
#endif
	  detectors[id]->stopAcquisition();
	  while ((retdet=detectors[id]->getDataFromDetector())) {  
#ifdef VERBOSE
	    cout << "Detector "<< id << " still sent data " << endl;
#endif
	    delete [] retdet;
	  }
	}
      }
    }
    delete [] retval;
    return NULL;
  }

  return retval;
};



int* multiSlsDetector::readFrame(){
  int nel=thisMultiDetector->dataBytes/sizeof(int);
  int n;
  int* retval=new int[nel];
  int *retdet, *p=retval;
  
  /** probably it's always better to have one integer per channel in any case! */

  for (int id=0; id<thisMultiDetector->numberOfDetectors; id++) {
    if (detectors[id]) {
      retdet=detectors[id]->readFrame();
      if (retdet) {
	n=detectors[id]->getDataBytes();
	memcpy(p,retdet,n);
	delete [] retdet;
	p+=n/sizeof(int);
      } else {
#ifdef VERBOSE
	cout << "Detector " << id << " does not have data left " << endl;
#endif
	delete [] retval;
	return NULL;
      }
    }
  }
  dataQueue.push(retval);
  return retval;

};



int* multiSlsDetector::readAll(){
  
  /** Thread for each detector?!?!?! */

  // int fnum=F_READ_ALL;
  int* retval; // check what we return!
  // int ret=OK, ret1=OK;

  int i=0;
#ifdef VERBOSE
  std::cout<< "Reading all frames "<< std::endl;
#endif
  if (thisMultiDetector->onlineFlag==ONLINE_FLAG) {
    
    for (int id=0; id<thisMultiDetector->numberOfDetectors; id++) {
      if (detectors[id]) {
	detectors[id]->readAllNoWait();
      }
    }
    while ((retval=getDataFromDetector())){
      i++;
#ifdef VERBOSE
      std::cout<< i << std::endl;
      //#else
      //std::cout << "-" << flush;
#endif
      dataQueue.push(retval);
    }
    for (int id=0; id<thisMultiDetector->numberOfDetectors; id++) {
      if (detectors[id]) {
	detectors[id]->disconnectControl();
      }
    }  
 
  }

#ifdef VERBOSE
  std::cout<< "received "<< i<< " frames" << std::endl;
  //#else
  // std::cout << std::endl; 
#endif
  return dataQueueFront(); // check what we return!

};

int* multiSlsDetector::startAndReadAll(){

  /** Thread for each detector?!?!?! */
#ifdef VERBOSE
  cout << "Start and read all " << endl;
#endif


  int* retval;
  int i=0;
  if (thisMultiDetector->onlineFlag==ONLINE_FLAG) {
    
    startAndReadAllNoWait();
   
    while ((retval=getDataFromDetector())){
      i++;
#ifdef VERBOSE
      std::cout<< i << std::endl;
      //#else
      //std::cout << "-" << flush;
#endif
      dataQueue.push(retval);
    }

    for (int id=0; id<thisMultiDetector->numberOfDetectors; id++) {
      if (detectors[id]) {
	detectors[id]->disconnectControl();
      }
    }  
 
  }

#ifdef VERBOSE
  std::cout<< "MMMM recieved "<< i<< " frames" << std::endl;
  //#else
  // std::cout << std::endl; 
#endif
  return dataQueueFront(); // check what we return!

  
};


int multiSlsDetector::startAndReadAllNoWait(){


  int i=0;
  int ret=OK, ret1=OK;

  for (i=0; i<thisMultiDetector->numberOfDetectors; i++) {
    if (i!=thisMultiDetector->masterPosition)
      if (detectors[i]) {
	ret=detectors[i]->startAndReadAllNoWait();
	if (ret!=OK)
	  ret1=FAIL;
      }
  }
  i=thisMultiDetector->masterPosition;
  if (thisMultiDetector->masterPosition>=0) {
    if (detectors[i]) {
      ret=detectors[i]->startAndReadAllNoWait();
      if (ret!=OK)
	ret1=FAIL;
    }
  }
  return ret1;

}


/**
   get run status
   \returns status mask
*/
slsDetectorDefs::runStatus  multiSlsDetector::getRunStatus() {

  runStatus s,s1;

  if (thisMultiDetector->masterPosition>=0)
    if (detectors[thisMultiDetector->masterPosition])
      return detectors[thisMultiDetector->masterPosition]->getRunStatus();


  if (detectors[0]) s=detectors[0]->getRunStatus(); 

  for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
    s1=detectors[i]->getRunStatus(); 
    if (s1==ERROR)
      s=ERROR;
    if (s1==IDLE && s!=IDLE)
      s=ERROR;
    
  }
  return s;
}


int* multiSlsDetector::popDataQueue() {
  int *retval=NULL;
  if( !dataQueue.empty() ) {
    retval=dataQueue.front();
    dataQueue.pop();
  }
  return retval;
}

detectorData* multiSlsDetector::popFinalDataQueue() {
  detectorData *retval=NULL;
  if( !finalDataQueue.empty() ) {
    retval=finalDataQueue.front();
    finalDataQueue.pop();
  }
  return retval;
}

void multiSlsDetector::resetDataQueue() {
  int *retval=NULL;
  while( !dataQueue.empty() ) {
    retval=dataQueue.front();
    dataQueue.pop();
    delete [] retval;
  }
 
}

void multiSlsDetector::resetFinalDataQueue() {
  detectorData *retval=NULL;
  while( !finalDataQueue.empty() ) {
    retval=finalDataQueue.front();
    finalDataQueue.pop();
    delete retval;
  }

}



/* 
   set or read the acquisition timers 
   enum timerIndex {
   FRAME_NUMBER,
   ACQUISITION_TIME,
   FRAME_PERIOD,
   DELAY_AFTER_TRIGGER,
   GATES_NUMBER,
   PROBES_NUMBER
   CYCLES_NUMBER,
   GATE_INTEGRATED_TIME
   }
*/
int64_t multiSlsDetector::setTimer(timerIndex index, int64_t t){
  int i;
  int64_t ret1=-100, ret;


  
  for (i=0; i<thisMultiDetector->numberOfDetectors; i++) {
    if (detectors[i]) {
      ret=detectors[i]->setTimer(index,t);
      if (ret1==-100)
	ret1=ret;
      else if (ret!=ret1)
	ret1=FAIL;
      
    }
  }
  // check return values!!!
  
  thisMultiDetector->timerValue[index]=ret1;
  
  return ret1;
};

 
int64_t multiSlsDetector::getTimeLeft(timerIndex index){
  int i;
  int64_t ret1=-100, ret;
  

  if (thisMultiDetector->masterPosition>=0)
    if (detectors[thisMultiDetector->masterPosition])
      return detectors[thisMultiDetector->masterPosition]->getTimeLeft(index);
  
  
  for (i=0; i<thisMultiDetector->numberOfDetectors; i++) {
    if (detectors[i]) {
      ret=detectors[i]->getTimeLeft(index);
      if (ret1==-100)
	ret1=ret;
      else if (ret!=ret1)
	ret1=-1;
    }
  }
  
  return ret1;
  
}



int multiSlsDetector::setSpeed(speedVariable index, int value){
  int i;
  int64_t ret1=-100, ret;
  

  
  for (i=0; i<thisMultiDetector->numberOfDetectors; i++) {
    if (detectors[i]) {
      ret=detectors[i]->setSpeed(index,value);
      if (ret1==-100)
	ret1=ret;
      else if (ret!=ret1)
	ret1=FAIL;
    }
  }
  
  return ret1;
  
}













int  multiSlsDetector::getDataBytes(){
  int ret=0;   
  for (int ip=0; ip<thisMultiDetector->numberOfDetectors; ip++) {
    if (detectors[ip]) 
      ret+=detectors[ip]->getDataBytes();
  }
  return ret;
}






// Flags
int multiSlsDetector::setDynamicRange(int n, int pos){

  //  cout << "multi "  << endl;
  int imi, ima, i;
  int ret, ret1=-100;

  if (pos<0) {
    imi=0;
    ima=thisMultiDetector->numberOfDetectors;
  } else {
    imi=pos;
    ima=pos+1;
  }
 
  for (i=imi; i<ima; i++) {
    //  cout << "multi ************ detector " << i << endl;

    if (detectors[i]) {
      thisMultiDetector->dataBytes-=detectors[i]->getDataBytes();
      ret=detectors[i]->setDynamicRange(n);
      if (ret1==-100)
	ret1=ret;
      else if (ret!=ret1)
	ret1=FAIL;
      thisMultiDetector->dataBytes+=detectors[i]->getDataBytes();
    }
  }

  return thisMultiDetector->dataBytes;
};

/*

int multiSlsDetector::setROI(int nroi, int *xmin, int *xmax, int *ymin, int *ymax){


};
*/


double* multiSlsDetector::decodeData(int *datain, double *fdata) {
  double *dataout;
  if (fdata)
    dataout=fdata;
  else
    dataout=new double[thisMultiDetector->numberOfChannels];
  
  // int ich=0;

  double *detp=dataout;
  int  *datap=datain;

  
  for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
    if (detectors[i]) {
      detectors[i]->decodeData(datap, detp);
#ifdef VERBOSE
      cout << "increment pointers " << endl;
#endif
      datap+=detectors[i]->getDataBytes()/sizeof(int);
      detp+=detectors[i]->getTotalNumberOfChannels();

#ifdef VERBOSE
      cout << "done " << endl;
#endif
      //       for (int j=0; j<detectors[i]->getTotalNumberOfChannels(); j++) {
      // 	dataout[ich]=detp[j];
      // 	ich++;
      //       }
      //delete [] detp;
    }
  }

  return dataout;
}
 
//Correction
/*
  enum correctionFlags {
  DISCARD_BAD_CHANNELS,
  AVERAGE_NEIGHBOURS_FOR_BAD_CHANNELS,
  FLAT_FIELD_CORRECTION,
  RATE_CORRECTION,
  ANGULAR_CONVERSION
  }
*/


///////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////














int multiSlsDetector::setFlatFieldCorrection(string fname){
  double data[thisMultiDetector->numberOfChannels];//,  xmed[thisMultiDetector->numberOfChannels];
  double ffcoefficients[thisMultiDetector->numberOfChannels], fferrors[thisMultiDetector->numberOfChannels];
  // int nmed=0;
  // int idet=0, ichdet=-1;
  char ffffname[MAX_STR_LENGTH*2];
  int  nch;//nbad=0,
  //int badlist[MAX_BADCHANS];
  int im=0;

  if (fname=="default") {
    fname=string(thisMultiDetector->flatFieldFile);
  }	
  

  thisMultiDetector->correctionMask&=~(1<<FLAT_FIELD_CORRECTION);
  
  
  
  if (fname=="") {
#ifdef VERBOSE
    std::cout<< "disabling flat field correction" << std::endl;
#endif
    thisMultiDetector->correctionMask&=~(1<<FLAT_FIELD_CORRECTION);
    //  strcpy(thisMultiDetector->flatFieldFile,"none");
    for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
      if (detectors[i])
	detectors[i]->setFlatFieldCorrection(NULL, NULL);
    }
  } else { 
#ifdef VERBOSE
    std::cout<< "Setting flat field correction from file " << fname << std::endl;
#endif
    sprintf(ffffname,"%s/%s",thisMultiDetector->flatFieldDir,fname.c_str());
    nch=readDataFile(string(ffffname),data);
    
    if (nch>thisMultiDetector->numberOfChannels)
      nch=thisMultiDetector->numberOfChannels;
    
    if (nch>0) {
      
      //???? bad ff chans?
      int nm=getNMods();
      int chpm[nm];
      int mMask[nm];
      for (int i=0; i<nm; i++) {
	chpm[im]=getChansPerMod(im);
	mMask[im]=im;
      }
      fillModuleMask(mMask);

      if ((postProcessingFuncs::calculateFlatField(&nm, chpm, mMask, badChannelMask, data, ffcoefficients, fferrors))>=0) {
	strcpy(thisMultiDetector->flatFieldFile,fname.c_str());
	
	
	thisMultiDetector->correctionMask|=(1<<FLAT_FIELD_CORRECTION);
	
	setFlatFieldCorrection(ffcoefficients, fferrors);
	
      } else
	std::cout<< "Calculated flat field from file " << fname << " is not valid " << nch << std::endl;
    } else {
      std::cout<< "Flat field from file " << fname << " is not valid " << nch << std::endl;
    }
    
  }
  return thisMultiDetector->correctionMask&(1<<FLAT_FIELD_CORRECTION);
}

int multiSlsDetector::fillModuleMask(int *mM) {
  int imod=0, off=0;
  if (mM) {
    for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
      if (detectors[i]) {
	for (int im=0; im<detectors[i]->getNMods(); im++) {
	  mM[imod]=im+off;
	  imod++;
	}
	off+=detectors[i]->getMaxMods();
      }
      
      
    }
  }
  return getNMods();
}



int multiSlsDetector::setFlatFieldCorrection(double *corr, double *ecorr) {
  int ichdet=0;
  double *p, *ep;
  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      if (corr!=NULL)
	p=corr+ichdet;
      else
	p=NULL;
      if (ecorr!=NULL)
	ep=ecorr+ichdet;
      else
	ep=NULL;
      detectors[idet]->setFlatFieldCorrection(p, ep);
      ichdet+=detectors[idet]->getTotalNumberOfChannels();
    }
  }
  return 0;
}









int multiSlsDetector::getFlatFieldCorrection(double *corr, double *ecorr) {
  int ichdet=0;
  double *p, *ep;
  int ff=1, dff;
  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      if (corr!=NULL)
	p=corr+ichdet;
      else
	p=NULL;
      if (ecorr!=NULL)
	ep=ecorr+ichdet;
      else
	ep=NULL;
      dff=detectors[idet]->getFlatFieldCorrection(p, ep);
      if (dff==0)
	ff=0;
      ichdet+=detectors[idet]->getTotalNumberOfChannels();
    }
  }
  return ff;
}















int multiSlsDetector::getNMods(){
  int nm=0;
  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      nm+=detectors[idet]->getNMods();
    }
  }
#ifdef VERBOSE
  cout << "total number of modules is " << nm << endl;
#endif

  return nm;
}


int multiSlsDetector::getChansPerMod(int imod){
  int id=-1, im=-1;
#ifdef VERBOSE
  cout << "get chans per mod " << imod << endl;
#endif
  decodeNMod(imod, id, im);
  if (id >=0) {
    if (detectors[id]) {
      return detectors[id]->getChansPerMod(im);
    }
  }
  return -1;

}

int  multiSlsDetector::getMoveFlag(int imod){
  int id=-1, im=-1;
  decodeNMod(imod, id, im);
  if (id>=0) {
    if (detectors[id]) {
      return detectors[id]->getMoveFlag(im);
    }
  }
  //default!!!
  return 1;
}




angleConversionConstant * multiSlsDetector::getAngularConversionPointer(int imod){
  int id=-1, im=-1;
#ifdef VERBOSE
  cout << "get angular conversion pointer " << endl;
#endif
  if (decodeNMod(imod, id, im)>=0) {
    if (detectors[id]) {
      return detectors[id]->getAngularConversionPointer(im);
    }
  }
  return NULL;

}



int multiSlsDetector::flatFieldCorrect(double* datain, double *errin, double* dataout, double *errout){

  int ichdet=0;
  double *perr=errin;//*pdata,
  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
#ifdef VERBOSE
      cout << " detector " << idet << " offset " << ichdet << endl;
#endif
      if (errin)
	perr+=ichdet;
      detectors[idet]->flatFieldCorrect(datain+ichdet, perr, dataout+ichdet, errout+ichdet);
      ichdet+=detectors[idet]->getTotalNumberOfChannels();//detectors[idet]->getNChans()*detectors[idet]->getNChips()*detectors[idet]->getNMods();
    }
  }
  return 0;
};








int multiSlsDetector::setRateCorrection(double t){
  // double tdead[]=defaultTDead;

  if (t==0) {
    thisMultiDetector->correctionMask&=~(1<<RATE_CORRECTION);
  } else {
    thisMultiDetector->correctionMask|=(1<<RATE_CORRECTION);
    
    for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
      
      if (detectors[idet]) {
	detectors[idet]->setRateCorrection(t);
      }
    }
#ifdef VERBOSE
    std::cout<< "Setting rate correction with dead time "<< thisMultiDetector->tDead << std::endl;
#endif
  }
  return thisMultiDetector->correctionMask&(1<<RATE_CORRECTION);
}


int multiSlsDetector::getRateCorrection(double &t){

  if (thisMultiDetector->correctionMask&(1<<RATE_CORRECTION)) {
#ifdef VERBOSE
    std::cout<< "Rate correction is enabled with dead time "<< thisMultiDetector->tDead << std::endl;
#endif
    //which t should we return if they are all different?
    return 1;
  } else
    t=0;
#ifdef VERBOSE
  std::cout<< "Rate correction is disabled " << std::endl;
#endif
  return 0;
};

double multiSlsDetector::getRateCorrectionTau(){

  double ret1=-100,ret;
  if (thisMultiDetector->correctionMask&(1<<RATE_CORRECTION)) {
#ifdef VERBOSE
    std::cout<< "Rate correction is enabled with dead time "<< thisMultiDetector->tDead << std::endl;
#endif
    //which t should we return if they are all different?

    for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
      if (detectors[idet]) {
        ret=detectors[idet]->getRateCorrectionTau();
    	if (ret1==-100)
    	  ret1=ret;
    	else if (ret!=ret1)
    	  ret1=-1;
      }
    }
  } else {
#ifdef VERBOSE
    std::cout<< "Rate correction is disabled " << std::endl;
#endif
    ret1=0;
  }
  return ret1;

};



int multiSlsDetector::getRateCorrection(){

  if (thisMultiDetector->correctionMask&(1<<RATE_CORRECTION)) {
    return 1;
  } else
    return 0;
};




int multiSlsDetector::rateCorrect(double* datain, double *errin, double* dataout, double *errout){

  int ichdet=0;
  double *perr=errin;
  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      if (errin)
	perr+=ichdet;
      detectors[idet]->rateCorrect(datain+ichdet, perr, dataout+ichdet, errout+ichdet);
      ichdet+=detectors[idet]->getTotalNumberOfChannels();
    }
  }
  return 0;
};


int multiSlsDetector::setBadChannelCorrection(string fname){

  int badlist[MAX_BADCHANS];// badlistdet[MAX_BADCHANS];
  int nbad=0;//, nbaddet=0, choff=0, idet=0;
  int ret=0;


  if (fname=="default")
    fname=string(thisMultiDetector->badChanFile);
  
  
  


  ret=setBadChannelCorrection(fname, nbad, badlist);
#ifdef VERBOSE
  cout << "file contained " << ret << " badchans" << endl; 
#endif
  if (ret==0) {
    thisMultiDetector->correctionMask&=~(1<<DISCARD_BAD_CHANNELS);
    nbad=0;
  } else {
    thisMultiDetector->correctionMask|=(1<<DISCARD_BAD_CHANNELS);
    strcpy(thisMultiDetector->badChanFile,fname.c_str());
  }

  return setBadChannelCorrection(nbad,badlist,0);

}



int multiSlsDetector::setBadChannelCorrection(int nbad, int *badlist, int ff) {
  

  int  badlistdet[MAX_BADCHANS];
  int nbaddet=0, choff=0, idet=0;

  if (nbad>0) {
    thisMultiDetector->correctionMask|=(1<<DISCARD_BAD_CHANNELS);
    
    for (int ich=0; ich<nbad; ich++) {
      if (detectors[idet]) {
	if ((badlist[ich]-choff)>=detectors[idet]->getMaxNumberOfChannels()) {
#ifdef VERBOSE
	  cout << "setting " << nbaddet << " badchans to detector " << idet << endl;
#endif
	  detectors[idet]->setBadChannelCorrection(nbaddet,badlist,0);
	  choff+=detectors[idet]->getMaxNumberOfChannels();
	  nbaddet=0;
	  idet++;
	  if (detectors[idet]==NULL)
	    break;
	}
	badlistdet[nbaddet]=(badlist[ich]-choff);
	nbaddet++;
#ifdef VERBOSE
	cout << nbaddet << " " << badlist[ich] << " " << badlistdet[nbaddet-1] << endl;
#endif
      }
    }
    if (nbaddet>0) {
      
      if (detectors[idet]) {
#ifdef VERBOSE
	cout << "setting " << nbaddet << " badchans to detector " << idet << endl;
#endif
	detectors[idet]->setBadChannelCorrection(nbaddet,badlist,0);
	choff+=detectors[idet]->getMaxNumberOfChannels();
	nbaddet=0;
	idet++;
      }
    }
    nbaddet=0;
    for (int i=idet; i<thisMultiDetector->numberOfDetectors; i++) {
#ifdef VERBOSE
      cout << "setting " << 0 << " badchans to detector " << i << endl;
#endif
      if (detectors[i]) {
	detectors[i]->setBadChannelCorrection(nbaddet,badlist,0);
      }
    }
    
  } else {
    nbaddet=0;
    for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
      if (detectors[idet]) {
#ifdef VERBOSE
	cout << "setting " << 0 << " badchans to detector " << idet << endl;
#endif
	detectors[idet]->setBadChannelCorrection(nbaddet,badlist,0);
      }
    }
    thisMultiDetector->correctionMask&=~(1<<DISCARD_BAD_CHANNELS);
  }  
#ifdef VERBOSE 
  cout << (thisMultiDetector->correctionMask&(1<<DISCARD_BAD_CHANNELS)) << endl;
#endif
  return thisMultiDetector->correctionMask&(1<<DISCARD_BAD_CHANNELS);

}






int multiSlsDetector::readAngularConversionFile(string fname) {

  
  ifstream infile;
  //int nm=0;
  infile.open(fname.c_str(), ios_base::in);
  if (infile.is_open()) {

    for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
      if (detectors[idet]) {
#ifdef VERBOSE
	cout << " detector " << idet << endl;
#endif
	detectors[idet]->readAngularConversion(infile);
      }
    }
    infile.close();
  } else {
    std::cout<< "Could not open calibration file "<< fname << std::endl;
    return -1;
  }
  return 0;

}


int multiSlsDetector::writeAngularConversion(string fname) {

  
  ofstream outfile;
  //  int nm=0;
  outfile.open(fname.c_str(), ios_base::out);
  if (outfile.is_open()) {

    for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
      if (detectors[idet]) {
	detectors[idet]->writeAngularConversion(outfile);
      }
    }
    outfile.close();
  } else {
    std::cout<< "Could not open calibration file "<< fname << std::endl;
    return -1;
  }
  return 0;

}

int  multiSlsDetector::getAngularConversion(int &direction,  angleConversionConstant *angconv) {

  int dir=-100, dir1;
  angleConversionConstant *a1=angconv;

  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      detectors[idet]->getAngularConversion(dir1,a1);
      if (dir==-100)
	dir = dir1;
      if (dir!=dir1)
	dir=0;
      if (angconv) {
	a1+=detectors[idet]->getNMods();
      }
    }
  }
  direction=dir;
  
  if (thisMultiDetector->correctionMask&(1<< ANGULAR_CONVERSION)) {
    return 1;
  } 
  return 0;
 

}



dacs_t multiSlsDetector::setDAC(dacs_t val, dacIndex idac, int imod) {
  dacs_t ret, ret1=-100;
  
  int id=-1, im=-1;
  int dmi=0, dma=thisMultiDetector->numberOfDetectors;
  
  if (decodeNMod(imod, id, im)>=0) {
    dmi=id;
    dma=dma+1;
  }

  for (int idet=dmi; idet<dma; idet++) {
    if (detectors[idet]) {
      ret=detectors[idet]->setDAC(val, idac, im);
      if (ret1==-100)
	ret1=ret;
      else if (ret!=ret1)
	ret1=-1;
    }
  }
  return ret1;
}

dacs_t multiSlsDetector::getADC(dacIndex idac, int imod) {
  dacs_t ret, ret1=-100;
  
  int id=-1, im=-1;
  int dmi=0, dma=thisMultiDetector->numberOfDetectors;
  
  if (decodeNMod(imod, id, im)>=0) {
    dmi=id;
    dma=dma+1;
  }

  for (int idet=dmi; idet<dma; idet++) {
    if (detectors[idet]) {
      ret=detectors[idet]->getADC(idac, im);
      if (ret1==-100)
	ret1=ret;
      else if (ret!=ret1)
	ret1=-1;
    }
  }
  return ret1;
}

int multiSlsDetector::setChannel(int64_t reg, int ichan, int ichip, int imod) {
  int ret, ret1=-100;
  int id=-1, im=-1;
  int dmi=0, dma=thisMultiDetector->numberOfDetectors;
  
  if (decodeNMod(imod, id, im)>=0) {
    dmi=id;
    dma=dma+1;
  }
  for (int idet=dmi; idet<dma; idet++) {
    if (detectors[idet]) {
      ret=detectors[idet]->setChannel(reg, ichan, ichip, im);
      if (ret1==-100)
	ret1=ret;
      else if (ret!=ret1)
	ret1=-1;
    }
  }
  return ret1;
   
}



/**
   sets the value of s angular conversion parameter
   \param c can be ANGULAR_DIRECTION, GLOBAL_OFFSET, FINE_OFFSET, BIN_SIZE
   \param v the value to be set
   \returns the actual value
*/

double multiSlsDetector::setAngularConversionParameter(angleConversionParameter c, double v) {
  double ret=slsDetectorUtils::setAngularConversionParameter(c,v);
  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    
    if (detectors[idet]) {
	
      detectors[idet]->setAngularConversionParameter(c,v);
    }
  }
  return ret;
}



// double* multiSlsDetector::convertAngles(double pos) {
//   double *ang=new double[thisMultiDetector->numberOfChannels];

//   double *p=ang;
//   int choff=0;

//   for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    
//     if (detectors[idet]) {
// #ifdef EPICS
//       //  cout << "convert angle det " << idet << endl;
//       if (idet<2)
// #endif
// 	p=detectors[idet]->convertAngles(pos);
// #ifdef EPICS
//       else //////////// GOOD ONLY AT THE BEAMLINE!!!!!!!!!!!!!
// 	p=detectors[idet]->convertAngles(0);
// #endif
//       for (int ich=0; ich<detectors[idet]->getTotalNumberOfChannels(); ich++) {
// 	ang[choff+ich]=p[ich];
//       }
//       choff+=detectors[idet]->getTotalNumberOfChannels();
//       delete [] p;
//     }
//   }
//   return ang;
// }


int multiSlsDetector::getBadChannelCorrection(int *bad) {
  //int ichan;
  int *bd, nd, ntot=0, choff=0;;

  
  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      nd=detectors[idet]->getBadChannelCorrection();
      bd = new int[nd];
      nd=detectors[idet]->getBadChannelCorrection(bd);
      for (int id=0; id<nd; id++) {
	if (bd[id]<detectors[idet]->getTotalNumberOfChannels()) {
	  if (bad) bad[ntot]=choff+bd[id];
	  ntot++;
	}
      }
      choff+=detectors[idet]->getTotalNumberOfChannels();
      delete [] bd;
    }
  }
  return ntot;

}


int multiSlsDetector::exitServer() {

  int ival=FAIL, iv;
  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      iv=detectors[idet]->exitServer();
      if (iv==OK)
	ival=iv;
    }
  }
  return ival;
}


/** returns the detector trimbit/settings directory  */
char* multiSlsDetector::getSettingsDir() {
  string s0="", s1="", s;
  

  //char ans[1000];
  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      s=detectors[idet]->getSettingsDir();

      if (s0=="")
	s0=s;
      else
	s0+=string("+")+s;
      if (s1=="")
	s1=s;
      else if (s1!=s)
	s1="bad";
    }
  }
  if (s1=="bad")
    strcpy(ans,s0.c_str());
  else
    strcpy(ans,s1.c_str());
  return ans;
}



/** sets the detector trimbit/settings directory  \sa sharedSlsDetector */
char* multiSlsDetector::setSettingsDir(string s){

  if (s.find('+')==string::npos) {
    for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
      if (detectors[idet]) {
	detectors[idet]->setSettingsDir(s);
      }
    }
  } else {
    size_t p1=0;
    size_t p2=s.find('+',p1);
    int id=0;
    while (p2!=string::npos) {

      if (detectors[id]) {
	detectors[id]->setSettingsDir(s.substr(p1,p2-p1));
      }
      id++;
      s=s.substr(p2+1);
      p2=s.find('+');
      if (id>=thisMultiDetector->numberOfDetectors)
	break;
    }

  }
  return getSettingsDir();


}




int multiSlsDetector::setTrimEn(int ne, int *ene) {

  int ret=-100, ret1;

  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      ret1=detectors[idet]->setTrimEn(ne,ene);
      if (ret==-100)
	ret=ret1;
      else if (ret!=ret1)
	ret=-1;
    }
  }
  return ret;

}



int multiSlsDetector::getTrimEn(int *ene) {

  int ret=-100, ret1;

  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      ret1=detectors[idet]->getTrimEn(ene);
      if (ret==-100)
	ret=ret1;
      else if (ret!=ret1)
	ret=-1;
    }
  }
  return ret;

}









/**
   returns the location of the calibration files
   \sa  sharedSlsDetector
*/
char* multiSlsDetector::getCalDir() {
  string s0="", s1="", s;
  //char ans[1000];
  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      s=detectors[idet]->getCalDir();

      if (s0=="")
	s0=s;
      else
	s0+=string("+")+s;
      if (s1=="")
	s1=s;
      else if (s1!=s)
	s1="bad";
    }
  }
  if (s1=="bad")
    strcpy(ans,s0.c_str());
  else
    strcpy(ans,s1.c_str());
  return ans;
}


/**
   sets the location of the calibration files
   \sa  sharedSlsDetector
*/
char* multiSlsDetector::setCalDir(string s){

  if (s.find('+')==string::npos) {
    for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
      if (detectors[idet]) {
	detectors[idet]->setCalDir(s);
      }
    }
  } else {
    size_t p1=0;
    size_t p2=s.find('+',p1);
    int id=0;
    while (p2!=string::npos) {

      if (detectors[id]) {
	detectors[id]->setCalDir(s.substr(p1,p2-p1));
      }
      id++;
      s=s.substr(p2+1);
      p2=s.find('+');
      if (id>=thisMultiDetector->numberOfDetectors)
	break;
    }

  }
  return getCalDir();

} 

/**
   returns the location of the calibration files
   \sa  sharedSlsDetector
*/
char* multiSlsDetector::getNetworkParameter(networkParameter p) {
  string s0="", s1="",s ;
  
  //char ans[1000];
  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      s=detectors[idet]->getNetworkParameter(p);

      if (s0=="")
	s0=s;
      else
	s0+=string("+")+s;
      if (s1=="")
	s1=s;
      else if (s1!=s)
	s1="bad";
    }
  }
  if (s1=="bad")
    strcpy(ans,s0.c_str());
  else
    strcpy(ans,s1.c_str());
  return ans;
}


/**
   sets the location of the calibration files
   \sa  sharedSlsDetector
*/
char* multiSlsDetector::setNetworkParameter(networkParameter p, string s){

  if (s.find('+')==string::npos) {
    for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
      if (detectors[idet]) {
	    detectors[idet]->setNetworkParameter(p,s);
	    if(detectors[idet]->getErrorMask())
		  setErrorMask(getErrorMask()|(1<<idet));
      }
    }
  } else {
    size_t p1=0;
    size_t p2=s.find('+',p1);
    int id=0;
    while (p2!=string::npos) {

      if (detectors[id]) {
	detectors[id]->setCalDir(s.substr(p1,p2-p1));
      }
      id++;
      s=s.substr(p2+1);
      p2=s.find('+');
      if (id>=thisMultiDetector->numberOfDetectors)
	break;
    }

  }
  return getNetworkParameter(p);

} 

int multiSlsDetector::setPort(portType t, int p) {

  int ret=-100, ret1;

  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      ret1=detectors[idet]->setPort(t,p);
      if(detectors[idet]->getErrorMask())
    	  setErrorMask(getErrorMask()|(1<<idet));
      if (ret==-100)
	ret=ret1;
      else if (ret!=ret1)
	ret=-1;
    }
  }
  return ret;

}

int multiSlsDetector::lockServer(int p) {

  int ret=-100, ret1;

  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      ret1=detectors[idet]->lockServer(p);
      if (ret==-100)
	ret=ret1;
      else if (ret!=ret1)
	ret=-1;
    }
  }

  return ret;

}

string multiSlsDetector::getLastClientIP() {
  string s0="", s1="",s ;
  
  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      s=detectors[idet]->getLastClientIP();

      if (s0=="")
	s0=s;
      else
	s0+=string("+")+s;
      if (s1=="")
	s1=s;
      else if (s1!=s)
	s1="bad";
    }
  }
  if (s1=="bad")
    return s0;
  else
    return s1;
}






int multiSlsDetector::setReadOutFlags(readOutFlags flag) {

  int ret=-100, ret1;

  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      ret1=detectors[idet]->setReadOutFlags(flag);
      if (ret==-100)
	ret=ret1;
      else if (ret!=ret1)
	ret=-1;
    }
  }

  return ret;


}


slsDetectorDefs::externalCommunicationMode multiSlsDetector::setExternalCommunicationMode(externalCommunicationMode pol) {

  externalCommunicationMode  ret, ret1;

  if (detectors[0])
    ret=detectors[0]->setExternalCommunicationMode(pol);

  
  for (int idet=1; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      ret1=detectors[idet]->setExternalCommunicationMode(pol);
      if (ret!=ret1)
	ret=GET_EXTERNAL_COMMUNICATION_MODE;
    }
  }

  setMaster();
  setSynchronization();
  return ret;

}



slsDetectorDefs::externalSignalFlag multiSlsDetector::setExternalSignalFlags(externalSignalFlag pol, int signalindex) {

  externalSignalFlag  ret, ret1;

  if (detectors[0])
    ret=detectors[0]->setExternalSignalFlags(pol,signalindex);

  for (int idet=1; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      ret1=detectors[idet]->setExternalSignalFlags(pol,signalindex);
      if (ret!=ret1)
	ret=GET_EXTERNAL_SIGNAL_FLAG;
    }
  }

  setMaster();
  setSynchronization();
  return ret;


}












const char * multiSlsDetector::getSettingsFile() {

  string s0="", s1="",s ;
  
  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      s=detectors[idet]->getSettingsFile();

      if (s0=="")
	s0=s;
      else
	s0+=string("+")+s;
      if (s1=="")
	s1=s;
      else if (s1!=s)
	s1="bad";
    }
  }
  if (s1=="bad")
    return s0.c_str();
  else
    return s1.c_str();

}


int multiSlsDetector::configureMAC(int adc) {

  int ret=-100, ret1;

  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      ret1=detectors[idet]->configureMAC(adc);
	  if(detectors[idet]->getErrorMask())
		setErrorMask(getErrorMask()|(1<<idet));
      if (ret==-100)
	ret=ret1;
      else if (ret!=ret1)
	ret=-1;
    }
  }

  return ret;

}


int multiSlsDetector::loadImageToDetector(imageType index,string const fname){

  int ret=-100, ret1;
  short int imageVals[thisMultiDetector->numberOfChannels];

  ifstream infile;
  infile.open(fname.c_str(), ios_base::in);
  if (infile.is_open()) {
#ifdef VERBOSE
    std::cout<< std::endl<< "Loading ";
    if(!index)
      std::cout<<"Dark";
    else
      std::cout<<"Gain";
    std::cout<<" image from file " << fname << std::endl;
#endif
    for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
      if (detectors[idet]) {
	if(detectors[idet]->readDataFile(infile,imageVals)>=0){
	  ret1=detectors[idet]->sendImageToDetector(index,imageVals);
	  if (ret==-100)
	    ret=ret1;
	  else if (ret!=ret1)
	    ret=-1;
	}
      }
    }
    infile.close();
  } else {
    std::cout<< "Could not open file "<< fname << std::endl;
    return -1;
  }
  return ret;
}

int multiSlsDetector::writeCounterBlockFile(string const fname,int startACQ){

  int ret=OK, ret1=OK;
  short int arg[thisMultiDetector->numberOfChannels];
  ofstream outfile;
  outfile.open(fname.c_str(), ios_base::out);
  if (outfile.is_open()) {
#ifdef VERBOSE
    std::cout<< std::endl<< "Reading Counter to \""<<fname;
    if(startACQ==1)
      std::cout<<"\" and Restarting Acquisition";
    std::cout<<std::endl;
#endif
    for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
      if (detectors[idet]) {
	ret1=detectors[idet]->getCounterBlock(arg,startACQ);
	if(ret1!=OK)
	  ret=FAIL;
	else{
	  ret1=detectors[idet]->writeDataFile(outfile,arg);
	  if(ret1!=OK)
	    ret=FAIL;
	}
      }
    }
    outfile.close();
  } else {
    std::cout<< "Could not open file "<< fname << std::endl;
    return -1;
  }
  return ret;
}


int multiSlsDetector::resetCounterBlock(int startACQ){

  int ret=-100, ret1;
  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      ret1=detectors[idet]->resetCounterBlock(startACQ);
      if (ret==-100)
	ret=ret1;
      else if (ret!=ret1)
	ret=-1;
    }
  }
  return ret;
}





int multiSlsDetector::setDynamicRange(int p) {

  int ret=-100, ret1;
  thisMultiDetector->dataBytes=0;
  thisMultiDetector->numberOfChannels=0;
  
  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      ret1=detectors[idet]->setDynamicRange(p);
      thisMultiDetector->dataBytes+=detectors[idet]->getDataBytes();
      thisMultiDetector->numberOfChannels+=detectors[idet]->getTotalNumberOfChannels();
      if (ret==-100)
	ret=ret1;
      else if (ret!=ret1)
	ret=-1;
    }
  }
 
  return ret;

}

int multiSlsDetector::getMaxMods() {


  int ret=0, ret1;
  
  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      ret1=detectors[idet]->getMaxMods();
#ifdef VERBOSE
      cout << "detector " << idet << " maxmods " << ret1 << endl;
#endif
      ret+=ret1;
    }
  }
#ifdef VERBOSE
  cout << "max mods is " << ret << endl;
#endif

  return ret;

}


int multiSlsDetector::getMaxNumberOfModules(dimension d) {

  int ret=0, ret1;
  
  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      ret1=detectors[idet]->getMaxNumberOfModules(d);
      ret+=ret1;
    }
  }
  return ret;

}

int multiSlsDetector::setNumberOfModules(int p, dimension d) {

  int ret=0;//, ret1;
  int nm, mm, nt=p;

  thisMultiDetector->dataBytes=0;
  thisMultiDetector->numberOfChannels=0;

  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {

    //   cout << "detector " << idet << endl;
    if (detectors[idet]) {
      if (p<0)
	nm=p;
      else {
	mm=detectors[idet]->getMaxNumberOfModules();
	//mm=detectors[idet]->getMaxMods();
	if (nt>mm) {
	  nm=mm;
	  nt-=nm;
	} else {
	  nm=nt;
	  nt-=nm;
	}
      }
      ret+=detectors[idet]->setNumberOfModules(nm);
      thisMultiDetector->dataBytes+=detectors[idet]->getDataBytes();
      thisMultiDetector->numberOfChannels+=detectors[idet]->getTotalNumberOfChannels();
    }
  }  
  return ret;

}

int multiSlsDetector::decodeNMod(int i, int &id, int &im) {
#ifdef VERBOSE
  cout << " Module " << i << " belongs to detector " << id << endl;;
  cout << getMaxMods();
#endif

  if (i<0 || i>=getMaxMods()) {
    id=-1;
    im=-1;
#ifdef VERBOSE
    cout  << " A---------" << id << " position " << im << endl;
#endif

    return -1;
  }
  int nm;
  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      nm=detectors[idet]->getNMods();
      if (nm>i) {
	id=idet;
	im=i;
#ifdef VERBOSE
	cout  << " B---------" <<id << " position " << im << endl;
#endif
	return im;
      } else {
	i-=nm;
      }
    }
  }
  id=-1;
  im=-1;
#ifdef VERBOSE
  cout  <<" C---------" << id << " position " << im << endl;
#endif
  return -1;
  

}


 
int64_t multiSlsDetector::getId(idMode mode, int imod) {

  int id, im;

  if (decodeNMod(imod, id, im)>=0) {
    if (detectors[id]) {
      return detectors[id]->getId(mode, im);
    }
  }

  return -1;

}

int multiSlsDetector::digitalTest(digitalTestMode mode, int imod) {

  int id, im;

  if (decodeNMod(imod, id, im)>=0) {
    if (detectors[id]) {
      return detectors[id]->digitalTest(mode, im);
    }
  }

  return -1;

}





int multiSlsDetector::executeTrimming(trimMode mode, int par1, int par2, int imod) {
  int id, im, ret;


  if (decodeNMod(imod, id, im)>=0) {
    if (detectors[id]) {
      return detectors[id]->executeTrimming(mode, par1, par2, im);
    }
  }  else if (imod<0) {
    for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
      if (detectors[idet]) {
	ret=detectors[idet]->executeTrimming(mode, par1, par2, imod);
      }
    }
    return ret;
  }
  return -1;
}





int multiSlsDetector::loadSettingsFile(string fname, int imod) {
  int id, im, ret;

  if (decodeNMod(imod, id, im)>=0) {
    if (detectors[id]) {
      return detectors[id]->loadSettingsFile(fname, im);
    }
  } else if (imod<0) {
    for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
      if (detectors[idet]) {
	ret=detectors[idet]->loadSettingsFile(fname, imod);
      }
    }
    return ret;
  }
  return -1;

}


int multiSlsDetector::saveSettingsFile(string fname, int imod) {
  int id, im, ret;

  if (decodeNMod(imod, id, im)>=0) {
    if (detectors[id]) {
      return detectors[id]->saveSettingsFile(fname, im);
    }
  } else if (imod<0) {
    for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
      if (detectors[idet]) {
	ret=detectors[idet]->saveSettingsFile(fname, imod);
      }
    }
    return ret;
  }
  return -1;

}




int multiSlsDetector::loadCalibrationFile(string fname, int imod) {
  int id, im, ret;

  if (decodeNMod(imod, id, im)>=0) {
    if (detectors[id]) {
      return detectors[id]->loadCalibrationFile(fname, im);
    }
  } else if (imod<0) {
    for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
      if (detectors[idet]) {
	ret=detectors[idet]->loadCalibrationFile(fname, imod);
      }
    }
    return ret;
  }
  return -1;

}


int multiSlsDetector::saveCalibrationFile(string fname, int imod) {
  int id, im, ret;

  if (decodeNMod(imod, id, im)>=0) {
    if (detectors[id]) {
      return detectors[id]->saveCalibrationFile(fname, im);
    }
  } else if (imod<0) {
    for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
      if (detectors[idet]) {
	ret=detectors[idet]->saveCalibrationFile(fname, imod);
      }
    }
    return ret;
  }
  return -1;

}





int multiSlsDetector::writeRegister(int addr, int val){

  int  i;//imi, ima,
  int ret, ret1=-100;

  for (i=0; i<thisMultiDetector->numberOfDetectors; i++) {
    if (detectors[i]) {
      ret=detectors[i]->writeRegister(addr,val);
      if (ret1==-100)
	ret1=ret;
      else if (ret!=ret1)
	ret1=-1;
    }
  }

  return ret1;
};


int multiSlsDetector::readRegister(int addr){

  int  i;//imi, ima,
  int ret, ret1=-100;

  for (i=0; i<thisMultiDetector->numberOfDetectors; i++) {
    if (detectors[i]) {
      ret=detectors[i]->readRegister(addr);
      if (ret1==-100)
	ret1=ret;
      else if (ret!=ret1)
	ret1=-1;
    }
  }

  return ret1;
};


int multiSlsDetector::readConfigurationFile(string const fname){

  

  int nd=thisMultiDetector->numberOfDetectors;
  
  for (int i=0; i<nd; i++) {
    //    sprintf(ext,".det%d",i);
    if (detectors[i]) {
      detectors[i]->freeSharedMemory();
    }
  }
  thisMultiDetector->numberOfDetectors=0;

  multiSlsDetectorClient *cmd;
  // char ext[100];



  string ans;
  string str;
  ifstream infile;
  int iargval;
  int interrupt=0;
  char *args[1000];

  char myargs[1000][1000];

    
  string sargname, sargval;
  int iline=0;
  std::cout<< "config file name "<< fname << std::endl;
  infile.open(fname.c_str(), ios_base::in);
  if (infile.is_open()) {


    while (infile.good() and interrupt==0) {
      sargname="none";
      sargval="0";
      getline(infile,str);
      iline++;
#ifdef VERBOSE
      std::cout<<  str << std::endl;
#endif
      if (str.find('#')!=string::npos) {
#ifdef VERBOSE
	std::cout<< "Line is a comment " << std::endl;
	std::cout<< str << std::endl;
#endif
	continue;
      } else if (str.length()<2) {
#ifdef VERBOSE
	std::cout<< "Empty line " << std::endl;
#endif
	continue;
      } else {
	istringstream ssstr(str);
	iargval=0;
	while (ssstr.good()) {
	  ssstr >> sargname;
	  //if (ssstr.good()) {
#ifdef VERBOSE 
	  std::cout<< iargval << " " << sargname  << std::endl;
#endif

	  strcpy(myargs[iargval], sargname.c_str());
	  args[iargval]=myargs[iargval];

#ifdef VERBOSE 
	  std::cout<< "--" << iargval << " " << args[iargval]  << std::endl;
#endif

	  iargval++;
	  //}
	}

#ifdef VERBOSE 
	cout << endl;
	for (int ia=0; ia<iargval; ia++) cout << args[ia] << " ??????? ";
	cout << endl;
#endif	
	cmd=new multiSlsDetectorClient(iargval, args, PUT_ACTION, this);
	delete cmd;
      }
      iline++;
    }




    infile.close();


  } else {
    std::cout<< "Error opening configuration file " << fname << " for reading" << std::endl;
    return FAIL;
  }
#ifdef VERBOSE
  std::cout<< "Read configuration file of " << iline << " lines" << std::endl;
#endif

  setNumberOfModules(-1);
  getMaxNumberOfModules();
  return iline;


};  





int multiSlsDetector::writeConfigurationFile(string const fname){




  string names[]={				\
    "type",					\
    "master",					\
    "sync",					\
    "outdir",					\
    "ffdir",					\
    "headerbefore",				\
    "headerafter",				\
    "headerbeforepar",				\
    "headerafterpar",				\
    "badchannels",				\
    "angconv",					\
    "globaloff",				\
    "binsize",					\
    "threaded"				};

  int nvar=14;
 
  //  char ext[100];
  
  int iv=0;
  char *args[100];
  for (int ia=0; ia<100; ia++) {
    args[ia]=new char[1000];
  }
  
  

  ofstream outfile;
#ifdef VERBOSE
  int ret;
#endif

  outfile.open(fname.c_str(),ios_base::out);
  if (outfile.is_open()) {



    slsDetectorCommand *cmd=new slsDetectorCommand(this);
    
    // detector types!!!
    cout << iv << " " << names[iv] << endl;
    strcpy(args[0],names[iv].c_str());
    outfile << names[iv] << " " << cmd->executeLine(1,args,GET_ACTION) << std::endl;

    // single detector configuration
    for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
      //    sprintf(ext,".det%d",i);
      if (detectors[i]) {
	iv+=detectors[i]->writeConfigurationFile(outfile,i);
      }
    }
  

    //other configurations
    for (iv=1; iv<nvar; iv++) {
      
      cout << iv << " " << names[iv] << endl;
      strcpy(args[0],names[iv].c_str());
      outfile << names[iv] << " " << cmd->executeLine(1,args,GET_ACTION) << std::endl;
    }
    
    
  
    delete cmd;
    outfile.close();
  }  else {
    std::cout<< "Error opening configuration file " << fname << " for writing" << std::endl;
    return FAIL;
  }
#ifdef VERBOSE
  std::cout<< "wrote " <<ret << " lines to configuration file " << std::endl;
#endif
  
  return iv;
  
};














int multiSlsDetector::writeDataFile(string fname, double *data, double *err, double *ang, char dataformat, int nch) {

#ifdef VERBOSE
  cout << "using overloaded multiSlsDetector function to write formatted data file " << endl;
#endif


  ofstream outfile;
  int choff=0, off=0; //idata, 
  double *pe=err, *pa=ang;
  int nch_left=nch, n;//, nd;

  if (nch_left<=0)
    nch_left=getTotalNumberOfChannels();


  if (data==NULL)
    return FAIL;

  outfile.open (fname.c_str(),ios_base::out);
  if (outfile.is_open())
    {
    
      for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
     
	if (detectors[i]) {
	  n=detectors[i]->getTotalNumberOfChannels();
	  if (nch_left<n)
	    n=nch_left;

#ifdef VERBOSE
	  cout << " write " << i << " position " << off << " offset " << choff << endl;
#endif
	  detectors[i]->writeDataFile(outfile,n, data+off, pe, pa, dataformat, choff);

	  nch_left-=n;

	  choff+=detectors[i]->getMaxNumberOfChannels();
       
	  off+=n;
    
	  if (pe)
	    pe=err+off;
       
	  if (pa)
	    pa=ang+off;
     
	}
     
      }
   
      outfile.close();
      return OK;
    } else {
    std::cout<< "Could not open file " << fname << "for writing"<< std::endl;
    return FAIL;
  }
}


int multiSlsDetector::writeDataFile(string fname, int *data) {
  ofstream outfile;
  int choff=0, off=0;

#ifdef VERBOSE
  cout << "using overloaded multiSlsDetector function to write raw data file " << endl;
#endif

  if (data==NULL)
    return FAIL;

  outfile.open (fname.c_str(),ios_base::out);
  if (outfile.is_open())
    {
      for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
	if (detectors[i]) {
#ifdef VERBOSE
	  cout << " write " << i << " position " << off << " offset " << choff << endl;
#endif
	  detectors[i]->writeDataFile(outfile, detectors[i]->getTotalNumberOfChannels(), data+off, choff);
	  choff+=detectors[i]->getMaxNumberOfChannels();
	  off+=detectors[i]->getTotalNumberOfChannels();
	}
      }


      outfile.close();
      return OK;
    } else {
    std::cout<< "Could not open file " << fname << "for writing"<< std::endl;
    return FAIL;
  }
}


int multiSlsDetector::readDataFile(string fname, double *data, double *err, double *ang, char dataformat){

#ifdef VERBOSE
  cout << "using overloaded multiSlsDetector function to read formatted data file " << endl;
#endif

  ifstream infile;
  int iline=0;//ichan, 
  //int interrupt=0;
  string str;
  int choff=0, off=0;
  double *pe=err, *pa=ang;

#ifdef VERBOSE
  std::cout<< "Opening file "<< fname << std::endl;
#endif
  infile.open(fname.c_str(), ios_base::in);
  if (infile.is_open()) {

    for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
      if (detectors[i]) {
	iline+=detectors[i]->readDataFile(detectors[i]->getTotalNumberOfChannels(), infile, data+off, pe, pa, dataformat, choff);
	choff+=detectors[i]->getMaxNumberOfChannels();
	off+=detectors[i]->getTotalNumberOfChannels();
	if (pe)
	  pe=pe+off;
	if (pa)
	  pa=pa+off;
      }
    }


    infile.close();
  } else {
    std::cout<< "Could not read file " << fname << std::endl;
    return -1;
  }
  return iline;

}

int multiSlsDetector::readDataFile(string fname, int *data) {

#ifdef VERBOSE
  cout << "using overloaded multiSlsDetector function to read raw data file " << endl;
#endif

  ifstream infile;
  int  iline=0;//ichan,
  //int interrupt=0;
  string str;
  int choff=0, off=0;

#ifdef VERBOSE
  std::cout<< "Opening file "<< fname << std::endl;
#endif
  infile.open(fname.c_str(), ios_base::in);
  if (infile.is_open()) {

    for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
      if (detectors[i]) {
	iline+=detectors[i]->readDataFile(infile, data+off,detectors[i]->getTotalNumberOfChannels(), choff);
	choff+=detectors[i]->getMaxNumberOfChannels();
	off+=detectors[i]->getTotalNumberOfChannels();
      }
    }
    infile.close();
  } else {
    std::cout<< "Could not read file " << fname << std::endl;
    return -1;
  }
  return iline;
}





//receiver


int multiSlsDetector::setReceiverOnline(int off) {
	int ret=-100,ret1;
	for (int i=0; i<thisMultiDetector->numberOfDetectors; i++)
		if (detectors[i]){
			ret1=detectors[i]->setReceiverOnline(off);
		    if(detectors[i]->getErrorMask())
			  setErrorMask(getErrorMask()|(1<<i));
			if(ret==-100)
				ret=ret1;
			else if (ret!=ret1)
				ret=-1;
		}
	return ret;
}



string multiSlsDetector::checkReceiverOnline() {
  string retval1 = "",retval;
  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      retval=detectors[idet]->checkReceiverOnline();
      if(!retval.empty()){
	retval1.append(retval);
	retval1.append("+");
      }
    }
  }
  return retval1;
}




string multiSlsDetector::setFilePath(string s) {
	if(!s.empty())
		fileIO::setFilePath(s);
	/*
  string ret="error", ret1;
  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      ret1=detectors[idet]->setFilePath(s);
      if (ret=="error")
	ret=ret1;
      else if (ret!=ret1)
	ret="";
    }
  }
  return ret;*/
	return fileIO::getFilePath();
}



string multiSlsDetector::setFileName(string s) {
  string ret="error", ret1;

  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      if(getNumberOfDetectors()>1)
        setDetectorIndex(idet);
      ret1=detectors[idet]->setFileName(s);
      if (ret=="error")
	ret=ret1;
      else if (ret!=ret1)
	ret="";
    }
  }
  return ret;
}



int multiSlsDetector::setFileIndex(int i) {
  int ret=-100, ret1;

  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      ret1=detectors[idet]->setFileIndex(i);
      if (ret==-100)
	ret=ret1;
      else if (ret!=ret1)
	ret=-1;
    }
  }
  return ret;
}



int multiSlsDetector::startReceiver(){
  int i=0;
  int ret=OK, ret1=OK;
  for (i=0; i<thisMultiDetector->numberOfDetectors; i++) {
    if (i!=thisMultiDetector->masterPosition)
      if (detectors[i]) {
	ret=detectors[i]->startReceiver();
	if (ret!=OK)
	  ret1=FAIL;
      }
  }
  i=thisMultiDetector->masterPosition;
  if (thisMultiDetector->masterPosition>=0) {
    if (detectors[i]) {
      ret=detectors[i]->startReceiver();
      if (ret!=OK)
	ret1=FAIL;
    }
  }
  return ret1;
}




int multiSlsDetector::stopReceiver(){
  int i=0;
  int ret=OK, ret1=OK;

  i=thisMultiDetector->masterPosition;
  if (thisMultiDetector->masterPosition>=0) {
    if (detectors[i]) {
      ret=detectors[i]->stopReceiver();
      if (ret!=OK)
	ret1=FAIL;
    }
  }
  for (i=0; i<thisMultiDetector->numberOfDetectors; i++) {
    if (detectors[i]) {
      ret=detectors[i]->stopReceiver();
      if (ret!=OK)
	ret1=FAIL;
    }
  }
  return ret1;
}




slsDetectorDefs::runStatus multiSlsDetector::getReceiverStatus(){

  runStatus s,s1;

  if (thisMultiDetector->masterPosition>=0)
    if (detectors[thisMultiDetector->masterPosition])
      return detectors[thisMultiDetector->masterPosition]->getReceiverStatus();


  if (detectors[0]) s=detectors[0]->getReceiverStatus();

  for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
    s1=detectors[i]->getReceiverStatus();
    if (s1==ERROR)
      s=ERROR;
    if (s1==IDLE && s!=IDLE)
      s=ERROR;

  }
  return s;
}





int multiSlsDetector::getFramesCaughtByReceiver() {
  int ret=0,ret1=0;
  for (int i=0; i<thisMultiDetector->numberOfDetectors; i++)
    if (detectors[i])
      ret1+=detectors[i]->getFramesCaughtByReceiver();

  if(!thisMultiDetector->numberOfDetectors)
	  return ret;
  ret=(int)(ret1/thisMultiDetector->numberOfDetectors);

  return ret;
}



int multiSlsDetector::getReceiverCurrentFrameIndex() {
  int ret=0,ret1=0;
  for (int i=0; i<thisMultiDetector->numberOfDetectors; i++)
    if (detectors[i])
      ret1+=detectors[i]->getReceiverCurrentFrameIndex();

  if(!thisMultiDetector->numberOfDetectors)
	  return ret;
  ret=(int)(ret1/thisMultiDetector->numberOfDetectors);

  return ret;
}



int multiSlsDetector::resetFramesCaught() {
	int ret=-100, ret1;
	for (int i=0; i<thisMultiDetector->numberOfDetectors; i++){
		if (detectors[i]){
			ret1=detectors[i]->resetFramesCaught();
		      if (ret==-100)
			ret=ret1;
		      else if (ret!=ret1)
			ret=-1;
		}
	}

	return ret;
}



int* multiSlsDetector::readFrameFromReceiver(char* fName, int &fIndex){
	int nel=(thisMultiDetector->dataBytes)/sizeof(int);
	int n;
	int* retval=new int[nel];
	int *retdet, *p=retval;

	for (int id=0; id<thisMultiDetector->numberOfDetectors; id++) {
		if (detectors[id]) {
			retdet=detectors[id]->readFrameFromReceiver(fName,fIndex);
			if (retdet) {
				n=detectors[id]->getDataBytes();
				memcpy(p,retdet,n);
				delete [] retdet;
				p+=n/sizeof(int);
			}else {
#ifdef VERBOSE
				cout << "Receiver for detector " << id << " does not have data left " << endl;
#endif
				delete [] retval;
				return NULL;
			}
		}
	}
	return retval;
};




int multiSlsDetector::lockReceiver(int lock) {

  int ret=-100, ret1;

  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      ret1=detectors[idet]->lockReceiver(lock);
      if (ret==-100)
	ret=ret1;
      else if (ret!=ret1)
	ret=-1;
    }
  }

  return ret;

}





string multiSlsDetector::getReceiverLastClientIP() {
  string s0="", s1="",s ;

  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      s=detectors[idet]->getReceiverLastClientIP();

      if (s0=="")
	s0=s;
      else
	s0+=string("+")+s;
      if (s1=="")
	s1=s;
      else if (s1!=s)
	s1="bad";
    }
  }
  if (s1=="bad")
    return s0;
  else
    return s1;
}




int multiSlsDetector::exitReceiver() {

  int ival=FAIL, iv;
  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      iv=detectors[idet]->exitReceiver();
      if (iv==OK)
	ival=iv;
    }
  }
  return ival;
}



int multiSlsDetector::enableWriteToFile(int enable){
	int ret=-100, ret1;

	for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
		if (detectors[idet]) {
			ret1=detectors[idet]->enableWriteToFile(enable);
			if (ret==-100)
				ret=ret1;
			else if (ret!=ret1)
				ret=-1;
		}
	}

	return ret;
}




int multiSlsDetector::setFrameIndex(int index){
	int ret=-100, ret1;

	for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
		if (detectors[idet]) {
			ret1=detectors[idet]->setFrameIndex(index);
			if (ret==-100)
				ret=ret1;
			else if (ret!=ret1)
				ret=-1;
		}
	}

	return ret;
}




string multiSlsDetector::getErrorMessage(int &critical){

	int64_t multiMask,slsMask=0;
	string retval="";
	char sNumber[100];
	critical=0;

	multiMask = getErrorMask();
	if(multiMask){
		  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
		    if (detectors[idet]) {
		    	//if the detector has error
		    	if(multiMask&(1<<idet)){
					//append detector id
					sprintf(sNumber,"%d",idet);
					retval.append("Detector " + string(sNumber)+string(":\n"));
					//get sls det error mask
					slsMask=detectors[idet]->getErrorMask();
#ifdef VERYVERBOSE
					//append sls det error mask
					sprintf(sNumber,"0x%x",slsMask);
					retval.append("Error Mask " + string(sNumber)+string("\n"));
#endif
					//get the error critical level
					if((slsMask>0xFFFFFFFF)|critical)
						critical = 1;
					//append error message
					retval.append(errorDefs::getErrorMessage(slsMask));

		    	}
		    }
		  }
	}
	return retval;
}
