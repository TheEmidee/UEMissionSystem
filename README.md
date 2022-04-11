# Unreal Engine - Mission System

This plug-in features a very barebone, yet versatile mission system to use in Unreal Engine.

It also provides numerous unit tests to validate the behavior of each class.

## Installation

You can use install this plugin as a submodule using the following command:

`git submodule add git@github.com:TheEmidee/UEMissionSystem.git Plugins/MissionSystem`

You can also download the ZIP file and manually extract the plugin in your `Plugins` directory.

## How to use

### MissionSystem

This is the entry point to the system. It allows you to start missions, check is a mission is active, or completed, or get the currently active missions.

### Missions

You create missions in the content browser by creating a new `DataAsset` of type `MSMissionData`.

For each mission, you can define actions to be executed when the mission starts, or when the mission ends. You can also define objectives to complete to set the mission as done. You can also define missions to start next automatically after the mission has been completed.

To start a mission, you need to call the `StartMission` function of the mission system.

### Objectives

You create objectives by creating blueprints of type `UMSMissionObjective`.

Like with missions, you can define actions to be executed when the objective starts, or when it ends.

You can override the function `Execute` to execute code when the objective starts.

When the objective needs to be set as done, you must call the function `CompleteObjective`. The system will automatically start the next objective of the mission. If all objectives have been completed, the mission itself becomes complete. 

You can implement the event `OnObjectiveEnded` in your objective blueprint to for example do some cleanup. This is useful if the objective gets cancelled somehow and you need to destroy actors that have been created in the `Execute` event.

The property `Tags` can be used to ignore objectives to be executed. For this, you need to use the console command `MissionSystem.IgnoreObjectivesWithTag`. You can pass any string parameters you want, they will be treated as individual tokens. The tags don't have to match 100%. If for example you have some mission objectives with a tag `Mission.Spawn.Wave` and you add a token using the command `MissionSystem.IgnoreObjectivesWithTag Spawn`, the objective will not be executed.

### Actions

Actions allows you to execute code before or after a mission or an objective start or end. They are great for example to activate a trigger which is needed by an objective for example. Or to play audio cues after a mission is complete.

You create actions by creating blueprints of type `UMSMissionAction`.

You need to implement the `Execute` function in the blueprint and do whatever the action needs to do. When the action is over, you need to call `FinishExecute`.

Note that all actions must be finished before going to the next step. This means that all start actions of an objective must be finished before the objective `Execute` function is called. Or that all end actions of a mission must be finished before the mission is effectively completed.

### Debug Commands

* `MissionSystem.SkipMission` will complete all active missions
* `MissionSystem.ListActiveMissions` will output in the log the list of active missions and their active objectives
* `MissionSystem.IgnoreObjectivesWithTag XXX YYY` will add all the parameters to a list of tokens to ignore objectives from being executed