/* =====================================
=== FIMREG - Fast Image Registration ===
========================================

Written by Roelof Berg, Berg Solutions (rberg@berg-solutions.de) with support from
Lars Koenig, Fraunhofer MEVIS (lars.koenig@mevis.fraunhofer.de) Jan Ruehaak, Fraunhofer
MEVIS (jan.ruehaak@mevis.fraunhofer.de).

THIS IS A LIMITED RESEARCH PROTOTYPE. Documentation: www.berg-solutions.de/fimreg.html

------------------------------------------------------------------------------

Copyright (c) 2014, Roelof Berg
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

* Neither the name of the owner nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

----------------------------------------------------------------------------*/

//Entry point called from c++ code generated by matlab coder
//(As if this method would be directly called from an .m file)

//precompiled header (MUST BE THE FIRST ENTRY IN EVERY CPP FILE)
#include "stdafx.h"

#include "matlab_sendToTarget.h"
#include "HPRPCCommon.h"
#include "CHPRPCRequestStoreImg.h"
#include "CHPRPCResponseStoreImg.h"
#include "CHPRPCConnection.h"
#include "CBufferedWriter.h"

#include "TimingUtils.h"

extern uint32_t guCoreAmount;

//Some global stuff (global because matlab generates global C code)
extern vector<CHPRPCConnection*> g_CHPRPCConnection;
vector<CHPRPCRequestStoreImg*> g_Request_SendToTarget;

/**
* Prepare data to be sent to the target in memory. The pointers TSinglePart and RSinglePart will point to allocated memory in the data buffer afterwards
* and the caller can write the image data to this memory locations.
*
* TSinglePart and RSinglePart will be set memory allocated here for the images when allocateMemory is set to true. Then one big buffer with header T image and R image will
* be allocated. Otherwise (when allocateMemory is false) only the header will be allocated and the caller is responsible itself for adding the image data by using
* CHPRPCCommand::AddAndOwnAditionalBuffer(). In this case TSinglePart and RSinglePart will be set to zero.
*/
CHPRPCRequestStoreImg* matlab_c_prepareSendToTarget(const uint32_T DSPIdx, const uint32_T BoundBox[4], const uint32_T MarginAddon[3], const real32_T DSPResponsibilityBox[4],
						   /*uint8_T*& TSinglePart,*/ const uint32_T TSinglePartLen, /*uint8_T*& RSinglePart,*/ const uint32_T RSinglePartLen,
						   const uint32_T d, const uint32_T levels)
{
	CHPRPCConnection& oConnection = *(g_CHPRPCConnection.at(DSPIdx-1));
	CHPRPCRequestStoreImg* pRequest = new CHPRPCRequestStoreImg(oConnection);	//Deletion: See comment below.
	
	//Ensure we have enough vector space
	while(g_Request_SendToTarget.size()<DSPIdx)
	{
		g_Request_SendToTarget.push_back(NULL);
	}

	//Remember request (connection between independently from matlab generated entry points).
	//Request will be deleted in matlab_c_waitUntilTargetReady() (this also ensured the data to
	//be valid during the async/nonblocking send operation).
	if(NULL != g_Request_SendToTarget.at(DSPIdx-1))
	{
		throw HPRPCProtocolFlowException();
	}
	g_Request_SendToTarget.at(DSPIdx-1) = pRequest;

	#ifdef _TRACE_OUTPUT_
	printf("MarginAddon %u, %u, %u\n", MarginAddon[0], MarginAddon[1], MarginAddon[2]);
	#endif

	//Assign HPRPC header content
	pRequest->AssignValuesToBeSent(BoundBox, MarginAddon, DSPResponsibilityBox, TSinglePartLen, RSinglePartLen, d, (uint16_T)levels, (uint16_T)guCoreAmount);
	return pRequest;
}

/**
* Send the data prepated in matlab_c_prepareSendToTarget() to the target DSP.
*/
void matlab_c_sendToTarget(const uint32_T DSPIdx, CHPRPCRequestStoreImg* Request)
{
	const string csCmdLogName = "Send_To_Target";	//For logging purposes only

	Request->StartSendToOtherPeer(
		#if defined(_TRACE_HPRPC_)
		csCmdLogName
		#endif
		);
}
