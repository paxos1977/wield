#pragma once

namespace test_color {

    // A different message base class than we normally use.
    // The purpose of this one is to help demonstrate the 
    // use of raw pointers in the queue.  
    template<class ProcessingFunctor>
    class TestRawPointerMessage 
    {
    public:
        using smartptr = TestRawPointerMessage*;

        virtual ~TestRawPointerMessage(){}
        virtual void processWith(ProcessingFunctor& process) = 0;
    };
}
