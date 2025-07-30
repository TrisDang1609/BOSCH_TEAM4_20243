/*********************************************************/
/*********BOSCH BEA PROGRAM SKELETON DEMO CODE************/
/*********************************************************/

#include "dcm.h"
/*for further services please add service header here*/
#include "dcm_rdbi.h"
#include "dcm_wdbi.h"
#include "dcm_seca.h"

// Global variables for security management
uint8_t DCM_SecurityLevel1_Unlocked = 0;
uint32_t DCM_SecurityUnlockTimestamp = 0;
uint8_t DCM_CurrentSeed[6] = {0, 0, 0, 0, 0, 0};
uint16_t DCM_StoredCANID = 0x712; // Default CAN ID

// CAN-TP Multiframe variables
CANTP_State_t CANTP_RxState = CANTP_IDLE;
uint8_t CANTP_RxBuffer[64];
uint16_t CANTP_RxLength = 0;
uint16_t CANTP_RxIndex = 0;
uint8_t CANTP_RxSequenceNumber = 0;

// External variables
extern CAN_HandleTypeDef hcan2;
extern CAN_TxHeaderTypeDef CAN2_pHeader;
extern uint32_t CAN2_pTxMailbox;
extern uint8_t CAN2_DATA_TX[8];
extern unsigned int TimeStamp;

/**
 * @brief Process incoming diagnostic request
 * @param request_data: Pointer to request data
 * @param length: Length of request data
 */
void DCM_ProcessRequest(uint8_t* request_data, uint16_t length)
{
    if (length == 0) {
        return; // Invalid request
    }
    
    // Manage security timeout (5 seconds)
    DCM_ManageSecurityTimeout();
    
    // Check if this is a CAN-TP frame (First Frame, Consecutive Frame, Flow Control)
    uint8_t pci_type = (request_data[0] >> 4) & 0x0F;
    
    if (pci_type == 0x0) {
        // Single Frame - normal processing
        uint8_t sf_length = request_data[0] & 0x0F;
        if (sf_length > 0 && sf_length <= 7) {
            DCM_ProcessDiagnosticRequest(&request_data[1], sf_length);
        }
    } else if (pci_type == 0x1) {
        // First Frame - start multiframe reception
        CANTP_HandleFirstFrame(request_data);
    } else if (pci_type == 0x2) {
        // Consecutive Frame - continue multiframe reception
        CANTP_HandleConsecutiveFrame(request_data);
    } else if (pci_type == 0x3) {
        // Flow Control - handle flow control
        CANTP_HandleFlowControl(request_data);
    }
}

/**
 * @brief Process diagnostic request after CAN-TP processing
 * @param request_data: Pointer to request data
 * @param length: Length of request data
 */
void DCM_ProcessDiagnosticRequest(uint8_t* request_data, uint16_t length)
{
    // Route request based on Service ID
    switch (request_data[0]) {
        case DCM_SID_READ_DATA_BY_ID:
            DCM_RDBI_ProcessRequest(request_data, length);
            break;
            
        case DCM_SID_SECURITY_ACCESS:
            DCM_SECA_ProcessRequest(request_data, length);
            break;
            
        case DCM_SID_WRITE_DATA_BY_ID:
            DCM_WDBI_ProcessRequest(request_data, length);
            break;
            
        default:
            // Service not supported - keep silent as per requirement
            break;
    }
}

/**
 * @brief Send positive response via CAN
 * @param response_data: Pointer to response data
 * @param length: Length of response data
 */
void DCM_SendResponse(uint8_t* response_data, uint16_t length)
{
    // Clear CAN2 TX buffer
    for (int i = 0; i < 8; i++) {
        CAN2_DATA_TX[i] = 0x55; // Padding with 0x55
    }
    
    // Copy response data (max 8 bytes for single frame)
    for (int i = 0; i < length && i < 8; i++) {
        CAN2_DATA_TX[i] = response_data[i];
    }
    
    // Configure CAN header for response
    CAN2_pHeader.StdId = DCM_RESPONSE_CAN_ID;
    CAN2_pHeader.ExtId = 0x00;
    CAN2_pHeader.RTR = CAN_RTR_DATA;
    CAN2_pHeader.IDE = CAN_ID_STD;
    CAN2_pHeader.DLC = 8; // Always 8 bytes as per requirement
    CAN2_pHeader.TransmitGlobalTime = DISABLE;
    
    // Send response
    HAL_CAN_AddTxMessage(&hcan2, &CAN2_pHeader, CAN2_DATA_TX, &CAN2_pTxMailbox);
}

/**
 * @brief Send negative response via CAN
 * @param service_id: Original service ID
 * @param nrc: Negative Response Code
 */
void DCM_SendNegativeResponse(uint8_t service_id, uint8_t nrc)
{
    uint8_t response_data[3];
    
    response_data[0] = DCM_NRC_SID;    // 0x7F
    response_data[1] = service_id;     // Original SID
    response_data[2] = nrc;            // NRC
    
    DCM_SendResponse(response_data, 3);
}

/**
 * @brief Manage security timeout (5 seconds)
 */
void DCM_ManageSecurityTimeout(void)
{
    if (DCM_SecurityLevel1_Unlocked) {
        // Check if 5 seconds have passed since unlock
        if ((HAL_GetTick() - DCM_SecurityUnlockTimestamp) > 5000) {
            DCM_SecurityLevel1_Unlocked = 0;
            // Turn off LED (PB0)
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
        }
    }
}

/**
 * @brief Check if Security Level 1 is unlocked
 * @return 1 if unlocked, 0 if locked
 */
