#pragma once

#include <wield/logging/Log.hpp>
#include <wield/platform/thread>
#include <wield/platform/list>

#include <algorithm>
#include <atomic>
#include <forward_list>
#include <utility>

namespace wield {

    template<class SchedulingPolicy>
    struct PolicyIsOwnedByScheduler
    {
        template<typename... Args>
        PolicyIsOwnedByScheduler(Args&&... args)
            : schedulingPolicy_(std::forward<Args>(args)...)
        {
        }

        SchedulingPolicy schedulingPolicy_;
    };

    template<class SchedulingPolicy>
    struct SchedulerHasReferenceToPolicy
    {
        template<typename... Args>
        SchedulerHasReferenceToPolicy(SchedulingPolicy& policy, Args&&...)
            : schedulingPolicy_(policy)
        {
        }

        SchedulingPolicy& schedulingPolicy_;
    };

    template<class SchedulingPolicy, bool SchedulerOwnsPolicy>
    struct PolicyHolder : public PolicyIsOwnedByScheduler<SchedulingPolicy>
    {
        template<typename... Args>
        PolicyHolder(Args&&... args)
            : PolicyIsOwnedByScheduler<SchedulingPolicy>(std::forward<Args>(args)...)
        {
        }
    };

    template<class SchedulingPolicy>
    struct PolicyHolder<SchedulingPolicy, false>
        : public SchedulerHasReferenceToPolicy<SchedulingPolicy>
    {
        template<typename... Args>
        PolicyHolder(Args&&... args)
            : SchedulerHasReferenceToPolicy<SchedulingPolicy>(std::forward<Args>(args)...)
        {
        }
    };


    template<class SchedulingPolicy, bool SchedulerOwnsPolicy = true>
    class SchedulerBase : public PolicyHolder<SchedulingPolicy, SchedulerOwnsPolicy>
    {
    public:
        
        template<typename... Args>
        SchedulerBase(Args&&... args)
            : PolicyHolder<SchedulingPolicy, SchedulerOwnsPolicy>(std::forward<Args>(args)...)
            , done_(false)
        {
        }
        
        ~SchedulerBase()
        {
        }

        void start(void)
        {
            auto processLambda = [this](const std::size_t thread_id)
            {
                tryProcess(thread_id);
            };
            
            std::size_t numberOfThreads = this->schedulingPolicy_.numberOfThreads();
            for(std::size_t t = 0; t < numberOfThreads; ++t)
            {
                threads_.emplace_front(processLambda, t);

                // ARG: TODO: forward the thread to an affinity policy.
            }
        }

        inline void join(void)
        {
            for(auto& t : threads_)
            {
                t.join();
            }
        }

        inline void stop(void)
        {
            done_.store(true, std::memory_order_release);
        }

    private:
        SchedulerBase(const SchedulerBase&) = delete;
        SchedulerBase& operator=(const SchedulerBase&) = delete;

        inline void tryProcess(const std::size_t thread_id)
        {
            try
            {
                while(!done())
                {
                    process(thread_id);
                }
            }
            catch (const std::exception& e)
            {
                logging::Log::Error(std::string("Scheduler: an exception occurred: ") + e.what());
            }
        }

        inline bool done(void) const
        {
            return done_.load(std::memory_order_relaxed);
        }

        inline void process(const std::size_t thread_id)
        {
            typename SchedulingPolicy::StageType& stage = this->schedulingPolicy_.nextStage(thread_id);
            typename SchedulingPolicy::PollingInformation pollingInfo(thread_id, stage.name());
            
            this->schedulingPolicy_.batchStart(pollingInfo);
            
            do
            {
                const bool messageProcessed = stage.process();
                pollingInfo.incrementMessageCount(messageProcessed);
                
            } while(this->schedulingPolicy_.continueProcessing(pollingInfo));
            
            this->schedulingPolicy_.batchEnd(pollingInfo);
        }

    private:
        std::forward_list<std::thread> threads_;
        std::atomic_bool done_;
    };
}
