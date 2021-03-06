/*
 * Copyright (C) 2011 Department of Robotics Brain and Cognitive Sciences - Istituto Italiano di Tecnologia
 * Author:  Valentina Gaggero
 * email:   valentina.gaggero@iit.it
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

/* @file       eODeb_eoProtoParser.c
    @brief      
    @author     valentina.gaggero@iit.it
    @date       03/18/2013
**/

// --------------------------------------------------------------------------------------------------------------------
// - external dependencies
// --------------------------------------------------------------------------------------------------------------------
#include "EoCommon.h"

#include "stdlib.h"
#include "string.h"

#include "EOropframe_hid.h"
#include "EOrop_hid.h"



// --------------------------------------------------------------------------------------------------------------------
// - declaration of extern public interface
// --------------------------------------------------------------------------------------------------------------------
#include "eODeb_eoProtoParser.h"


// --------------------------------------------------------------------------------------------------------------------
// - declaration of extern hidden interface 
// --------------------------------------------------------------------------------------------------------------------
#include "eODeb_eoProtoParser_hid.h"



// --------------------------------------------------------------------------------------------------------------------
// - #define with internal scope
// --------------------------------------------------------------------------------------------------------------------
#define ROPFRAME_HEADER_SIZE        sizeof(EOropframeHeader_t)
#define ROP_HEADER_SIZE             sizeof(eOrophead_t)


// --------------------------------------------------------------------------------------------------------------------
// - definition (and initialisation) of extern variables, but better using _get(), _set() 
// --------------------------------------------------------------------------------------------------------------------
// empty-section



// --------------------------------------------------------------------------------------------------------------------
// - typedef with internal scope
// --------------------------------------------------------------------------------------------------------------------
// empty-section


// --------------------------------------------------------------------------------------------------------------------
// - declaration of static functions
// --------------------------------------------------------------------------------------------------------------------
static eOresult_t s_eodeb_eoProtoParser_CheckNV(eODeb_eoProtoParser *p, eOethLowLevParser_packetInfo_t *pktInfo_ptr);
static uint8_t s_eodeb_eoProtoParser_NVisrequired(eODeb_eoProtoParser *p, eOprotID32_t id32);
static uint8_t s_eodeb_eoProtoParser_CheckSeqnum(eODeb_eoProtoParser *p, eOethLowLevParser_packetInfo_t *pktInfo_ptr, 
                                                uint32_t *rec_seqnum, uint32_t *expeted_seqnum);
static uint8_t s_eodeb_eoProtoParser_isvalidropframe(uint8_t *payload, uint32_t size);
//static eOresult_t s_eodeb_eoProtoParser_DumpNV(eODeb_eoProtoParser *p, eOethLowLevParser_packetInfo_t *pktInfo_ptr);



// --------------------------------------------------------------------------------------------------------------------
// - definition (and initialisation) of static variables
// --------------------------------------------------------------------------------------------------------------------
static eODeb_eoProtoParser s_debParser_singleton = 
{
    EO_INIT(.initted)   0
};



// --------------------------------------------------------------------------------------------------------------------
// - definition of extern public functions
// --------------------------------------------------------------------------------------------------------------------
extern eODeb_eoProtoParser * eODeb_eoProtoParser_Initialise(const eODeb_eoProtoParser_cfg_t *cfg)
{
    //se gia configurato fatto qc????
    
    if(NULL == cfg)
    {
        //cfg = &eo_ethLowLevParser_DefaultCfg;
        return(NULL);
    }
    
    memcpy(&s_debParser_singleton.cfg, cfg, sizeof(eODeb_eoProtoParser_cfg_t));
    s_debParser_singleton.initted = 1;
    
    return(&s_debParser_singleton);
}

extern eODeb_eoProtoParser * eODeb_eoProtoParser_GetHandle(void)
{
    return( (s_debParser_singleton.initted == 1)? &s_debParser_singleton : NULL);
}



extern eOresult_t eODeb_eoProtoParser_RopFrameDissect(eODeb_eoProtoParser *p, eOethLowLevParser_packetInfo_t *pktInfo_ptr)
{
    uint32_t rec_seqnum, expeted_seqnum;
    if((NULL == p) || (NULL == pktInfo_ptr))
    {
        return(eores_NOK_nullpointer);
    }

    //1) verify if i received a valid ropframe
    if(!(s_eodeb_eoProtoParser_isvalidropframe(pktInfo_ptr->payload_ptr, pktInfo_ptr->size)))
    {
        if(p->cfg.checks.invalidRopFrame.cbk != NULL)
        {
            p->cfg.checks.invalidRopFrame.cbk(pktInfo_ptr);
        }
        return(eores_NOK_generic); 
    }
    
    //2) if a check sequence number callback has been configured, I check seqNum
    if(p->cfg.checks.seqNum.cbk_onErrSeqNum != NULL)
    {
        if(!s_eodeb_eoProtoParser_CheckSeqnum(p, pktInfo_ptr, &rec_seqnum, &expeted_seqnum))
        {
            p->cfg.checks.seqNum.cbk_onErrSeqNum(pktInfo_ptr, rec_seqnum, expeted_seqnum);
        }
    }
    
    //3) search nv configured in received ropframe
    if(p->cfg.checks.nv.cbk_onNVfound != NULL)
    {
        s_eodeb_eoProtoParser_CheckNV(p, pktInfo_ptr);
    }

//    s_eodeb_eoProtoParser_DumpNV(p, pktInfo_ptr);
    
    return(eores_OK);
}





