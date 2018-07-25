#ifndef ACTIONS_H
#define ACTIONS_H

#include "context.h"

class GenerateUUIDAction: public ContextAction
{
    void exec();
};

class CreateImagesCacheAction: public ContextAction
{
    void exec();
};

class SaveImagesAction: public ContextAction
{
    void exec();
};

class SaveTestImagesAction: public ContextAction
{
    void exec();
};

class EnableNewDataCollectionAction: public ContextAction
{
    void exec();
};
#endif // ACTIONS_H
