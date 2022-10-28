#pragma once

#include "../Property.h"

#include "Core/Class.h"

class FStructProperty : public FProperty
{
public:

	friend class FPropertyFactory;

private:

	UStructPtr Struct;
};