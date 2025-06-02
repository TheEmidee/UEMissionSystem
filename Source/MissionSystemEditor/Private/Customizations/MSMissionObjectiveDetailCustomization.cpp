#include "Customizations/MSMissionObjectiveDetailCustomization.h"

#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "MSMissionObjective.h"
#include "Widgets/Input/SButton.h"

TSharedRef< IDetailCustomization > FMsMissionObjectiveDetailCustomization::MakeInstance()
{
    return MakeShareable( new FMsMissionObjectiveDetailCustomization );
}

void FMsMissionObjectiveDetailCustomization::CustomizeDetails( IDetailLayoutBuilder & detail_layout )
{
    TArray< TWeakObjectPtr< UObject > > objects;
    detail_layout.GetObjectsBeingCustomized( objects );

    if ( objects.Num() != 1 )
    {
        return;
    }

    if ( auto * objective = Cast< UMSMissionObjective >( objects[ 0 ] ) )
    {
        auto & my_category = detail_layout.EditCategory( "Mission System", FText::GetEmpty(), ECategoryPriority::Important );
        my_category.AddCustomRow( FText::GetEmpty() )
            [ SNew( SButton )
                    .Text( FText::FromString( "Regenerate Id" ) )
                    .OnClicked_Lambda( [ objective ]() {
                        objective->GenerateGuidIfNeeded( true );
                        return FReply::Handled();
                    } ) ];
    }
}