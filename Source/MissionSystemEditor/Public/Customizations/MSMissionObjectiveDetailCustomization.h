#pragma once

#include "IDetailCustomization.h"

class FMsMissionObjectiveDetailCustomization : public IDetailCustomization
{
public:
    static TSharedRef< IDetailCustomization > MakeInstance();

    virtual void CustomizeDetails( IDetailLayoutBuilder & detail_layout ) override;
};