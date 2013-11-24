#include "platform/UnitTestSupport.h"
#include "test/Traits.h"
#include "test/ProcessingFunctor.h"
#include "test/Message.h"

namespace {

    using namespace test;
    using Dispatcher = Traits::Dispatcher;
    using Queue = Traits::Queue;
    using Stage = Traits::Stage;
    using Message = Traits::Message;
    
    TEST(verifyStageInstantiation)
    {
        Dispatcher d;
        Queue q;
        ProcessingFunctor f;

        Stage s(Stages::Stage1, d, q, f);
        CHECK(Stages::Stage1 == s.name());
    }
    
    TEST(verifyStageProcessWorks)
    {
        Dispatcher d;
        Queue q;
        ProcessingFunctor f;
        Message::smartptr m = new TestMessage();
        
        Stage s(Stages::Stage1, d, q, f);
        s.push(m);

        CHECK(s.process());
        CHECK(f.message1Called_);
    }

    TEST(verifyStageProcessingForMultipleMessages)
    {
        Dispatcher d;
        Queue q;
        ProcessingFunctor f;
        Message::smartptr m = new TestMessage();

        Stage s(Stages::Stage1, d, q, f);
        s.push(m);
        s.push(m);
        
        CHECK(s.process());
        CHECK(s.process());
        CHECK(!s.process());
    }
}