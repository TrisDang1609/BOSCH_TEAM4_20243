
/*********************************************************/
/*********BOSCH BEA PROGRAM SKELETON DEMO CODE************/
/*********************************************************/

#include "dcm_rdbi.h"

// External variables
extern CAN_RxHeaderTypeDef CAN1_pHeaderRx;

/**
 * @brief Process ReadDataByIdentifier (0x22) service request
 * @param request_data: Pointer to request data
 * @param length: Length of request data
 */
void DCM_RDBI_ProcessRequest(uint8_t* request_data, uint16_t length)
{
    // Check minimum length (SID + DID_High + DID_Low = 3 bytes)
    if (length != 3) {
        DCM_SendNegativeResponse(DCM_SID_READ_DATA_BY_ID, DCM_NRC_INVALID_LENGTH);
        return;
    }
    
    // Extract Data Identifier from request
    uint16_t data_identifier = (request_data[1] << 8) | request_data[2];
    
    // Process based on Data Identifier
    switch (data_identifier) {
        case DCM_DID_CANID_VALUE:
            DCM_RDBI_ReadCANIDValue(data_identifier);
            break;
            
        default:
            // DID not supported
            DCM_SendNegativeResponse(DCM_SID_READ_DATA_BY_ID, DCM_NRC_REQUEST_OUT_OF_RANGE);
            break;
    }
}

/**
 * @brief Read CAN ID Value from tester (DID: 0x0123)
 * @param data_identifier: Data Identifier
 */
void DCM_RDBI_ReadCANIDValue(uint16_t data_identifier)
{
    uint8_t response_data[5];
    
    // Prepare positive response
    response_data[0] = DCM_SID_READ_DATA_BY_ID_RESP; // 0x62
    response_data[1] = (data_identifier >> 8) & 0xFF; // DID High Byte
    response_data[2] = data_identifier & 0xFF;         // DID Low Byte
    
    // Get CAN ID value from last received CAN message from tester (CAN1)
    // According to requirement: "Read CANID Value From Tester"
    response_data[3] = (CAN1_pHeaderRx.StdId >> 8) & 0xFF;  // CAN ID High Byte
    response_data[4] = CAN1_pHeaderRx.StdId & 0xFF;          // CAN ID Low Byte
    
    // Send positive response
    DCM_SendResponse(response_data, 5);
}
