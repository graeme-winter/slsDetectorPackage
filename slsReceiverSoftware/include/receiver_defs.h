#ifndef RECEIVER_DEFS_H
#define RECEIVER_DEFS_H

#include "sls_receiver_defs.h"
#include <stdint.h> 

#define MAX_DIMENSIONS					2
//socket
#define GOODBYE 						-200
#define RECEIVE_SOCKET_BUFFER_SIZE 		(100*1024*1024)
#define MAX_SOCKET_INPUT_PACKET_QUEUE 	250000

//files
#define DO_NOTHING						0
#define CREATE_FILES					1
#define DO_EVERYTHING					2

//binary
#define FILE_FRAME_HEADER_SIZE			16
#define FILE_BUFFER_SIZE        		(16*1024*1024) //16mb

//hdf5
#define MAX_CHUNKED_IMAGES 				1

//versions
#define HDF5_WRITER_VERSION 			1.0
#define BINARY_WRITER_VERSION 			1.0 //1 decimal places

//fifo
#define FIFO_HEADER_NUMBYTES			4

//parameters to calculate fifo depth
#define SAMPLE_TIME_IN_NS				100000000//100ms
#define MAX_JOBS_PER_THREAD				1000


#define DUMMY_PACKET_VALUE				0xFFFFFFFF



