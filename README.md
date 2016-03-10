# MUSIC-8903-2016
Repo for the class Audio Software Engineering Spring 2016
# Assignment 3: Fast Convolution
# Group: Liang & Rithesh

1, Use the class interface based on FastConv.h und implement FastConv.cpp to implement a "normal" time domain convolution (FIR filter)


2, Implement a 'flushBuffer' function that will return the remaining result (reverb tail) at the end of the input signal. When client has multiple times of processing call, the current output which has the same length to the input buffer is the summation of current processing result and previous reverb tail. After last processing call, the flushBuffer function returns the reverb tail to the client.

3, Implement two tests (in Test_FastConv) for this implementation
	1, identity: generate a random IR (length 51 s), set it (init) and check the the output of a delayed (5 samples) impulse is the shifted IR.
    Test pass.

	2, block size: for an input signal of length 10000, run a similar test with a succession of different input/output block sizes (1, 13, 1023, 2048, 1, 17, 5000, 1897)
    Test pass

4, Implement partitioned Fast Convolution
Block-wised convolution algorithm. First, we block both the input and impulse response signal with the block size of power of 2 (greater than the block size that client passes in); then, we use FFT by communicating with the wrapper interface to get the frequency domain representation of the input and impulse reponse; Multiply them; Take inverse FFT; Split the result into output signal and reverb tail.

5, build program in release mode and use ctime, then compare the runtime performance between your time domain and frequency domain implementations