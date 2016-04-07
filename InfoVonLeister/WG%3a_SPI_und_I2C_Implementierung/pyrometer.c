#include "pyrometer.h"
#include "adressconf.h"
#include "IntegraError.h"
#include "IntegraDefs.h"

t_pyrometer pyrometer[MAX_NUM_PYROMETERS];
uint16 fount_pyrometers;
uint16 sendbuffer[MAX_NUM_PYROMETERS+3];
uint32 count;
uint32 errors;
uint32 count2;
uint32 last_conversion_time;

//variables for controling the commands to the pyrometers
//because there are various devices that want to access the
//sensors
uint8 pyro_command;

//circular buffer for storing high speed pyrometer data
uint16 pyrobuffer[MAX_NUM_PYROMETERS][PYROMETER_BUF_SIZE];
uint32 pyrobuffer_conversiontimes[PYROMETER_BUF_SIZE];
volatile uint8  pyrobuffer_readindex;
volatile uint8  pyrobuffer_writeindex;

//stores how much samples have to be sent to the master
uint32 pyro_send_data_bootloader;
uint32 pyro_send_acceleration_data_bootloader;

// Mark when data are available to send...
uint8  pyro_ready_to_send_cs;

uint32 bytes_per_sample;
uint8 samples_per_frame_bootloader;
uint8 samples_per_frame_acceleration_bootolader;
uint32 data_base_address_bootloader;

//calibrated temperatures
uint16 temperature_sensor0;

//current value (temporary for highspeed data...)
uint16 pyrometer_raw_value0;

//general purpose data pointers
uint8*   dptr_8;
uint16*  dptr_16;
uint32*  dptr_32;

//for averaging pyrometer values if neccessary
uint16 pyro_average;
uint16 pyro_average_len;
uint16 pyro_average_buf;
uint32 pyro_average_sum;

//for error handling
extern uint16 node_error;
uint32 error_counter;

extern t_bios_func bios_func;


/*
 * Initialize Pyrometer
 *
 * Return values
 *  1 = Too many Pyrometer
 */
uint8 pyrometer_init(void) {
	
	extern uint32 __VECTOR_RAM[];
	
	uint8 i;
	
	count = 0;
	errors = 0;
	
	//init spi, 1Mbps
	spi_init(30);
	
	//get SPI interrupt vector
	__VECTOR_RAM[82] = (uint32)&pyrometer_usi_isr;
	
	//define CS0 as GPIO output
	MCF_GPIO_PQSPAR &= 0xFF3F;
	MCF_GPIO_DDRQS  |= MCF_GPIO_DDRQS_DDRQS3;
	
	if(NUM_PYROMETERS > MAX_NUM_PYROMETERS)
		return 1;
	for (i=0; i<NUM_PYROMETERS; i++) {
		pyrometer[i].available = 0;
		pyrometer[i].calibration_table_calculated = 0;
		pyrometer[i].calib_data_valid = 0;
		pyrometer[i].sensor_data_valid = 0;
	}
		
	//init sync variables and buffers
	pyro_command = 0;
	
	pyrobuffer_readindex = 0;
	pyrobuffer_writeindex = 0;
	
	error_counter = 0;
	
	pyro_average_len = 0;
	
	return 0;
}

