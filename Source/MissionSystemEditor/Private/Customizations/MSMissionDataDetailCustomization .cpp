#include "Customizations/MSMissionDataDetailCustomization.h"

#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "MSMissionData.h"
#include "Widgets/Input/SButton.h"

TSharedRef< IDetailCustomization > FMsMissionDataDetailCustomization::MakeInstance()
{
    return MakeShareable( new FMsMissionDataDetailCustomization );
}

void FMsMissionDataDetailCustomization::CustomizeDetails( IDetailLayoutBuilder & detail_layout )
{
    TArray< TWeakObjectPtr< UObject > > objects;
    detail_layout.GetObjectsBeingCustomized( objects );

    if ( objects.Num() != 1 )
    {
        return;
    }

    if ( auto * mission_data = Cast< UMSMissionData >( objects[ 0 ] ) )
    {
        auto & my_category = detail_layout.EditCategory( "Mission System", FText::GetEmpty(), ECategoryPriority::Important );
        my_category.AddCustomRow( FText::GetEmpty() )
            [ SNew( SButton )
                    .Text( FText::FromString( "Regenerate Id" ) )
                    .OnClicked_Lambda( [ mission_data ]() {
                        mission_data->GenerateGuidIfNeeded( true );
                        return FReply::Handled();
                    } ) ];
    }
}