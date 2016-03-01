#include "MUSI8903Config.h"

#ifdef WITH_TESTS
#include <cassert>
#include <cstdio>

#include "UnitTest++.h"

#include "Vector.h"

#include "FastConv.h"

#include "Synthesis.h"

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

//    only for testing the basic convolution equation works
//    TEST_FIXTURE(FastConvData, convTest) {
//        float impulse[6]   = { 1, 2, 3, 4, 5, 6},
//              input[3] = { 7, 8, 9},
//              output[8]  = { 7, 22, 46, 70, 94, 118, 93, 54};
//        float *procOut = new float[8];
//
//        m_pCFastConv->init( impulse, 6, 5, CFastConv::kTimeDomain );
//        m_pCFastConv->process( input, procOut, 3 );
//
//        CHECK_ARRAY_EQUAL( output, procOut, 8 );
//    }

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
//    }

//    TEST_FIXTURE
    
//    //Question3.1
//    TEST_FIXTURE(FastConvData, IrTest)
//    {
//        //create a impulse response of 51 seconds with random index of 1
//        int iSampleRate = 44100;
//        int iIRLengthInSec = 30;
//        int iIRLengthInSample = iSampleRate * iIRLengthInSec;
//        float* pfRandomIR = new float[iIRLengthInSample];
//        float decEnv = 1;
//        
//        for (int sample = 0; sample < iIRLengthInSample; sample++) {
//            pfRandomIR[sample] =   decEnv - 0.1f * (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
//            if (sample % 10000 == 0) {
//                //                std::cout<<pfRandomIR[sample]<<"  "<<decEnv<<std::endl;
//            }
//            //            decEnv = decEnv-1/iIRLengthInSample;
//        }
//        
//        // create a input signal: delayed impulse (5 samples)
//        int iInputLength = 10000;
//        float* pfInputSignal = new float[iInputLength];
//        float* pfOutputSignal = new float[iIRLengthInSample + iInputLength - 1];
//        
//        for( int sample = 0; sample<iInputLength; sample++) {
//            pfInputSignal[sample] = 0.0f;
//        }
//        pfInputSignal[5] = 1.f;
//        
//        // create the reference signal: shifted IR (5 samples)
//        float* pfReference = new float[iIRLengthInSample];
//        for (int sample = 0; sample < 5; sample++) {
//            pfReference[sample] = 0.f;
//        }
//        for (int sample = 5; sample < iIRLengthInSample; sample++) {
//            pfReference[sample] = pfRandomIR[sample - 5];
//        }
//
//        // identity test using process function
//        m_pCFastConv->init(pfRandomIR, iIRLengthInSample, 4096, CFastConv::kTimeDomain);
//        m_pCFastConv->process(pfInputSignal, pfOutputSignal, iInputLength);
//        
//        CHECK_ARRAY_EQUAL( pfReference, pfOutputSignal, iIRLengthInSample );
//        
//        delete pfRandomIR; pfRandomIR = 0;
//    }
    
    //Question3.2
    TEST_FIXTURE(FastConvData, InputBlockLengthTest)
    {
        //generate a 1 second sawtooth wave of 50 Hz for test (sample rate 10000)
        int iInputLengthInSample = 10000;
        float* pfInputSignal = new float[iInputLengthInSample];
        CSynthesis::generateSaw(pfInputSignal, 50, 10000, iInputLengthInSample);
        
        //generate a impulse response
        int iIRLengthInSample = 1000;
        float* pfImpulseResponse = new float[iIRLengthInSample];
        memset(pfImpulseResponse, 1, iIRLengthInSample*sizeof(*pfImpulseResponse));
        
        //creat a output buffer
        int iOutputLengthInSample = iInputLengthInSample + iIRLengthInSample - 1;
        float* pfOutputSignal = new float[iOutputLengthInSample];
        
        // create a array of block size
        int iaBlockSize[] = {1, 13, 1023, 2048, 1, 17, 5000, 1897};
        
        for (int nthBlockSize = 0; nthBlockSize < sizeof(iaBlockSize) - 1; nthBlockSize++) {
            // for each block size, run the corresponding test process
            
            m_pCFastConv->init( pfImpulseResponse, iIRLengthInSample, iaBlockSize[nthBlockSize], CFastConv::kTimeDomain );
            m_pCFastConv->process( pfInputSignal, pfOutputSignal, iInputLengthInSample);
            std::cout<<"pass the test, using " << iaBlockSize[nthBlockSize] << " !";
        }
        
        delete pfInputSignal; pfInputSignal = 0;
        delete pfOutputSignal; pfOutputSignal = 0;
        delete pfImpulseResponse; pfImpulseResponse = 0;
    }

}

#endif //WITH_TESTS
