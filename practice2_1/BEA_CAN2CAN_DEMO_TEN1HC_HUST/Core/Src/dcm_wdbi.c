
/*********************************************************/
/*********BOSCH BEA PROGRAM SKELETON DEMO CODE************/
/*********************************************************/

#include "dcm_wdbi.h"

// External variables
extern uint16_t DCM_StoredCANID;

/**
 * @brief Process WriteDataByIdentifier (0x2E) service request
 * @param request_data: Pointer to request data
 * @param length: Length of request data
 */
void DCM_WDBI_ProcessRequest(uint8_t* request_data, uint16_t length)
{
    // Check minimum length (SID + DID_High + DID_Low + 2 data bytes = 5 bytes minimum)
    if (length < 5) {
        DCM_SendNegativeResponse(DCM_SID_WRITE_DATA_BY_ID, DCM_NRC_INVALID_LENGTH);
        return;
    }
    
    // Check if security access is required and granted
    if (!DCM_IsSecurityLevel1Unlocked()) {
        DCM_SendNegativeResponse(DCM_SID_WRITE_DATA_BY_ID, DCM_NRC_SECURITY_ACCESS_DENIED);
        return;
    }
    
    // Extract Data Identifier from request
    uint16_t data_identifier = (request_data[1] << 8) | request_data[2];
    
    // Process based on Data Identifier
    switch (data_identifier) {
        case DCM_DID_CANID_VALUE:
            if (length == 5) { // SID + DID + 2 CAN ID bytes
                DCM_WDBI_WriteCANIDValue(data_identifier, &request_data[3]);
            } else {
                DCM_SendNegativeResponse(DCM_SID_WRITE_DATA_BY_ID, DCM_NRC_INVALID_LENGTH);
            }
            break;
            
        default:
            // DID not supported
            DCM_SendNegativeResponse(DCM_SID_WRITE_DATA_BY_ID, DCM_NRC_REQUEST_OUT_OF_RANGE);
            break;
    }
}

/**
 * @brief Write CAN ID Value from tester (DID: 0x0123)
 * @param data_identifier: Data Identifier
 * @param write_data: Pointer to write data (2 bytes for CAN ID)
 */
void DCM_WDBI_WriteCANIDValue(uint16_t data_identifier, uint8_t* write_data)
{
    uint8_t response_data[1];
    
    // Extract new CAN ID from write data
    uint16_t new_can_id = (write_data[0] << 8) | write_data[1];
    
    // Store new CAN ID (will be applied after ignition cycle)
    DCM_StoredCANID = new_can_id;
    
    // Prepare positive response (only SID, no additional data)
    response_data[0] = DCM_SID_WRITE_DATA_BY_ID_RESP; // 0x6E
    
    // Send positive response
    DCM_SendResponse(response_data, 1);
    
    // Note: The new CAN ID will be applied after ignition cycle (User Button press)
    // This is handled in the main loop when BtnU is pressed
}

