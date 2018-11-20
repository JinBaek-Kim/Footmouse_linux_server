#ifndef FOOTPEDAL_H_
#define FOOTPEDAL_H_

#include <sys/time.h>
#include "vl53l0x_api.h"
#include "vl53l0x_platform.h"
#include <wiringPi.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <queue>
//#include <chrono>

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>
#include <bluetooth/rfcomm.h>

// for M4K
typedef struct m4kbluetooth{
	uint8_t comm;
	uint8_t type;
	uint8_t linear;
	uint8_t angular;
}M4KBluetooth;

// Mouse input data
typedef struct footpedaldata{
	struct timeval time;
	int x;
	int y;
	int delx;
	int dely;
	float velx;
	float vely;
	float accx;
	float accy;
	double delt;
}FootPedalData;

typedef struct avatardirect{
	bool direct[8];
	uint8_t dcount[8];
}AvatarDirect;

// DPI = dot per inch
// 1 m = 39.370079 inch
// 1000 DPI -> 39.37 dot / mm

typedef struct footmousedata{
	uint8_t srcAddr;
	uint8_t destAddr;
	uint8_t comm;
	int32_t data[2];
}FootMouseData;

typedef struct footmousehapticimpulse{
	uint8_t srcAddr;
	uint8_t destAddr;
	uint8_t comm;
	uint8_t state;
}FootMouseHapticImpulse;

typedef struct footmousehaptic{
	uint8_t srcAddr;
	uint8_t destAddr;
	uint8_t comm;
	uint8_t blank;
	int32_t data;
}FootMouseHaptic;

typedef struct footpedalprotocol{
	uint8_t srcAddr;
	uint8_t destAddr;
	uint8_t comm;
	uint8_t blank;
	int32_t time[2];
	int32_t data[5];
}FootPedalProtocol;

typedef struct footpedalprotocolbluetooth{
	short srcAddr;
	short destAddr;
	short comm;
	char sec_time[30];
	char usec_time[30];
	char data1[2][30];
	char data2[4][30];
}FootPedalProtocolBluetooth;

// For RPI3
//#define RPI3
// For RPI Zero
#define RPIZERO
// For Banana Pi Zero
//#define BPIZERO

// For Interpreter builtin
#define INTERPRETERINPC

// For Daemonize
//#define DAEMONIZE
#define	PID_FILE_PATH	"/var/run/footmouse.pid"

// M4K
#define M4K_CONTROLLER_MAC "A8:7C:01:E1:DC:4D"
//#define M4K_CONTROLLER_MAC "00:34:DA:D6:CC:31"
#define M4K_CONTROLLER_UUID "fa87c0d0-afac-11de-8a39-0800200c9a66"
#define BDADDR_ANY_INITIALIZER   {{0, 0, 0, 0, 0, 0}}

// CRSF
#define CRSF_BLUETOOTH_MAC "F8:28:19:67:0A:5C"

// TEST Server
#define TEST_BLUETOOTH_MAC "00:1A:7D:DA:71:13" // Zio-BT
//#define TEST_BLUETOOTH_MAC "94:DB:C9:3A:5F:72"
#define AORUS_BLUETOOTH_MAC "9C:B6:D0:18:F7:A8"

// WIFI Direct Addr
#define MASTERWIFIADDR "123.123.1.1"
#define SLAVEWIFIADDR "123.123.1.3"

// Bluetooth Channel
#define FOOTSERVER_BLUETOOTH_MASTER_CHPORT 3
#define FOOTSERVER_BLUETOOTH_SLAVE_CHPORT 10

// usec UNIT
#define SEC1 1000000

// Mouse Sampling Time
#define M_MOUSESAMPLETIME 1.0 // 1.0 ms
#define S_MOUSESAMPLETIME 1.0 // 1 ms

// Addr
#define FOOTSERVER 0x10
#define FOOTMASTER 0x20
#define FOOTSLAVE 0x30

