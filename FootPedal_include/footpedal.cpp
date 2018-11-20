#include "footpedal.h"
#include <stdlib.h>

int daemonize(void)
{
	pid_t	pid;

	// daemon initialzation
	if ( (pid = fork()) < 0 )
		return -1;
	else if (pid != 0)
		exit(0);		// parent goes bye-bye
		
	// child continues
	setsid();			// become session leader
	chdir("/");			// change working directory
	umask(0);			// clear our file mode creation mask
	return 0;
}

//=======================================================================================
int mask_signals(sigset_t* p_sigset)
{
	int		retcode;
	// Mask Signal
	signal( SIGPIPE, SIG_IGN );
	sigemptyset(p_sigset);
	sigaddset(p_sigset, SIGTERM);
	sigaddset(p_sigset, SIGINT);
	sigaddset(p_sigset, SIGUSR1);
	sigaddset(p_sigset, SIGUSR2);

	if ((retcode = pthread_sigmask(SIG_BLOCK, p_sigset, NULL)) != 0)
		fprintf(stderr, "ERROR(%d) pthread_sigmask err %d\n", __LINE__, retcode);
	return 0;
}


//=======================================================================================
int wait_signals(sigset_t* p_sigset)
{
	int			signum;
	
	FILE* fp;
	if ( (fp=fopen( PID_FILE_PATH, "wt")) == NULL )
	{
		fprintf(stderr, "[%s] fopen %s fail \n", __FUNCTION__, PID_FILE_PATH);
	}
	else
	{
		fprintf(fp, "%d\n", getpid());
		fclose( fp );
	}

	while (1)
	{
		sigwait(p_sigset, &signum);
		if (signum == SIGINT)
		{
			fprintf(stderr, "\nGot SIGINT\n");
			break;
		}
		else if (signum == SIGTERM)
		{
			fprintf(stderr, "\nGot SIGTERM\n");
			break;
		}
		else
			fprintf(stderr, "\nGot %d signal\n",signum);
	}
	remove(PID_FILE_PATH);
	return signum;
}

int str2uuid( const char *uuid_str, uuid_t *uuid ){
    uint32_t uuid_int[4];
    char *endptr;

    if( strlen( uuid_str ) == 36 ) {
        // Parse uuid128 standard format: 12345678-9012-3456-7890-123456789012
        char buf[9] = { 0 };

        if( uuid_str[8] != '-' && uuid_str[13] != '-' &&
            uuid_str[18] != '-'  && uuid_str[23] != '-' ) {
            return 0;
        }
        // first 8-bytes
        strncpy(buf, uuid_str, 8);
        uuid_int[0] = htonl( strtoul( buf, &endptr, 16 ) );
        if( endptr != buf + 8 ) return 0;

        // second 8-bytes
        strncpy(buf, uuid_str+9, 4);
        strncpy(buf+4, uuid_str+14, 4);
        uuid_int[1] = htonl( strtoul( buf, &endptr, 16 ) );
        if( endptr != buf + 8 ) return 0;

        // third 8-bytes
        strncpy(buf, uuid_str+19, 4);
        strncpy(buf+4, uuid_str+24, 4);
        uuid_int[2] = htonl( strtoul( buf, &endptr, 16 ) );
        if( endptr != buf + 8 ) return 0;

        // fourth 8-bytes
        strncpy(buf, uuid_str+28, 8);
        uuid_int[3] = htonl( strtoul( buf, &endptr, 16 ) );
        if( endptr != buf + 8 ) return 0;

        if( uuid != NULL ) sdp_uuid128_create( uuid, uuid_int );
    } else if ( strlen( uuid_str ) == 8 ) {
        // 32-bit reserved UUID
        uint32_t i = strtoul( uuid_str, &endptr, 16 );
        if( endptr != uuid_str + 8 ) return 0;
        if( uuid != NULL ) sdp_uuid32_create( uuid, i );
    } else if( strlen( uuid_str ) == 4 ) {
        // 16-bit reserved UUID
        int i = strtol( uuid_str, &endptr, 16 );
        if( endptr != uuid_str + 4 ) return 0;
        if( uuid != NULL ) sdp_uuid16_create( uuid, i );
    } else {
        return 0;
    }

    return 1;
}

int open_hci_dev(int *dev_id){
	int socket = -1;
	struct hci_dev_info dev_info;
	*dev_id = hci_get_route(NULL);
	if (*dev_id < 0) {
		perror("No Bluetooth Adapter Available");
		return -1;
	}

	if (hci_devinfo(*dev_id, &dev_info) < 0) {
		perror("Can't get device info");
		return -1;
	}

	socket = hci_open_dev( *dev_id );
	if (socket < 0) {
		perror("HCI device open failed");
		return -1;
	}
	return socket;
}

