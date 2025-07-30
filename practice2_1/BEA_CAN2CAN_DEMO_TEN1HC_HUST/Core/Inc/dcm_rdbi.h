/*********************************************************/
/*********BOSCH BEA PROGRAM SKELETON DEMO CODE************/
/*********************************************************/

#ifndef _DCM_RDBI_H
#define _DCM_RDBI_H

#include "dcm.h"

// Function prototypes for ReadDataByIdentifier service
void DCM_RDBI_ProcessRequest(uint8_t* request_data, uint16_t length);
void DCM_RDBI_ReadCANIDValue(uint16_t data_identifier);

#endif
