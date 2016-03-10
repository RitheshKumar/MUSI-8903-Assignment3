
#include "Vector.h"
#include "Util.h"
#include "Fft.h"

#include "FastConv.h"

CFastConv::CFastConv( void ): _pfIR(0),
                             buffer(0)
{
    reset();
    
}

CFastConv::~CFastConv( void )
{
    reset();
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
    
    
    //Check the block length for IR is power of 2
    //if it is not power of 2, we find the next power of 2 value to be the iBlockLength
    if (iBlockLength % 2 != 0) {
        iBlockLength = NextPowerOf2(iBlockLength);
    }

    _eDomainChoice = domainChoice;

    //Zero pad IR to multiple integer No. of blocks
    int nZerosToPad = iBlockLength - iLengthOfIr%iBlockLength;
    _iNumBlocks = iLengthOfIr/iBlockLength + 1;
    _iIRLen = iLengthOfIr + nZerosToPad;
    _iIRLenNoPad = iLengthOfIr;
    _iBlockLen = iBlockLength;
    _iOutputLength = iLengthOfIr;
    _pfIR   = new float[_iIRLen];
    for( int sample=0; sample<iLengthOfIr; sample++ ) {
        _pfIR[sample] = pfImpulseResponse[sample];
    }
    for( int sample = iLengthOfIr; sample<_iIRLen; sample++ ) {
        _pfIR[sample] = 0.0f;
    }

    _bIsInit = true;
    
    buffer = new CBuffer(100, 600000, 600000);
    buffer->init(_iNumBlocks);

    
    return kNoError;
}

Error_t CFastConv::reset()
{
    _iIRLen = 0;
    _iIRLenNoPad = 0;
    _iBlockLen = 0;
    if(_pfIR != NULL){
        delete [] _pfIR;
    }
    _pfIR = 0;
    
    if(buffer!=NULL) {
        delete buffer;
    }
    buffer = 0;
    
    _bIsInit = false;

    return kNoError;
}

int CFastConv::NextPowerOf2(int value) {
    int nextPowerOf2 = 1;
    while (nextPowerOf2 < value) {
        nextPowerOf2 *= 2;
    }
    return nextPowerOf2;
}

Error_t CFastConv::process (float *pfInputBuffer, float *pfOutputBuffer, int iBufferLength )
{
    //zeros pad for the input buffer
    //    int iNumZerosPadInput = _iBlockLen - (iBufferLength % _iBlockLen);
    //    int iNumBlockInput    = iBufferLength / _iBlockLen + 1;
    //    int iBlockNumCounter  = 1;
    
    if( !_bIsInit ) {
        return kNotInitializedError;
    }
    
    buffer->storeInput(pfInputBuffer, iBufferLength);

    //1, zero pad the input buffer to multiple integer number of iBlockLength (IR block length)
    int numOfZeroToPad = 0;
    int iNumIpBlks = 0;
    if (iBufferLength%_iBlockLen == 0) {
        numOfZeroToPad = 0;
        iNumIpBlks = iBufferLength / _iBlockLen;
    } else {
        numOfZeroToPad = _iBlockLen - (iBufferLength%_iBlockLen);
        iNumIpBlks = iBufferLength / _iBlockLen + 1;
    }

    float *tempOut   = new float[_iBlockLen + _iIRLen - 1];
    CRingBuffer<float> tempOpBuff( iBufferLength + _iIRLen - 1);
    //for each inputblock
    for (int nthBlock = 0; nthBlock < iNumIpBlks; nthBlock++) {

        int curBlkIdx = nthBlock*_iBlockLen;
        if ( iBufferLength < curBlkIdx + _iBlockLen ) {
            float *lastBlk   = new float[_iBlockLen];
            for( int sample=curBlkIdx, i = 0; sample<iBufferLength; sample++, i++ ) {
                lastBlk[i] = pfInputBuffer[sample];
            }
            for( int sample= iBufferLength%_iBlockLen; sample< _iBlockLen; sample++ ) {
                lastBlk[sample] = 0.0f;
            }
            processBlockedTimeDomain( lastBlk, tempOut );
        }
        else {
            processBlockedTimeDomain( &pfInputBuffer[curBlkIdx], tempOut );
        }

        tempOpBuff.addPostInc( tempOut, _iBlockLen );
        tempOpBuff.add( &tempOut[curBlkIdx], _iIRLen -1 );

    }

    
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

Error_t CFastConv::blockTimeConv (float *pfInputBuffer, float *pfOutputBuffer, int iblockIdx ) {
    
    
    for ( int sample=0; sample<_iBlockLen; sample++ ) {
        
        for( int ir=0; ir<_iBlockLen; ir++ ) {
            pfOutputBuffer[ir+sample] += pfInputBuffer[sample]*_pfIR[ir + iblockIdx];
            
        }
        
    }
    
    return kNoError;
    
}

void CFastConv::getTail(float* pfTail,int ipLength, int ipBLockSize) {
    
    int iNumZeroToPad_IP = ipBLockSize - ipLength%ipBLockSize;
    buffer->getTail( pfTail, _iIRLenNoPad-1, iNumZeroToPad_IP );
}


void CFastConv::processBlockedTimeDomain( float *pfInput, float *pfOutput ) {
    
    CRingBuffer<float> tempBuff(_iBlockLen + _iIRLen - 1);
    float *tempOut = new float[2*_iBlockLen - 1];
    int curIRBlkIdx = 0;

    //for each IRblock
    for (int nthIRBlock = 0; nthIRBlock < _iNumBlocks; nthIRBlock++) {

        curIRBlkIdx = nthIRBlock*_iBlockLen;

        blockTimeConv( pfInput, tempOut, curIRBlkIdx );

        tempBuff.addPostInc( tempOut, _iBlockLen );
        tempBuff.add( &tempOut[_iBlockLen], _iBlockLen -1 );
        
    }

    for (int sample; sample< _iBlockLen + _iIRLen - 1; sample++ ) {
        pfOutput[sample] = tempBuff.getPostInc();
    }

}




