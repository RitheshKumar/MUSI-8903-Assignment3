
#include "Vector.h"
#include "Util.h"

#include "FastConv.h"

CFastConv::CFastConv( void ): _pfIR(0), inputStorage(0), outputStorage(0)
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
    
//    if(!inputStorage) {
//        delete inputStorage;
//        inputStorage = new CRingBuffer<float>( _iIRLen );
//        inputStorage->reset();
//    }
//    
//    else {
//        inputStorage = new CRingBuffer<float>( _iIRLen );
//        inputStorage->reset();
//    }

    
    if(!outputStorage) {
        delete outputStorage;
        outputStorage = new CRingBuffer<float>( 6000 );
        outputStorage->reset();
    }
    
    else {
        outputStorage = new CRingBuffer<float>( 6000 );
        outputStorage->reset();
    }

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
//    int iNumZerosPadInput = _iBlockLen - (iBufferLength % _iBlockLen);
//    int iNumBlockInput    = iBufferLength / _iBlockLen + 1;
//    int iBlockNumCounter  = 1;
    
    if( !_bIsInit ) {
        return kNotInitializedError;
    }

 
    
    if ( _eDomainChoice == kTimeDomain ) {
        
        float *pfBridgeOut = new float[iBufferLength + _iIRLen ];
        memset (pfBridgeOut, 0, sizeof(float)*(iBufferLength + _iIRLen) );
        
        processTimeDomain( pfInputBuffer, pfBridgeOut, iBufferLength );
        
//        std::cout<<"pfBridgeOut: ";
//        for( int sample = 0; sample < iBufferLength + _iIRLen; sample++ ) {
//            std::cout<<pfBridgeOut[sample]<<",";
//        }std::cout<<std::endl;


        outputStorage->addPostInc(pfBridgeOut, iBufferLength);
        outputStorage->add(&pfBridgeOut[iBufferLength], _iIRLen-1);
        
        
        
//        std::cout<<"pfOutputBuffer: ";
        for( int sample = 0; sample< iBufferLength; sample++ ) {
            pfOutputBuffer[sample] = outputStorage->getPostInc();
//            std::cout<<pfOutputBuffer[sample]<<",";
        }
//        std::cout<<std::endl;
        
        delete [] pfBridgeOut;
        pfBridgeOut = 0;    
    }

    return kNoError;
}


Error_t CFastConv::processTimeDomain (float *pfInputBuffer, float *pfOutputBuffer, int iLengthOfBuffer ) {

//    std::cout<<"Ip: ";
//    for( int sample =0; sample<iLengthOfBuffer; sample++){
//        std::cout<<pfInputBuffer[sample]<<", ";
//    }std::cout<<std::endl;
//    
//    std::cout<<"OpBufferBefore: ";
//    for( int sample =0; sample<2*iLengthOfBuffer-1; sample++){
//        std::cout<<pfOutputBuffer[sample]<<", ";
//    }std::cout<<std::endl;
    
    for ( int sample=0; sample<iLengthOfBuffer; sample++ ) {

        for( int ir=0; ir<_iIRLen; ir++ ) {
            pfOutputBuffer[ir+sample] += pfInputBuffer[sample]*_pfIR[ir];

        }

    }

//    std::cout<<"OpBufferAfter: ";
//    for( int sample =0; sample<2*iLengthOfBuffer-1; sample++){
//        std::cout<<pfOutputBuffer[sample]<<", ";
//    }std::cout<<std::endl<<std::endl;
    return kNoError;

}


Error_t CFastConv::bufferInput( float *pfInputBuffer ) {

    inputStorage->putPostInc( pfInputBuffer, _iBlockLen );

    for( int sample=0; sample<_iBlockLen; sample++ ) {
        std::cout<<inputStorage->getPostInc()<<", ";
    }std::cout<<std::endl;

    return kNoError;
}