//calculates the calibration tables for the sensors
//the table is made of 1024 sample points per pyrometer
//this means 4 sensor values share a common table index
//therefore sub-linearization is necessary
void  pyrometer_calculate_calibration(void) {
	
	uint8  i;
	uint8  j;
	uint8  datapoints;
	uint16 BlackBodyTemp0;
	uint16 BlackBodyTemp1;
	uint16 SensorReading0;
	uint16 SensorReading1;
	uint16 TargetReading0;
	uint16 TargetReading1;
	uint16 bottom_index;
	uint16 top_index;
	uint16 index;
	uint16 temp;
	
	uint16 dx,dy;
	
	float gain;
	uint16 offset;
	
	for (j=0; j<NUM_PYROMETERS; j++) {
	
		BlackBodyTemp0 = 0;
		BlackBodyTemp1 = 0;
		TargetReading0 = 0;
		TargetReading1 = 0;
		SensorReading0 = 0;
		SensorReading1 = 0;
		
		//check if the sensor has a calibration
		if (pyrometer[j].calib_data_valid) {
				
			datapoints = pyrometer[j].calibration.num_samples;
			if(datapoints > 15)
				datapoints = 0;
			
			//loop over datapoints and calculate table
			for (i=0; i<datapoints; i++) {
				BlackBodyTemp0 = BlackBodyTemp1;
				SensorReading0 = SensorReading1;
				
				//get next values from calibration_data
				BlackBodyTemp1 = pyrometer[j].calibration.pair[i].temperature;
				SensorReading1 = pyrometer[j].calibration.pair[i].sensor;
				
				TargetReading0 = (uint16)(BlackBodyTemp0 * 10);
				TargetReading1 = (uint16)(BlackBodyTemp1 * 10);
				
				//calculate the highest and lowest table index
				bottom_index 	= (uint16)(SensorReading0 >> 2);
				top_index 		= (uint16)(SensorReading1 >> 2);
				
				//dy is the target reading difference, dx the sensor reading difference
				dy = (uint16)(TargetReading1 - TargetReading0);
				dx = (uint16)(SensorReading1 - SensorReading0);
				
				//linearization
				gain = (float)dy / (float)dx;
				offset = TargetReading0;
				
				for (index = bottom_index; index < top_index; index++) {
					temp = (uint16)(((float)gain * (float)((index-bottom_index) << 2)) + (float)offset);
					pyrometer[j].calibration_table[index] = temp;
				}
				
			}
			
			//calculate the rest of the table
			//assume the same gain but limit the output at 0x0FFF
			
			pyrometer[j].calibration_table[top_index] = TargetReading1;
			offset = TargetReading1;
					
			for (index = top_index+1; index < 1024; index++) {
				uint32 temp2;
				temp2 = (uint32)(((float)gain * (float)((index-top_index) << 2)) + (float)offset);
				
				if (temp2 < 4095) pyrometer[j].calibration_table[index] = (uint16)temp2;
				else pyrometer[j].calibration_table[index] = 4095;
			}
			
			pyrometer[j].calibration_table_calculated = 1;
				
		}
	}
}

/*
 * read the calibration table and interpolates betwettn sample points in table
 * when the calibration table is correct the output is the calibrated tenperature
 * to the sensor reading
 */
static inline uint16 get_calibrated_sample(uint16 raw_reading, uint16 sensornumber) {

	uint16 t1,t2;
	uint16 delta;
	uint16 values[4];
	uint16 index;
	
	if (pyrometer[sensornumber].calibration_table_calculated) {
		
		if (raw_reading < 4095) {
		
			index = raw_reading >> 2;
			
			t1 = pyrometer[sensornumber].calibration_table[index];
			t2 = pyrometer[sensornumber].calibration_table[index + 1];
			
			delta = (t2 - t1);
			
			values[0] = t1;
			values[1] = t1 + (delta >> 2);
			values[2] = t1 + (delta >> 1);
			values[3] = t2 - (delta >> 2);
			
			return values[raw_reading & 0x03];
			
		}
		else {
			return pyrometer[sensornumber].calibration_table[1023];
		}
		
	}
	else return 0;	
}

/*
 * tries to read the inquiry word of all pyrometers 1000 times to check if they work
 * and to check if the fount number of pyros matches the number of configured sensors
 * 0 = if succesful
 * 1 = read error
 */
uint8 pyrometer_inquiry(void) {
	uint16 address = 0;
	uint16 i = 0;
	uint16 count = 0;
	uint16 response = 0;
	uint16 data = 0;
	uint16 tries = 0;
	
	tries = 0;	
	fount_pyrometers = 0;
	
	if (NUM_PYROMETERS > MAX_NUM_PYROMETERS) return 0;
	
	while (fount_pyrometers != NUM_PYROMETERS) {
		
		tries++;
		if (tries > MAX_NUM_INQUIRIES) {
			return 1;	
		}
		
		fount_pyrometers = 0;		
		
		address = 1;
	
		while(address < 8) {		
			count = 0;		
			//set CS line
			CSSET;			
			
			data = (uint16) (0x3000 | (address << 8));
			
			//send 1000 inquiris to each address
			for (i=0; i<=1000; i++) {
				if(spi_read_write_single(data) == INQUIRY_WORD) count++;
				//cpu_pause(1);	
			}
			//when count is 1000, node is ready
			if (count > 990) {
				pyrometer[fount_pyrometers].available = 1;
				pyrometer[fount_pyrometers].address = address;		
				fount_pyrometers++;
			}			
			//clear CS line
			CSCLR;

			address++;
		}
	}
	
	pyro_command = 1;
	
	return 0;
}

/*
 * tries to read the calibration memory of the sensors
 * 0 = if succesful
 * 1 = read error
 * 2 = calibration error
 */
