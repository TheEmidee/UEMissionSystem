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

USTRUCT()
struct MISSIONSYSTEM_API FMSMissionHistory
{
    GENERATED_USTRUCT_BODY()

public:
    const TArray< UMSMissionData * > & GetActiveMissionData() const;

    bool HasData() const;

    bool IsMissionActive( UMSMissionData * mission_data ) const;
    bool IsMissionCancelled( UMSMissionData * mission_data ) const;
    bool IsMissionComplete( UMSMissionData * mission_data ) const;
    bool IsMissionFinished( UMSMissionData * mission_data ) const;
    bool AddActiveMission(UMSMissionData* mission_data);
    bool SetMissionComplete( UMSMissionData * mission_data, bool was_cancelled );

    bool IsObjectiveActive( const TSubclassOf< UMSMissionObjective > & mission_objective_class ) const;
    bool IsObjectiveCancelled( const TSubclassOf< UMSMissionObjective > & mission_objective_class ) const;
    bool IsObjectiveComplete( const TSubclassOf< UMSMissionObjective > & mission_objective_class ) const;
    bool IsObjectiveFinished( const TSubclassOf< UMSMissionObjective > & mission_objective_class ) const;
    bool AddActiveObjective( const TSubclassOf< UMSMissionObjective > & mission_objective_class );
    bool SetObjectiveComplete( const TSubclassOf< UMSMissionObjective > & mission_objective_class, bool was_cancelled );

    friend FArchive & operator<<( FArchive & archive, FMSMissionHistory & mission_history );
    void Clear();

private:
    bool DoesMissionHasState( UMSMissionData * mission_data, EMSState state ) const;
    bool DoesObjectiveHasState( const TSubclassOf< UMSMissionObjective > & mission_objective_class, EMSState state ) const;

    UPROPERTY()
    TArray< UMSMissionData * > ActiveMissionsData;

    TMap< FGuid, EMSState > MissionStates;
    TMap< FGuid, EMSState > ObjectiveStates;
};

FORCEINLINE const TArray< UMSMissionData * > & FMSMissionHistory::GetActiveMissionData() const
{
    return ActiveMissionsData;
}