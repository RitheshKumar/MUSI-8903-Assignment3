//
//  Buffer.h
//  MUSI8903
//
//  Created by Music Technology on 3/5/16.
//
//

#ifndef __MUSI8903__Buffer__
#define __MUSI8903__Buffer__


#include <stdio.h>
#include <cmath>
#include <algorithm>
#include <iostream>
#include "RingBuffer.h"

class CBuffer {
// The buffer class is able to block the input signal, save the blocked signal using ring buffer
public:
    CBuffer(int iBlockSize, int iBufferLength, int inputLength):
    m_iBlockSize(iBlockSize),
    m_iNumCycles(0),
    m_iInputStorageLength(inputLength),
    m_iOutputStorageLength(iBufferLength),
    m_iCycleIdx(0),
    m_pfInputStorageBuffer(0),
    m_pfOutputStorageBuffer(0)
    {
        assert(iBlockSize > 0);
    }
    
    ~CBuffer() {
        delete m_pfInputStorageBuffer;
        delete m_pfOutputStorageBuffer;
        m_pfInputStorageBuffer = 0;
        m_pfOutputStorageBuffer = 0;
    }
    
    void init( int numCycles = 1 ) {
        m_iNumCycles = numCycles;
        
        if (m_iInputStorageLength != 0) {
//            delete m_pfInputStorageBuffer;
//            m_pfInputStorageBuffer = 0;
            m_pfInputStorageBuffer = new CRingBuffer<float> (m_iInputStorageLength*m_iNumCycles);
        } else {
            m_pfInputStorageBuffer = new CRingBuffer<float> (m_iInputStorageLength*m_iNumCycles);
        }

        if (m_pfOutputStorageBuffer != 0) {
//            delete m_pfOutputStorageBuffer;
//            m_pfOutputStorageBuffer = 0;
            m_pfOutputStorageBuffer = new CRingBuffer<float> (m_iOutputStorageLength);
            
        } else {
            m_pfOutputStorageBuffer = new CRingBuffer<float> (m_iOutputStorageLength);

        }

    }
    
    void reset() {
        m_pfInputStorageBuffer -> reset();
        m_pfOutputStorageBuffer -> reset();
        m_iOutputStorageLength = 0;
        m_iBlockSize    = 0;
    }
    
    void storeInput(float *pfInput, int iInputLength) {
        m_iCycleIdx++;
        for (int sample = 0; sample < iInputLength; sample++) {
            m_pfInputStorageBuffer->putPostInc(pfInput[sample]);
        }
        if( m_iCycleIdx == m_iNumCycles ) {
            m_pfInputStorageBuffer->resetIdx();
            m_iCycleIdx = 0;
        }
//        for (int sample = 0; sample<50; sample++) {
//            std::cout<<m_pfInputStorageBuffer->get(sample)<<",";
//        }std::cout<<std::endl;
    }
    
    void overLapAdd(float* pfInput, int iInputLength, int iIRLength) {
        assert(iInputLength < m_iOutputStorageLength);
        
        m_pfOutputStorageBuffer->addPostInc(pfInput, iInputLength);
        m_pfOutputStorageBuffer->add(&pfInput[iInputLength], iIRLength-1);
//        printOutStorageBuffer(25);//+iIRLength-1);
        
    }
    
    void getOutput(float* pfOutput, int iOutputLength) {
        for (int sample = 0; sample < iOutputLength; sample++) {
            pfOutput[sample] = m_pfOutputStorageBuffer->getPostInc();
        }
//        std::cout<<"Read_Idx: "<<m_pfOutputStorageBuffer->getReadIdx()<<std::endl;
    }
    
    void getTail(float* pfTail, int iTailLength, int numOfZeroToPad) {
//        std::cout<<"Tail:";
        for (int sample = 0; sample < iTailLength; sample++) {
//            pfTail[sample] = m_pfOutputStorageBuffer->getPostInc();
            pfTail[sample] = m_pfOutputStorageBuffer->get(sample - numOfZeroToPad);//numOfZeroToPad);
//            std::cout<<pfTail[sample]<<" , ";
        }
//        std::cout<<std::endl;
    }
    
    void printOutStorageBuffer (int printLen) {
        m_pfOutputStorageBuffer->printContent(printLen);
    }
    
    void getInputBlock (int blockNum, float *pfInput, int iInputLength) {
        
        int curReadIdx = m_pfInputStorageBuffer->getReadIdx();
        
        
        m_pfInputStorageBuffer->setReadIdx(blockNum * iInputLength);
        
//        std::cout<<"ReadIdx: "<<m_pfInputStorageBuffer->getReadIdx()<<std::endl;
        
        for (int sample = 0; sample < iInputLength; sample++) {
            pfInput[sample] = m_pfInputStorageBuffer->get(sample);
        }
        
        m_pfInputStorageBuffer->setReadIdx(curReadIdx);
        
//        std::cout<<"ReadIdx: "<<m_pfInputStorageBuffer->getReadIdx()<<std::endl;
    }
    
    
private:
    CBuffer(const CBuffer& that);
    
    int m_iBlockSize;
    int m_iNumCycles,
        m_iInputStorageLength,
        m_iOutputStorageLength,
        m_iCycleIdx;
    
    CRingBuffer<float>* m_pfInputStorageBuffer;
    CRingBuffer<float>* m_pfOutputStorageBuffer;
    
    
};


#endif /* defined(__MUSI8903__Buffer__) */
