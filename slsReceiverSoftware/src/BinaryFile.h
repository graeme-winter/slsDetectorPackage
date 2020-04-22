#pragma once
/************************************************
 * @file BinaryFile.h
 * @short sets/gets properties for the binary file,
 * creates/closes the file and writes data to it
 ***********************************************/
/**
 *@short sets/gets properties for the binary file, creates/closes the file and writes data to it
 */

#include "File.h"
#include "BinaryFileStatic.h"

#include <string>


class BinaryFile : private virtual slsDetectorDefs, public File, public BinaryFileStatic {
	
 public:
	/**
	 * Constructor
	 * creates the File Writer
	 * @param ind self index
	 * @param maxf pointer to max frames per file
	 * @param nd pointer to number of detectors in each dimension
	 * @param fname pointer to file name prefix
	 * @param fpath pointer to file path
	 * @param findex pointer to file index
	 * @param owenable pointer to over write enable
	 * @param dindex pointer to detector index
	 * @param nunits pointer to number of theads/ units per detector
	 * @param nf pointer to number of images in acquisition
	 * @param dr pointer to dynamic range
	 * @param portno pointer to udp port number for logging
	 * @param smode pointer to silent mode
	 */
	BinaryFile(int ind, uint32_t* maxf,
			int* nd, std::string* fname, std::string* fpath, uint64_t* findex, bool* owenable,
			int* dindex, int* nunits, uint64_t* nf, uint32_t* dr, uint32_t* portno,
			bool* smode);

	/**
	 * Destructor
	 */
	~BinaryFile();

	/**
	 * Print all member values
	 */
	void PrintMembers(TLogLevel level = logDEBUG1) override;

	/**
	 * Create file
	 */
	void CreateFile() override;

	 /**
	  * Create master file
	 * @param mfwenable master file write enable
	 * @param attr master file attributes
	  */
	 void CreateMasterFile(bool mfwenable, masterAttributes& attr) override;

	/**
	 * Close Current File
	 */
	void CloseCurrentFile() override;

	/**
	 * Close all Files
	 */
	void CloseAllFiles() override;

	/**
	 * Write data to file
	 * @param buffer buffer to write from
	 * @param buffersize size of buffer
	 * @param fnum current image number
	 * @param nump number of packets caught
	 */
	 void WriteToFile(char* buffer, int buffersize, uint64_t fnum, uint32_t nump) override;



 private:

	/**
	 * Get Type
	 * @return type
	 */
	 fileFormat GetFileType() override;



	/** File Descriptor */
	FILE* filefd;

	/** Master File Descriptor */
	static FILE* masterfd;

	/** Number of frames in file */
	uint32_t numFramesInFile;

	/** Number of actual packets caught in file */
	uint64_t numActualPacketsInFile;

};
