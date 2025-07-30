/*********************************************************/
/*********BOSCH BEA PROGRAM SKELETON DEMO CODE************/
/*********************************************************/

#ifndef _DCM_SECA_H
#define _DCM_SECA_H

#include "dcm.h"

// Function prototypes for SecurityAccess service
void DCM_SECA_ProcessRequest(uint8_t* request_data, uint16_t length);
void DCM_SECA_RequestSeed(uint8_t security_level);
void DCM_SECA_SendKey(uint8_t* request_data, uint16_t length);
void DCM_SECA_GenerateSeed(void);
uint8_t DCM_SECA_ValidateKey(uint8_t* received_key);
void DCM_SECA_CalculateKey(uint8_t* seed, uint8_t* key);

#endif
