
#include "Vector.h"
#include "Util.h"

#include "FastConv.h"

CFastConv::CFastConv( void ): _pfIR(0), m_pCFft(0)
{
    reset();
    CFft::create(m_pCFft);
}

CFastConv::~CFastConv( void )
{
    reset();
    CFft::destroy(m_pCFft);
}

Error_t CFastConv::create( CFastConv * &pFastConv ) {
    pFastConv = new CFastConv();
    if( pFastConv == NULL ) {
        return kMemError;
    }
    else {
        return kNoError;
    }
}

Error_t CFastConv::destroy( CFastConv * &pFastConv ) {
    if( pFastConv == NULL ) {
        return kNoError;
    }
    delete pFastConv; pFastConv = 0;
    return kNoError;
}

Error_t CFastConv::init( float *pfImpulseResponse, int iLengthOfIr, int iBlockLength /*= 8192*/, ConvDomain domainChoice )
{
    reset();
    
    _eDomainChoice = domainChoice;
    
    //Check the block length for IR is power of 2
    //if it is not power of 2, we find the next power of 2 value to be the block length
    if (iBlockLength == 0) {
        return kNumErrors;
    }
    _iBlockLen = NextPowerOf2(iBlockLength);
    
    //Zero pad IR to multiple integer No. of blocks
    int nZerosToPad = 0;
    if (iLengthOfIr%iBlockLength == 0) {
        nZerosToPad = 0;
        _iNumBlocks = iLengthOfIr / iBlockLength;
    } else {
        nZerosToPad = iBlockLength - iLengthOfIr%iBlockLength;
        _iNumBlocks = iLengthOfIr/iBlockLength + 1;
    }
    
    //Variables' name declaration:
    //iIRLen is the IR length with zero pad (class member)
    //iIRLenNoPad is the IR length without zero pad (class member)
    //iLengthOfIr is the original IR length user pass in
    
    _iIRLen      = iLengthOfIr + nZerosToPad;
    
    //Initialize the Impulse Response
    _pfIR        = new float[_iIRLen];
    _iIRLenNoPad = iLengthOfIr;
    for( int sample=0; sample<iLengthOfIr; sample++ ) {
        _pfIR[sample] = pfImpulseResponse[sample];
    }
    for( int sample = iLengthOfIr; sample<_iIRLen; sample++ ) {
        _pfIR[sample] = 0.0f;
    }
    
    //Declare the tail buffer for holding the reverb tail
    m_pfTailBuffer = new float[_iIRLenNoPad - 1];
    memset(m_pfTailBuffer, 0.0f, (_iIRLenNoPad-1)*sizeof(float));
    
    if (_eDomainChoice == kFreqDomain) {
        m_pCFft->init(_iBlockLen, 2, CFft::kWindowNone, CFft::kNoWindow); //To avoid circular convolution
    }
    
    _bIsInit = true;
    return kNoError;
}

int CFastConv::NextPowerOf2(int value) {
    int nextPowerOf2 = 1;
    while (nextPowerOf2 < value) {
        nextPowerOf2 *= 2;
    }
    return nextPowerOf2;
}

Error_t CFastConv::reset()
{
    _iIRLen        = 0;
    _iBlockLen     = 0;
    _iNumBlocks    = 0;
    _iIRLenNoPad   = 0;
    
    if(_pfIR != NULL){
        delete [] _pfIR;
    }
    _pfIR = 0;
    
    if (m_pfTailBuffer != NULL) {
        delete [] m_pfTailBuffer;
    }
    m_pfTailBuffer = 0;
    
    
    _bIsInit = false;
    
    return kNoError;
}

