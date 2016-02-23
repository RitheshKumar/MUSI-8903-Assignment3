
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

    _pfIR   = new float[iLengthOfIr + nZerosToPad];

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

Error_t CFastConv::process (float *pfInputBuffer, float *pfOutputBuffer, int iBlockLen )
{

    if( !_bIsInit ) {
        return kNotInitializedError;
    }

    inputStorage->putPostInc( pfInputBuffer, _iBlockLen );
    
    if ( _eDomainChoice == kTimeDomain ) {
        processTimeDomain( pfInputBuffer, pfOutputBuffer, iBlockLen );
        outputStorage->addPostInc(pfOutputBuffer, iBlockLen);
        outputStorage->add(pfOutputBuffer+iBlockLen, iBlockLen-1);
//        for( int sample = 0; sample< _iIRLen; sample++ ) {
//            std::cout<<outputStorage->getPostInc()<<", ";
//        }std::cout<<std::endl;
        
    }

    return kNoError;
}


Error_t CFastConv::processTimeDomain (float *pfInputBuffer, float *pfOutputBuffer, int iLengthOfBuffer ) {

    for( int sample =0; sample<iLengthOfBuffer; sample++){
        std::cout<<pfInputBuffer[sample]<<", ";
    }std::cout<<std::endl;

    
    for ( int sample=0; sample<iLengthOfBuffer; sample++ ) {
        for( int ir=0; ir<_iBlockLen; ir++ ) {
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





