/*
compilation step

# gcc -o server server.c -lbluetooth -lpthread

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <iostream>
#include <fcntl.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <sys/time.h>
#include <queue>
#include <footpedal.h>

int s,client, s2, client2;
void ctrl_c_handler(int signal);
void close_sockets();
void *Interpret_thread_func(void *data);
void *Mbluetooth1_sendthread_func(void *data);
void *Mbluetooth1_receivethread_func(void *data);
void *Mbluetooth2_sendthread_func(void *data);
void *Mbluetooth2_receivethread_func(void *data);
FootPedalProtocol ConvertFootPedalProtocol(uint8_t buff[sizeof(FootPedalProtocol)]);

// Time
struct timeval server_time, cur_time;
bool time_request_master_flag = false;
bool time_request_slave_flag = false;

// Thread variables
int thread_return, thread_id, thread_status;
pthread_t Interpret_thread;
pthread_t Mbluetooth1_sendthread, Mbluetooth1_receivethread;
pthread_t Mbluetooth2_sendthread, Mbluetooth2_receivethread;

// Foot Pedal Protocol
FootPedalProtocol footpedaldata;
std::queue<FootPedalProtocol> Interpreter_que;
FILE *footpedal_data;

pthread_mutex_t Imutex = PTHREAD_MUTEX_INITIALIZER;

int main(void){
	std::queue<FootPedalProtocol> empty_que;
	std::swap(Interpreter_que, empty_que);

	//test
	/*
	float testval = -4.63;
	uint8_t tmpbyte;
	if( testval < -2.62 )
		testval = -2.62;
	if( testval > 2.62 )
		testval = 2.62;
	tmpbyte = (testval/2.62)*128 + 127;
	printf("%f %d\n", testval, tmpbyte);
	printf("size: %d %d\n", sizeof(testval), sizeof(tmpbyte));
	*/
	/*
	int32_t a[5];
	int32_t x = 0;
	float b = 123.456;
	printf("%d\n",sizeof(float));
	memcpy(&a[0],&b,sizeof(float));
	memcpy(&x, &b, sizeof(float));
	int8_t c[4];
	memcpy(c, &b, sizeof(float));
	printf("%f %f %f %f\n", b, (float)x, (float)a[0], *((float *)c));

	float cc;
	memcpy(&cc, &a[0], sizeof(float));
	printf("%f %f\n", b, cc);
	*/
	/*
	int i;
	struct timeval test_time, rtest_time;
	int n1 = 1234;
	int n2 = -1234;
	int rn1, rn2;
	unsigned char buff[40];
	while(1){
		printf("==================================\n");
		printf("Type change test start\n");
		gettimeofday(&test_time, NULL);
		printf("s time: %d %d\n", test_time.tv_sec, test_time.tv_usec);
		printf("n1: %d, n2: %d\n", n1, n2);
		for( i = 0 ; i < 8 ; i++ ){
			buff[i] = ( test_time.tv_sec >> (i*8) );
		}
		for( i = 8 ; i < 16 ; i++ ){
			buff[i] = ( test_time.tv_usec >> ((i-8)*8) );
		}
		for( i = 16 ; i < 20 ; i++ ){
			buff[i] = ( n1 >> ((i-16)*8) );
		}
		for( i = 20 ; i < 24 ; i++ ){
			buff[i] = ( n2 >> ((i-20)*8) );
		}


		rtest_time.tv_sec = 0;
		for( i = 0 ; i < 8 ; i++ ){
			rtest_time.tv_sec += ( buff[i] << (i*8) );
		}
		rtest_time.tv_usec = 0;
		for( i = 8 ; i < 16 ; i++ ){
			rtest_time.tv_usec += ( buff[i] << ((i-8)*8) );
		}
		rn1 = 0;
		for( i = 16 ; i < 20 ; i++ ){
			rn1 += ( buff[i] << ((i-16)*8) );
		}
		rn2= 0;
		for( i = 20 ; i < 24 ; i++ ){
			rn2 += ( buff[i] << ((i-20)*8) );
		}

		printf("r time: %d, %d\n", rtest_time.tv_sec, rtest_time.tv_usec);
		printf("rn1: %d, rn2: %d\n", rn1, rn2);
		sleep(5);
	}
	*/
	/*
	struct timeval test_time, rtest_time;
	char buff1[10], buff2[6], buff3[40], intbuff[4];
	char rbuff1[10], rbuff2[6], rintbuff[4];
	int n1 = 1234;
	int n2 = -1234;
	int rn1, rn2;
	while(1){
		printf("==================================\n");
		printf("Type change test start\n");

		gettimeofday(&test_time, NULL);

		sprintf(buff1, "%d\n", test_time.tv_sec);
		sprintf(buff2, "%d\n", test_time.tv_usec);

		memcpy(buff3,buff1,sizeof(buff1));
		memcpy(&buff3[sizeof(buff1)],buff2,sizeof(buff1));
		sprintf(intbuff, "%d\n", n1);
		memcpy(&buff3[sizeof(buff1)+sizeof(buff2)], intbuff, sizeof(intbuff));
		sprintf(intbuff, "%d\n", n2);
		memcpy(&buff3[sizeof(buff1)+sizeof(buff2)+sizeof(intbuff)], intbuff, sizeof(intbuff));

		printf("Time: %f\n", (double)test_time.tv_sec + (double)test_time.tv_usec*1E-6);
		printf("buff1 byte sec[%d]: %s\n", sizeof(buff1), buff1);
		printf("buff2 byte usec[%d]: %s\n", sizeof(buff2), buff2);
		printf("%d %d\n",n1, n2);

		memcpy(rbuff1, buff3, sizeof(rbuff1));
		memcpy(rbuff2, &buff3[sizeof(rbuff1)], sizeof(rbuff2));

		printf("After bluetooth\n");
		printf("rbuff1 byte [%d]: %s\n", sizeof(rbuff1), rbuff1);
		printf("rbuff2 byte [%d]: %s\n", sizeof(rbuff2), rbuff2);
		sscanf(rbuff1, "%d", &rtest_time.tv_sec);
		sscanf(rbuff2, "%d", &rtest_time.tv_usec);
		memcpy(rintbuff, &buff3[sizeof(rbuff1)+sizeof(rbuff2)], sizeof(intbuff));
		sscanf(rintbuff, "%d", &rn1);
		memcpy(rintbuff, &buff3[sizeof(rbuff1)+sizeof(rbuff2)+sizeof(intbuff)], sizeof(intbuff));
		sscanf(rintbuff, "%d", &rn2);

		printf("rTime: %f\n", (double)rtest_time.tv_sec + (double)rtest_time.tv_usec*1E-6);
		printf("%d %d\n",rn1, rn2);

		sleep(10);
	}
	*/


	// File open for test
	footpedal_data = fopen("./footpedal_data.txt", "w");
	if( footpedal_data == NULL )
		printf("Error open file\n");

