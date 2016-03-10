#include "MUSI8903Config.h"

#ifdef WITH_TESTS
#include <cassert>
#include <cstdio>
#include <fstream>

#include "UnitTest++.h"

#include <vector>
#include <fstream>

#include "FastConv.h"

#include "Synthesis.h"

#include "Buffer.h"
#include "Fft.h"


extern std::string cTestDataDir;


SUITE(FastConv)
{
    struct FastConvData
    {
        FastConvData()
        {
            CFastConv::create( m_pCFastConv );
            CFft::create(m_pcFft);
        }

        ~FastConvData() 
        {
            CFastConv::destroy( m_pCFastConv );
            CFft::destroy(m_pcFft);
        }


        CFastConv *m_pCFastConv;
        CFft      *m_pcFft;
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
//        m_pCFastConv->getTail(procOut+6, 3-1);
//
//        CHECK_ARRAY_EQUAL( output, procOut, 8 );
//    }

//        //Question3.1
//        TEST_FIXTURE(FastConvData, IrTest)
//        {
//            //create a impulse response of 51 seconds with random index of 1
//            int iSampleRate = 44100;
////            int iIRLengthInSec = 3;
//            float fIRLengthInSec = 1.f/441.f;
//            int iIRLengthInSample = iSampleRate * fIRLengthInSec;
//            float* pfRandomIR = new float[iIRLengthInSample];
////            float decEnv = 1.f;
//            
//            std::ofstream outFile("/Users/Rithesh/Documents/Learn C++/ASE/notes/Matlab_ASE/radnomIR.txt");
//            for (int sample = 0; sample < iIRLengthInSample; sample++) {
//                pfRandomIR[sample] =   /*decEnv */ 0.1f * (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
//                outFile<<pfRandomIR[sample]<<"\n";
//            }
//            outFile.close();
//    
//            // create a input signal: delayed impulse (5 samples)
//            int iInputLength = 200;
//            float* pfInputSignal = new float[iInputLength];
//            float* pfOutputSignal = new float[iIRLengthInSample + iInputLength - 1];
//    
//            for( int sample = 0; sample<iInputLength; sample++) {
//                pfInputSignal[sample] = 0.0f;
//            }
//            pfInputSignal[5] = 1.f;
//    
//            outFile.open("/Users/Rithesh/Documents/Learn C++/ASE/notes/Matlab_ASE/inputSignal.txt");
//            for (int sample = 0; sample < iInputLength; sample++) {
//                outFile<<pfInputSignal[sample]<<"\n";
//            }
//            outFile.close();
//            
//            // create the reference signal: shifted IR (5 samples)
//            int convLen = iIRLengthInSample + iInputLength - 1;
//            float* pfReference = new float[convLen];
//            for (int sample = 0; sample < 5; sample++) {
//                pfReference[sample] = 0.f;
//            }
//            for (int sample = 5; sample < iIRLengthInSample; sample++) {
//                pfReference[sample] = pfRandomIR[sample - 5];
//            }
//            for (int sample=iIRLengthInSample; sample < convLen; sample++ ) {
//                pfReference[sample] = 0.f;
//            }
//    
//            // identity test using process function
//            m_pCFastConv->init(pfRandomIR, iIRLengthInSample, 20, CFastConv::kTimeDomain);
//            m_pCFastConv->process(pfInputSignal, pfOutputSignal, iInputLength);
//            m_pCFastConv->getTail(pfInputSignal+iInputLength, iInputLength, 20);
//
//            outFile.open("/Users/Rithesh/Documents/Learn C++/ASE/notes/Matlab_ASE/refOut.txt");
//            for (int sample=0; sample<convLen; sample++) {
//                outFile<<pfReference[sample]<<"\n";
//            }
//            outFile.close();
//
//            outFile.open("/Users/Rithesh/Documents/Learn C++/ASE/notes/Matlab_ASE/convOut.txt");
//            for (int sample=0; sample<convLen; sample++) {
//                outFile<<pfOutputSignal[sample]<<"\n";
//            }
//            outFile.close();
//
//
//            CHECK_ARRAY_EQUAL( pfReference, pfOutputSignal, convLen );
//    
//            delete pfRandomIR; pfRandomIR = 0;
//            delete pfInputSignal; pfInputSignal = 0;
//            delete pfOutputSignal; pfOutputSignal = 0;
//            delete pfReference; pfReference = 0;
//        }
    

//    //Question 3.2
//    TEST_FIXTURE(FastConvData, inputBufferStorageTest) {
//        const int ipLen = 22,
//        impLen= 4,
//        opLen = ipLen + impLen - 1,
//        numBlkSzs         = 12,
//        blkLen[numBlkSzs] = {3, 17, 2048, 5000, 3, 13, 1023, 2048, 1,17, 5000, 1897};
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
//            
//            const int numBlks   = ipLen/blkLen[blkIdx] + 1;
////            std::cout<<"BlockLen: "<<blkLen[blkIdx]<<"\n";
////            std::cout<<"prcOutLen: "<<std::max(opLen, numBlks*blkLen[blkIdx]+1)<<std::endl;
//            float *procOut = new float[std::max(opLen, blkLen[blkIdx]*numBlks+1)];
//            m_pCFastConv->init( impulse, impLen, 15, CFastConv::kTimeDomain );
//            
//            
//            
//            for( int block = 0; block<numBlks; block++ ) {
//                int curIdx = block*blkLen[blkIdx];
////                std::cout<<"curIdx: "<<curIdx<<std::endl;
//                if ( ipLen < curIdx + blkLen[blkIdx] ) {
//                    //Do zero padding
////                    float *lastBlk = 0; delete lastBlk;
////                    lastBlk = new float[ blkLen[blkIdx] ];
//                    float *lastBlk = new float [ blkLen[blkIdx] ];
//                    for( int sample=curIdx, i = 0; sample<ipLen; sample++, i++ ) {
//                        lastBlk[i] = input[sample];
//                    }
//                    for( int sample= ipLen%blkLen[blkIdx]; sample< blkLen[blkIdx]; sample++ ) {
//                        lastBlk[sample] = 0.0f;
//                    }
//                    
////                    std::cout<<"CurIdx: "<<curIdx<<", curIdx+blkLen: "<<blkLen[blkIdx]+curIdx<<std::endl;
//                    m_pCFastConv->process( lastBlk, &procOut[ curIdx ], blkLen[blkIdx] );
//                    delete [] lastBlk; lastBlk = 0;
//                }
//                else {
////                    std::cout<<"CurIdx: "<<curIdx<<", curIdx+blkLen: "<<blkLen[blkIdx]+curIdx<<std::endl;
//                    m_pCFastConv->process( &input[curIdx], &procOut[curIdx], blkLen[blkIdx] );
//                }
//                
//            }
////            std::cout<<"ProcLen: ";
////            for (int sample=0; sample<ipLen; sample++) {
////                std::cout<<procOut[sample]<<",";
////            }std::cout<<std::endl;
//            m_pCFastConv->getTail(procOut+ipLen, ipLen, blkLen[blkIdx]);
//            CHECK_ARRAY_EQUAL( output, procOut, opLen );
//            
//            
//            delete [] procOut; procOut = 0;
////            std::cout<<std::endl;
//        }
//    }
    
    
    //Figuring FFT out
    TEST_FIXTURE(FastConvData, fftTest) {

        float * pfInput  = new float[1024],
              * pfInput2 = new float[1024],
              * pfOutput = new float[1024],
              * pfMulOut = new float[1024];
        
//        generateSine (float *pfOutBuf, float fFreqInHz, float fSampleFreqInHz, int iLength, float fAmplitude = 1.F, float fStartPhaseInRad = 0.F)
        CSynthesis::generateSine (pfInput, 100, 10000, 1024);
        CSynthesis::generateSine (pfInput2, 500, 10000, 1024);
        
        m_pcFft->init(1024);
        CFft::complex_t *mySpectrum = new float[1024],
                        *mySpectrum2= new float[1024];
        
        m_pcFft->doFft(mySpectrum, pfInput );
        m_pcFft->getMagnitude (pfOutput, mySpectrum);
        m_pcFft->doFft(mySpectrum2, pfInput2 );
        
        m_pcFft->mulCompSpectrum( mySpectrum2, mySpectrum );
        m_pcFft->getMagnitude (pfMulOut, mySpectrum2);
        
        std::ofstream outFile("/Users/Rithesh/Documents/Learn C++/ASE/notes/Matlab_ASE/fftMag.txt");
        for( int sample=0; sample<1024; sample++) {
            outFile<<pfOutput[sample]<<"\n";
        }
        outFile.close();
        outFile.open("/Users/Rithesh/Documents/Learn C++/ASE/notes/Matlab_ASE/inputSine.txt");
        for( int sample=0; sample<1024; sample++) {
            outFile<<pfInput[sample]<<"\n";
        }
        outFile.close();
        outFile.open("/Users/Rithesh/Documents/Learn C++/ASE/notes/Matlab_ASE/inputSine2.txt");
        for( int sample=0; sample<1024; sample++) {
            outFile<<pfInput2[sample]<<"\n";
        }
        outFile.close();
        
        outFile.open("/Users/Rithesh/Documents/Learn C++/ASE/notes/Matlab_ASE/specMul.txt");
        for( int sample=0; sample<1024; sample++) {
            outFile<<pfMulOut[sample]<<"\n";
        }
        outFile.close();
        
        
        m_pcFft->doFft(mySpectrum, pfInput);
        m_pcFft->doInvFft(pfOutput, mySpectrum);
        
        CHECK_ARRAY_CLOSE(pfOutput, pfInput, 1024, .999f);

    }
}

#endif //WITH_TESTS