int scan_bt_dev(int dev_id, bdaddr_t *bdaddr) {
	inquiry_info *info = NULL;
	int num_rsp = 0, i;
	char addr[19] = { 0 };
	char name[248] = { 0 };

	printf("Scanning ...\n");
	do {
		num_rsp = hci_inquiry(dev_id, 8 /* ~10 seconds */, num_rsp, NULL, &info, 0);
		if (num_rsp < 0) {
			perror("Inquiry failed");
			return num_rsp;
		}

		for (i = 0; i < num_rsp; i++) {
			ba2str(&(info+i)->bdaddr, addr);
			if (strcmp(M4K_CONTROLLER_MAC, addr) != 0 ) {
				continue;
			} else {
				printf("Found %s\n", addr);
				str2ba(addr, bdaddr);
				// free(info);
				return num_rsp;
			}
			continue;
		}
	} while (1); 
	
	return num_rsp;
}

int search_rc_channel(sdp_session_t *session, uuid_t *uuid) {
	int err;
	int loco_channel = -1;
	int responses = 0;
	sdp_list_t *rsp = NULL, *search_list, *attrid_list;
	sdp_record_t *rec;

	search_list = sdp_list_append( 0, uuid );
	uint32_t range = 0x0000ffff;
	attrid_list = sdp_list_append( 0, &range );
	err = 0;
	err = sdp_service_search_attr_req( session, search_list, SDP_ATTR_REQ_RANGE, attrid_list, &rsp);

	for (; rsp; rsp = rsp->next ) {
			responses++;
			rec = (sdp_record_t*) rsp->data;
			sdp_list_t *proto_list;
			
			// get a list of the protocol sequences
			if( sdp_get_access_protos( rec, &proto_list ) == 0 ) {
			sdp_list_t *p = proto_list;

				// go through each protocol sequence
				for( ; p ; p = p->next ) {
						sdp_list_t *pds = (sdp_list_t*)p->data;

						// go through each protocol list of the protocol sequence
						for( ; pds ; pds = pds->next ) {

								// check the protocol attributes
								sdp_data_t *d = (sdp_data_t*)pds->data;
								int proto = 0;
								for( ; d; d = d->next ) {
										switch( d->dtd ) { 
												case SDP_UUID16:
												case SDP_UUID32:
												case SDP_UUID128:
														proto = sdp_uuid_to_proto( &d->val.uuid );
														break;
												case SDP_UINT8:
														if( proto == RFCOMM_UUID ) {
																printf("rfcomm channel: %d\n",d->val.int8);
																loco_channel = d->val.int8;
																return loco_channel;
														}
														break;
										}
								}
						}
						sdp_list_free( (sdp_list_t*)p->data, 0 );
				}
				sdp_list_free( proto_list, 0 );

			}
			if (loco_channel > 0)
				break;

	}
	printf("Number of Responses %d\n", responses);

	return loco_channel;
}

void SHAGenerator(int pinNum, int pinDir, int* state, int tpusec, int tdusec, int tickNum){
	//pinMode(pinNum, OUTPUT);
	while(tickNum > 0){
		digitalWrite(pinDir, 1);
		if( *state == 1 ){
			digitalWrite(pinNum, 0);
			*state = 0;
		}
		else{
			digitalWrite(pinNum, 1);
			*state = 1;
		}
		usleep(tpusec);
		tickNum--;
	}
	usleep(tdusec);
	digitalWrite(pinDir, 0);
	//pinMode(pinNum, INPUT);
}

void HapticGenerator(int pinNum, int pinDir, int* state, int tickNum, int period, int periodNum){
	int i;
	int operatingtime = TPTIME*tickNum - TDTIME;
	if( operatingtime > period )
		printf("Operating time must be higher than period\n");
	else{
		for( i = 0 ; i < periodNum ; i++ ){
			SHAGenerator(pinNum, pinDir, state, TPTIME, TDTIME, tickNum);
			usleep(period - operatingtime);
		}
	}
}

FootPedalProtocol ConvertFootPedalProtocol(uint8_t buff[sizeof(FootPedalProtocol)]){
	FootPedalProtocol res;
	res.srcAddr = buff[0];
	res.destAddr = buff[1];
	res.comm = buff[2];
	res.time[0] = 0;
	memcpy(&res.time[0], &buff[4], sizeof(float));
	memcpy(&res.time[1], &buff[8], sizeof(float));
	memcpy(&res.data[0], &buff[12], sizeof(float));
	memcpy(&res.data[1], &buff[16], sizeof(float));
	memcpy(&res.data[2], &buff[20], sizeof(float));
	memcpy(&res.data[3], &buff[24], sizeof(float));
	memcpy(&res.data[4], &buff[28], sizeof(float));
	return res;
}

FootMouseHaptic ConvertFootMouseHaptic(uint8_t buff[sizeof(FootMouseHaptic)]){
	FootMouseHaptic res;
	res.srcAddr = buff[0];
	res.destAddr = buff[1];
	res.comm = buff[2];
	//printf("buff: %0x, %0x, %0x, %0x\n", buff[4], buff[5], buff[6], buff[7]);
	memcpy(&res.data, &buff[4], sizeof(int32_t));
	return res;
}