uint8 pyrometer_read_calibration(void) {

	uint16 i;
	uint16 j;
	uint16 index;
	uint16 data;
	uint8 tries;
	uint8 count;
	uint16 result;
	uint16 cs;
	uint8 readerror;
	uint8 calibration_error = 0;
	
	for (i=0; i<NUM_PYROMETERS; i++) {
		
		pyrometer[i].calib_data_valid = 0;
		count = 0;
		tries = 0;
		readerror = 0;
		
		do {
		
			
			tries++;
			if (tries > MAX_NUM_INQUIRIES) 
				return 1;
			
			CSSET;
			
			//read data and store it in pyrometer structure
			data = (uint16) (0x8000 | (pyrometer[i].address << 8));
			spi_read_write_single(data);
			
			for (index=1; index<65; index++) {					
				data = (uint16) (0x8000 | (pyrometer[i].address << 8) | index);
				result = spi_read_write_single(data);
				pyrometer[i].calibration.raw[index-1] = result;
			}
			
			//the memory is read three more times for finding transmission errors
			for (j=0; j<3; j++) {
				data = (uint16) (0x8000 | (pyrometer[i].address << 8));
				spi_read_write_single(data);
				for (index=1; index<65; index++) {					
					data = (uint16) (0x8000 | (pyrometer[i].address << 8) | index);
					result = spi_read_write_single(data);
					if (result != pyrometer[i].calibration.raw[index -1]) 
						readerror = 1;
				}	
			}			
	
			CSCLR;
			
		} while (readerror);
			
		// Read successful, calibration valid?
		for (j=0; j<64;j++) {
			if (pyrometer[i].calibration.raw[j] != 0xFF) {
				pyrometer[i].calib_data_valid = 1;
			}
		}
		
		if(!pyrometer[i].calibration.valid)
			pyrometer[i].calib_data_valid = 0;
				
		if(!pyrometer[i].calib_data_valid)		
			calibration_error = 1;
	}
	
	// Any pyrometer sensor with calibration error?	
	if(calibration_error)
		return 2;
	
	pyrometer_calculate_calibration();
	
	return 0;
}

/* 
 * tries to read the sensor memory of the sensors
 * 0 = if succesful
 * 1 = read error
 
 */
uint8 pyrometer_read_sensordata(void) {

	uint16 i;
	uint16 j;
	uint16 index;
	uint16 data;
	uint8 tries;
	uint8 count;
	uint16 result;
	uint8 readerror;
	
	for (i=0; i<NUM_PYROMETERS; i++) {
		pyrometer[i].sensor_data_valid = 0;
		count = 0;
		tries = 0;
		readerror = 0;
		
		do {
		
			
			tries++;
			if (tries > MAX_NUM_INQUIRIES) 
				return 1;
			
			CSSET;

			data = (uint16) (0x9000 | (pyrometer[i].address << 8));
			spi_read_write_single(data);
			for (index=1; index<65; index++) {					
				data = (uint16) (0x9000 | (pyrometer[i].address << 8) | index);
				result = spi_read_write_single(data);
				pyrometer[i].data.raw[index -1] = result;
			}
			
			//the memory is read three more times for avoiding transmission errors
			for (j=0; j<3; j++) {
				data = (uint16) (0x9000 | (pyrometer[i].address << 8));
				spi_read_write_single(data);
				for (index=1; index<65; index++) {					
					data = (uint16) (0x9000 | (pyrometer[i].address << 8) | index);
					result = spi_read_write_single(data);
					if (result != pyrometer[i].data.raw[index -1]) 
						readerror = 1;
				}	
			}
	
			CSCLR;	
			
		} while (readerror);
		
		// Read successful...
		pyrometer[i].sensor_data_valid = 1;
	}		

	return 0;
}

/* 
 * starts a read of all pyrometers. This function is called periodically. 
 * The results become available in the SPI interrupt, linked to pyrometer_usi_isr()
 */
void pyrometer_readall() {
	uint8 i;
	
	//set CS line
	CSSET;
	//setup command frames
	for (i=0; i<fount_pyrometers; i++) {
		sendbuffer[i] = (uint16)((pyro_command << 12) | (pyrometer[i].address << 8));
	}	
	//store the conversion time (tos ticks from bios)
	last_conversion_time = bios_func.tos_ticks;	
	spi_read_write_several((uint16*)sendbuffer, fount_pyrometers+1);
}


/* 
 * Interrupt is triggered if pyrometer values have been read by the hardware
 */
