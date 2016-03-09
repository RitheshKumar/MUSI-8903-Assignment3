
#if !defined(__FastConv_HEADER_INCLUDED__)
#define __FastConv_HEADER_INCLUDED__

#pragma once

#include "ErrorDef.h"
#include "RingBuffer.h"
#include "Buffer.h"
#include <iostream>

/*! \brief interface for fast convolution
*/
class CFastConv
{
public:
    
    enum ConvDomain {
        kTimeDomain,
        kFreqDomain
    };

    //Create an object!
    static Error_t create( CFastConv * &pFastConv );

    //Destroy the object!
    static Error_t destroy( CFastConv * &pFastConv );
    /*! initializes the class with the impulse response and the block length
    \param pfImpulseResponse impulse response samples (mono only)
    \param iLengthOfIr length of impulse response
    \param iBlockLength processing block size
    \return Error_t
    */
    Error_t init ( float *pfImpulseResponse, int iLengthOfIr, int iBlockLength = 8192, ConvDomain domainChoice = kFreqDomain );
    
    /*! resets all internal class members
    \return Error_t
    */
    Error_t reset ();

    /*! computes cost and path w/o back-tracking
    \param pfInputBuffer (mono)
    \param pfOutputBuffer (mono)
    \param iLengthOfBuffers can be anything from 1 sample to 10000000 samples
    \return Error_t
    */
    Error_t process (float *pfInputBuffer, float *pfOutputBuffer, int iBufferLength );
    
    void getTail(float* pfTail, int iTailLength, int ipLength, int ipBlockSize);
    
    void flushBuffer(float* pfTail);
    
    void blockImpulseResponse();
protected:

    CFastConv();
    ~CFastConv();

 
private:

    int _iIRLen, _iBlockLen, _iNumBlocks, _iOutputLength, _iIRLenNoPad;
    float *_pfIR;
    bool _bIsInit;
    ConvDomain _eDomainChoice;
//    CRingBuffer<float> *inputStorage, *outputStorage;
    
    CBuffer* buffer;
    
    float* m_pfTailBuffer;

    //Private Functions
    Error_t processTimeDomain( float *pfInputBuffer, float *pfOutputBuffer, int iLengthOfBuffer );
    Error_t bufferInput( float *pfInputBuffer );
    
    Error_t blockedProcessTimeDomain(float* pfInputBuffer, float* pfImpulseResponse, float* pfOutputBuffer, int iLengthOfInput, int iLengthOfIr);
    
    int NextPowerOf2(int value);
    
};


#endif
