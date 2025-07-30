/*********************************************************/
/*********BOSCH BEA PROGRAM SKELETON DEMO CODE************/
/*********************************************************/

#include "dcm_seca.h"

// External variables
extern uint8_t DCM_SecurityLevel1_Unlocked;
extern uint32_t DCM_SecurityUnlockTimestamp;
extern uint8_t DCM_CurrentSeed[6];
extern uint8_t TEST_SEED_VALUES[6]; // Predefined seed values from main.c

/**
 * @brief Process SecurityAccess (0x27) service request
 * @param request_data: Pointer to request data
 * @param length: Length of request data
 */
void DCM_SECA_ProcessRequest(uint8_t* request_data, uint16_t length)
{
    // Check minimum length (SID + Level = 2 bytes for seed request)
    if (length < 2) {
        DCM_SendNegativeResponse(DCM_SID_SECURITY_ACCESS, DCM_NRC_INVALID_LENGTH);
        return;
    }
    
    uint8_t security_level = request_data[1];
    
    // Handle based on security level
    if (security_level == DCM_SECA_LEVEL1_SEED) {
        // Request seed for level 1
        if (length != 2) {
            DCM_SendNegativeResponse(DCM_SID_SECURITY_ACCESS, DCM_NRC_INVALID_LENGTH);
            return;
        }
        DCM_SECA_RequestSeed(security_level);
    }
    else if (security_level == DCM_SECA_LEVEL1_KEY) {
        // Send key for level 1
        if (length != 8) { // SID + Level + 6 key bytes
            DCM_SendNegativeResponse(DCM_SID_SECURITY_ACCESS, DCM_NRC_INVALID_LENGTH);
            return;
        }
        DCM_SECA_SendKey(request_data, length);
    }
    else {
        // Invalid security level
        DCM_SendNegativeResponse(DCM_SID_SECURITY_ACCESS, DCM_NRC_REQUEST_OUT_OF_RANGE);
    }
}

/**
 * @brief Handle request seed (sub-function 0x01)
 * @param security_level: Security level (0x01)
 */
void DCM_SECA_RequestSeed(uint8_t security_level)
{
    uint8_t response_data[8];
    
    // Generate new seed
    DCM_SECA_GenerateSeed();
    
    // Prepare positive response - Single Frame (8 bytes total)
    response_data[0] = DCM_SID_SECURITY_ACCESS_RESP; // 0x67
    response_data[1] = security_level;               // 0x01
    response_data[2] = DCM_CurrentSeed[0];           // SEED 0
    response_data[3] = DCM_CurrentSeed[1];           // SEED 1
    response_data[4] = DCM_CurrentSeed[2];           // SEED 2
    response_data[5] = DCM_CurrentSeed[3];           // SEED 3
    response_data[6] = DCM_CurrentSeed[4];           // SEED 4
    response_data[7] = DCM_CurrentSeed[5];           // SEED 5
    
    // Send positive response (exactly 8 bytes, no multiframe needed)
    DCM_SendResponse(response_data, 8);
}

/**
 * @brief Handle send key (sub-function 0x02)
 * @param request_data: Pointer to request data
 * @param length: Length of request data
 */
void DCM_SECA_SendKey(uint8_t* request_data, uint16_t length)
{
    uint8_t received_key[6];
    uint8_t response_data[2];
    
    // Extract key from request
    received_key[0] = request_data[2]; // KEY 0
    received_key[1] = request_data[3]; // KEY 1
    received_key[2] = request_data[4]; // KEY 2
    received_key[3] = request_data[5]; // KEY 3
    received_key[4] = request_data[6]; // KEY 4
    received_key[5] = request_data[7]; // KEY 5
    
    // Validate key
    if (DCM_SECA_ValidateKey(received_key)) {
        // Key is valid - unlock security
        DCM_SecurityLevel1_Unlocked = 1;
        DCM_SecurityUnlockTimestamp = HAL_GetTick();
        
        // Turn on LED (PB0) to indicate security unlock
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
        
        // Send positive response
        response_data[0] = DCM_SID_SECURITY_ACCESS_RESP; // 0x67
        response_data[1] = request_data[1];              // Echo level (0x02)
        
        DCM_SendResponse(response_data, 2);
    }
    else {
        // Invalid key - send NRC and implement 10s delay
        DCM_SendNegativeResponse(DCM_SID_SECURITY_ACCESS, DCM_NRC_INVALID_KEY);
        
        // TODO: Implement 10s delay before processing next seed request
        // For now, we'll just send the NRC
    }
}