Error_t CFastConv::process (float *pfInputBuffer, float *pfOutputBuffer, int iBufferLength )
{
    
    if( !_bIsInit ) {
        return kNotInitializedError;
    }
    
    //1, zero pad the input buffer to multiple integer number of iBlockLength (IR block length)
    int numOfZeroToPad = 0;
    int numOfInputBlock = 0;
    if (iBufferLength%_iBlockLen == 0) {
        numOfZeroToPad = 0;
        numOfInputBlock = iBufferLength / _iBlockLen;
    } else {
        numOfZeroToPad = _iBlockLen - (iBufferLength%_iBlockLen);
        numOfInputBlock = iBufferLength / _iBlockLen + 1;
    }
    int iPadInputLength = iBufferLength + numOfZeroToPad;
    float* pfPadInputBuffer = new float[iPadInputLength];
    for (int sample = 0; sample < iBufferLength; sample++) {
        pfPadInputBuffer[sample] = pfInputBuffer[sample];
    }
    for (int remaining = iBufferLength; remaining < iPadInputLength; remaining++) {
        pfPadInputBuffer[remaining] = 0.0f;
    }
    
    //2, convolve each block of the input with the blocked impulse response
    // overlap and add each convolution result to the padded output storage buffer
    
    //this buffer has the length of the summation of both padded input and impulse length
    float* pfPadOutputStorageBuffer = new float[iPadInputLength + _iIRLen - 1];
    memset(pfPadOutputStorageBuffer, 0.0f, (iPadInputLength+_iIRLen-1)*sizeof(float));

    //for each inputblock
    for (int nthBlock = 0; nthBlock < numOfInputBlock; nthBlock++) {
        //creat a single processing block based on input buffer
        float* pfProcessingBlock = new float[_iBlockLen];
        

        memcpy(pfProcessingBlock, &pfPadInputBuffer[nthBlock*_iBlockLen], _iBlockLen*sizeof(float));
        
        //for each ir block
        for (int nthIrBlock = 0; nthIrBlock < _iNumBlocks; nthIrBlock++) {
            float* pfProcessingIrBlock = new float[_iBlockLen];
            float* pfStorageBuffer = new float[2*_iBlockLen - 1];
            
            memcpy(pfProcessingIrBlock, &_pfIR[nthIrBlock*_iBlockLen], _iBlockLen*sizeof(float));
            memset(pfStorageBuffer, 0.0f, (2*_iBlockLen-1)*sizeof(float));
            
            if (_eDomainChoice == kTimeDomain) {
                blockedProcessTimeDomain(pfProcessingBlock, pfProcessingIrBlock, pfStorageBuffer, _iBlockLen, _iBlockLen);
            } else if (_eDomainChoice == kFreqDomain){
                blockedProcessFreqDomain(pfProcessingBlock, pfProcessingIrBlock, pfStorageBuffer);
            }
            
            for (int i = 0; i < 2*_iBlockLen - 1; i++) {
                pfPadOutputStorageBuffer[(nthBlock+nthIrBlock)*_iBlockLen + i] += pfStorageBuffer[i];
            }
            
            //3, adding the current process output with the previous reverb tail
            for (int i = 0; i < _iIRLenNoPad - 1; i++) {
                pfPadOutputStorageBuffer[(nthBlock+nthIrBlock)*_iBlockLen + i] += m_pfTailBuffer[i];
            }
            
            delete [] pfProcessingIrBlock;
            delete [] pfStorageBuffer;
            pfProcessingIrBlock = 0;
            pfStorageBuffer = 0;
        }
        
        //give the (input length + ir length - 1) samples to the pfOutputBuffer

        
    
        delete [] pfProcessingBlock;
        pfProcessingBlock = 0;
    }
    
    memcpy(pfOutputBuffer, pfPadOutputStorageBuffer, iBufferLength*sizeof(float));
    
    //4, update the reverb tail
    memcpy(m_pfTailBuffer, &pfPadOutputStorageBuffer[iBufferLength], (_iIRLenNoPad-1)*sizeof(float));
    
    delete [] pfPadOutputStorageBuffer;
    pfPadOutputStorageBuffer = 0;
    
    delete [] pfPadInputBuffer;
    pfPadInputBuffer = 0;

    return kNoError;
}

Error_t CFastConv::processTimeDomain (float *pfInputBuffer, float *pfOutputBuffer, int iLengthOfBuffer ) {
    for ( int sample=0; sample<iLengthOfBuffer; sample++ ) {
        for( int ir=0; ir<_iIRLen; ir++ ) {
            pfOutputBuffer[ir+sample] += pfInputBuffer[sample]*_pfIR[ir];
        }
    }
    return kNoError;
}

Error_t CFastConv::blockedProcessTimeDomain (float* pfInputBuffer, float* pfImpulseResponse, float* pfOutputBuffer, int iLengthOfInput, int iLengthOfIr) {
    for (int sample = 0; sample < iLengthOfInput; sample++) {
        for (int ir = 0; ir < iLengthOfIr; ir++) {
            pfOutputBuffer[ir+sample] += pfInputBuffer[sample] * pfImpulseResponse[ir];

        }
    }
    return kNoError;
}

void CFastConv::flushBuffer(float* pfTail) {
    for (int i = 0; i < _iIRLenNoPad-1; i++) {
        pfTail[i] = m_pfTailBuffer[i];
    }
}


Error_t CFastConv::blockedProcessFreqDomain(float* pfInputBuffer, float* pfImpulseResponse, float* pfOutputBuffer) {
    float *mySpectrum = new float[2*_iBlockLen],
          *mulSpecOut = new float[2*_iBlockLen],
          *tempOut    = new float[2*_iBlockLen];

    
    std::memset(mySpectrum, 0.0f, 2*_iBlockLen*(sizeof(float)) );
    
    std::memset(mulSpecOut, 0.0f, 2*_iBlockLen*(sizeof(float)) );
    
    std::memset(tempOut, 0.0f, 2*_iBlockLen*(sizeof(float)) );
    
    
    
    
    for (int sample=0; sample<_iBlockLen; sample++ ){
        
        pfInputBuffer[sample]   *= 2*_iBlockLen;
        
    }
    
    
    
    m_pCFft->doFft(mySpectrum, pfInputBuffer );
    
    m_pCFft->doFft(mulSpecOut, pfImpulseResponse );
    
    
    
    m_pCFft->mulCompSpectrum( mulSpecOut, mySpectrum );
    

    
    m_pCFft->doInvFft(tempOut, mulSpecOut);
    
    
    
    memcpy(pfOutputBuffer, tempOut, (2*_iBlockLen-1)*sizeof(float));
    
    delete [] mySpectrum; mySpectrum = 0;
    delete [] mulSpecOut; mulSpecOut = 0;
    delete [] tempOut;    tempOut    = 0;
    
    
    return kNoError;
    
    
    
}





