#ifndef FRAMECOUNTER_H
#define FRAMECOUNTER_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

/*
 *
 * Usage:
 * 1. Construct an instance of FrameCounter
 * 2. Call FrameCounter::countFrame() in the OpenGL display function.
 * 3. Call FrameCounter::framesPerSecond() to get the refresh rate.
 */

class FrameCounter
{
public:
   FrameCounter() : prevTime(0), frameCount(0), frameRate(0) {}
   void countFrame()
   {
      frameCount++;
      DWORD timeNow = timeGetTime(); // milliseconds
      if (timeNow - prevTime > 1000)
      {
         frameRate = (1000.0 * frameCount) / (timeNow - prevTime);
         frameCount = 0;
         prevTime = timeNow;
      }
   }
   double framesPerSecond() { return frameRate; }
private:
   DWORD prevTime;
   DWORD frameCount;
   double frameRate;
};

#endif