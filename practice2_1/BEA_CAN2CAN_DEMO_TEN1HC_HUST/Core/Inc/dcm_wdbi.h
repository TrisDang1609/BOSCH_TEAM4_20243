/*********************************************************/
/*********BOSCH BEA PROGRAM SKELETON DEMO CODE************/
/*********************************************************/

#ifndef _DCM_WDBI_H
#define _DCM_WDBI_H

#include "dcm.h"

// Function prototypes for WriteDataByIdentifier service
void DCM_WDBI_ProcessRequest(uint8_t* request_data, uint16_t length);
void DCM_WDBI_WriteCANIDValue(uint16_t data_identifier, uint8_t* write_data);

#endif
