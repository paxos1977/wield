#include "./platform/UnitTestSupport.hpp"
#include <wield/schedulers/RoundRobin.hpp>
#include <wield/polling_policies/ExhaustivePollingPolicy.hpp>

#include "./test/Stages.hpp"
#include "./test/Traits.hpp"
#include "./test/ProcessingFunctor.hpp"

namespace {

    struct RoundRobinFixture
    {
        RoundRobinFixture()
            : schedulingPolicy(dispatcher)
        {
        }

        using Dispatcher = test::Traits::Dispatcher;
        using Stage = test::Traits::Stage;
        using PollingPolicy = wield::polling_policies::ExhaustivePollingPolicy<test::Stages>;
        using SchedulingPolicy = wield::schedulers::RoundRobin<Dispatcher, PollingPolicy>;
        using Queue = test::Traits::Queue;

        Dispatcher dispatcher;
        SchedulingPolicy schedulingPolicy;
    };

    TEST_FIXTURE(RoundRobinFixture, verifyInstantiationOfRoundRobin)
    {
    }

    TEST_FIXTURE(RoundRobinFixture, verifyRoundRobinOrdering)
    {
        test::ProcessingFunctor pf;
        Queue q;

        Stage s1(test::Stages::Stage1, dispatcher, q, pf);
        Stage s2(test::Stages::Stage2, dispatcher, q, pf);
        Stage s3(test::Stages::Stage3, dispatcher, q, pf);

        CHECK_EQUAL(&s1, &schedulingPolicy.nextStage(0));
        CHECK_EQUAL(&s2, &schedulingPolicy.nextStage(0));
        CHECK_EQUAL(&s3, &schedulingPolicy.nextStage(0));
        CHECK_EQUAL(&s1, &schedulingPolicy.nextStage(0));
    }

    TEST_FIXTURE(RoundRobinFixture, verifyCanBeUsedInSchedulerBase)
    {
        using Scheduler = wield::SchedulerBase<SchedulingPolicy>;
        Scheduler scheduler(dispatcher);
    }
    
    struct RoundRobin2ThreadsFixture
    {
        RoundRobin2ThreadsFixture()
            : schedulingPolicy(dispatcher, SchedulingPolicy::MaxThreads(), std::size_t(2))
        {
        }

        using Dispatcher = test::Traits::Dispatcher;
        using Stage = test::Traits::Stage;
        using PollingPolicy = wield::polling_policies::ExhaustivePollingPolicy<test::Stages>;
        using SchedulingPolicy = wield::schedulers::RoundRobin<Dispatcher, PollingPolicy>;
        using Queue = test::Traits::Queue;

        Dispatcher dispatcher;
        SchedulingPolicy schedulingPolicy;
    };
    
    TEST_FIXTURE(RoundRobin2ThreadsFixture, verifyTwoThreadsAssignmentIsCorrect)
    {
        test::ProcessingFunctor pf;
        Queue q;

        Stage s1(test::Stages::Stage1, dispatcher, q, pf);
        Stage s2(test::Stages::Stage2, dispatcher, q, pf);
        Stage s3(test::Stages::Stage3, dispatcher, q, pf);

        CHECK_EQUAL(&s1, &schedulingPolicy.nextStage(0));
        CHECK_EQUAL(&s2, &schedulingPolicy.nextStage(1));   // s1 is busy, assign to s2
        CHECK_EQUAL(&s3, &schedulingPolicy.nextStage(0));   // s2 is busy, assign to s3
        
        CHECK_EQUAL(&s1, &schedulingPolicy.nextStage(0));
        CHECK_EQUAL(&s3, &schedulingPolicy.nextStage(0));   // s1 still busy, assign to s2
    }
}
