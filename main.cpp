#include "context.h"
#include "actions.h"
#include "viewport.h"


int main(int argc, char **argv)
{
    Context* context = Context::instance();
    context->init(argc, argv);

    context->addAction('1', new GenerateUUIDAction);
    context->addAction('2', new CreateImagesCacheAction);
    context->addAction('9', new SaveImagesAction);
    context->addAction('0', new EnableNewDataCollectionAction);

    context->addAction('f', new SetImageTypeAction);
    context->addAction('t', new SetImageTypeAction);
    context->addAction('d', new SetImageTypeAction);
    context->addAction('l', new SetImageTypeAction);
    context->addAction('r', new SetImageTypeAction);
    context->addAction('b', new SetImageTypeAction);

    {
        int wind= context->initWindow("Controll");
        context->addViewport(wind, new InfoViewPort);
        {
            context->addViewport(wind, new PointCamViewPort);
            context->addViewport(wind, new TriangleCamViewPort);
        }
        context->addViewport(wind, new BoxCamViewPort);

        context->addViewport(wind, new BoxExtractViewPort);

        context->addViewport(wind, new LeftCamViewPort(1));
        context->addViewport(wind, new FrontCamViewPort(1));
        context->addViewport(wind, new RightCamViewPort(1));
    }
    {
        int wind= context->initWindow("RAP3DF");

        new LeftCamViewPort(2);
        context->addViewport(wind, new LeftCamViewPort(2));
        context->addViewport(wind, new FrontCamViewPort(2));
        context->addViewport(wind, new RightCamViewPort(2));
    }

    context->start();

    return 0;
}