// --------------------------------------------------------------------------------------------------------------------
// - definition of extern hidden functions 
// --------------------------------------------------------------------------------------------------------------------



// --------------------------------------------------------------------------------------------------------------------
// - definition of static functions 
// --------------------------------------------------------------------------------------------------------------------
static uint8_t s_eodeb_eoProtoParser_isvalidropframe(uint8_t *payload, uint32_t size)
{
    EOropframeHeader_t *ropframeHdr = (EOropframeHeader_t *)payload;
    uint32_t footer =*((uint32_t*)(&payload[size-4]));
    if((ropframeHdr->startofframe == 0x12345678) && (footer == 0x87654321))
    {
        return(1);
    }
    else
    {
        return(0);
    }
}


static eOresult_t s_eodeb_eoProtoParser_CheckNV(eODeb_eoProtoParser *p, eOethLowLevParser_packetInfo_t *pktInfo_ptr)
{
    EOropframeHeader_t *ropframeheader;
	eOrophead_t *ropheader;
	uint8_t *rop_ptr;
    uint16_t i;
    


	//get ropframe header
	ropframeheader = (EOropframeHeader_t *)pktInfo_ptr->payload_ptr;

    //get rop to first rop
	rop_ptr = &pktInfo_ptr->payload_ptr[ROPFRAME_HEADER_SIZE];
	ropheader = (eOrophead_t*)rop_ptr;

	for(i=0; i<ropframeheader->ropsnumberof; i++)
	{
		//uint8_t *enddata_ptr;
		eODeb_eoProtoParser_ropAdditionalInfo_t ropAddInfo = {0};
		int32_t filldata = 0, totdatasize = 0, signaturesize = 0, timesize = 0;
                uint32_t signature = EOK_uint32dummy;
                uint64_t time = EOK_uint64dummy;

		//calulate fill data size
		filldata = ropheader->dsiz%4;
		if(filldata!=0)
		{
			filldata=4-filldata;
		}

		//calculate total data size
		totdatasize = (ropheader->dsiz) + filldata;

// 		//save ptr to enddata
// 		enddata_ptr = &rop_ptr[(ropheader->dsiz) + filldata];
        
        

		if(ropheader->ctrl.plussign == 1)
		{
			uint8_t ropheader_size = sizeof(eOrophead_t);
                        //uint8_t *data_ptr = &rop_ptr[totdatasize];
                        uint8_t *data_ptr = &rop_ptr[ropheader_size + totdatasize];
			signature = *((uint32_t*)data_ptr);
			signaturesize = 4;
		}
        
		if(ropheader->ctrl.plustime == 1)
		{
                        uint8_t ropheader_size = sizeof(eOrophead_t);
			//uint8_t *data_ptr = &rop_ptr[totdatasize + signaturesize];
                        uint8_t *data_ptr = &rop_ptr[ropheader_size + totdatasize + signaturesize];
			memcpy((uint8_t*)&time, data_ptr, sizeof(uint64_t));
			timesize = 8;
		}

		if(s_eodeb_eoProtoParser_NVisrequired(p, ropheader->id32))
		{

			//prepare rop additional info
			ropAddInfo.desc.control.plussign = ropheader->ctrl.plussign;
			ropAddInfo.desc.control.plustime = ropheader->ctrl.plustime;
			ropAddInfo.desc.ropcode = ropheader->ropc;
			ropAddInfo.desc.id32 = ropheader->id32;
			ropAddInfo.desc.data = &rop_ptr[ROP_HEADER_SIZE];
			ropAddInfo.desc.size = ropheader->dsiz;
			ropAddInfo.desc.signature = signature;
			ropAddInfo.time = time;
                        ropAddInfo.seqnum = ropframeheader->sequencenumber;

			p->cfg.checks.nv.cbk_onNVfound(pktInfo_ptr, &ropAddInfo);
		}


		//go to next rop
		rop_ptr = &rop_ptr[ROP_HEADER_SIZE + totdatasize + signaturesize + timesize];
		ropheader = (eOrophead_t*)rop_ptr;

	} //end for

    return(eores_OK);
}

