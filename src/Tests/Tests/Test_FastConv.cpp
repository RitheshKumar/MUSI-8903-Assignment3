#include "MUSI8903Config.h"

#ifdef WITH_TESTS
#include <cassert>
#include <cstdio>

#include "UnitTest++.h"

#include "Vector.h"

#include "FastConv.h"

SUITE(FastConv)
{
    struct FastConvData
    {
        FastConvData()
        {
            CFastConv::create( m_pCFastConv );  
        }

        ~FastConvData() 
        {
            CFastConv::destroy( m_pCFastConv );
        }


        CFastConv *m_pCFastConv;
        float m_fImpulseResponse,
              m_fInputSignal;
        
    };

//
    TEST_FIXTURE(FastConvData, convTest) {
        float input[6]   = { 1, 2, 3, 4, 5, 6},
              impulse[3] = { 7, 8, 9},
              output[8]  = { 7, 22, 46, 70, 94, 118, 93, 54};
        float *procOut = new float[8];

        m_pCFastConv->init( impulse, 3, 4, CFastConv::kTimeDomain );
        m_pCFastConv->process( input, procOut, 6 );

        CHECK_ARRAY_EQUAL( output, procOut, 8 );
    }

//    TEST_FIXTURE(FastConvData, inputBufferStorageTest) {
//        float input[18],
//              impulse[4] = { 1, 1, 1, 1}; 
//        float *procOut = new float[21];
//        int blockLen   = 3;
//
//        for( int sample =0; sample<18; sample++) {
//            input[sample] = sample*1.0f; 
//        }
//
//        m_pCFastConv->init( impulse, 4, blockLen, CFastConv::kTimeDomain );
//        for( int block = 0; block<6; block++ ) {
//            m_pCFastConv->process( &input[block*blockLen], &procOut[block*blockLen], blockLen );
//        }
//
//    }

//    TEST_FIXTURE
    
 
//    TEST_FIXTURE(FastConvData, IrTest)
//    {
//    }
//
//    TEST_FIXTURE(FastConvData, InputBlockLengthTest)
//    {
//    }

}

#endif //WITH_TESTS
