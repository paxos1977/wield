#pragma once

#include <algorithm>
#include <atomic>
#include <thread>
#include <list>

namespace wield {

    template<typename SchedulingPolicy>
    class SchedulerBase final
    {
    public:
        SchedulerBase()
            : done_(false)
        {
        }

        ~SchedulerBase()
        {
        }

        void start(size_t numberOfThreads = std::thread::hardware_concurrency() )
        {
            auto processLambda = [this]()
            {
                try_process();
            };

            for(size_t t = 0; t < numberOfThreads; ++t)
            {
                threads_.emplace_back(std::thread(processLambda));
            }

            waitForThreads();
        }

        inline void stop(void)
        {
            done_.store(true, std::memory_order_release);
        }

    private:
        inline void try_process(void) const
        {
            try
            {
                while(!done())
                {
                    process();
                }
            }
            catch (const std::exception&)
            {
                // TODO: log the exception
            }
        }

        inline bool done(void) const
        {
            return done_.load(std::memory_order_acquire);
        }

        inline void process(void) const
        {
            size_t batchCount = 0;
        }

        inline void waitForThreads(void)
        {
            std::for_each(threads_.begin(), threads_.end(), [](std::thread& t)
            {
                t.join();
            });
        }

    private:
        std::atomic<bool> done_;
        std::list<std::thread> threads_;
    };
}