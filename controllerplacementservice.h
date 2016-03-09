#include "network.h"
#include "controllerplacementalgorothm.h"
#include "exceptions.h"

#ifndef CONTROLLERPLACEMENTSERVICE_H
#define CONTROLLERPLACEMENTSERVICE_H


class ControllerPlacementService
{
public:
	ControllerPlacementService();
private:
	Network network;
	ControllerPlacementAlgorothm algorithm;
};

#endif // CONTROLLERPLACEMENTSERVICE_H
