#ifndef ACTIONS_H
#define ACTIONS_H

#include "context.h"

class GenerateUUIDAction: public ContextAction
{
    void exec(char);
};

class CreateImagesCacheAction: public ContextAction
{
    void exec(char);
};

class StartDemographicDataAction: public ContextAction
{
    void exec(char);
};

class CloseDemographicDataAction: public ContextAction
{
    void exec(char);
};


class SaveImagesAction: public ContextAction
{
    void exec(char key);
};

class EnableNewDataCollectionAction: public ContextAction
{
    void exec(char);
};

class SetImageTypeAction: public ContextAction
{
    void exec(char key);
};

class SetDemograpichInfoAction: public ContextAction
{
    void exec(char key);
};
#endif // ACTIONS_H
