/*********************************************************/
/*********BOSCH BEA PROGRAM SKELETON DEMO CODE************/
/*********************************************************/

#ifndef _DCM_H
#define _DCM_H

#include "main.h"
#include "stm32f4xx_it.h"
#include <stdint.h>

// DCM Service IDs
#define DCM_SID_READ_DATA_BY_ID         0x22
#define DCM_SID_SECURITY_ACCESS         0x27
#define DCM_SID_WRITE_DATA_BY_ID        0x2E

// Positive Response SIDs
#define DCM_SID_READ_DATA_BY_ID_RESP    0x62
#define DCM_SID_SECURITY_ACCESS_RESP    0x67
#define DCM_SID_WRITE_DATA_BY_ID_RESP   0x6E

// Negative Response Code
#define DCM_NRC_SID                     0x7F
#define DCM_NRC_INVALID_LENGTH          0x13
#define DCM_NRC_REQUEST_OUT_OF_RANGE    0x31
#define DCM_NRC_SECURITY_ACCESS_DENIED  0x33
#define DCM_NRC_INVALID_KEY             0x35

// CAN IDs for Diagnostic Communication
#define DCM_REQUEST_CAN_ID              0x712
#define DCM_RESPONSE_CAN_ID             0x7A2

// Data Identifiers
#define DCM_DID_CANID_VALUE             0x0123

// Security Access Levels
#define DCM_SECA_LEVEL1_SEED            0x01
#define DCM_SECA_LEVEL1_KEY             0x02

// Security state management
extern uint8_t DCM_SecurityLevel1_Unlocked;
extern uint32_t DCM_SecurityUnlockTimestamp;
extern uint8_t DCM_CurrentSeed[6];
extern uint16_t DCM_StoredCANID; // For Practice 3

// CAN-TP Multiframe support
typedef enum {
    CANTP_IDLE,
    CANTP_RECEIVING_FF,
    CANTP_RECEIVING_CF,
    CANTP_SENDING_FF,
    CANTP_SENDING_CF
} CANTP_State_t;

extern CANTP_State_t CANTP_RxState;
extern uint8_t CANTP_RxBuffer[64];
extern uint16_t CANTP_RxLength;
extern uint16_t CANTP_RxIndex;
extern uint8_t CANTP_RxSequenceNumber;

// Function prototypes
void DCM_ProcessRequest(uint8_t* request_data, uint16_t length);
void DCM_ProcessDiagnosticRequest(uint8_t* request_data, uint16_t length);
void DCM_SendResponse(uint8_t* response_data, uint16_t length);
void DCM_SendNegativeResponse(uint8_t service_id, uint8_t nrc);
void DCM_ManageSecurityTimeout(void);
uint8_t DCM_IsSecurityLevel1Unlocked(void);

// CAN-TP Function prototypes
void CANTP_ProcessFrame(uint8_t* frame_data, uint8_t length);
void CANTP_SendMultiFrame(uint8_t* data, uint16_t length);
void CANTP_SendFlowControl(uint8_t fc_flag);
void CANTP_HandleFirstFrame(uint8_t* frame_data);
void CANTP_HandleConsecutiveFrame(uint8_t* frame_data);
void CANTP_HandleFlowControl(uint8_t* frame_data);

#endif
