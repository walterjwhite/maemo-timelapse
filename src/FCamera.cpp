#include <stdlib.h>
#include <sys/time.h>

#include "FCamera.h"

#define STORAGE_PATTERN "/home/user/MyDocs/DCIM/%Y-%m-%d_%H.%M.%S.jpeg"

int main(int argumentCount, char* argument[]) {

    LogLevel logLevel = Error;

    if(argumentCount == 1)
    {
        int logLevelInt = atoi(argument[0]);

        if(logLevelInt == 1)
            logLevel = Trace;
        else if(logLevelInt == 2)
            logLevel = Debug;
    }

    FCamera* fcamera = new FCamera();

    fcamera->setLogLevel(logLevel);
    fcamera->takePictures();
    fcamera->exit();
    return(0);
}

void FCamera::takePictures()
{
    while(keepTakingPictures())
    {
        if(!isTakePicture())
            adjust();
        else
            takePicture();

        FCam::Event e;
        while (FCam::getNextEvent(&e, FCam::Event::Error)) {
            log(Error, this, "Error: %s\n", e.description.c_str());
        }
    }
}

void FCamera::adjust()
{
    //if(!deleted)
        //delete frame;

    sensor.capture(shot);
    frame = sensor.getFrame();
    expose();

    //deleted = false;
}

bool FCamera::keepTakingPictures()
{
    update();
    return(withinDateRange(timeInfo));
}

void FCamera::update()
{
    log(Trace, this, "updating ...");
    updateStartTime();
    updateTimeOfDay();
    updateInterval();
}

void FCamera::updateStartTime()
{
    log(Trace, this, "updating current time");
    currentTime = time(NULL);
    timeInfo = localtime(&currentTime);
}

void FCamera::updateTimeOfDay()
{
    log(Trace, this, "updating time of day");
    bool currentDay = isDay(timeInfo);
    if(currentDay != day)
    {
        stateChange = true;
        log(Trace, this, "state change");
    }
    else
        stateChange = false;

    day = currentDay;
}

void FCamera::updateInterval()
{
    log(Trace, this, "updating interval");
    if(day)
        interval = DAY_INTERVAL;
    else
        interval = NIGHT_INTERVAL;

    log(Trace, this, "interval:%d:%d:%d", interval, DAY_INTERVAL, NIGHT_INTERVAL);
}

bool FCamera::isTakePicture()
{
    if(previousTime == 0)
        previousTime = currentTime;

    log(Trace, this, "determining if we should take a picture or not.");
    return(currentTime - previousTime >= interval);
}

void FCamera::takePicture()
{
    log(Debug, this, "taking picture");
    previousTime = currentTime;
    save();
}

void FCamera::sleep()
{
    timespec sleepTime = {0, SLEEP_TIME};
    timespec remainder = {0, 0};

    sleep(sleepTime, remainder);
}

void FCamera::expose()
{
    log(Trace, this, "Exposing picture:%d-%d", &shot, &frame);
    log(Trace, this, "frame properties:%d-%ld", frame.exposure(), frame.gain());

    // maximum ISO speed of: ISO 400
    // maximum exposure time of: 250 ms
    autoExpose(&shot, frame, MAXIMUM_GAIN, getMaximumExposureTime());
    autoWhiteBalance(&shot, frame);
}

long FCamera::getMaximumExposureTime()
{
    if(day)
        return(MAXIMUM_DAY_EXPOSURE);

    return(MAXIMUM_NIGHT_EXPOSURE);
}

void FCamera::exit()
{
    sensor.stop();
}

void FCamera::save()
{
    char path[64];
    strftime(path, sizeof(path), STORAGE_PATTERN, timeInfo);

    log(Trace, this, "saving file to:%s %d-%d\n", path, &shot, &frame);
    log(Trace, this, "image dimensions:%d-%d", shot.image.width(), shot.image.height());
    FCam::saveJPEG(frame, path);

    // free up memory
    //delete frame;

    //deleted = true;
}
