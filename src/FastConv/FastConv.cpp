
#include "Vector.h"
#include "Util.h"

#include "FastConv.h"

CFastConv::CFastConv( void ): _pfIR(0)
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
    
    delete pFastConv;
    pFastConv = 0;
    return kNoError;
}

Error_t CFastConv::init( float *pfImpulseResponse, int iLengthOfIr, int iBlockLength /*= 8192*/, ConvDomain domainChoice )
{
    
    reset();

    _eDomainChoice = domainChoice;

    //Zero pad ir to integer no. of blocks
    int nZerosToPad = iBlockLength - iLengthOfIr%iBlockLength;
    _iNumBlocks = iLengthOfIr/iBlockLength + 1;
    _iIRLen = iLengthOfIr + nZerosToPad;
    _iBlockLen = iBlockLength;

//    _pfIR   = new float[iLengthOfIr + nZerosToPad];
    _pfIR   = new float[_iIRLen];

    for( int sample=0; sample<iLengthOfIr; sample++ ) {
        _pfIR[sample] = pfImpulseResponse[sample];
    }
    for( int sample = iLengthOfIr; sample<_iIRLen; sample++ ) {
        _pfIR[sample] = 0.0f;
    }

    _bIsInit = true;

    inputStorage = new CRingBuffer<float>( _iIRLen );
    inputStorage->reset();

    outputStorage = new CRingBuffer<float>( _iIRLen );
    outputStorage->reset();
    
    return kNoError;
}

Error_t CFastConv::reset()
{
    _iIRLen = 0;
    _iBlockLen = 0;
//    if(_pfIR){
//        delete [] _pfIR;
//    }
    delete [] _pfIR;
    _pfIR = 0;
    _bIsInit = false;

    return kNoError;
}

Error_t CFastConv::process (float *pfInputBuffer, float *pfOutputBuffer, int iBufferLength )
{
    //zeros pad for the input buffer
    int iNumZerosPadInput = _iBlockLen - (iBufferLength % _iBlockLen);
    int iNumBlockInput    = iBufferLength / _iBlockLen + 1;
    int iBlockNumCounter  = 1;
    
    if( !_bIsInit ) {
        return kNotInitializedError;
    }
    for (int i = 0; i < iBufferLength; i = i+_iBlockLen) {
        if (iBlockNumCounter == iNumBlockInput) {
            //zero padding for last block
            inputStorage->putPostInc(pfInputBuffer+i, iBufferLength % _iBlockLen);
            for (int remainings = 0; remainings < iNumZerosPadInput; remainings++) {
                inputStorage->putPostInc(0.f);
            }
        } else {
        //otherwise, use ringbuffer to save the input buffer to storage buffer
            inputStorage->putPostInc(pfInputBuffer+i, _iBlockLen);
        }

//        for (int sample=0; sample< _iIRLen; sample++) {
//            std::cout<<inputStorage->getPostInc()<<",";
//        }std::cout<<"\n";
        
        iBlockNumCounter++;
    }
    
//    inputStorage->putPostInc( pfInputBuffer, _iBlockLen );
    
    if ( _eDomainChoice == kTimeDomain ) {
        
        float *pfBridgeOut = new float[iBufferLength + _iIRLen ];
    
        processTimeDomain( pfInputBuffer, pfBridgeOut, iBufferLength );
        
        for (int sample=0; sample<iBufferLength + 4096*2; sample++) {
            pfOutputBuffer[sample] = pfBridgeOut[sample];
        }
//        outputStorage->addPostInc(pfOutputBuffer, iBufferLength);
//        outputStorage->add(pfOutputBuffer+iBufferLength, iBufferLength-1);
//        for( int sample = 0; sample< _iIRLen; sample++ ) {
//            std::cout<<outputStorage->getPostInc()<<", ";
//        }std::cout<<std::endl;
        
    }

    return kNoError;
}


Error_t CFastConv::processTimeDomain (float *pfInputBuffer, float *pfOutputBuffer, int iLengthOfBuffer ) {

//    for( int sample =0; sample<iLengthOfBuffer; sample++){
//        std::cout<<pfInputBuffer[sample]<<", ";
//    }std::cout<<std::endl;

    
    for ( int sample=0; sample<iLengthOfBuffer; sample++ ) {
        for( int ir=0; ir<_iIRLen; ir++ ) {
            pfOutputBuffer[ir+sample] += pfInputBuffer[sample]*_pfIR[ir];
        }
    }

//    for( int sample =0; sample<2*iLengthOfBuffer-1; sample++){
//        std::cout<<pfOutputBuffer[sample]<<", ";
//    }std::cout<<std::endl;
    return kNoError;

}


Error_t CFastConv::bufferInput( float *pfInputBuffer ) {

    inputStorage->putPostInc( pfInputBuffer, _iBlockLen );

    for( int sample=0; sample<_iBlockLen; sample++ ) {
        std::cout<<inputStorage->getPostInc()<<", ";
    }std::cout<<std::endl;

    return kNoError;
}





