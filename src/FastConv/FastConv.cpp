
#include "Vector.h"
#include "Util.h"

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

//Error_t CFastConv::init( float *pfImpulseResponse, int iLengthOfIr, int iBlockLength /*= 8192*/, ConvDomain domainChoice )
//{
//    reset();
//    
//    _eDomainChoice = domainChoice;
//
//    //Zero pad IR to multiple integer No. of blocks
//    int nZerosToPad = iBlockLength - iLengthOfIr%iBlockLength;
//    _iNumBlocks = iLengthOfIr/iBlockLength + 1;
//    _iIRLen = iLengthOfIr + nZerosToPad;
//    _iBlockLen = iBlockLength;
//    _iOutputLength = iLengthOfIr;
//    _pfIR   = new float[_iIRLen];
//    for( int sample=0; sample<iLengthOfIr; sample++ ) {
//        _pfIR[sample] = pfImpulseResponse[sample];
//    }
//    for( int sample = iLengthOfIr; sample<_iIRLen; sample++ ) {
//        _pfIR[sample] = 0.0f;
//    }
//
//    _bIsInit = true;
//    
//    buffer->init(_iNumBlocks);
//
//    inputStorage = new CRingBuffer<float>( _iIRLen );
//    inputStorage->reset();
//
//    outputStorage = new CRingBuffer<float>( _iIRLen );
//    outputStorage->reset();
//    
//    return kNoError;
//}

Error_t CFastConv::init( float *pfImpulseResponse, int iLengthOfIr, int iBlockLength /*= 8192*/, ConvDomain domainChoice )
{
    reset();
    
    _eDomainChoice = domainChoice;
    
    //Check the block length for IR is power of 2
    //if it is not power of 2, we find the next power of 2 value to be the iBlockLength
    if (iBlockLength % 2 != 0) {
        iBlockLength = NextPowerOf2(iBlockLength);
    }
    
    //Zero pad IR to multiple integer No. of blocks
    int nZerosToPad = 0;
    if (iLengthOfIr%iBlockLength == 0) {
        nZerosToPad = 0;
        _iNumBlocks = iLengthOfIr / iBlockLength;
    
    } else {
        nZerosToPad = iBlockLength - iLengthOfIr%iBlockLength;
        _iNumBlocks = iLengthOfIr/iBlockLength + 1;
    }
    
    //iIRLen is the IR length with zero pad
    //iIRLenNoPad is the IR length without zero pad
    //iLengthOfIr is the original IR length user pass in
    _iIRLen = iLengthOfIr + nZerosToPad;
    _iBlockLen = iBlockLength;
//    _iOutputLength = iLengthOfIr;
    _pfIR   = new float[_iIRLen];
    _iIRLenNoPad = iLengthOfIr;
    for( int sample=0; sample<iLengthOfIr; sample++ ) {
        _pfIR[sample] = pfImpulseResponse[sample];
    }
    for( int sample = iLengthOfIr; sample<_iIRLen; sample++ ) {
        _pfIR[sample] = 0.0f;
    }
    
    m_pfTailBuffer = new float[_iIRLenNoPad - 1];
    memset(m_pfTailBuffer, 0.0f, (_iIRLenNoPad-1)*sizeof(float));
    
    buffer = new CBuffer(100, 600000, 600000);
    
    _bIsInit = true;
//    
//    buffer->init(_iNumBlocks);
//    
//    inputStorage = new CRingBuffer<float>( _iIRLen );
//    inputStorage->reset();
//    
//    outputStorage = new CRingBuffer<float>( _iIRLen );
//    outputStorage->reset();
    
    return kNoError;
}

/*
 * returns the next power of 2 of a given number
 */
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
    _iOutputLength = 0;
    _iIRLenNoPad   = 0;
    
    if(_pfIR){
        delete [] _pfIR;
        _pfIR = 0;
    }
//    delete [] _pfIR; _pfIR = 0;
//    delete inputStorage; inputStorage = 0;
//    delete outputStorage; outputStorage = 0;
    _bIsInit = false;
    
    delete m_pfTailBuffer;
     m_pfTailBuffer = 0;
    
    if (buffer != NULL) {
        delete buffer;
    }
    buffer = 0;
   
    
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
    
