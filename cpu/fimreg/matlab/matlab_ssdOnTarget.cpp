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

#include "matlab_ssdOnTarget.h"
#include "HPRPCCommon.h"
/** Common header for both CHPRPCRequestCalcSSDJacHess and virtually 'CHPRPCRequestCalcSSD' because parameters are the same */
#include "CHPRPCRequestCalcSSDJacHess.h"
#include "CHPRPCResponseCalcSSD.h"
#include "CHPRPCConnection.h"

//Some global stuff (global because matlab generates global C code)
extern vector<CHPRPCConnection*> g_CHPRPCConnection;
extern uint32_t guCoreAmount;
extern vector<CHPRPCRequestCalcSSDJacHess*> g_Request_SSDOnTarget;

/**
* Receive the result of the remote DSP calculation of the function value (SSD) on the target DSP
* This will be executed during Armijo Line Search. Calculation of gradient (JD) and Hessian (JD2) will
* be skipped here, for line search this can be omitted.
*/
real32_T matlab_c_ssdOnTarget(uint32_T DSPIdx)
{
	real32_T fRetVal=0;
	const string csCmdLogName = "Calc_SSD";	//For logging purposes only

	//Fetch pointer to request object from global namespace (connection to independend matlab code is on global namespace).
	//Memory has to be deleted here lateron.
	CHPRPCRequestCalcSSDJacHess* pRequest = g_Request_SSDOnTarget.at(DSPIdx-1);
	if(NULL == pRequest)
	{
		throw HPRPCProtocolFlowException();
	}

	try
	{
		g_Request_SSDOnTarget.at(DSPIdx-1) = NULL;	//Mark that we took ownership of the memory

		//Receive response (it is ok to be blocking here, all DSPs calculate for the same time and answer processing is
		//only possible when all answers arrived. (And parsing the arrival data is quick enough for not measuring any
		//difference.)
		uint8_t* pBuffer=0;
		uint32_t iBufferLen=0;
		pRequest->ReceiveResponse(pBuffer, iBufferLen
			#if defined(_TRACE_HPRPC_)
			, csCmdLogName
			#endif
			);

		//Parse response. 
		CHPRPCConnection& oConnection = *(g_CHPRPCConnection.at(DSPIdx-1));
		CHPRPCResponseCalcSSD oResponse(oConnection);
		/*uint16_t iReturncode =*/ oResponse.AssignRcvBuffer(pBuffer, iBufferLen);
		
		//Throw exception when the result was an error or if an unexpected call index was received
		if(!oResponse.IsResponseValid(*pRequest, DSPIdx, csCmdLogName))
		{
			//IsResultValid() allready goves comprehensive console output in case of an error
			throw HPRPCErrorUnexpectedCallIndexException();
		}

		//Success, extract return values
		HPRPC_HDR_Response_CalcSSD* pPayloadHeader = oResponse.GetPayloadHeader();
		#ifdef _TRACE_OUTPUT_
		printf("RCV: SSD=%f\n", pPayloadHeader->SSD);
		#endif

		fRetVal = pPayloadHeader->SSD;
	}
	catch(...)
	{
		delete pRequest;
		throw;
	}
	delete pRequest;

	return fRetVal;
}