uint8_t DCM_IsSecurityLevel1Unlocked(void)
{
    return DCM_SecurityLevel1_Unlocked;
}

//==============================================================================
// CAN-TP (Transport Protocol) Implementation
//==============================================================================

/**
 * @brief Handle First Frame reception
 * @param frame_data: CAN frame data
 */
void CANTP_HandleFirstFrame(uint8_t* frame_data)
{
    // First Frame format: [1F:FF_DL] [Data_bytes_1_to_6]
    uint16_t total_length = ((frame_data[0] & 0x0F) << 8) | frame_data[1];
    
    if (total_length > sizeof(CANTP_RxBuffer)) {
        // Buffer overflow - send FC with overflow flag
        CANTP_SendFlowControl(0x32); // FC_OVFLW
        return;
    }
    
    // Store message info
    CANTP_RxLength = total_length;
    CANTP_RxIndex = 0;
    CANTP_RxSequenceNumber = 1;
    CANTP_RxState = CANTP_RECEIVING_FF;
    
    // Copy first 6 data bytes
    for (int i = 0; i < 6 && CANTP_RxIndex < total_length; i++) {
        CANTP_RxBuffer[CANTP_RxIndex++] = frame_data[2 + i];
    }
    
    // Send Flow Control - Continue to Send
    CANTP_SendFlowControl(0x30); // FC_CTS (Continue to Send)
    
    CANTP_RxState = CANTP_RECEIVING_CF;
}

/**
 * @brief Handle Consecutive Frame reception
 * @param frame_data: CAN frame data
 */
void CANTP_HandleConsecutiveFrame(uint8_t* frame_data)
{
    if (CANTP_RxState != CANTP_RECEIVING_CF) {
        return; // Not expecting consecutive frame
    }
    
    uint8_t sequence_number = frame_data[0] & 0x0F;
    
    // Check sequence number
    if (sequence_number != CANTP_RxSequenceNumber) {
        // Wrong sequence - abort
        CANTP_RxState = CANTP_IDLE;
        return;
    }
    
    // Copy data bytes (up to 7 bytes)
    for (int i = 0; i < 7 && CANTP_RxIndex < CANTP_RxLength; i++) {
        CANTP_RxBuffer[CANTP_RxIndex++] = frame_data[1 + i];
    }
    
    // Update sequence number (1-15, then wrap to 0)
    CANTP_RxSequenceNumber++;
    if (CANTP_RxSequenceNumber > 15) {
        CANTP_RxSequenceNumber = 0;
    }
    
    // Check if message is complete
    if (CANTP_RxIndex >= CANTP_RxLength) {
        // Message complete - process it
        DCM_ProcessDiagnosticRequest(CANTP_RxBuffer, CANTP_RxLength);
        CANTP_RxState = CANTP_IDLE;
    }
}

/**
 * @brief Handle Flow Control frame
 * @param frame_data: CAN frame data
 */
void CANTP_HandleFlowControl(uint8_t* frame_data)
{
    uint8_t fc_flag = frame_data[0] & 0x0F;
    
    switch (fc_flag) {
        case 0x0: // Continue to Send (CTS)
            // Continue sending consecutive frames if we were sending
            break;
            
        case 0x1: // Wait (WT)
            // Wait for next FC frame
            break;
            
        case 0x2: // Overflow (OVFLW)
            // Abort transmission
            break;
            
        default:
            // Invalid FC flag
            break;
    }
}

/**
 * @brief Send Flow Control frame
 * @param fc_flag: Flow Control flag (0x30=CTS, 0x31=WT, 0x32=OVFLW)
 */
void CANTP_SendFlowControl(uint8_t fc_flag)
{
    uint8_t fc_frame[8];
    
    // Flow Control format: [3X:BS:STmin] [00 00 00 00 00]
    fc_frame[0] = fc_flag;  // 30=CTS, 31=WT, 32=OVFLW
    fc_frame[1] = 0x00;     // Block Size = 0 (no blocking)
    fc_frame[2] = 0x00;     // STmin = 0 (no separation time)
    fc_frame[3] = 0x55;     // Padding
    fc_frame[4] = 0x55;
    fc_frame[5] = 0x55;
    fc_frame[6] = 0x55;
    fc_frame[7] = 0x55;
    
    DCM_SendResponse(fc_frame, 8);
}

/**
 * @brief Send multiframe message
 * @param data: Data to send
 * @param length: Length of data
 */
void CANTP_SendMultiFrame(uint8_t* data, uint16_t length)
{
    if (length <= 7) {
        // Can fit in single frame
        uint8_t sf_frame[8];
        sf_frame[0] = 0x00 | length;  // Single Frame PCI + length
        
        for (int i = 0; i < length; i++) {
            sf_frame[1 + i] = data[i];
        }
        
        // Padding
        for (int i = 1 + length; i < 8; i++) {
            sf_frame[i] = 0x55;
        }
        
        DCM_SendResponse(sf_frame, 8);
    } else {
        // Need multiframe - send First Frame
        uint8_t ff_frame[8];
        ff_frame[0] = 0x10 | ((length >> 8) & 0x0F);  // First Frame PCI + length high
        ff_frame[1] = length & 0xFF;                   // Length low
        
        // Copy first 6 data bytes
        for (int i = 0; i < 6; i++) {
            ff_frame[2 + i] = data[i];
        }
        
        DCM_SendResponse(ff_frame, 8);
        
        // Wait for Flow Control before sending Consecutive Frames
        // This would be handled in FC reception
    }
}