// Command
#define REQUESTTIME 0x10
#define REPLYTIME 0x20
#define MASTERDATA 0x30
#define SLAVEDATA 0x40
#define ALIGNMENT 0x50
#define INTERPRET 0x60
#define RESET 0x70
#define HAPTICTOE 0x80
#define HAPTICHEEL 0x90
#define BETWEENDIST 0xA0
#define TCPRECEIPT 0xB0
#define PEDALDATA 0xC0
#define TUNEPARA 0xD0
#define HAPTICALL 0xE0

// Alignment Distance
#define ALIGNMINDIST 200
#define ALIGNMAXDIST 300
#define RESETCOUNT 5 

#define RESENDTIME 1E-2 // sec
#define RESETTIME 1000 // multiply PERIODICTIME

// Avatar Velocity
#define VXSPRINTMAX 1000 // 1000 cm/s
#define VXRUNMAX 400 // 400 cm/s
#define VYRUNMAX 100  // 800 cm/s
#define VXWALKMAX 100 // 100 cm/s
#define VYWALKMAX 50 // 50 cm/s
#define VTHBASE 10*M_PI/180 // 10 degree/s
#define VTHMAX M_PI/180 // 180 degree/s
#define VMINVALUE 0.1 // Min Velocity from Hanyang univ.
#define VMAXVALUE 2.62 // Max Velocity from Hanyanag univ.
#define VSIDEMAXVALUE 3.0 // Max Side Velocity from Hanyanag univ.

// -------------- Area Based Interpreter ----------------
// Footpedal Mouse Margin for stop-zone.
// if footpedal is insied this area, stop
// Master is left, Slave is right
#define AREASCALE 3
#define MXSMARGIN 600*AREASCALE
#define MYSMARGIN 600*AREASCALE
#define SXSMARGIN 600*AREASCALE
#define SYSMARGIN 600*AREASCALE
// Footpedal Mouse Margin for walk-zone
// if footpedal outside this area, walk until run-zone
#define MXWMARGIN 1500*AREASCALE
#define MYWMARGIN 1500*AREASCALE
#define SXWMARGIN 1500*AREASCALE
#define SYWMARGIN 1500*AREASCALE
// Footpedal Mouse Margin for run-zone
// if footpedal outside this area, run according to velocity
#define MXRMARGIN 3000*AREASCALE
#define MYRMARGIN 3000*AREASCALE
#define SXRMARGIN 3000*AREASCALE
#define SYRMARGIN 3000*AREASCALE

// -------------- Velocity Based Interpreter ----------------

#ifdef RPIZERO
	// ------------- for RPI Zero -------------------
	#define PERIODICTIME 1E-2 // sec
	#define MXWEIGHT 1
	#define MYWEIGHT 1 // Master To compensate X Y counting unbalance
	#define SXWEIGHT 1
	#define SYWEIGHT 1 // Slave To compensate X Y counting unbalance
	// Window Size for Average Non-zero Inputs
	#define AVATARDIRECTCOUNT 5  // For MOT Decision Counter Size
	#define COUPLEDCOUNT 20 // For Coupled Mode Decision Counter Size
	#define TIMEMARGIN 2000 //2ms
	#define WAITTIME 1E-3 //1ms
	#define AVGWINDOWSIZE 200 // Queue Size
	#define VELOCITYRATIO 1E-2  // Normaling factor for Raw Ratio -> Reasonable velocity by trial and error since depending Avatar Velocity
	#define MINVELX 8.0 // for saturator
	#define MINVELY 10.0 // for saturator
	#define MINVELTH 1.0 // for saturator
	#define MAXVELX 3.0 // for saturator
	#define MAXVELY 3.0 // for saturator
	#define MAXVELTH 3.0 // for saturator
	#define AVATARMINVELX 0.25 // for MOT
	#define AVATARMINVELY 0.4 // for MOT
	#define AVATARMINVELTH 0.3 // for MOT
	#define AVATARJUMPMARGIN 0.2
	#define AVATARMAXVELTH 2.5
	// ----------------------------------------------
