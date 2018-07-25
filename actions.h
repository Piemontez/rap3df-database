#ifndef ACTIONS_H
#define ACTIONS_H

#include "context.h"

class GenerateUUIDAction: public ContextAction
{
    void exec(char key);
};

class CreateImagesCacheAction: public ContextAction
{
    void exec(char key);
};

class SaveImagesAction: public ContextAction
{
    void exec(char key);
};

class SaveTestImagesAction: public ContextAction
{
    void exec(char key);
};

class EnableNewDataCollectionAction: public ContextAction
{
    void exec(char key);
};

class SetImageTypeAction: public ContextAction
{
    void exec(char key);
};
#endif // ACTIONS_H