(void) signal(SIGINT,ctrl_c_handler);

// Find local Bluetooth address
//=======================================
	bdaddr_t local_btaddr, local_btaddr_inv;
    int dev_id, sock;

    char name[248] = { 0 };
    uint8_t bt_addr_int[6];


    dev_id = hci_get_route(NULL);
    sock = hci_open_dev( dev_id );
    if (dev_id < 0 || sock < 0) {
        perror("opening socket");
        exit(1);
    }

    hci_read_local_name(sock, sizeof(name), name, 0);

    if (hci_read_local_name(sock, sizeof(name), name, 1000) < 0) {
    	printf("Can't read local name\n");
    }

    hci_read_bd_addr(sock, &local_btaddr, 1000);

    printf("\t%s\t%s\n", name, batostr(&local_btaddr));
    printf("address: %02X:%02X:%02X:%02X:%02X:%02X\n", local_btaddr.b[5], local_btaddr.b[4], local_btaddr.b[3],
    		local_btaddr.b[2], local_btaddr.b[1], local_btaddr.b[0]);

    for(int i = 0 ; i < 6 ; i++ ){
    	bt_addr_int[i] = local_btaddr.b[5-i];
    	local_btaddr_inv.b[i] = local_btaddr.b[5-i];
    }

    char *bt_addr = batostr(&local_btaddr_inv);
    printf("\t%s\t%s\n", name, bt_addr);
    close( sock );