//    float* pfPadOutputBuffer = new float[iPadInputLength];
//    memset(pfPadOutputBuffer, 0.0f, iPadInputLength*sizeof(float));
//    float* pfTailBuffer = new float[_iIRLen-1];
//    memset(pfTailBuffer, 0.0f, (_iIRLen-1)*sizeof(float));
    //this buffer has the length of the summation of both padded input and impulse length
    float* pfPadOutputStorageBuffer = new float[iPadInputLength + _iIRLen - 1];
    memset(pfPadOutputStorageBuffer, 0.0f, (iPadInputLength+_iIRLen-1)*sizeof(float));

    //for each inputblock
    for (int nthBlock = 0; nthBlock < numOfInputBlock; nthBlock++) {
        //creat a single processing block based on input buffer
        float* pfProcessingBlock = new float[_iBlockLen];
        
//        for(int sample = 0; sample < _iBlockLen; sample++) {
//            pfProcessingBlock[sample] = pfPadInputBuffer[(nthBlock*_iBlockLen) + sample];
        memcpy(pfProcessingBlock, &pfPadInputBuffer[nthBlock*_iBlockLen], _iBlockLen*sizeof(float));
        
        //for each ir block
        for (int nthIrBlock = 0; nthIrBlock < _iNumBlocks; nthIrBlock++) {
            float* pfProcessingIrBlock = new float[_iBlockLen];
            float* pfStorageBuffer = new float[2*_iBlockLen - 1];
            
//            for (int irSample = 0; irSample < _iBlockLen; irSample++) {
//                    pfProcessingIrBlock[sample] = _pfIR[(nthIrBlock*_iBlockLen) + irSample];
//                }
            memcpy(pfProcessingIrBlock, &_pfIR[nthIrBlock*_iBlockLen], _iBlockLen*sizeof(float));
            memset(pfStorageBuffer, 0.0f, (2*_iBlockLen-1)*sizeof(float));
            
            blockedProcessTimeDomain(pfProcessingBlock, pfProcessingIrBlock, pfStorageBuffer, _iBlockLen, _iBlockLen);
            
            for (int i = 0; i < 2*_iBlockLen - 1; i++) {
                pfPadOutputStorageBuffer[(nthBlock+nthIrBlock)*_iBlockLen + i] += pfStorageBuffer[i];
            }
            
//            if (nthBlock + nthIrBlock <= numOfInputBlock - 2) {
//                // overlap and add to the pfPadOutputBuffer
//                for (int i = 0; i < 2*iBufferLength - 1; i++) {
//                        pfPadOutputBuffer[(nthBlock+nthIrBlock)*_iBlockLen + i] += pfStorageBuffer[i];
//                    }
//            } else if (nthBlock + nthIrBlock == numOfInputBlock - 1) {
//                // overlap and add first half of the result to the pfPadOutputBuffer, second half to the pfTailBuffer
//                for (int i = 0; i < iBufferLength; i++) {
//                    pfPadOutputBuffer[(nthBlock+nthIrBlock)*_iBlockLen + i] += pfStorageBuffer[i];
//                }
//                for (int i = 0; i < _iIRLenNoPad - 1; i++) {
//                    pfTailBuffer[i] += pfStorageBuffer[iBufferLength + i];
//                
//                }
//            } else {
//                //overlap and add to the pfTailBuffer
////                for (int i = 0; i < 2*_iBlockLen-1; i++) {
//                for (int i = 0; i < _iBlockLen-1; i++) {
//                    pfTailBuffer[(nthBlock+nthIrBlock-numOfInputBlock)*_iBlockLen+i] += pfStorageBuffer[i];
//                }
//            }
            
            
            
            delete pfProcessingIrBlock;
            delete pfStorageBuffer;
            pfProcessingIrBlock = 0;
            pfStorageBuffer = 0;
        }
        
        //give the (input length + ir length - 1) samples to the pfOutputBuffer

        
    
        delete pfProcessingBlock;
        pfProcessingBlock = 0;
    }
    
    memcpy(pfOutputBuffer, pfPadOutputStorageBuffer, iBufferLength*sizeof(float));
    memcpy(m_pfTailBuffer, &pfPadOutputStorageBuffer[iBufferLength], (_iIRLenNoPad-1)*sizeof(float));
    
    delete pfPadOutputStorageBuffer;
    pfPadOutputStorageBuffer = 0;
    
