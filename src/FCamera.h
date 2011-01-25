#ifndef CAMERA_THREAD_H
#define CAMERA_THREAD_H

#define FRAME_WIDTH 1296
#define FRAME_HEIGHT 984

#define DAY_START_HOUR 05
#define DAY_START_MINUTE 0
#define DAY_END_HOUR 19
#define DAY_END_MINUTE 0

#define END_YEAR 2011
#define END_MONTH 01
#define END_DAY_OF_MONTH 05

#define START_YEAR 2011
#define START_MONTH 0
#define START_DAY_OF_MONTH 29

#define MILLISECONDS 1000
#define MICROSECONDS 1000*MILLISECONDS
#define NANOSECONDS 1000*MICROSECONDS

#define DAY_INTERVAL 1
#define NIGHT_INTERVAL 10

#define MAXIMUM_EXPOSURE_PERCENTAGE 0.25
#define MAXIMUM_GAIN 4.0f

// sleep time in nanoseconds
// sleep for 1 ms
#define SLEEP_TIME 1000*1000

#define MAXIMUM_DAY_EXPOSURE DAY_INTERVAL*MAXIMUM_EXPOSURE_PERCENTAGE*NANOSECONDS
#define MAXIMUM_NIGHT_EXPOSURE NIGHT_INTERVAL*MAXIMUM_EXPOSURE_PERCENTAGE*NANOSECONDS
#define DATE_FORMAT "%Y-%m-%d %H:%M:%S"

#include <stdio.h>
#include <ctime>
#include <time.h>
#include <stdarg.h>

#include <FCam/N900.h>

enum LogLevel {Trace, Debug, Error};

/**
 * A very custom time-lapse application that takes pictures at a specified rate in the specified time period.
 */
class FCamera {
  public:
    FCamera() {
        sensor.attach(&lens);
        setFocus();

        shot.exposure = MAXIMUM_DAY_EXPOSURE;
        shot.gain = MAXIMUM_GAIN;
        shot.image = FCam::Image(FRAME_WIDTH, FRAME_HEIGHT, FCam::UYVY, FCam::Image::AutoAllocate);

        shot.histogram.enabled = true;
        shot.histogram.region = FCam::Rect(0,0,FRAME_WIDTH,FRAME_HEIGHT);

        previousTime = 0;
        currentTime = 0;
    }

    /**
     * Take pictures.
     */
    void takePictures();

    /**
     * Get the interval for the current time of day.
     */
    int getInterval();
    /**
     * Should we continue taking pictures or are we outside the date range?
     */
    bool withinDateRange();
    /**
     * Cleanup when done, stop the sensor, make sure all files are saved.
     */
    void exit();

    /**
     * Get the current log level.
     */
    LogLevel getLogLevel(){return(logLevel);}
    /**
     * Set the current log level.
     */
    void setLogLevel(LogLevel logLevel){this->logLevel = logLevel;}
    
  protected:
        /**
         * Get the maximum exposure time (varies depending on the time of day, at night, we are assuming we're not moving so we can use a longer exposure).
         */
        long getMaximumExposureTime();
        /**
         * Adjust the exposure and white balance.
         */
        void expose();
        /**
         * Should we continue taking pictures, ie. are we within the time frame?
         */
        bool keepTakingPictures();
        /**
         * Should we take a picture this time around or adjust the exposure?
         */
        bool isTakePicture();
        /**
         * Adjust the exposure.
         */
        void adjust();
        /**
         * Take a picture.
         */
        void takePicture();
        /**
         * Automatically sleep in between taking pictures / adjusting the exposure.
         */
        void sleep();

        /**
         * Determines if it is day/night, used to determine the frame interval and other parameters.
         */
	bool isDay();
        /**
         * Save a picture asynchronously.
         */
        void save();

        /**
         * Update the parameters for the current frame.
         */
        void update();
        /**
         * Update the time processing for this frame began.
         */
        void updateStartTime();
        /**
         * Update the time of day (day or night).
         */
        void updateTimeOfDay();
        /**
         * Update the interval between capturing frames.
         */
        void updateInterval();
	
	/**
	 * Set the focus to infinity.
	 */
	void setFocus()
	{
		lens.setFocus(lens.farFocus(), lens.maxFocusSpeed());
		while(lens.focusChanging()){}

                log(Trace, this, "set focus to infinity.");
	}

        void sleep(timespec sleepTime, timespec remainder)
        {
            if(sleepTime.tv_sec < 0 || sleepTime.tv_nsec < 0)
            {
                log(Error, this, "passed a negative sleep time:%d:%f", sleepTime.tv_sec, sleepTime.tv_nsec);
                return;
            }

            log(Trace, this, "sleeping for:%d-%ld", sleepTime.tv_sec, sleepTime.tv_nsec);
            if(nanosleep(&sleepTime, &remainder) == -1)
            {
                log(Error, this, "did not finish sleep.");
                if(&remainder != NULL && (remainder.tv_sec > 0 || remainder.tv_nsec > 0))
                {
                    log(Debug, this, "remainder:%d-%ld", remainder.tv_sec, remainder.tv_nsec);
                    sleep(remainder, remainder);
                }
            }
        }

        static bool withinDateRange(tm* timeInfo)
        {
            int currentYear, currentMonth, currentDayOfMonth;

            currentYear = timeInfo->tm_year;
            currentMonth = timeInfo->tm_mon;
            currentDayOfMonth = timeInfo->tm_mday;

            if(currentYear > END_YEAR)
                return(false);
            if(currentYear == END_YEAR)
            {
                if(currentMonth > END_MONTH)
                    return(false);
                if(currentMonth == END_MONTH)
                    return(currentDayOfMonth <= END_DAY_OF_MONTH);
            }

            return(true);
        }

        static bool isDay(tm* timeInfo)
        {
            int currentHour, currentMinute;
            currentHour = timeInfo->tm_hour;
            currentMinute = timeInfo->tm_min;

            if(currentHour < DAY_START_HOUR)
                    return(false);
            if(currentHour > DAY_END_HOUR)
                    return(false);

            if(currentHour == DAY_START_HOUR &&
                    currentMinute < DAY_START_MINUTE)
                    return(false);

            if(currentHour == DAY_END_HOUR &&
                    currentMinute > DAY_END_MINUTE)
                    return(false);

            return(true);
        }

        static void log(const LogLevel logLevel, const FCamera* fCamera, const char* message, ...)
        {
            //if(logLevel >= fCamera->logLevel)
            {
                va_list args;

                char dateStamp[32];
                strftime(dateStamp, sizeof(dateStamp), DATE_FORMAT, getCurrentTime());

                printf(dateStamp);
                printf(" - ");

                va_start(args, message);
                    vfprintf(stdout, message, args);
                va_end(args);
                printf("\n");
            }
        }

        static tm* getCurrentTime()
        {
            struct tm* timeInfo_;
            time_t currentTime;

            time(&currentTime);
            timeInfo_ = localtime(&currentTime);

            return(timeInfo_);
        }

  private:
        // day or night
        bool day;
        // interval between capturing frames
        int interval;
        // did we just switch from day to night?
        bool stateChange;

        // current time.
        time_t currentTime;
        // last time a frame was captured.
        time_t previousTime;

        // the current log level.
        LogLevel logLevel;

        // the current time info.
        struct tm* timeInfo;
	
        FCam::N900::Sensor sensor;
        FCam::N900::Lens lens;
	
        FCam::Shot shot;
        FCam::Frame frame;
};

#endif
