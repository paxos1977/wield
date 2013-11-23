#pragma once
#include <wield/MessageBase.h>

namespace wield {

    template<class ProcessingFunctor, class QueueType>
    class QueueBase final
    {
    public:
        using SizeType = typename QueueType::size_type;
        using MessagePtr = typename MessageBase<ProcessingFunctor>::smartptr;
        
        template<typename... Args>
        QueueBase(Args&&... arg)
            : queue_(std::forward<Args>(arg)...)
        {
        }

        inline void push(const MessagePtr& value)
        {
            queue_.push(value);
        }

        inline bool tryPop(MessagePtr& value)
        {
            return queue_.try_pop(value);
        }
        
        inline SizeType unsafeSize(void) const
        {
            return queue_.unsafe_size();
        }

    private:
        QueueBase(const QueueBase&) = delete;
        QueueBase& operator=(const QueueBase&) = delete;

    private:
        QueueType queue_;
    };
}
