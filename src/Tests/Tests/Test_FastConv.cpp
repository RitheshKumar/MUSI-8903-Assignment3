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
/*
    Test 0.0
    only for testing the basic convolution equation works
*/
    TEST_FIXTURE(FastConvData, convTest) {
        float input[6]     = { 1, 2, 3, 4, 5, 6},
              impulse[3]   = { 7, 8, 9},
              reference[8] = { 7, 22, 46, 70, 94, 118, 93, 54};
        float *procOut     = new float[8];
        float *tail        = new float[2];
        int iIrBlk         = 5;
        
        m_pCFastConv->init( impulse, 3, iIrBlk, CFastConv::kFreqDomain );
        m_pCFastConv->process( input, procOut, 6 );
        m_pCFastConv->flushBuffer(tail);
        
        //concatenate the tail
        for (int i = 0; i < 2; i++) {
            procOut[6+i] = tail[i];
        }

        CHECK_ARRAY_EQUAL( reference, procOut, 8 );
    }

///*
// Question 3.1: Identity Check
// The output of a delayed impulse is the shifted IR
// */
//    TEST_FIXTURE(FastConvData, IrTest)
//    {
//        //create a impulse response of 51 seconds with a random indexing of 1
//        int iSampleRate       = 100;
//        int iIRLengthInSec    = 51;
//        int iIRLengthInSample = iSampleRate * iIRLengthInSec;
//        float* pfRandomIR     = new float[iIRLengthInSample];
//        float* pfTail         = new float[iIRLengthInSample - 1];
//        float decEnv          = 1;
//        
//        for (int sample = 0; sample < iIRLengthInSample; sample++) {
//            pfRandomIR[sample] =   decEnv - 0.1f * (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
//        }
//
//        // create a input signal: delayed impulse (5 samples)
//        int iInputLength      = 10;
//        float* pfInputSignal  = new float[iInputLength];
//        float* pfOutputSignal = new float[iInputLength];
//        
//        for( int sample = 0; sample<iInputLength; sample++) {
//            pfInputSignal[sample] = 0.0f;
//        }
//        pfInputSignal[5] = 1.f;
//        
//        // create the reference signal: shifted IR (shifted by 5 samples)
//        float* pfReference = new float[iInputLength+iIRLengthInSample-1];
//        for (int sample = 0; sample < iInputLength+iIRLengthInSample-1; sample++) {
//            pfReference[sample] = 0.f;
//        }
//        for (int sample = 5; sample < 5+iIRLengthInSample; sample++) {
//            pfReference[sample] = pfRandomIR[sample - 5];
//        }
//
//        // identity test using process function
//        m_pCFastConv->init(pfRandomIR, iIRLengthInSample, 4096, CFastConv::kTimeDomain);
//        m_pCFastConv->process(pfInputSignal, pfOutputSignal, iInputLength);
//        m_pCFastConv->flushBuffer(pfTail);
//        
//        CHECK_ARRAY_EQUAL( pfReference, pfOutputSignal, iInputLength);
//        CHECK_ARRAY_EQUAL( &pfReference[iInputLength], pfTail, (iIRLengthInSample - 1));
//        
//        delete pfRandomIR; pfRandomIR = 0;
//        delete pfInputSignal; pfInputSignal = 0;
//        delete pfOutputSignal; pfOutputSignal = 0;
//        delete pfReference; pfReference = 0;
//    }
//
///*
// Question 3.2: Testing for input buffer length variation
// Test with a succession of different input/output buffer length
// */
//    
//    TEST_FIXTURE(FastConvData, inputBufferStorageTest) {
//        const int ipLen = 22,
//        impLen= 4,
//        opLen = ipLen + impLen - 1,
//        numBlkSzs         = 12,
//        blkLen[numBlkSzs] = {3, 17, 2, 7001, 3, 13, 1023, 2048, 1,17, 5000, 1897};
//        
//        
//        float input[ipLen],
//        impulse[impLen] = { 1, 1, 1, 1},
//        
//        output[opLen]   = {  0, 1, 3, 6, 10, 14, 18, 22, 26, 30, 34, 38, 42, 46, 50, 54, 58, 62, 66, 70, 74, 78, 60, 41, 21 };
//        
//        for( int sample =0; sample<ipLen; sample++) {
//            input[sample] = sample*1.0f;
//        }
//        
//        
//        
//        for( int blkIdx = 0; blkIdx< numBlkSzs; blkIdx++ ) {
//            int numBlks = 0;
//            if (ipLen % blkLen[blkIdx] == 0) {
//                numBlks = ipLen/blkLen[blkIdx];
//            } else {
//                numBlks = ipLen/blkLen[blkIdx] + 1;
//            }
////            std::cout<<"BlockLen: "<<blkLen[blkIdx]<<"\n";
//            //            std::cout<<"prcOutLen: "<<std::max(opLen, numBlks*blkLen[blkIdx]+1)<<std::endl;
//            float *procOut = new float[std::max(opLen, blkLen[blkIdx]*numBlks+1)];
//            m_pCFastConv->init( impulse, impLen, 15, CFastConv::kTimeDomain );
//            
//            
//            
//            for( int block = 0; block<numBlks; block++ ) {
//                int curIdx = block*blkLen[blkIdx];
//                //                std::cout<<"curIdx: "<<curIdx<<std::endl;
//                if ( ipLen < curIdx + blkLen[blkIdx] ) {
//                    //Do zero padding
//                    //                    float *lastBlk = 0; delete lastBlk;
//                    //                    lastBlk = new float[ blkLen[blkIdx] ];
//                    float *lastBlk = new float [ blkLen[blkIdx] ];
//                    for( int sample=curIdx, i = 0; sample<ipLen; sample++, i++ ) {
//                        lastBlk[i] = input[sample];
//                    }
//                    for( int sample= ipLen%blkLen[blkIdx]; sample< blkLen[blkIdx]; sample++ ) {
//                        lastBlk[sample] = 0.0f;
//                    }
//                    
//                    //                    std::cout<<"CurIdx: "<<curIdx<<", curIdx+blkLen: "<<blkLen[blkIdx]+curIdx<<std::endl;
//                    m_pCFastConv->process( lastBlk, &procOut[ curIdx ], ipLen-curIdx );
//                    delete [] lastBlk; lastBlk = 0;
//                }
//                else {
//                    //                    std::cout<<"CurIdx: "<<curIdx<<", curIdx+blkLen: "<<blkLen[blkIdx]+curIdx<<std::endl;
//                    m_pCFastConv->process( &input[curIdx], &procOut[curIdx], blkLen[blkIdx] );
//                }
//                
//            }
//            //            std::cout<<"ProcLen: ";
//            //            for (int sample=0; sample<ipLen; sample++) {
//            //                std::cout<<procOut[sample]<<",";
//            //            }std::cout<<std::endl;
//            //                m_pCFastConv->getTail(procOut+ipLen, impLen-1, ipLen, blkLen[blkIdx]);
//            float* pfTail = new float[impLen-1];
//            m_pCFastConv->flushBuffer(pfTail);
//            CHECK_ARRAY_EQUAL( output, procOut, ipLen );
//                            CHECK_ARRAY_EQUAL( &output[ipLen], pfTail, (impLen-1));
//            
//            
//            delete [] procOut; procOut = 0;
//            //            std::cout<<std::endl;
//        }
//    }
//    
}


#endif //WITH_TESTS
