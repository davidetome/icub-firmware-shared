/*
 * Copyright (C) 2013 iCub Facility - Istituto Italiano di Tecnologia
 * Author:  Marco Accame
 * email:   marco.accame@iit.it
 * website: www.robotcub.org
 * Permission is granted to copy, distribute, and/or modify this program
 * under the terms of the GNU General Public License, version 2 or any
 * later version published by the Free Software Foundation.
 *
 * A copy of the license can be found at
 * http://www.robotcub.org/icub/license/gpl.txt
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details
*/

// - include guard ----------------------------------------------------------------------------------------------------
#ifndef _EOCFG_EPS_EB1_H_
#define _EOCFG_EPS_EB1_H_

#ifdef __cplusplus
extern "C" {
#endif


/** @file       eOcfg_EPs_eb1.h          
	@brief      This header file contains a const configuration for the endpoints of board EB1 (left arm, upper)
	@author     marco.accame@iit.it
	@date       06/06/2013
**/

/** @defgroup eo_eOcfg_EPs_eb1 Configuration of the NVs for management of the ems board eb1
    Tcecece 
    
    @{		
 **/



// - external dependencies --------------------------------------------------------------------------------------------

#include "EoCommon.h"
#include "EOconstvector.h"
#include "EOnvSet.h"



// - public #define  --------------------------------------------------------------------------------------------------
// empty-section

// - declaration of public user-defined types ------------------------------------------------------------------------- 
// empty-section

    
// - declaration of extern public variables, ... but better using use _get/_set instead -------------------------------


extern const eOnvset_DEVcfg_t* eo_cfg_EPs_eb1;
extern const eOnvset_DEVcfg_t eo_cfg_EPs_eb1_object;


// - declaration of extern public functions ---------------------------------------------------------------------------


/** @fn         extern void* eo_cfg_nvsEP_eb1_Get_RAM(eOnvEP_t ep, eOnvsetOwnership_t ownership)
    @brief      retrieves all the the RAM of the endpoint specified by @e ep. This functions returns the locally or
                remotely owned RAM. 
                This ram can be used with a proper cast to the struct representing the endpoint (e.g., for the endpoint 
                eoprot_endpoint_mc_leftupperarm, the cast is to be done with eOprot_ep_mc_leftupperarm_t).
    @param      ep              The endpoint.
    @param      ownership       the ownership
    @return     The pointer to the memory or NULL if the memory is not initialised yet or if the @e ep is not recognised
 **/
extern void* eo_cfg_nvsEP_eb1_Get_RAM(eOnvEP_t ep, eOnvsetOwnership_t ownership);


/** @fn         extern void* eo_cfg_nvsEP_eb1_Get_locallyownedRAM(eOnvEP_t ep)
    @brief      retrieves all the the RAM of the endpoint specified by @e ep. This functions returns the locally owned RAM.
                This ram can be used with a proper cast to the struct representing the endpoint (e.g., for the endpoint 
                eoprot_endpoint_mc_leftupperarm, the cast is to be done with eOprot_ep_mc_leftupperarm_t).
    @param      ep              The endpoint.
    @return     The pointer to the memory or NULL if the memory is not initialised yet or if the @e ep is not recognised
 **/
extern void* eo_cfg_nvsEP_eb1_Get_locallyownedRAM(eOnvEP_t ep);




/** @}            
    end of group eo_eOcfg_EPs_eb1  
 **/

#ifdef __cplusplus
}       // closing brace for extern "C"
#endif 
 
#endif  // include-guard


// - end-of-file (leave a blank line after)----------------------------------------------------------------------------



