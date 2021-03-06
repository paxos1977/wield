#pragma once
#include <wield/schedulers/utils/MessageCount.hpp>
#include <wield/schedulers/utils/NumberOfThreads.hpp>
#include <wield/schedulers/utils/ThreadAssignments.hpp>

#include <cstdint>

namespace wield { namespace schedulers { namespace color_minus {

    // This scheduling policy implements the Color-
    // scheduling algorithm (Color minus the queue).
    //
    // As events are enqueued with a stage, a
    // monotonically increasing counter is incremented.
    //
    // Threads needing work, walk over the stats array
    // looking for the stage with the most work,
    // which has fewer than the maximum allowed
    // threads visiting and attempts to visit
    // that stage.
    template<class DispatcherType, class PollingPolicy>
    class ColorMinus : public PollingPolicy
    {
    public:

        using Dispatcher = DispatcherType;
        using StageType = typename Dispatcher::StageType;
        using StageEnumType = typename Dispatcher::StageEnumType;
        using MessageCount = schedulers::utils::MessageCount<StageEnumType>;

        using ThreadAssignments = utils::ThreadAssignments<StageEnumType>;
        using MaxConcurrencyContainer = typename ThreadAssignments::MaxConcurrencyContainer;

        // This constructor assumes a maximum concurrency of 1 thread per stage.
        template<typename... Args>
        ColorMinus(Dispatcher& dispatcher, MessageCount& stats, Args&&... args);

        // This constructor assumes a maximum concurrency of 1 thread per stage,
		// the maximum number of threads running is determined by @maxNumberOfThreads
        template<typename... Args>
        ColorMinus(Dispatcher& dispatcher, MessageCount& stats, const std::size_t maxNumberOfThreads, Args&&... args);

        // This constructor takes a concurrency map describing the maximum allowed concurrency
		// at each stage, and this is used to determine the maximum number of threads to run. 
        template<typename... Args>
        ColorMinus(Dispatcher& dispatcher, MessageCount& stats, MaxConcurrencyContainer& maxConcurrency, Args&&... args);

        // This constructor takes a concurrency map describing the maximum allowed concurrency
		// at each stage.
		// @maxNumberOfThreads determines the maximum number of threads to run.
        template<typename... Args>
        ColorMinus(Dispatcher& dispatcher, MessageCount& stats, MaxConcurrencyContainer& maxConcurrency, const std::size_t maxNumberOfThreads, Args&&... args);

        // number of threads the scheduler should create.
        std::size_t numberOfThreads() const;

        // assign the next stage to visit.
        StageType& nextStage(const std::size_t threadId);

    private:

        // get the stage that has the most work.
        StageEnumType dequeNextStage();

    private:
        Dispatcher& dispatcher_;
        MessageCount& stats_;

        ThreadAssignments threadAssignments_;
    };
    
    
    template<class DispatcherType, class PollingPolicy>
    template<typename... Args>
    ColorMinus<DispatcherType, PollingPolicy>::ColorMinus(Dispatcher& dispatcher, MessageCount& stats, Args&&... args)
        : PollingPolicy(std::forward<Args>(args)...)
        , dispatcher_(dispatcher)
        , stats_(stats)
    {
    }

    template<class DispatcherType, class PollingPolicy>
    template<typename... Args>
    ColorMinus<DispatcherType, PollingPolicy>::ColorMinus(Dispatcher& dispatcher, MessageCount& stats, const std::size_t maxNumberOfThreads, Args&&... args)
        : PollingPolicy(std::forward<Args>(args)...)
        , dispatcher_(dispatcher)
        , stats_(stats)
        , threadAssignments_(maxNumberOfThreads)
    {
    }

    template<class DispatcherType, class PollingPolicy>
    template<typename... Args>
    ColorMinus<DispatcherType, PollingPolicy>::ColorMinus(Dispatcher& dispatcher, MessageCount& stats, MaxConcurrencyContainer& maxConcurrency, Args&&... args)
        : PollingPolicy(std::forward<Args>(args)...)
        , dispatcher_(dispatcher)
        , stats_(stats)
        , threadAssignments_(maxConcurrency)
    {
    }

    template<class DispatcherType, class PollingPolicy>
    template<typename... Args>
    ColorMinus<DispatcherType, PollingPolicy>::ColorMinus(Dispatcher& dispatcher, MessageCount& stats, MaxConcurrencyContainer& maxConcurrency, const std::size_t maxNumberOfThreads, Args&&... args)
        : PollingPolicy(std::forward<Args>(args)...)
        , dispatcher_(dispatcher)
        , stats_(stats)
        , threadAssignments_(maxConcurrency, maxNumberOfThreads)
    {
    }

    template<class DispatcherType, class PollingPolicy>
    inline
    std::size_t ColorMinus<DispatcherType, PollingPolicy>::numberOfThreads() const
    {
        return utils::numberOfThreads(threadAssignments_.size());
    }

    template<class DispatcherType, class PollingPolicy>
    typename DispatcherType::StageType& ColorMinus<DispatcherType, PollingPolicy>::nextStage(const std::size_t threadId)
    {
        threadAssignments_.removeCurrentAssignment(threadId);

        auto next = StageEnumType::NumberOfEntries;

        do {
            next = dequeNextStage();
            const bool success = threadAssignments_.tryAssign(threadId, next);

            stats_.updatePrevious(next);
            if(!success)
            {
                next = StageEnumType::NumberOfEntries;
            }

        // TODO: implement Idle policy.
        }
        while(next == StageEnumType::NumberOfEntries);

        return dispatcher_[next];
    }

    template<class DispatcherType, class PollingPolicy>
    inline
    typename DispatcherType::StageEnumType ColorMinus<DispatcherType, PollingPolicy>::dequeNextStage()
    {
        return stats_.highwaterStage();
    }
}}}