//    //3, adding the current process output with the previous reverb tail
//    for (int sample = 0; sample < _iIRLen-1; sample++) {
//        pfPadOutputBuffer[sample] += m_pfTailBuffer[sample];
//    }
    
    //4, copy the data from pfPadOutBuffer to the pfOutputBuffer
//    memcpy(pfOutputBuffer, pfPadOutputBuffer, iBufferLength*sizeof(float));
    
    //4, update the reverb tail
//    memcpy(m_pfTailBuffer, pfTailBuffer, (_iIRLen-1)*sizeof(float));
    
    delete pfPadInputBuffer;
//    delete pfPadOutputBuffer;
//    delete pfTailBuffer;
    pfPadInputBuffer = 0;
//    pfPadOutputBuffer = 0;
//    pfTailBuffer = 0;

    return kNoError;
}
//Error_t CFastConv::process (float *pfInputBuffer, float *pfOutputBuffer, int iBufferLength )
//{
//    //zeros pad for the input buffer
//    int iNumZerosPadInput = _iBlockLen - (iBufferLength % _iBlockLen);
//    int iNumBlockInput    = iBufferLength / _iBlockLen + 1;
//    int iBlockNumCounter  = 1;
//    _iOutputLength = _iOutputLength + iBufferLength - 1;
//    
//    if( !_bIsInit ) {
//        return kNotInitializedError;
//    }
//    for (int i = 0; i < iBufferLength; i = i+_iBlockLen) {
//        if (iBlockNumCounter == iNumBlockInput) {
//            //zero padding for last block
//            inputStorage->putPostInc(pfInputBuffer+i, iBufferLength % _iBlockLen);
//            for (int remainings = 0; remainings < iNumZerosPadInput; remainings++) {
//                inputStorage->putPostInc(0.f);
//            }
//        } else {
//        //otherwise, use ringbuffer to save the input buffer to storage buffer
//            inputStorage->putPostInc(pfInputBuffer+i, _iBlockLen);
//        }
//
////        for (int sample=0; sample< _iIRLen; sample++) {
////            std::cout<<inputStorage->getPostInc()<<",";
////        }std::cout<<"\n";
//        
//        iBlockNumCounter++;
//    }
//    
////    inputStorage->putPostInc( pfInputBuffer, _iBlockLen );
//    
//    if ( _eDomainChoice == kTimeDomain ) {
//        // Length of Input + length of IR + length of input 0 pad + length of IR 0 pad
//        float *pfBridgeOut = new float[iBufferLength + iNumZerosPadInput + _iIRLen ];
//    
//        processTimeDomain( pfInputBuffer, pfBridgeOut, iBufferLength );
//        
//        for (int sample=0; sample< _iOutputLength; sample++) {
//            pfOutputBuffer[sample] = pfBridgeOut[sample];
//        }
//        
////        outputStorage->addPostInc(pfOutputBuffer, iBufferLength);
////        outputStorage->add(pfOutputBuffer+iBufferLength, iBufferLength-1);
////        for( int sample = 0; sample< _iIRLen; sample++ ) {
////            std::cout<<outputStorage->getPostInc()<<", ";
////        }std::cout<<std::endl;
//        _iOutputLength = _iOutputLength - iBufferLength + 1;
//    }
//
//    return kNoError;
//}