//=======================================


char *message1 = "Read thread\n";
char *message2 = "Write thread\n";
int iret1, iret2;
thread_id = 1;

struct sockaddr_rc loc_addr={ 0 }, loc_addr2={ 0 }, client_addr={ 0 }, client_addr2={ 0 };
char buf[18] = { 0 };

unsigned int opt = sizeof(client_addr) ;
unsigned int opt2 = sizeof(client_addr2) ;

//allocate socket
s = socket(AF_BLUETOOTH,SOCK_STREAM,BTPROTO_RFCOMM);
s2 = socket(AF_BLUETOOTH,SOCK_STREAM,BTPROTO_RFCOMM);

//bind socket to port 1 of the first available
loc_addr.rc_family = AF_BLUETOOTH;
//00:1A:7D:DA:71:13 // ZIO

//str2ba("00:1A:7D:DA:71:13",&loc_addr.rc_bdaddr);//hci0; server device address is given
//str2ba("94:DB:C9:3A:5F:72",&loc_addr.rc_bdaddr);//hci0; server device address is given
str2ba(bt_addr,&loc_addr.rc_bdaddr);
loc_addr.rc_channel = 3; //port (maximum should be 30 for RFCOMM)

bind(s,(struct sockaddr *)&loc_addr,sizeof(loc_addr));
printf("Bluetooth 1 Binding success\n");


//put socket into listen mode
listen(s,1) ;
printf("Bluetooth 1 socket in listen mode\n");
//accept one connection

client = accept(s,(struct sockaddr *)&client_addr,&opt);
ba2str(&client_addr.rc_bdaddr,buf);
fprintf(stdout,"Bluetooth Connection 1 accepted from %s\n",buf);

/*
//bind socket to port 2 of the first available
loc_addr2.rc_family = AF_BLUETOOTH;
//00:1A:7D:DA:71:13
//94:DB:C9:3A:5F:72 // Ubuntu
str2ba("00:1A:7D:DA:71:13",&loc_addr2.rc_bdaddr);//hci0; server device address is given
//str2ba("94:DB:C9:3A:5F:72",&loc_addr2.rc_bdaddr);//hci0; server device address is given
loc_addr2.rc_channel = 10; //port (maximum should be 30 for RFCOMM)

bind(s2,(struct sockaddr *)&loc_addr2,sizeof(loc_addr2));
printf("Bluetooth 2 Binding success\n");

//put socket into listen mode
listen(s2,1) ;
printf("Bluetooth 2 socket in listen mode\n");
//accept one connection

client2 = accept(s2,(struct sockaddr *)&client_addr2,&opt2);
ba2str(&client_addr2.rc_bdaddr,buf);
fprintf(stdout,"Bluetooth Connection 2 accepted from %s\n",buf);
*/

/* Create independent threads each of which will execute function */

thread_return = pthread_create(&Mbluetooth1_sendthread, NULL, Mbluetooth1_sendthread_func, (void*)&thread_id);
if( thread_return < 0 ){
	perror("thread create error\n");
	exit(0);
}
thread_id++;
thread_return = pthread_create(&Mbluetooth1_receivethread, NULL, Mbluetooth1_receivethread_func, (void*)&thread_id);
if( thread_return < 0 ){
	perror("thread create error\n");
	exit(0);
}
thread_id++;
thread_return = pthread_create(&Mbluetooth2_sendthread, NULL, Mbluetooth2_sendthread_func, (void*)&thread_id);
if( thread_return < 0 ){
	perror("thread create error\n");
	exit(0);
}
thread_id++;
thread_return = pthread_create(&Mbluetooth2_receivethread, NULL, Mbluetooth2_receivethread_func, (void*)&thread_id);
if( thread_return < 0 ){
	perror("thread create error\n");
	exit(0);
}
thread_id++;

thread_return = pthread_create(&Interpret_thread, NULL, Interpret_thread_func, (void*)&thread_id);
if( thread_return < 0 ){
	perror("thread create error\n");
	exit(0);
}

