#include "MUSI8903Config.h"

#ifdef WITH_TESTS
#include <cassert>
#include <cstdio>
#include <fstream>

#include "UnitTest++.h"

#include <vector>

#include "FastConv.h"

#include "Synthesis.h"

#include "Buffer.h"


extern std::string cTestDataDir;


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

////    only for testing the basic convolution equation works
//    TEST_FIXTURE(FastConvData, convTest) {
//        float input[6]   = { 1, 2, 3, 4, 5, 6},
//              impulse[3] = { 7, 8, 9},
//              output[8]  = { 7, 22, 46, 70, 94, 118, 93, 54};
//        float *procOut = new float[8];
//
//        m_pCFastConv->init( impulse, 3, 5, CFastConv::kTimeDomain );
//        m_pCFastConv->process( input, procOut, 6 );
//        
//
//        CHECK_ARRAY_EQUAL( output, procOut, 6 );
//    }
//
//
//    TEST_FIXTURE(FastConvData, inputBufferStorageTest) {
//        float input[16],
//              impulse[4] = { 1, 1, 1, 1}; 
//        float *procOut = new float[21];
//        int blockLen   = 3;
//
//        for( int sample =0; sample<16; sample++) {
//            input[sample] = sample*1.0f; 
//        }
//
//        m_pCFastConv->init( impulse, 4, blockLen, CFastConv::kTimeDomain );
//        for( int block = 0; block<6; block++ ) {
//            m_pCFastConv->process( &input[block*blockLen], &procOut[block*blockLen], blockLen );
//            for (int sample=0; sample< blockLen; sample++) {
//                std::cout<<procOut[block*blockLen +  sample]<<", ";
//            }std::cout<<std::endl;
//        }
//        delete procOut; procOut = 0;
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
//        delete procOut; procOut = 0;
//    }
//
//
//    

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
//        delete pfInputSignal; pfInputSignal = 0;
//        delete pfOutputSignal; pfOutputSignal = 0;
//        delete pfReference; pfReference = 0;
//    }
//    
    //Question 3.2
    TEST_FIXTURE(FastConvData, inputBufferStorageTest) {
        const int ipLen = 22,
        impLen= 4,
        opLen = ipLen + impLen - 1,
        numBlkSzs         = 8,
        blkLen[numBlkSzs] = { 3, 13, 1023, 2048,1,17, 5000, 1897};
        
        
        float input[ipLen],
        impulse[impLen] = { 1, 1, 1, 1},
        output[ipLen]   = { 0, 1, 3, 6, 10, 14, 18, 22, 26, 30, 34, 38, 42, 46, 50, 54 };
        
        for( int sample =0; sample<ipLen; sample++) {
            input[sample] = sample*1.0f;
        }
        
        
        
        for( int blkIdx = 0; blkIdx< numBlkSzs; blkIdx++ ) {
            float *procOut = new float[std::max(opLen, blkLen[blkIdx])];
            m_pCFastConv->init( impulse, impLen, 15, CFastConv::kTimeDomain );
            
            const int numBlks   = ipLen/blkLen[blkIdx] + 1;
            
            for( int block = 0; block<numBlks; block++ ) {
                int curIdx = block*blkLen[blkIdx];
                if ( ipLen < curIdx + blkLen[blkIdx] ) {
                    //Do zero padding
                    float *temp = new float[ blkLen[blkIdx] ];
                    for( int sample=curIdx, i = 0; sample<ipLen; sample++, i++ ) {
                        temp[i] = input[sample];
                    }
                    for( int sample= ipLen%blkLen[blkIdx]; sample< blkLen[blkIdx]; sample++ ) {
                        temp[sample] = 0.0f;
                    }
                    
                    m_pCFastConv->process( temp, &procOut[ curIdx ], blkLen[blkIdx] );
                    delete [] temp;
                }
                else {
                    
                    m_pCFastConv->process( &input[curIdx], &procOut[curIdx], blkLen[blkIdx] );
                }
                
            }
            
            CHECK_ARRAY_EQUAL( output, procOut, 16 );
            
            
            delete procOut; procOut = 0;
        }
    }
    
}

#endif //WITH_TESTS
