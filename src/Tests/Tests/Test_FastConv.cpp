#include "MUSI8903Config.h"

#ifdef WITH_TESTS
#include <cassert>
#include <cstdio>
#include <fstream.h>

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
        ofstream fileOpen;
        
    };

    //only for testing the basic convolution equation works
    TEST_FIXTURE(FastConvData, convTest) {
        //float input[6]   = { 1, 2, 3, 4, 5, 6},
        //      impulse[3] = { 7, 8, 9},
        //      output[8]  = { 7, 22, 46, 70, 94, 118, 93, 54};
        //float *procOut = new float[8];
        
        //create a impulse response of 51 seconds with random index of 1
        //int iSampleRate = 44100;
        //int iIRLengthInSec = 10;
        int iIRLengthInSample = 44100 * 51 ;
        float* pfRandomIR = new float[ iIRLengthInSample ];
        //float* pfInputSignal = new float[4096*2];
        //float *procOut = new float[iIRLengthInSample + 4096*2 - 1 ];
        fileOpen.open("/Users/Rithesh/Documents/Learn C++/ASE/notes/Matlab_ASE/randomIR.txt");
        for( int sample=0; sample< iIRLengthInSample; sample++ ) {
            pfRandomIR << fileOpen;
        }


        //m_pCFastConv->init( input, 6, 4, CFastConv::kTimeDomain );
        //m_pCFastConv->process( impulse, procOut, 3 );

        m_pCFastConv->init( pfRandomIR, iIRLengthInSample, 4096, CFastConv::kTimeDomain );
        m_pCFastConv->process( pfInputSignal, procOut, 4096*2 );

        //CHECK_ARRAY_EQUAL( output, procOut, iIRLengthInSample + 4096*2 - 1);
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
    
//    //Question3.1
//    TEST_FIXTURE(FastConvData, IrTest)
//    {
//        //create a impulse response of 51 seconds with random index of 1
//        int iSampleRate = 44100;
//        int iIRLengthInSec = 10;
//        int iIRLengthInSample = iSampleRate * iIRLengthInSec;
////        int iIndexOfOne = rand() % 100 + 1;
//        float* pfRandomIR = new float[iIRLengthInSample];
////        float* pfRandomIR = new float[10];
//        float decEnv = 1;
//        float* pfInputSignal = new float[4096*2];
//        float* pfOutputSignal = new float[iIRLengthInSample + 4096*2 - 1];
//        
//        for( int sample = 0; sample<4096*2; sample++) {
//            pfInputSignal[sample] = 0.0f;
//        } pfInputSignal[0] = 1.f;
//        
//        for (int sample = 0; sample < iIRLengthInSample; sample++) {
//            pfRandomIR[sample] =   decEnv - 0.1f * (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
//            //            decEnv = decEnv-1/iIRLengthInSample;
//        }
//        m_pCFastConv->init(pfRandomIR, iIRLengthInSample, 4096, CFastConv::kTimeDomain);
//        m_pCFastConv->process(pfInputSignal, pfOutputSignal, 4096*2);
//        
//        CHECK_ARRAY_EQUAL( pfOutputSignal,pfRandomIR, iIRLengthInSample );
//        
//        delete pfRandomIR; pfRandomIR = 0;
//    }
    
//    //Question3.2
//    TEST_FIXTURE(FastConvData, InputBlockLengthTest)
//    {
//        //generate a 1 second sawtooth wave of 50 Hz for test (sample rate 10000)
//        int iInputLengthInSample = 10000;
//        float* pfInputSignal = new float[iInputLengthInSample];
//        CSynthesis::generateSaw(pfInputSignal, 50, 10000, iInputLengthInSample);
//        
//        //generate a impulse response
//        int iIRLengthInSample = 1000;
//        float* pfImpulseResponse = new float[iIRLengthInSample];
//        memset(pfImpulseResponse, 1, iIRLengthInSample*sizeof(*pfImpulseResponse));
//        
//        //creat a output buffer
//        int iOutputLengthInSample = iInputLengthInSample + iIRLengthInSample - 1;
//        float* pfOutputSignal = new float[iOutputLengthInSample];
//        
//        // create a array of block size
//        int iaBlockSize[] = {1, 13, 1023, 2048, 1, 17, 5000, 1897};
//        
//        for (int nthBlockSize = 0; nthBlockSize < sizeof(iaBlockSize) - 1; nthBlockSize++) {
//            // for each block size, run the corresponding test process
//            
//            m_pCFastConv->init( pfImpulseResponse, iIRLengthInSample, iaBlockSize[nthBlockSize], CFastConv::kTimeDomain );
//            m_pCFastConv->process( pfInputSignal, pfOutputSignal, iInputLengthInSample);
//            std::cout<<"pass the test, using " << iaBlockSize[nthBlockSize] << " !";
//        }
//        
//        delete pfInputSignal; pfInputSignal = 0;
//        delete pfOutputSignal; pfOutputSignal = 0;
//        delete pfImpulseResponse; pfImpulseResponse = 0;
//    }

}

#endif //WITH_TESTS