pthread_join(Mbluetooth1_sendthread, (void **)&thread_status);
pthread_join(Mbluetooth1_receivethread, (void **)&thread_status);
pthread_join(Mbluetooth2_sendthread, (void **)&thread_status);
pthread_join(Mbluetooth2_receivethread, (void **)&thread_status);
pthread_join(Interpret_thread, (void **)&thread_status);

close_sockets();
return 0;
}

void *Interpret_thread_func(void *data){
	int status;
	float velx, vely, velph;
	FootPedalProtocol receiptPedal;
	while(1){
		if( !Interpreter_que.empty() ){
			pthread_mutex_lock(&Imutex);
			receiptPedal = Interpreter_que.front();
			printf("\r Interpreting: %f, %d : ", (double)receiptPedal.time[0] + (double)receiptPedal.time[1]*1E-6, receiptPedal.data[0]);
			switch(receiptPedal.data[0]){
				case STOP:
					printf("STOP");
					break;
				case STAND:
					printf("STAND");
					break;
				case WALK:
					printf("WALK");
					break;
				case WALKR:
					printf("WALKR");
					break;
				case WALKL:
					printf("WALKL");
					break;
				case RUN:
					printf("RUN");
					break;
				case RUNR:
					printf("RUNR");
					break;
				case RUNL:
					printf("RUNL");
					break;
				case JUMP:
					printf("JUMP");
					break;
				case LTURN:
					printf("LTURN");
					break;
				case RTURN:
					printf("RTURN");
					break;
				case LSIDEWALK:
					printf("LSIDEWALK");
					break;
				case RSIDEWALK:
					printf("RSIDEWALK");
					break;
				case SIT:
					printf("SIT");
					break;
				case SQUAT:
					printf("SQUAT");
					break;
				case BACKWALK:
					printf("BACKWALK");
					break;
				case BACKWALKR:
					printf("BACKWALKR");
					break;
				case RSIDERUN:
					printf("RSIDERUN");
					break;
				case LSIDERUN:
					printf("LSIDERUN");
					break;
				case BACKWALKL:
					printf("BACKWALKL");
					break;
				case BACKRUN:
					printf("BACKRUN");
					break;
				case BACKRUNR:
					printf("BACKRUNR");
					break;
				case BACKRUNL:
					printf("BACKRUNL");
					break;
				case TIPTOE:
					printf("TIPTOE");
					break;
			}
			memcpy(&velx, &receiptPedal.data[1], sizeof(float));
			memcpy(&vely, &receiptPedal.data[2], sizeof(float));
			memcpy(&velph, &receiptPedal.data[3], sizeof(float));
			printf(", Avatar Vel X: %f, Y: %f, Theta: %f", velx, vely, velph);
			Interpreter_que.pop();
			pthread_mutex_unlock(&Imutex);
			fflush(stdout);
		}
	}
}

void *Mbluetooth1_sendthread_func(void *data){
	FootPedalProtocol fdata;
	int status;
	while(1){
		if( time_request_master_flag == true ){
			time_request_master_flag = false;
			fdata.srcAddr = 0x10;
			fdata.destAddr = 0x20;
			fdata.comm = 0x20;
			gettimeofday(&server_time, NULL);
			fdata.time[0] = server_time.tv_sec;
			fdata.time[1] = server_time.tv_usec;
			status = send(client,&fdata,sizeof(FootPedalProtocol),0);
			//printf("Sent Time to Master : %5.6f\n", (double)fdata.time.tv_sec + (double)fdata.time.tv_usec*1E-6);
			printf("Sent Time to Master : %5.6f\n", (double)server_time.tv_sec + (double)server_time.tv_usec*1E-6);
		}
	}
}

