
#include "common.h"
#include "parameters_pyrometer.h"
#include "bios_app_interface.h"
#include "spi.h"

//maximal numberd of errors before entering the error state
#define MAX_NUM_ERRORS 1000

//is defined in the parameter table
#define NUM_PYROMETERS *num_pyros

//maximal supported number of pyrometers
#define MAX_NUM_PYROMETERS 2

//number of data points in the calibration table
//should not be changed!!
#define PYRO_CALIB_TABLE_SIZE 1024

//number of inquiry attempts
#define MAX_NUM_INQUIRIES 10

//defines the size of the circular buffer for storing pyrometer data per pyrometer
#define PYROMETER_BUF_SIZE 16

//Chip Selct Line Macros
#define CSSET 	MCF_GPIO_PORTQS |= (uint8) MCF_GPIO_PORTQS_PORTQS3
#define CSCLR 	MCF_GPIO_PORTQS &= (uint8) ~MCF_GPIO_PORTQS_PORTQS3

#define INQUIRY_WORD 0xA005
#define UNKNOWN_COMM 0xE001

#define PYROMETER_TOS_SLOT	3
#define DEST_BOOTLOADER 	1

#pragma options align=packed
typedef struct {
	uint8  available;
	uint8  address;
	union 
	{
		uint8  raw[64];
		struct 
		{
			uint8 num_samples;
			uint8 valid;
			struct 
			{
				uint16 temperature;
				uint16 sensor;
			} pair[15];
			uint16 checksum;
		};
	} calibration;
	
	union
	{
		uint8  raw[64];
		struct
		{
			uint8 valid;
			union
			{
				uint32 all;
				uint8 raw[4];
			} sensor_unique_id;
			uint8  software_version;
			uint8  software_revision;
		};
	} data;
	
	uint8  calib_data_valid;
	uint8  sensor_data_valid;
	uint16 calibration_table[PYRO_CALIB_TABLE_SIZE];	
	uint8  calibration_table_calculated;
} t_pyrometer;
#pragma options align=reset

//for frequently data sending 
typedef struct 
{
	uint16 sequence;
	uint32 sync;
	uint32 conversion_start;
	uint8  num_channels;
	uint8  num_samples;
	uint8  data[INTEGRA_DATALEN-12];
} t_SFT_000C;
#define SFT_000C 0x000C

typedef struct
{
	uint16 calibration_temperature;
	uint16 calibration_reading;
} t_calibdata_sample;


extern uint8  pyro_ready_to_send_cs;
extern uint16 pyrometer_raw_value0;
extern uint16 temperature_sensor0;

uint8 pyrometer_init(void);
uint8 pyrometer_inquiry(void);

uint8 pyrometer_read_calibration(void);
uint8 pyrometer_read_sensordata(void);
void  pyrometer_usi_isr(void);
void  pyrometer_readall(void);
uint8 pyrometer_process_data(void);
void  pyrometer_calculate_calibration(void);
uint16 get_calibrated_sample(uint16, uint16);


//void  pyrometer_erase_calibration(uint8);
void  pyrometer_erase_sensordata(uint8);
void  pyrometer_store_calibration(uint8,uint8,uint8);
void  pyrometer_store_sensordata(uint8,uint8,uint8);
