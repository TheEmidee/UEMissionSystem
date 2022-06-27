#include "MSMissionTypes.h"

#include "MSLog.h"
#include "MSMission.h"
#include "MSMissionAction.h"
#include "MSMissionData.h"

#include <Templates/SubclassOf.h>

void FMSActionExecutor::Initialize( UObject & action_owner, const TArray< TSubclassOf< UMSMissionAction > > & action_classes, const TFunction< void() > callback )
{
    ActionClasses = action_classes;
    Callback = callback;

    FString owning_object_name;
    if ( const auto * owning_mission_data = Cast< UMSMission >( &action_owner ) )
    {
        if ( auto * mission_data = owning_mission_data->GetMissionData() )
        {
            owning_object_name = mission_data->GetName();
        }
    }
    else
    {
        owning_object_name = action_owner.GetName();
    }

    for ( const auto & action_class : action_classes )
    {
        if ( !ensureAlwaysMsgf( IsValid( action_class ), TEXT( "%s has an invalid Mission Action!" ), *owning_object_name ) )
        {
            continue;
        }

        auto * action = NewObject< UMSMissionAction >( &action_owner, action_class );
        check( action );

        InstancedActions.Add( action );
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
