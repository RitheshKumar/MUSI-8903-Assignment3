
#include "Vector.h"
#include "Util.h"

#include "FastConv.h"

CFastConv::CFastConv( void )
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
    _iIRLen = iLengthOfIr;
    _pfIR   = new float[iLengthOfIr];

    for( int sample=0; sample<_iIRLen; sample++ ) {
        _pfIR[sample] = pfImpulseResponse[sample];
    }

    _bIsInit = true;

    return kNoError;
}

Error_t CFastConv::reset()
{
    _iIRLen = 0;
    _iBlockLen = 0;
    delete [] _pfIR;
    _pfIR = 0;
    _bIsInit = false;

    return kNoError;
}

Error_t CFastConv::process (float *pfInputBuffer, float *pfOutputBuffer, int iLengthOfBuffers )
{

    if( !_bIsInit ) {
        return kNotInitializedError;
    }

    if ( _eDomainChoice == kTimeDomain ) {
        processTimeDomain( pfInputBuffer, pfOutputBuffer, iLengthOfBuffers );
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



