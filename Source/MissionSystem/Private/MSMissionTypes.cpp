#include "MSMissionTypes.h"

#include "MSLog.h"
#include "MSMission.h"
#include "MSMissionAction.h"
#include "MSMissionData.h"

void FMSActionExecutor::Initialize( UObject * action_owner, const TArray< UMSMissionAction * > & action_classes, const TFunction< void() > callback )
{
    Outer = action_owner;
    Callback = callback;

    FString owning_object_name;
    if ( const auto * owning_mission_data = Cast< UMSMission >( action_owner ) )
    {
        if ( auto * mission_data = owning_mission_data->GetMissionData() )
        {
            owning_object_name = mission_data->GetName();
        }
    }
    else if ( IsValid( action_owner ) )
    {
        owning_object_name = action_owner->GetName();
    }

    InstancedActions = action_classes;

    for ( auto * action : InstancedActions )
    {
        action->Initialize( action_owner );
    }
}

void FMSActionExecutor::Execute()
{
    if ( InstancedActions.Num() == 0 )
    {
        TryExecuteCallback();
        return;
    }

    PendingActions.Append( InstancedActions );

    for ( auto index = PendingActions.Num() - 1; index >= 0; index-- )
    {
        auto * action = PendingActions[ index ];

        action->OnMissionActionComplete().AddRaw( this, &FMSActionExecutor::OnActionExecuted );

        UE_LOG( LogMissionSystem, Verbose, TEXT( "Execute action %s" ), *GetNameSafe( action ) );

        action->Execute();
    }
}

void FMSActionExecutor::OnActionExecuted( UMSMissionAction * action )
{
    action->OnMissionActionComplete().RemoveAll( this );
    ensureAlways( PendingActions.Remove( action ) > 0 );
    TryExecuteCallback();
}

void FMSActionExecutor::TryExecuteCallback()
{
    if ( PendingActions.Num() == 0 && Callback != nullptr )
    {
        Callback();
    }
}
