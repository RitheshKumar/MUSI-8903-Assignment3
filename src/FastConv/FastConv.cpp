
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
    
    

    _eDomainChoice = domainChoice;

    //Zero pad IR to multiple integer No. of blocks
    int nZerosToPad = iBlockLength - iLengthOfIr%iBlockLength;
    _iNumBlocks = iLengthOfIr/iBlockLength + 1;
    _iIRLen = iLengthOfIr + nZerosToPad;
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
    
//    float *tempInput = new float(iBufferLength);
//    buffer->getInputBlock(0, tempInput, iBufferLength);
//    delete [] tempInput; tempInput=0;
//    for (int sample= 0; sample < iBufferLength; sample++) {
//        
//        std::cout<<tempInput[sample];
//    }
    
    
    if ( _eDomainChoice == kTimeDomain ) {
        
        float *pfBridgeOut = new float[iBufferLength + _iIRLen ];
        memset (pfBridgeOut, 0, sizeof(float)*(iBufferLength + _iIRLen) );
        
        
        
        processTimeDomain( pfInputBuffer, pfBridgeOut, iBufferLength );
        
        //        std::cout<<"pfBridgeOut: ";
        //        for( int sample = 0; sample < iBufferLength + _iIRLen; sample++ ) {
        //            std::cout<<pfBridgeOut[sample]<<",";
        //        }std::cout<<std::endl;
        
        
//        outputStorage->addPostInc(pfBridgeOut, iBufferLength);
//        outputStorage->add(&pfBridgeOut[iBufferLength], _iIRLen-1);
        
        buffer->overLapAdd(pfBridgeOut, iBufferLength, _iIRLen);
        
        
        //        std::cout<<"pfOutputBuffer: ";
//        for( int sample = 0; sample< iBufferLength; sample++ ) {
//            pfOutputBuffer[sample] = outputStorage->getPostInc();
//            //            std::cout<<pfOutputBuffer[sample]<<",";
//        }
        //        std::cout<<std::endl;
        
        buffer->getOutput(pfOutputBuffer, iBufferLength);//+_iIRLen-1);

        //        std::cout<<"pfOutputBuffer: ";
//        for (int sample = 0; sample < iBufferLength + _iIRLen - 1; sample++) {
//            std::cout<<pfOutputBuffer[sample];
//        } std::cout<<",";
        
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

void CFastConv::getTail(float* pfTail, int iTailLength, int ipLength, int ipBLockSize) {
    int iNumZeroToPad_IP = ipBLockSize - ipLength%ipBLockSize;
    buffer->getTail( pfTail, iTailLength, iNumZeroToPad_IP );
}

void CFastConv::blockImpulseResponse() {
    
} 