FootMouseHapticImpulse ConvertFootMouseHapticImpulse(uint8_t buff[sizeof(FootMouseHapticImpulse)]){
	FootMouseHapticImpulse res;
	res.srcAddr = buff[0];
	res.destAddr = buff[1];
	res.comm = buff[2];
	//printf("buff: %0x, %0x, %0x, %0x\n", buff[4], buff[5], buff[6], buff[7]);
	res.state = buff[3];
	return res;
}

void PrintVL53l0XError(VL53L0X_Error Status) {
	char buf[VL53L0X_MAX_STRING_LENGTH];
	VL53L0X_GetPalErrorString(Status, buf);
	printf("API Status: %i : %s\n", Status, buf);
}

VL53L0X_Error WaitMeasurementDataReady(VL53L0X_DEV Dev) {
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint8_t NewDatReady = 0;
	uint32_t LoopNb;

	// Wait Measurement Data Ready until it finished
	// use timeout to avoid deadlock
	if (Status == VL53L0X_ERROR_NONE) {
		LoopNb = 0;
		do {
			Status = VL53L0X_GetMeasurementDataReady(Dev, &NewDatReady);
			if ((NewDatReady == 0x01) || Status != VL53L0X_ERROR_NONE) {
				break;
			}
			LoopNb = LoopNb + 1;
			VL53L0X_PollingDelay(Dev);
		} while (LoopNb < VL53L0X_DEFAULT_MAX_LOOP);

		if (LoopNb >= VL53L0X_DEFAULT_MAX_LOOP) {
			Status = VL53L0X_ERROR_TIME_OUT;
		}
	}

	return Status;
}

VL53L0X_Error WaitStopCompleted(VL53L0X_DEV Dev) {
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint32_t StopCompleted = 0;
	uint32_t LoopNb;

	// Wait Stop to be completed until it finished
	// use timeout to avoid deadlock
	if (Status == VL53L0X_ERROR_NONE) {
		LoopNb = 0;
		do {
			Status = VL53L0X_GetStopCompletedStatus(Dev, &StopCompleted);
			if ((StopCompleted == 0x00) || Status != VL53L0X_ERROR_NONE) {
				break;
			}
			LoopNb = LoopNb + 1;
			VL53L0X_PollingDelay(Dev);
		} while (LoopNb < VL53L0X_DEFAULT_MAX_LOOP);

		if (LoopNb >= VL53L0X_DEFAULT_MAX_LOOP) {
			Status = VL53L0X_ERROR_TIME_OUT;
		}

	}

	return Status;
}

VL53L0X_Error MeasureInitialization(VL53L0X_Dev_t *pMyDevice){
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint32_t refSpadCount;
	uint8_t isApertureSpads, VhvSettings, PhaseCal;

	if (Status == VL53L0X_ERROR_NONE) {
		printf("Call of VL53L0X_StaticInit\n");
		Status = VL53L0X_StaticInit(pMyDevice); // Device Initialization
		// StaticInit will set interrupt by default
		PrintVL53l0XError(Status);
	}
	if (Status == VL53L0X_ERROR_NONE) {
		printf("Call of VL53L0X_PerformRefCalibration\n");
		Status = VL53L0X_PerformRefCalibration(pMyDevice, &VhvSettings,	&PhaseCal); // Device Initialization
		PrintVL53l0XError(Status);
	}

	if (Status == VL53L0X_ERROR_NONE) {
		printf("Call of VL53L0X_PerformRefSpadManagement\n");
		Status = VL53L0X_PerformRefSpadManagement(pMyDevice, &refSpadCount,	&isApertureSpads); // Device Initialization
		PrintVL53l0XError(Status);
	}

	if (Status == VL53L0X_ERROR_NONE) {
		printf("Call of VL53L0X_SetDeviceMode\n");
		Status = VL53L0X_SetDeviceMode(pMyDevice, VL53L0X_DEVICEMODE_CONTINUOUS_RANGING); // Setup in single ranging mode
		PrintVL53l0XError(Status);
	}

	if (Status == VL53L0X_ERROR_NONE) {
		printf("Call of VL53L0X_StartMeasurement\n");
		Status = VL53L0X_StartMeasurement(pMyDevice);
		PrintVL53l0XError(Status);
	}

	return Status;
}

VL53L0X_RangingMeasurementData_t MeasureDistance(VL53L0X_Dev_t *pMyDevice){
	VL53L0X_RangingMeasurementData_t RangingMeasurementData;
	VL53L0X_RangingMeasurementData_t *pRangingMeasurementData =	&RangingMeasurementData;
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;

	if (Status == VL53L0X_ERROR_NONE) {
		Status = VL53L0X_GetRangingMeasurementData(pMyDevice, pRangingMeasurementData);
		// Clear the interrupt
		VL53L0X_ClearInterruptMask(pMyDevice, VL53L0X_REG_SYSTEM_INTERRUPT_GPIO_NEW_SAMPLE_READY);
		// VL53L0X_PollingDelay(pMyDevice);
	}

	return RangingMeasurementData;
}
