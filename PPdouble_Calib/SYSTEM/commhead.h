#ifndef PICA_CAN_PROTO_H
#define PICA_CAN_PROTO_H

#pragma pack(push)
#pragma pack(1)

#define	CAN_SET_LED			0x01  // ??????
#define	CAN_SET_LOCK		0x02  // ??????
#define	CAN_SET_LEDLOCK		0x03  // ??????
#define CAN_GET_RFID		0x04  // ??RFID??
#define CAN_SET_RFID 		0X05  // ??RFID??
#define CAN_RETURN_RFID		0x06  // ?????
#define CAN_BORROW_RFID		0x07  // ?????
#define CAN_ERROR_RFID		0x08  // ??RFID????
#define	CAN_CALIB   		0x09  // ???????
#define	CAN_MATCH   		0x0a  // ????????
#define CAN_GET_HOLE_INFO	0x0b  // ??????

#define HOLE_A	0x00  
#define HOLE_B	0x01

#define HOLE_RETURN_A 1
#define HOLE_RETURN_B 2
#define HOLE_BORROW_A 1
#define HOLE_BORROW_B 2


enum lock_state
{
	LOCK_STATE_OPENED = 0,
	LOCK_STATE_LOCKED = 1
};

struct led_color {
	unsigned char	R;
	unsigned char	G;
	unsigned char	B;
	unsigned char	BREATHE; //??
};

#define	RFID_LENGTH			16
struct rfid_data {
	unsigned char rfid[RFID_LENGTH];
};

struct pica_can_cmd {
	 unsigned char cmd_type;
	 unsigned char src;
	 unsigned char dest;
	 unsigned char hole;
	 unsigned char frame_idx;
	 unsigned char data_length;
	 unsigned char data[8];
};

#pragma pack(pop)

#endif // PICA_CAN_PROTO_H