#endif
#ifdef BPIZERO
	// ------------- for BPI Zero -------------------
	#define PERIODICTIME 1E-2 // sec
	#define MXWEIGHT 1
	#define MYWEIGHT 1 // Master To compensate X Y counting unbalance
	#define SXWEIGHT 1
	#define SYWEIGHT 1 // Slave To compensate X Y counting unbalance
	// Window Size for Average Non-zero Inputs
	#define AVATARDIRECTCOUNT 5  // For MOT Decision Counter Size
	#define COUPLEDCOUNT 20 // For Coupled Mode Decision Counter Size
	#define TIMEMARGIN 2000 //2ms
	#define WAITTIME 1E-3 //1ms
	#define AVGWINDOWSIZE 500 // Queue Size
	#define VELOCITYRATIO 5E-3  // Normaling factor for Raw Ratio -> Reasonable velocity by trial and error since depending Avatar Velocity
	#define MINVELX 10.0 // for saturator
	#define MINVELY 10.0 // for saturator
	#define MINVELTH 1.0 // for saturator
	#define MAXVELX 3.0 // for saturator
	#define MAXVELY 3.0 // for saturator
	#define MAXVELTH 3.0 // for saturator
	#define AVATARMINVELX 0.5 // for MOT
	#define AVATARMINVELY 0.5 // for MOT
	#define AVATARMINVELTH 0.4 // for MOT
	#define AVATARJUMPMARGIN 0.2
	#define AVATARMAXVELTH 2.5
	// ----------------------------------------------
#endif
#ifdef RPI3
	// ------------------ for RPI3 ---------------------------
	#define PERIODICTIME 1E-3 // sec
	#define MXWEIGHT 1
	#define MYWEIGHT 1 // Master To compensate X Y counting unbalance
	#define SXWEIGHT 1
	#define SYWEIGHT 1 // Slave To compensate X Y counting unbalance
	// Window Size for Average Non-zero Inputs
	#define AVATARDIRECTCOUNT 10  // For MOT Decision Counter Size
	#define COUPLEDCOUNT 70 // For Coupled Mode Decision Counter Size
	#define TIMEMARGIN 2000 //2ms
	#define WAITTIME 1E-3 //1ms
	#define AVGWINDOWSIZE 1000 // Queue Size
	#define VELOCITYRATIO 0.008  // Normaling factor for Raw Ratio -> Reasonable velocity by trial and error since depending Avatar Velocity
	#define MINVELX 1.5 // for saturator
	#define MINVELY 1.5 // for saturator
	#define MINVELTH 1.5 // for saturator
	#define MAXVELX 3.0 // for saturator
	#define MAXVELY 3.0 // for saturator
	#define MAXVELTH 3.0 // for saturator
	#define AVATARMINVELX 0.4 // for MOT
	#define AVATARMINVELY 0.4 // for MOT
	#define AVATARMINVELTH 0.1 // for MOT
	#define AVATARJUMPMARGIN 0.1
	#define AVATARMAXVELTH 2.5
	// -----------------------------------------------------
#endif

// Not used.....
#define MAXWALKVELX 100.0
#define MAXWALKVELY 100.0
#define MAXRUNVELX 200.0
#define MAXRUNVELY 200.0
#define AVATARXVELMARGIN 0.5
#define AVATARXVELMARGIN_2 AVATARXVELMARGIN*2.0
#define AVATARYVELMARGIN 0.5
#define AVATARYVELMARGIN_2 AVATARYVELMARGIN*2.0
#define AVATARYVELMARGIN_12 AVATARYVELMARGIN/2.0
#define AVATARTHVELMARGIN 50.0
#define MASTERXMARGINVEL 50.0
#define MASTERYMARGINVEL 50.0
#define SLAVEXMARGINVEL 50.0
#define SLAVEYMARGINVEL 50.0

typedef struct footmousevel{
	float avg_velx;
	float avg_vely;
	std::queue<float> velx_que;
	std::queue<float> vely_que;
}FootMouseVel;

// Decoupled Directional Number
#define FORWARD 0
#define FORWARDRIGHT 1
#define RIGHT 2
#define BACKWARDRIGHT 3
#define BACKWARD 4
#define BACKWARDLEFT 5
#define LEFT 6
#define FORWARDLEFT 7

// From Footpedal To CRSF, DATA[0] definition
#define STOP  0

