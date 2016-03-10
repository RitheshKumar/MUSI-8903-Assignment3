
#if !defined(__FastConv_HEADER_INCLUDED__)
#define __FastConv_HEADER_INCLUDED__

#pragma once

#include "ErrorDef.h"
#include "RingBuffer.h"
#include "Buffer.h"
#include "Fft.h"
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

    /*! returns the reverb tail
     \param pfTail: the reverb tail receiver
     */
    void flushBuffer(float* pfTail);
    
protected:

    CFastConv();
    ~CFastConv();

private:
    //Variables
    int _iIRLen, _iBlockLen, _iNumBlocks, _iIRLenNoPad;
    bool _bIsInit;
    
    ConvDomain _eDomainChoice;
    
    float* _pfIR;
    float* m_pfTailBuffer;
    CFft * m_pCFft;

    //Functions
    /*! Compute the convolution in time domain
     \ Convolve the input signal with impulse response (without blocking)
     \ return Error_t
     \ To understand the convolution equation
     */
    Error_t processTimeDomain( float *pfInputBuffer, float *pfOutputBuffer, int iLengthOfBuffer );
    
    /*! Compute the convolution in time domain
     \ Block-wised Convolution: both of the input and impulse response are blocked
     \ return Error_t
     \ Call by the process function when the user chooses the time domain convolution
     */
    Error_t blockedProcessTimeDomain(float* pfInputBuffer, float* pfImpulseResponse, float* pfOutputBuffer, int iLengthOfInput, int iLengthOfIr);
    
    /*! Compute the convolution in frequency domain
     \ Block-wised Convolution: both of the input and impulse response are blocked
     \ Using the Fft.h API
     \ return Error_t
     \ Call by the process function when the user chooses the frequency domain convolution
     */
    Error_t blockedProcessFreqDomain(float* pfInputBuffer, float* pfImpulseResponse, float* pfOutputBuffer);
    
    /*! Compute next power of 2 value
     \ return an integer
     \ called by the init function
     */
    int NextPowerOf2(int value);
    
};


#endif
