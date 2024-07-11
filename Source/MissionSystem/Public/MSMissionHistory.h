#pragma once

#include <CoreMinimal.h>

#include "MSMissionHistory.generated.h"

class UMSMissionObjective;
class UMSMissionData;

enum class EMSState : uint8
{
    Active,
    Cancelled,
    Complete
};

USTRUCT( BlueprintType )
struct MISSIONSYSTEM_API FMSMissionHistory
{
    GENERATED_USTRUCT_BODY()

public:
    bool IsMissionActive( const UMSMissionData * mission_data ) const;
    bool IsMissionCancelled( const UMSMissionData * mission_data ) const;
    bool IsMissionComplete( const UMSMissionData * mission_data ) const;
    bool AddActiveMission( const UMSMissionData * mission_data );
    bool SetMissionComplete( const UMSMissionData * mission_data, bool was_cancelled );

    bool IsObjectiveActive( const TSubclassOf< UMSMissionObjective > & mission_objective_class ) const;
    bool IsObjectiveCancelled( const TSubclassOf< UMSMissionObjective > & mission_objective_class ) const;
    bool IsObjectiveComplete( const TSubclassOf< UMSMissionObjective > & mission_objective_class ) const;
    bool AddActiveObjective( const TSubclassOf< UMSMissionObjective > & mission_objective_class );
    bool SetObjectiveComplete( const TSubclassOf< UMSMissionObjective > & mission_objective_class, bool was_cancelled );

    friend FArchive & operator<<( FArchive & archive, FMSMissionHistory & mission_history );

private:
    bool DoesMissionHasState( const UMSMissionData * mission_data, EMSState state ) const;
    bool DoesObjectiveHasState( const TSubclassOf< UMSMissionObjective > & mission_objective_class, EMSState state ) const;

    TMap< FGuid, EMSState > MissionStates;
    TMap< FGuid, EMSState > ObjectiveStates;
};