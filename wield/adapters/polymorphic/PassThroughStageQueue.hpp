#pragma once
#include <wield/adapters/polymorphic/QueueInterface.hpp>
#include <wield/MessageBase.hpp>

namespace wield { namespace adapters { namespace polymorphic {
  
    // A pass-through stage is a stage that does not queue a message
    // but processes it immediately upon dispatch.
    //
    // This can be used to easily combine stages into aggregate
    // stages without having to rework the application logic.
    //
    // This is convenient while experimenting with work breakout
    // during development and performance testing.
    //
    // I hope to eventually provide tools for doing pass-through
    // stage combination at compile time (without requiring client
    // code to introduce virtual function calls for queue functionality)
    // but that is a long way off I think.
    template<class ProcessingFunctor, class MessagePtr>
    class PassThroughStageQueue : public QueueInterface<MessagePtr>
    {
    public:
        PassThroughStageQueue(ProcessingFunctor& pf);
        
        // called in the same thread as the stage invoking dispatch to
        // the stage owning this queue (the previous stage in the
        // stage graph).
        void push(const MessagePtr& message) override;
        
        bool try_pop(MessagePtr&) override { return false; }
        std::size_t unsafe_size(void) const override { return 0; }

    private:
        ProcessingFunctor& processingFunctor_;
    };


    template<class ProcessingFunctor, class MessagePtr>
    PassThroughStageQueue<ProcessingFunctor, MessagePtr>::PassThroughStageQueue(ProcessingFunctor& pf)
        : processingFunctor_(pf)
    {
    }

    template<class ProcessingFunctor, class MessagePtr>
    void PassThroughStageQueue<ProcessingFunctor, MessagePtr>::push(const MessagePtr& message)
    {
        // process the message immediately.
        message->processWith(processingFunctor_);

        // the dispatcher increments the reference count before push'ing
        // we have to decrement it here to ensure memory is cleaned up.
        message->decrementReferenceCount();
    }

}}}
