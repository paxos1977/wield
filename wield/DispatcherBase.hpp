#pragma once
#include <wield/CloneMessageTag.hpp>
#include <wield/DispatcherInterface.hpp>
#include <wield/Exceptions.hpp>

#include <array>
#include <cstddef>
#include <type_traits>

namespace wield {

    /* Dispatcher
    
       This is the dispatcher class, parameterized on your applications StageEnum type
       and Stage type. The dispatcher knows about all the stages in the system and is
       passed to stages so they can get messages from their processing functor to another
       stage in the system. Routing is done by StageEnum which defines the stage names.
    */
    template<typename StageEnum, class Stage>
    class DispatcherBase : public DispatcherInterface<StageEnum, Stage>
    {
    public:
        static_assert(std::is_enum<StageEnum>::value, "StageEnum parameter is not an enum type.");

        using StageType = Stage;
        using StageEnumType = StageEnum;
        
        DispatcherBase();

        // Register a <Stage> with the Dispatcher
        // @stageName is the name this stage will be referenced as
        // @stage is the pointer to the stage
        void registerStage(StageEnumType stageName, StageType* stage) override;

        // Send a message to a stage
        // @stageName the stage to dispatch the message to.
        // @message is the message to send
        void dispatch(StageEnumType stageName, typename StageType::MessageType& message);
        
        // Send a copy of a message to a stage
        // @stageName the stage to dispatch the message to.
        // @message the message te send
        // @clone a tag type for tag-dispatching this overloaded function
        template<class ConcreteMessageType>
        void dispatch(StageEnumType stageName, ConcreteMessageType& message, CloneMessageTagType);
        
        // Stage lookup function
        // @stageName is the name of the stage to get a reference to.
        //
        // @return a reference to the requested stage.
        StageType& operator[](StageEnumType stageName);

    private:
        DispatcherBase(const DispatcherBase&) = delete;
        DispatcherBase& operator=(const DispatcherBase&) = delete;
        
    private:
        std::array<StageType*, static_cast<std::size_t>(StageEnum::NumberOfEntries)> stages_;
    };
    
    
    template<typename StageEnum, class Stage>
    DispatcherBase<StageEnum, Stage>::DispatcherBase()
    {
        for(auto& stage : stages_)
        {
            stage = nullptr;
        }
    }

    template<typename StageEnum, class Stage>
    void DispatcherBase<StageEnum, Stage>::registerStage(StageEnumType stageName, StageType* stage)
    {
        if(nullptr != stages_[static_cast<std::size_t>(stageName)])
        {
            throw wield::DuplicateStageRegistrationException(); 
        }

        stages_[static_cast<std::size_t>(stageName)] = stage;
    }

    template<typename StageEnum, class Stage>
    inline
    void DispatcherBase<StageEnum, Stage>::dispatch(StageEnumType stageName, typename StageType::MessageType& message)
    {
        // increment the reference count so the message isn't deleted while
        // in the queue.
        message.incrementReferenceCount();

        stages_[static_cast<std::size_t>(stageName)]->push(&message);
    }

    template<typename StageEnum, class Stage>
    template<class ConcreteMessageType>
    inline
    void DispatcherBase<StageEnum, Stage>::dispatch(StageEnumType stageName, ConcreteMessageType& message, CloneMessageTagType)
    {
        static_assert(std::is_base_of<typename StageType::MessageType, ConcreteMessageType>::value, "ConcreteMessageType must be derived from Message.");
        
        typename StageType::MessageType::ptr clone = new ConcreteMessageType(message);
        clone->incrementReferenceCount();

        stages_[static_cast<std::size_t>(stageName)]->push(clone);
    }

    template<typename StageEnum, class Stage>
    inline
    Stage& DispatcherBase<StageEnum, Stage>::operator[](StageEnumType stageName)
    {
        return *stages_[static_cast<std::size_t>(stageName)];
    }
}