__interrupt__
void pyrometer_usi_isr(void) {
	
	uint16 i;
	int old_ipl;
	
	old_ipl = asm_set_ipl(SPI_INTERRUPT_LEVEL);
	
	pyrobuffer_writeindex++;
	pyrobuffer_writeindex %= PYROMETER_BUF_SIZE;
	
	//clear CS line
	CSCLR;
	
	//select receive ram
	MCF_QSPI_QAR = 0x00;
	
	//the first data is dummy
	i = MCF_QSPI_QDR;
	
	//store the start of conversion time
	pyrobuffer_conversiontimes[pyrobuffer_writeindex] = last_conversion_time;
	
	//store tha data
	for (i=0; i<fount_pyrometers; i++) {
		pyrobuffer[i][pyrobuffer_writeindex] = MCF_QSPI_QDR >> 4;	
	}	
	
	//clear interrupt flags
	MCF_QSPI_QIR |= MCF_QSPI_QIR_SPIF;
	//disable local interrupts
	MCF_QSPI_QIR |= MCF_QSPI_QIR_SPIFE;

	asm_set_ipl(old_ipl);
}


/*
 * Data processing, called from Mainloop 
 * 0 = No new data available
 * 1 = New data available
 */
uint8 pyrometer_process_data(void) {

	extern uint16 acceleration_x;
	extern uint16 acceleration_y;
	extern uint16 acceleration_z;
	extern uint16 old_acceleration_x;
	extern uint16 old_acceleration_y;
	extern uint16 old_acceleration_z;
	extern int32 velocity_buffer_x;
	extern int32 velocity_buffer_y;
	extern int32 velocity_buffer_z;
	
	uint32 i;
	uint8 diff;
	uint8 new_data = 0;
	
	int old_ipl;
	
	
	//disable spi intrerrupt
	old_ipl = asm_set_ipl(SPI_INTERRUPT_LEVEL);
	
	while (pyrobuffer_readindex != pyrobuffer_writeindex) {
	
		pyrobuffer_readindex++;
		pyrobuffer_readindex %= PYROMETER_BUF_SIZE; 
		
		//calibration lookup
		pyrometer_raw_value0 = pyrobuffer[0][pyrobuffer_readindex];
		temperature_sensor0 = get_calibrated_sample(pyrometer_raw_value0, 0);
		
		//error checking on pyrometer 0
		if (pyrobuffer[0][pyrobuffer_readindex] == 0) {
			if (error_counter < MAX_NUM_ERRORS) 
				error_counter++;
			else 
				node_error |= ERR_OIP_PYR_SENSOR;
		}
		else {
			if (error_counter) 
				error_counter--;	
		}
		
		//pyrometer averaging
		if (pyro_average_len) {
			pyro_average_sum += (uint32)pyrobuffer[0][pyrobuffer_readindex];
			pyro_average_len--;
			if (!pyro_average_len) {
				pyro_average = (uint16)(pyro_average_sum / (uint32)pyro_average_buf);
			}
		}
		new_data = 1;
	}
	
	//reenable spi interrupt
	asm_set_ipl(old_ipl);
	
	return new_data;
}

/*
//this erases the calibration table of the addressed sensor
void  pyrometer_erase_calibration(uint8 address) {
	uint16 data;
	
	CSSET;
	cpu_pause(20);
	data = (uint16) (0x4000 | (address << 8));
	spi_read_write_single(data);
	cpu_pause(20000);
	CSCLR;
	cpu_pause(20);
		
}
*/
//this erases the sensor data table of the addressed sensor
void  pyrometer_erase_sensordata(uint8 address) {
	uint16 data;
	
	CSSET;
	cpu_pause(20);
	data = (uint16) (0x5000 | (address << 8));
	spi_read_write_single(data);
	cpu_pause(20000);
	CSCLR;
	cpu_pause(20);
		
}

//this flashes one byte of calibration data with the given index to the sensor
void  pyrometer_store_calibration(uint8 address, uint8 index, uint8 calibdata) {	
	uint16 data;

	CSSET;
	cpu_pause(20);
	//execute read calibration command
	data = (uint16) (0x8000 | (address << 8) | index);
	spi_read_write_single(data);
	cpu_pause(100);
	//execute set calibration command
	data = (uint16) (0x6000 | (address << 8) | calibdata);
	spi_read_write_single(data);	
	cpu_pause(1000);
	CSCLR;
	cpu_pause(20);	
	
};

//this flashes one byte of sensor data with the given index to the sensor
void  pyrometer_store_sensordata(uint8 address, uint8 index, uint8 sensordata) {	
	uint16 data;

	CSSET;
	cpu_pause(20);
	//execute read sensordata command
	data = (uint16) (0x9000 | (address << 8) | index);
	spi_read_write_single(data);
	cpu_pause(100);
	//execute set senaordata command
	data = (uint16) (0x7000 | (address << 8) | sensordata);
	spi_read_write_single(data);	
	cpu_pause(1000);
	CSCLR;
	cpu_pause(20);	
	
};