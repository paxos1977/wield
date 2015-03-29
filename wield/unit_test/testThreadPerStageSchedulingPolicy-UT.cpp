#include "./platform/UnitTestSupport.hpp"
#include <wield/schedulers/ThreadPerStage.hpp>

#include "./test/Stages.hpp"
#include "./test/Traits.hpp"
#include "./test/ProcessingFunctor.hpp"

namespace {

    TEST(verifyInstantiationOfThreadPerStageScheduler)
    {
        using Dispatcher = test::Traits::Dispatcher;
        using Stage = test::Traits::Stage;

        Dispatcher dispatcher;

        using SchedulingPolicy =
            wield::schedulers::ThreadPerStage<test::Stages, Dispatcher, Stage>;

        SchedulingPolicy schedulingPolicy(dispatcher);
    }

    TEST(verifyNumberOfThreadsIsCorrect)
    {
        using Dispatcher = test::Traits::Dispatcher;
        using Stage = test::Traits::Stage;

        Dispatcher dispatcher;

        using SchedulingPolicy =
            wield::schedulers::ThreadPerStage<test::Stages, Dispatcher, Stage>;

        SchedulingPolicy schedulingPolicy(dispatcher);

        std::size_t expectedThreads = static_cast<std::size_t>(test::Stages::NumberOfEntries);
        CHECK_EQUAL(expectedThreads, schedulingPolicy.numberOfThreads());
    }

    TEST(verifyThreadsAreScheduledToCorrespondingStage)
    {
        using namespace test;

        using Dispatcher = Traits::Dispatcher;
        using Stage = Traits::Stage;
        using Queue = Traits::Queue;

        using SchedulingPolicy =
            wield::schedulers::ThreadPerStage<test::Stages, Dispatcher, Stage>;

        Dispatcher dispatcher;

        Queue q;
        ProcessingFunctor pf;

        Stage stage1(Stages::Stage1, dispatcher, q, pf);
        Stage stage2(Stages::Stage2, dispatcher, q, pf);
        Stage stage3(Stages::Stage3, dispatcher, q, pf);

        SchedulingPolicy schedulingPolicy(dispatcher);

        CHECK_EQUAL(&stage1, & schedulingPolicy.nextStage(0));
        CHECK_EQUAL(&stage2, & schedulingPolicy.nextStage(1));
        CHECK_EQUAL(&stage3, & schedulingPolicy.nextStage(2));
    }
}