void *Mbluetooth1_receivethread_func(void *data){
	FootPedalProtocol Rdata, Idata, Mdata;
	uint8_t buff[sizeof(FootPedalProtocol)];
	int read_size, i;
	float velx, vely, velth;

	while(1){
		if( time_request_master_flag == false ){
			//read data from the client
			//read_size = recv(client,&fdata,sizeof(FootPedalProtocolBluetooth),0);
			read_size = recv(client,buff,sizeof(FootPedalProtocol),0);
			Rdata = ConvertFootPedalProtocol(buff);
			/*
			Rdata.srcAddr = buff[0];
			Rdata.destAddr = buff[1];
			Rdata.comm = buff[2];
			Rdata.time[0] = 0;
			for( i = 4 ; i < 8 ; i++ ){
				Rdata.time[0] += ( buff[i] << ((i-4)*8) );
			}
			Rdata.time[1] = 0;
			for( i = 8 ; i < 12 ; i++ ){
				Rdata.time[1] += ( buff[i] << ((i-8)*8) );
			}
			Rdata.data[0] = 0;
			for( i = 12 ; i < 16 ; i++ ){
				Rdata.data[0] += ( buff[i] << ((i-12)*8) );
			}
			Rdata.data[1] = 0;
			for( i = 16 ; i < 20 ; i++ ){
				Rdata.data[1] += ( buff[i] << ((i-16)*8) );
			}
			Rdata.data[2] = 0;
			for( i = 20 ; i < 24 ; i++ ){
				Rdata.data[2] += ( buff[i] << ((i-20)*8) );
			}
			Rdata.data[3] = 0;
			for( i = 24 ; i < 28 ; i++ ){
				Rdata.data[3] += ( buff[i] << ((i-24)*8) );
			}
			Rdata.data[4] = 0;
			for( i = 28 ; i < 32 ; i++ ){
				Rdata.data[4] += ( buff[i] << ((i-28)*8) );
			}
			*/

			if( Rdata.srcAddr == 0x20 && Rdata.destAddr == 0x10 && Rdata.comm == 0x10 && read_size > 0 ){
				printf("Time request receipt from Master\n");
				time_request_master_flag = true;
			}
			else if( Rdata.srcAddr == 0x20 && Rdata.destAddr == 0x10 && Rdata.comm == 0x50 && read_size > 0 ){
				//printf("Receipt Interpreted Data\n");
				//printf("srcAddr: %x, destAddr: %x, Comm: %x\n",Rdata.srcAddr, Rdata.destAddr, Rdata.comm);
				//printf("Time: %f\n", (double)Rdata.time[0] + (double)Rdata.time[1]*1E-6);
				//memcpy(&velx, &Rdata.data[1], sizeof(float));
				//memcpy(&vely, &Rdata.data[2], sizeof(float));
				//memcpy(&velth, &Rdata.data[3], sizeof(float));
				//printf("data: %d %f %f %f %d\n", Rdata.data[0], velx, vely, velth, Rdata.data[4]);
				//fprintf(footpedal_data,"%f %d %f %f %f %d\n", (double)Rdata.time[0] + (double)Rdata.time[1]*1E-6, Rdata.data[0], velx, vely, velth, Rdata.data[4]);
				pthread_mutex_lock(&Imutex);
				Interpreter_que.push(Rdata);
				pthread_mutex_unlock(&Imutex);
				//Idata = Rdata;
			}
			else if( Rdata.srcAddr == 0x20 && Rdata.destAddr == 0x10 && Rdata.comm == 0xB0 && read_size > 0 ){
				Mdata = Rdata;
				printf("\n %f sec: Master X,Y : (%d, %d), Slave X,Y: (%d, %d)\n", (double)Rdata.time[0] + (double)Rdata.time[1]*1E-6, Rdata.data[0], Rdata.data[1], Rdata.data[2], Rdata.data[3]);
			}

			/*
			printf("\r Interpreting: %5.6f %d : ", (double)Idata.time[0] + (double)Idata.time[1]*1E-6, Idata.data[0]);
			switch(Idata.data[0]){
				case STOP:
					printf("STOP");
					break;
				case STAND:
					printf("STAND");
					break;
				case WALK:
					printf("WALK");
					break;
				case WALKR:
					printf("WALKR");
					break;
				case WALKL:
					printf("WALKL");
					break;
				case RUN:
					printf("RUN");
					break;
				case RUNR:
					printf("RUNR");
					break;
				case RUNL:
					printf("RUNL");
					break;
				case JUMP:
					printf("JUMP");
					break;
				case LTURN:
					printf("LTURN");
					break;
				case RTURN:
					printf("RTURN");
					break;
				case LSIDEWALK:
					printf("LSIDEWALK");
					break;
				case RSIDEWALK:
					printf("RSIDEWALK");
					break;
				case SIT:
					printf("SIT");
					break;
				case SQUAT:
					printf("SQUAT");
					break;
				case BACKWALK:
					printf("BACKWALK");
					break;
				case BACKWALKR:
					printf("BACKWALKR");
					break;
				case BACKWALKL:
					printf("BACKWALKL");
					break;
				case BACKRUN:
					printf("BACKRUN");
					break;
				case BACKRUNR:
					printf("BACKRUNR");
					break;
				case BACKRUNL:
					printf("BACKRUNL");
					break;
				case TIPTOE:
					printf("TIPTOE");
					break;
			}

			printf(": Master X,Y : (%d, %d), Slave X,Y: (%d, %d)", Mdata.data[0], -1*Mdata.data[1], Mdata.data[2], -1*Mdata.data[3]);
			*/
		}
	}
}

