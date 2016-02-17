
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

Error_t CFastConv::init(float *pfImpulseResponse, int iLengthOfIr, int iBlockLength /*= 8192*/)
{

    return kNoError;
}

Error_t CFastConv::reset()
{

    return kNoError;
}

Error_t CFastConv::process (float *pfInputBuffer, float *pfOutputBuffer, int iLengthOfBuffers )
{

    return kNoError;
}
