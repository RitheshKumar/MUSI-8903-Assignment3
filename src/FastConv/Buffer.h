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
#include "RingBuffer.h"

class CBuffer {
// The buffer class is able to block the input signal, save the blocked signal using ring buffer
public:
    CBuffer(int iBlockSize, int iBufferLength):
    m_iBufferLength(iBufferLength),
    m_pfInputStorageBuffer(0),
    m_pfOutputStorageBuffer(0)
    {
        assert(iBlockSize > 0);
        m_pfInputStorageBuffer = new CRingBuffer<float> (m_iBufferLength);
        m_pfOutputStorageBuffer = new CRingBuffer<float> (m_iBufferLength);
    }
    
    ~CBuffer() {
        delete [] m_pfInputStorageBuffer;
        delete [] m_pfOutputStorageBuffer;
        m_pfInputStorageBuffer = 0;
        m_pfOutputStorageBuffer = 0;
    }
    
    void reset() {
        memset(m_pfInputStorageBuffer, 0.f, sizeof(float)*m_iBufferLength);
        memset(m_pfOutputStorageBuffer, 0.f, sizeof(float)*m_iBufferLength);
        
    }
private:
    CBuffer(const CBuffer& that);
    
    int m_iBlockSize;
    int m_iBufferLength;
    CRingBuffer<float>* m_pfInputStorageBuffer;
    CRingBuffer<float>* m_pfOutputStorageBuffer;
    
    
};


#endif /* defined(__MUSI8903__Buffer__) */
