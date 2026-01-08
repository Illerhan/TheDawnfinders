// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SlateIMWidgetBase.h"
#include "SlateIM.h"

class THEDAWNFINDERS_API FDebugWindow : public FSlateIMWindowBase
{
public:
	FDebugWindow();
protected:
	virtual void DrawWindow(float DeltaTime) override;

private:

};