void *Mbluetooth2_sendthread_func(void *data){
	FootPedalProtocol fdata;
	int status;
	while(1){

		if( time_request_slave_flag == true ){
			time_request_slave_flag = false;
			fdata.srcAddr = 0x10;
			fdata.destAddr = 0x30;
			fdata.comm = 0x20;
			gettimeofday(&server_time, NULL);
			fdata.time[0] = server_time.tv_sec;
			fdata.time[1] = server_time.tv_usec;
			status = send(client2,&fdata,sizeof(FootPedalProtocol),0);
			printf("Sent Time to Master : %5.6f\n", (double)server_time.tv_sec + (double)server_time.tv_usec*1E-6);
			//printf("Sent Time to Slave : %5.6f\n", (double)fdata.time.tv_sec + (double)fdata.time.tv_usec*1E-6);
		}
	}
}

void *Mbluetooth2_receivethread_func(void *data){
	FootPedalProtocol Rdata;
	uint8_t buff[sizeof(FootPedalProtocol)];
	int read_size;
	while(1){
		if( time_request_slave_flag == false ){
			//read data from the client
			read_size = recv(client2,buff,sizeof(FootPedalProtocol),0);
			Rdata.srcAddr = buff[0];
			Rdata.destAddr = buff[1];
			Rdata.comm = buff[2];
			if( Rdata.srcAddr == 0x30 && Rdata.destAddr == 0x10 && Rdata.comm == 0x10 && read_size > 0 ){
				printf("Time request receipt from Slave\n");
				time_request_slave_flag = true;
			}
		}
	}
}

void close_sockets(){
	//close connection
	close(client);
	close(s);
	close(client2);
	close(s2);
	printf("sockets closed\n");
}

void ctrl_c_handler(int signal) {
	printf("Catched signal: %d ... !!\n", signal);
	close_sockets();
	exit(0);
	//(void) signal(SIGINT, SIG_DFL);
}

FootPedalProtocol ConvertFootPedalProtocol(uint8_t buff[sizeof(FootPedalProtocol)]){
	int i;
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
	/*
	for( i = 4 ; i < 8 ; i++ ){
		res.time[0]  += ( buff[i] << ((i-4)*8) );
	}
	res.time[1] = 0;
	for( i = 8 ; i < 12 ; i++ ){
		res.time[1]  += ( buff[i] << ((i-8)*8) );
	}
	res.data[0] = 0;
	for( i = 12 ; i < 16 ; i++ ){
		res.data[0]  += ( buff[i] << ((i-12)*8) );
	}
	res.data[1] = 0;
	for( i = 16 ; i < 20 ; i++ ){
		res.data[1]  += ( buff[i] << ((i-16)*8) );
	}
	res.data[2] = 0;
	for( i = 20 ; i < 24 ; i++ ){
		res.data[2]  += ( buff[i] << ((i-20)*8) );
	}
	res.data[3] = 0;
	for( i = 24 ; i < 28 ; i++ ){
		res.data[3]  += ( buff[i] << ((i-24)*8) );
	}
	res.data[4] = 0;
	for( i = 28 ; i < 32 ; i++ ){
		res.data[4]  += ( buff[i] << ((i-32)*8) );
	}
	*/
	return res;
}
