#pragma once
#include "Session.h"

bool beginDisplay();
void renderDisplay(const Session &session, bool targetInRange, uint32_t now);
