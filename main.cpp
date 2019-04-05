#include "context.h"
#include "actions.h"
#include "viewport.h"


int main(int argc, char **argv)
{
    Context* context = Context::instance();
    context->init(argc, argv);

    context->addAction('1', new GenerateUUIDAction, STEP_IMAGES_STEPS);
    context->addAction('2', new CreateImagesCacheAction, STEP_IMAGES_STEPS);
    context->addAction('3', new StartDemographicDataAction, STEP_IMAGES_STEPS);
    context->addAction('-', new CloseDemographicDataAction, STEP_DEMOGRAPHI);
    context->addAction('9', new SaveImagesAction, STEP_IMAGES_STEPS);
    context->addAction('0', new EnableNewDataCollectionAction, STEP_IMAGES_STEPS);

    context->addAction('f', new SetImageTypeAction, STEP_START | STEP_CACHE_IMAGE);
    context->addAction('u', new SetImageTypeAction, STEP_START | STEP_CACHE_IMAGE);
    context->addAction('d', new SetImageTypeAction, STEP_START | STEP_CACHE_IMAGE);
    context->addAction('l', new SetImageTypeAction, STEP_START | STEP_CACHE_IMAGE);
    context->addAction('r', new SetImageTypeAction, STEP_START | STEP_CACHE_IMAGE);
    context->addAction('b', new SetImageTypeAction, STEP_START | STEP_CACHE_IMAGE);

    context->addAction('*', new SetDemograpichInfoAction, STEP_DEMOGRAPHI);
    {
        int wind= context->initWindow("Controll");
        context->addViewport(wind, new InfoViewPort);
        {
            context->addViewport(wind, new PointCamViewPort);
            context->addViewport(wind, new TriangleCamViewPort(1));
        }
        context->addViewport(wind, new BoxCamViewPort);

        context->addViewport(wind, new BoxExtractViewPort);

        context->addViewport(wind, new LeftCamViewPort(1));
        context->addViewport(wind, new FrontCamViewPort(1));
        context->addViewport(wind, new RightCamViewPort(1));
    }
    {
        int wind= context->initWindow("RAP3DF");

        context->addViewport(wind, new InfoVoluntaryViewPort);
        new LeftCamViewPort(2);
        context->addViewport(wind, new LeftCamViewPort(2));
        context->addViewport(wind, new FrontCamViewPort(2));
        context->addViewport(wind, new FrontCamViewPort(3));
        context->addViewport(wind, new RightCamViewPort(2));
    }

    context->start();

    return 0;
}
