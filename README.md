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

When the objective needs to be set as done, you must call the function `CompleteObjective`. The system will automatically start the next objective of the mission. If all objectives have been completed, the mission becomes itself complete. 

### Actions

Actions allows you to execute code before or after a mission or an objective start or end. They are great for example to activate a trigger which is needed by an objective for example. Or to play audio cues after a mission is complete.

You create actions by creating blueprints of type `UMSMissionAction`.

You need to implement the `Execute` function in the blueprint and do whatever the action needs to do. When the action is over, you need to call `FinishExecute`.

Note that all actions must be finished before going to the next step. This means that all start actions of an objective must must be finished before the objective `Execute` function is called. Or that all end actions of a mission must be finished before the mission is effectively completed.

## Run unit tests

Open the `Session Frontend` from the menu `Developer Tools`. Then select the tab `Automation`. All tests are under the category `MissionSystem`.