/**
 * @brief Generate seed for security access
 * Uses predefined test values for consistent testing
 */
void DCM_SECA_GenerateSeed(void)
{
    // Use predefined seed values for testing
    // SEED: [0xA3, 0x5C, 0x91, 0x2F, 0x84, 0x67]
    for (int i = 0; i < 6; i++) {
        DCM_CurrentSeed[i] = TEST_SEED_VALUES[i];
    }
    
    // Alternative: Use random values for real implementation
    /*
    uint32_t seed_value1, seed_value2;
    
    // Use system tick as pseudo-random source
    seed_value1 = HAL_GetTick();
    seed_value2 = HAL_GetTick() + 0x12345678;
    
    // Ensure seed is not all 0x00 or all 0xFF
    if (seed_value1 == 0x00000000) seed_value1 = 0x12345678;
    if (seed_value1 == 0xFFFFFFFF) seed_value1 = 0x87654321;
    if (seed_value2 == 0x00000000) seed_value2 = 0xABCDEF01;
    if (seed_value2 == 0xFFFFFFFF) seed_value2 = 0x10FEDCBA;
    
    // Split into 6 bytes
    DCM_CurrentSeed[0] = (seed_value1 >> 24) & 0xFF;
    DCM_CurrentSeed[1] = (seed_value1 >> 16) & 0xFF;
    DCM_CurrentSeed[2] = (seed_value1 >> 8) & 0xFF;
    DCM_CurrentSeed[3] = seed_value1 & 0xFF;
    DCM_CurrentSeed[4] = (seed_value2 >> 8) & 0xFF;
    DCM_CurrentSeed[5] = seed_value2 & 0xFF;
    */
}

/**
 * @brief Validate received key against expected key
 * @param received_key: Pointer to received key (6 bytes)
 * @return 1 if valid, 0 if invalid
 */
uint8_t DCM_SECA_ValidateKey(uint8_t* received_key)
{
    uint8_t expected_key[6];
    
    // Calculate expected key using algorithm
    DCM_SECA_CalculateKey(DCM_CurrentSeed, expected_key);
    
    // Compare received key with expected key
    for (int i = 0; i < 6; i++) {
        if (received_key[i] != expected_key[i]) {
            return 0; // Invalid key
        }
    }
    
    return 1; // Valid key
}

/**
 * @brief Calculate key from seed using specified algorithm
 * @param seed: Pointer to seed (6 bytes)
 * @param key: Pointer to output key (6 bytes)
 * 
 * Algorithm:
 * KEY-0 = SEED-0 XOR SEED-1
 * KEY-1 = SEED-1 + SEED-2
 * KEY-2 = SEED-2 XOR SEED-3
 * KEY-3 = SEED-3 + SEED-0
 * KEY-4 = SEED-4 AND 0xF0
 * KEY-5 = SEED-5 AND 0x0F
 */
void DCM_SECA_CalculateKey(uint8_t* seed, uint8_t* key)
{
    key[0] = seed[0] ^ seed[1];         // KEY-0 = SEED-0 XOR SEED-1
    key[1] = seed[1] + seed[2];         // KEY-1 = SEED-1 + SEED-2
    key[2] = seed[2] ^ seed[3];         // KEY-2 = SEED-2 XOR SEED-3
    key[3] = seed[3] + seed[0];         // KEY-3 = SEED-3 + SEED-0
    key[4] = seed[4] & 0xF0;            // KEY-4 = SEED-4 AND 0xF0
    key[5] = seed[5] & 0x0F;            // KEY-5 = SEED-5 AND 0x0F
}