/*
//binary
#define FILE_BUF_SIZE        		(16*1024*1024) //16mb
#define FILE_HEADER_BUNCHID_OFFSET	8

//hdf5
#ifdef HDF5C
#define HDF5_WRITER_VERSION 		1.0
#define MAX_CHUNKED_IMAGES 			1
#define MAX_IMAGES_IN_DATASET 		1000
#endif


#define HEADER_SIZE_NUM_TOT_PACKETS	4
#define ALL_MASK_32					0xFFFFFFFF



//gottard
#define GOTTHARD_FIFO_SIZE					25000 //cannot be less than max jobs per thread = 1000
#define GOTTHARD_PIXELS_IN_ROW				1280
#define GOTTHARD_PIXELS_IN_COL				1
#define GOTTHARD_PACKETS_PER_FRAME			2
#define GOTTHARD_ONE_PACKET_SIZE			1286
#define GOTTHARD_ONE_DATA_SIZE				1280
#define GOTTHARD_BUFFER_SIZE 				(GOTTHARD_ONE_PACKET_SIZE*GOTTHARD_PACKETS_PER_FRAME) 	//1286*2
#define GOTTHARD_DATA_BYTES	 				(GOTTHARD_ONE_DATA_SIZE*GOTTHARD_PACKETS_PER_FRAME)		//1280*2
#define GOTTHARD_FRAME_INDEX_MASK			0xFFFFFFFE
#define GOTTHARD_FRAME_INDEX_OFFSET			1
#define GOTTHARD_PACKET_INDEX_MASK			0x1


//short gotthard
#define GOTTHARD_SHORT_PIXELS_IN_ROW		256
#define GOTTHARD_SHORT_PIXELS_IN_COL		1
#define GOTTHARD_SHORT_PACKETS_PER_FRAME	1
#define GOTTHARD_SHORT_ONE_PACKET_SIZE		518
#define GOTTHARD_SHORT_ONE_DATA_SIZE		512
#define GOTTHARD_SHORT_BUFFER_SIZE			(GOTTHARD_SHORT_ONE_PACKET_SIZE*GOTTHARD_SHORT_PACKETS_PER_FRAME)//518*1
#define GOTTHARD_SHORT_DATABYTES			(GOTTHARD_SHORT_ONE_DATA_SIZE*GOTTHARD_SHORT_PACKETS_PER_FRAME)	 //512*1
#define GOTTHARD_SHORT_FRAME_INDEX_MASK		0xFFFFFFFF
#define GOTTHARD_SHORT_FRAME_INDEX_OFFSET	0
#define GOTTHARD_SHORT_PACKET_INDEX_MASK	0


//propix
#define PROPIX_FIFO_SIZE					25000 //cannot be less than max jobs per thread = 1000
#define PROPIX_PIXELS_IN_ROW				22
#define PROPIX_PIXELS_IN_COL				22
#define PROPIX_PACKETS_PER_FRAME			2
#define PROPIX_ONE_PACKET_SIZE				1286
#define PROPIX_ONE_DATA_SIZE				1280
#define PROPIX_BUFFER_SIZE 					(PROPIX_ONE_PACKET_SIZE*PROPIX_PACKETS_PER_FRAME) 	//1286*2
#define PROPIX_DATABYTES_PER_PIXEL			2
//#define PROPIX_DATA_BYTES 				(1280*PROPIX_PACKETS_PER_FRAME) 	//1280*2
#define PROPIX_DATA_BYTES	 				(PROPIX_PIXELS_IN_ROW * PROPIX_PIXELS_IN_COL * PROPIX_DATABYTES_PER_PIXEL) //22 * 22 * 2
#define PROPIX_FRAME_INDEX_MASK				0xFFFFFFFE
#define PROPIX_FRAME_INDEX_OFFSET			1
#define PROPIX_PACKET_INDEX_MASK			0x1


//moench
#define MOENCH_FIFO_SIZE					2500 //cannot be less than max jobs per thread = 1000
#define MOENCH_BYTES_IN_ONE_ROW				(MOENCH_PIXELS_IN_ONE_ROW*2)
#define MOENCH_BYTES_PER_ADC				(40*2)
#define MOENCH_PIXELS_IN_ONE_ROW			160
#define MOENCH_PACKETS_PER_FRAME			40
#define MOENCH_ONE_PACKET_SIZE				1286
#define MOENCH_ONE_DATA_SIZE				1280
#define MOENCH_BUFFER_SIZE 					(MOENCH_ONE_PACKET_SIZE*MOENCH_PACKETS_PER_FRAME) 	//1286*40
#define MOENCH_DATA_BYTES	 				(MOENCH_ONE_DATA_SIZE*MOENCH_PACKETS_PER_FRAME)		//1280*40
#define MOENCH_FRAME_INDEX_MASK				0xFFFFFF00
#define MOENCH_FRAME_INDEX_OFFSET			8
#define MOENCH_PACKET_INDEX_MASK			0xFF


//jungfrau
#define JFRAU_FIFO_SIZE						2500
#define JFRAU_PIXELS_IN_ONE_ROW				(256*4)
#define JFRAU_PIXELS_IN_ONE_COL				(256)
#define JFRAU_BYTES_IN_ONE_ROW				(JFRAU_PIXELS_IN_ONE_ROW*2)
#define JFRAU_PACKETS_PER_FRAME				128
#define JFRAU_HEADER_LENGTH					22
#define JFRAU_ONE_DATA_SIZE					8192
#define JFRAU_ONE_PACKET_SIZE				(JFRAU_HEADER_LENGTH+JFRAU_ONE_DATA_SIZE) 		//8214
#define JFRAU_BUFFER_SIZE 					(JFRAU_ONE_PACKET_SIZE*JFRAU_PACKETS_PER_FRAME) //8214*128
#define JFRAU_DATA_BYTES	 				(JFRAU_ONE_DATA_SIZE*JFRAU_PACKETS_PER_FRAME)	//8192*128
#define JFRAU_FRAME_INDEX_MASK				0xffffff 				//mask after using struct (48 bit)
#define JFRAU_FRAME_INDEX_OFFSET			0x0 					//Not Applicable, use struct
#define JFRAU_PACKET_INDEX_MASK				0x0						//Not Applicable, use struct


//jungrau chip test board
#define JCTB_FIFO_SIZE						2500
#define JCTB_PIXELS_IN_ONE_ROW				32
#define JCTB_BYTES_IN_ONE_ROW		       	(JCTB_PIXELS_IN_ONE_ROW*2)
#define JCTB_BYTES_PER_ADC		        	(2)
#define JCTB_PACKETS_PER_FRAME			    1
#define JCTB_ONE_PACKET_SIZE				8224
#define JCTB_BUFFER_SIZE 					(JCTB_ONE_PACKET_SIZE*40)
#define JCTB_DATA_BYTES	 					(8192*JCTB_PACKETS_PER_FRAME)
#define JCTB_FRAME_INDEX_MASK				0xFFFFFFFF
#define JCTB_FRAME_INDEX_OFFSET				6+8
#define JCTB_PACKET_INDEX_MASK				0xFFFFFFFF


//eiger
#define EIGER_FIFO_SIZE						100
#define EIGER_PIXELS_IN_ONE_ROW				(256*2)
#define EIGER_PIXELS_IN_ONE_COL				(256)
#define EIGER_PORTS_PER_READOUT				2
#define EIGER_HEADER_PACKET_LENGTH			48
#define EIGER_ONE_GIGA_CONSTANT				16
#define EIGER_TEN_GIGA_CONSTANT				4
#define EIGER_ONE_GIGA_ONE_PACKET_SIZE		1040
#define EIGER_ONE_GIGA_ONE_DATA_SIZE		1024
#define EIGER_TEN_GIGA_ONE_PACKET_SIZE		4112
#define EIGER_TEN_GIGA_ONE_DATA_SIZE		4096
#define EIGER_DATA_PACKET_HEADER_SIZE		8
#define EIGER_FRAME_INDEX_MASK				0xFFFFFFFF //32 bit for now
#define EIGER_FRAME_INDEX_OFFSET			0
#define EIGER_PACKET_INDEX_MASK				0x0


*/
#endif