static uint8_t s_eodeb_eoProtoParser_NVisrequired(eODeb_eoProtoParser *p, eOprotID32_t id32)
{
    uint8_t i, max;
    eODeb_eoProtoParser_nv_identify_t *id;
    
    max = p->cfg.checks.nv.NVs2searchArray.head.size;
    
    for(i=0; i<max; i++)
    {
        id  = (eODeb_eoProtoParser_nv_identify_t *)eo_array_At((EOarray*)&p->cfg.checks.nv.NVs2searchArray, i);
        if(id->id32 == id32)
        {
            return(1);
        }
    }
    return(0);
}


static uint8_t s_eodeb_eoProtoParser_CheckSeqnum(eODeb_eoProtoParser *p, eOethLowLevParser_packetInfo_t *pktInfo_ptr, uint32_t *rec_seqnum, uint32_t *expeted_seqnum)
{
    static uint8_t isfirstframe = 1;
    static uint32_t curr_seqNum; //current sequence number
    
    EOropframeHeader_t *ropframeHdr = (EOropframeHeader_t *)pktInfo_ptr->payload_ptr;
    uint32_t *u32ptr =  (uint32_t*)&ropframeHdr->sequencenumber;
    *rec_seqnum = *u32ptr;


    //if it is first pkt received i save start seqNum (initialized curr_seqNum with received seqnum)
    if(isfirstframe)
    {
        curr_seqNum = *rec_seqnum;
        *expeted_seqnum = curr_seqNum;
        isfirstframe = 0;
        return(1);
    }
    
    //calculate expected seqnum
    *expeted_seqnum = curr_seqNum+1;
    
    
    if(*expeted_seqnum == *rec_seqnum)
    {
        curr_seqNum = *expeted_seqnum;
        return(1);
    }

    //if i'm here i lost a packt or packets arrived not in order.
    
    if(*rec_seqnum != curr_seqNum)
    {
        //i lost a pkt, so i restart with received seqnum
        curr_seqNum = *rec_seqnum;
    }
    
    return(0);
}


#if 0
static eOresult_t s_eodeb_eoProtoParser_DumpNV(eODeb_eoProtoParser *p, eOethLowLevParser_packetInfo_t *pktInfo_ptr)
{
    EOropframeHeader_t *ropframeheader;
	eOrophead_t *ropheader;
	uint8_t *rop_ptr;
    uint16_t i;


	//get ropframe header
	ropframeheader = (EOropframeHeader_t *)pktInfo_ptr->payload_ptr;

    //get rop to first rop
	rop_ptr = &pktInfo_ptr->payload_ptr[ROPFRAME_HEADER_SIZE];
	ropheader = (eOrophead_t*)rop_ptr;

	for(i=0; i<ropframeheader->ropsnumberof; i++)
	{

//		uint8_t *enddata_ptr;
		eODeb_eoProtoParser_ropAdditionalInfo_t ropAddInfo = {0};
		int32_t filldata = 0, totdatasize = 0, signaturesize = 0, timesize = 0;

		//prepare rop additional info
		ropAddInfo.desc.configuration.plussign = ropheader->ctrl.plussign;
		ropAddInfo.desc.configuration.plustime = ropheader->ctrl.plustime;
		ropAddInfo.desc.ropcode = ropheader->ropc;
		ropAddInfo.desc.ep = ropheader->endp;
		ropAddInfo.desc.id = ropheader->nvid;
		ropAddInfo.desc.data = &rop_ptr[ROP_HEADER_SIZE];
		ropAddInfo.desc.size = ropheader->dsiz;
		ropAddInfo.desc.signature = EOK_uint32dummy;
		ropAddInfo.time = EOK_uint64dummy;

		//calulate fill data size
		filldata = ropheader->dsiz%4;
		if(filldata!=0)
		{
			filldata=4-filldata;
		}

		//calculate total data size
		totdatasize = (ropheader->dsiz) + filldata;

// 		//save ptr to enddata
// 		enddata_ptr = &rop_ptr[(ropheader->dsiz) + filldata];

		if(ropheader->ctrl.plussign == 1)
		{
			uint8_t *data_ptr = &rop_ptr[totdatasize];
			ropAddInfo.desc.signature = *((uint32_t*)data_ptr);
			signaturesize = 4;

		}
		if(ropheader->ctrl.plustime == 1)
		{
			uint8_t *data_ptr = &rop_ptr[totdatasize + signaturesize];
			memcpy((uint8_t*)&ropAddInfo.time, data_ptr, sizeof(uint64_t));
			timesize = 8;
		}


		//go to next rop
		rop_ptr = &rop_ptr[ROP_HEADER_SIZE + totdatasize + signaturesize + timesize];
		ropheader = (eOrophead_t*)rop_ptr;
	}

    return(eores_OK);
}

#endif

// --------------------------------------------------------------------------------------------------------------------
// - end-of-file (leave a blank line after)
// --------------------------------------------------------------------------------------------------------------------