#define WALK  100
#define WALKR 110
#define RSIDEWALK 120
#define BACKWALKR 130
#define BACKWALK 140
#define BACKWALKL 150
#define LSIDEWALK 160
#define WALKL 170

#define RUN   200
#define RUNR 210
#define RSIDERUN 220
#define BACKRUNR 230
#define BACKRUN 240
#define BACKRUNL 250
#define LSIDERUN 260
#define RUNL 270

#define RTURN 500
#define LTURN 510
#define JUMP  520
#define SQUAT 530
#define SIT   540
#define TIPTOE 550
#define STAND 560

#define NOTCONFIRM 999


// Avatar State
#define AVATARWALK 0x10
#define AVATARBACKWALK 0x20
#define AVATARRUN 0x30
#define AVATARBACKRUN 0x40
#define AVATARTURN 0x50

// ---------------------
// ----- GPIO pin

// #define ACTLED 0 // based on wiringPi -> 17 based on BCM at /boot/config.txt

// Haptic
#define HAPTIC2 23 // based on wiringPi, Toe
#define HAPTICDIR2 24 // based on wiringPi
#define HAPTIC1 27 // based on wiringPi, Heel
#define HAPTICDIR1 28 // based on wiringPi
// VL53L0X
#define VL53L0X1GPIO 2 // based on wiringPi
#define VL53L0X1XSHUT 3 // based on wiringPi
#define VL53L0X2GPIO 21 // based on wiringPi
#define VL53L0X2XSHUT 22 // based on wiringPi

// ---------------------

// For Haptic
#define TPTIME 65000 // us : interval width
#define TFTIME 100000 // us : Last interval width = Tp+Td
#define TPTIME2 85000 // us
#define TDTIME 35000 // us : necessary interval for last

// From Footpedal To CRSF, DATA[1] definition
#define FOOT_LEFT 10 // Left foot is swing foot
#define FOOT_RIGHT 11 // Right foot is swing foot

// VL53L0X version
#define VERSION_REQUIRED_MAJOR 1
#define VERSION_REQUIRED_MINOR 0
#define VERSION_REQUIRED_BUILD 1

// Address for VL53L0X
#define VL53L0XINITADDR 0x29
#define VL53L0X1ADDR 0x10
#define VL53L0X2ADDR 0x20

// Offset for VL53L0X
#define OFFSET1 55000
#define OFFSET2 25000  //21000

#ifdef __cplusplus
	extern "C" {
#endif

int daemonize(void);
int mask_signals(sigset_t* p_sigset);
int wait_signals(sigset_t* p_sigset);

int str2uuid( const char *uuid_str, uuid_t *uuid );
int open_hci_dev(int *dev_id);
int scan_bt_dev(int dev_id, bdaddr_t *bdaddr);
int search_rc_channel(sdp_session_t *session, uuid_t *uuid);
void SHAGenerator(int pinNum, int pinDir, int* state, int tpusec, int tdusec, int tickNum);
void HapticGenerator(int pinNum, int pinDir, int* state, int tickNum, int period, int periodNum);
FootPedalProtocol ConvertFootPedalProtocol(uint8_t buff[sizeof(FootPedalProtocol)]);
FootMouseHaptic ConvertFootMouseHaptic(uint8_t buff[sizeof(FootMouseHaptic)]);
FootMouseHapticImpulse ConvertFootMouseHapticImpulse(uint8_t buff[sizeof(FootMouseHapticImpulse)]);
int ConnectNonBlocking(int sockfd, struct sockaddr *saddr, int addrsize);
void ctrl_c_handler(int signal);
void PrintVL53l0XError(VL53L0X_Error Status);
VL53L0X_Error WaitMeasurementDataReady(VL53L0X_DEV Dev);
VL53L0X_Error WaitStopCompleted(VL53L0X_DEV Dev);
VL53L0X_Error MeasureInitialization(VL53L0X_Dev_t *pMyDevice);
VL53L0X_RangingMeasurementData_t MeasureDistance(VL53L0X_Dev_t *pMyDevice);

#ifdef __cplusplus
	}
#endif

#endif//#ifndef FOOTPEDAL_H_