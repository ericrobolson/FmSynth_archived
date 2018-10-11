#include <cstddef>

#include "PositionSystem.h"
#include "EntityComponentManager.h"
#include "PositionComponent.h"
#include "MovementComponent.h"
#include <mutex>
#include "ThreadPool.h"

PositionSystem::PositionSystem() : BaseSystem()
{
    //ctor
}

PositionSystem::~PositionSystem()
{
    //dtor
}

void ProcessJob(ECS::EntityComponentManager &ecs, int entityIndex){
    ecs.Lock();

    MovementComponent& movementComponent = *ecs.GetComponent<MovementComponent>(entityIndex);
    PositionComponent& positionComponent = *ecs.GetComponent<PositionComponent>(entityIndex);

    if (movementComponent.ForwardSpeed != 0 || movementComponent.HorizontalSpeed != 0){
        // Convert the angle of movement to X and Y
        positionComponent.PositionX = positionComponent.PositionX + movementComponent.GetXDelta();
        positionComponent.PositionY = positionComponent.PositionY + movementComponent.GetYDelta();
    }

    ecs.Unlock();
}

bool PositionSystem::Process(ECS::EntityComponentManager &ecs){

    // ignore everything else for not

    ecs.Lock();
        std::vector<int> entities = ecs.Search<PositionComponent>();
        entities = ecs.SearchOn<MovementComponent>(entities);
    ecs.Unlock();

    std::vector<int>::iterator it;

    for (it = entities.begin(); it != entities.end(); ++it){
        int entityId = *it;

        ThreadPool::Instance().submit([&ecs, entityId](){
                                          ProcessJob(ecs, entityId);
                                      });
    }

    std::future<bool> isDone = ThreadPool::Instance().submit([](){

                                                        return true;
                                                        });
    return isDone.get();
}