//Error_t CFastConv::process (float *pfInputBuffer, float *pfOutputBuffer, int iBufferLength )
//{
//    //zeros pad for the input buffer
//    //    int iNumZerosPadInput = _iBlockLen - (iBufferLength % _iBlockLen);
//    //    int iNumBlockInput    = iBufferLength / _iBlockLen + 1;
//    //    int iBlockNumCounter  = 1;
//    
//    if( !_bIsInit ) {
//        return kNotInitializedError;
//    }
//    
//    buffer->storeInput(pfInputBuffer, iBufferLength);
////    float *tempInput = new float(iBufferLength);
////    buffer->getInputBlock(0, tempInput, iBufferLength);
////    for (int sample= 0; sample < iBufferLength; sample++) {
////        
////        std::cout<<tempInput[sample];
////    }
////    delete [] tempInput;
//    
//    /*
//     psudo code for partitioned fast convolution
//     nested for loop:
//        1. cycle
//        2, input block
//        3, impulse response block
//        4, NthInputBLock are based on the ring buffer
//     for (int NthOutputBlock = 0; NthOutputBlock < numOfBlockOfOutput; NthOutputBlock++) {
//        for (int NthImpulseBlock = 0; NthImpulseBlock < numOfImpulseBlock; NthImpulseBlock++) {
//            for (int NthInputBlock = 0; NthInputBLock < numOfInputBlock; NthInputBlock++) {
//                while (NthInputBlock + NthImpulseBlock = NthOutputBlock + 1) {
//                    //do the process
//                    //add to the output storage buffer
//                }
//            }
//
//        }
//     
//     }
//     */
//    
//    if ( _eDomainChoice == kTimeDomain ) {
//        
//        float *pfBridgeOut = new float[iBufferLength + _iIRLen -1 ];
//        memset (pfBridgeOut, 0, sizeof(float)*(iBufferLength + _iIRLen - 1) );
//        
//        
//        
//        processTimeDomain( pfInputBuffer, pfBridgeOut, iBufferLength );
//        
//        //        std::cout<<"pfBridgeOut: ";
//        //        for( int sample = 0; sample < iBufferLength + _iIRLen; sample++ ) {
//        //            std::cout<<pfBridgeOut[sample]<<",";
//        //        }std::cout<<std::endl;
//        
//        
////        outputStorage->addPostInc(pfBridgeOut, iBufferLength);
////        outputStorage->add(&pfBridgeOut[iBufferLength], _iIRLen-1);
//        
//        buffer->overLapAdd(pfBridgeOut, iBufferLength, _iIRLen);
//        
//        
//        //        std::cout<<"pfOutputBuffer: ";
////        for( int sample = 0; sample< iBufferLength; sample++ ) {
////            pfOutputBuffer[sample] = outputStorage->getPostInc();
////            //            std::cout<<pfOutputBuffer[sample]<<",";
////        }
//        //        std::cout<<std::endl;
//        
//        buffer->getOutput(pfOutputBuffer, iBufferLength);//+_iIRLen-1);
//
//        //        std::cout<<"pfOutputBuffer: ";
////        for (int sample = 0; sample < iBufferLength + _iIRLen - 1; sample++) {
////            std::cout<<pfOutputBuffer[sample];
////        } std::cout<<",";
//        
//        delete [] pfBridgeOut;
//        pfBridgeOut = 0;    
//    }
//    
//    return kNoError;
//}



//Error_t CFastConv::processTimeDomain (float *pfInputBuffer, float *pfOutputBuffer, int iLengthOfBuffer ) {
//
////    for( int sample =0; sample<iLengthOfBuffer; sample++){
////        std::cout<<pfInputBuffer[sample]<<", ";
////    }std::cout<<std::endl;
//
//    
//    for ( int sample=0; sample<iLengthOfBuffer; sample++ ) {
//        for( int ir=0; ir<_iIRLen; ir++ ) {
//            pfOutputBuffer[ir+sample] += pfInputBuffer[sample]*_pfIR[ir];
//        }
//    }
//
//    return kNoError;
//
//}

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

void CFastConv::getTail(float* pfTail, int iTailLength, int ipLength, int ipBLockSize) {
    int iNumZeroToPad_IP = ipBLockSize - ipLength%ipBLockSize;
    buffer->getTail( pfTail, iTailLength, iNumZeroToPad_IP );
}

void CFastConv::blockImpulseResponse() {
    
}

//Error_t CFastConv::bufferInput( float *pfInputBuffer ) {
//
//    inputStorage->putPostInc( pfInputBuffer, _iBlockLen );
//
//    for( int sample=0; sample<_iBlockLen; sample++ ) {
//        std::cout<<inputStorage->getPostInc()<<", ";
//    }std::cout<<std::endl;
//
//    return kNoError;
//}





