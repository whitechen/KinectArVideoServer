#ifndef ARVIDEOEXTERNALSOURCE_H
#define ARVIDEOEXTERNALSOURCE_H

#include "Aria.h"
#include "ArVideoInterface.h"

/** An easy way to supply image data to ArVideoServer from an external source
 * such as image frames generated by or stored in your own code, or read
 * asynchronously from some source.  You can supply a pointer to read RGB image data from, or you can copy 
 * data of an image to this class, which will then be used if any network
 * requests are received.  
 * Image data is array of RGB pixel structures with one unsigned 8-bit byte for
 * each of R, G and B components of that pixel structure (so each pixel is 24
 * bits). This corresponds to the CV_8UC3 format in OpenCV.
 * 
 * For example, If you have data already in RGB unsigned char array,
 * you can just set it:
 * @code
    // ... set up ARIA, ArNetworking and ArVideo ...

    unsigned char image[width*height*3];
    // ... initialize image data ...

    ArVideoExternalSource videoSource("OpenCV Example");
    ArVideoServer *videoServer = ArVideo::createVideoServer(&serverBase, &videoSource);
    videoSource.open();

    // You can copy updated data:
    while(true)
    {
      // ... do some image processingand update the im object ...
      videoSource.setDataCopy(image, width, height);
    }


    // Or you can just supply a pointer but be sure to lock it while modifying:
    videoSource.setDataPtr(image, width, height);
    while(true)
    {
      videoSource.lock();
      // ... do some image processing and update the im object ...
      videoSource.unlock();
      videoSource.updated();
    }
  


    videoSource.close();

    
   @endcode


 * @todo XXX does ArVideoServer make a copy? If so does it do that during
 * update()? If so we can avoid copying here, and lock during update().
 *
 */
class ArVideoExternalSource : public virtual ArVideoInterface {
protected:
  std::string myName;
  bool myOpen;
  bool myUpdated;
  int myWidth;
  int myHeight;
  bool myDataAllocated;
  unsigned char *myData;
  ArMutex myDataMutex;
// todo  ArGenericCallbackList<ArVideoDataCallback*> myVideoDataCallbacks;
public:
  ArVideoExternalSource(const char *name) : 
    myName(name), myOpen(false), myUpdated(false), 
    myWidth(0), myHeight(0), 
    myDataAllocated(false), myData(0)
  {
  
  }
  virtual ~ArVideoExternalSource() {
    lock();
    if(myDataAllocated)
      delete[] myData;
    unlock();
  }
  /// After setting the pointer, you must use lock() and unlock() while
  /// accessing the data, and call updated() each time it is changed.
  virtual bool setVideoDataPtr(unsigned char *ptr, int width, int height) {
    if(myDataAllocated)
    {
      delete[] myData;
      myDataAllocated = false;
    }
    myData = ptr;
    myWidth = width;
    myHeight = height;
    return true;
  }
  void updated() { 
    myUpdated = true; 
  //todo  myVideoDataCallbacks.invoke(...);
  }
  void lock() {
    myDataMutex.lock();
  }
  void unlock() { 
    myDataMutex.unlock();
  }
  /// Copy data from @a data. You do not need to lock.
  virtual bool updateVideoDataCopy(unsigned char *data, int width, int height) {
    lock();
    if(!myDataAllocated)
    {
      myData = new unsigned char[width * height * bytesPerPixel()];
      myDataAllocated = true;
    }
    // TODO reallocate if needed
    memcpy(myData, data, width*height*bytesPerPixel());
    unlock();
    updated();
    return true;
  }  
  virtual VideoFormat videoFormat() { return VIDEO_RGB24; }
  virtual int bytesPerPixel() { return 3; }
  virtual bool updateVideo() { return true; } 
  virtual bool updateVideoNow() { return myUpdated; } // todo could wait on a condition broadcast by updated
  virtual void addDataCallback(ArVideoDataCallback *functor) {
    // todo myVideoDataCallbacks.push_back(functor); // todo move to default implementation in ArVideoInterface?
  }
  virtual void remDataCallback(ArVideoDataCallback *functor) {
    // todo myVideoDataCallbacks.erase(functor); // todo move to default implementation in ArVideoInterface?
  }
  void open() {
    myOpen = true;
  }
  void close() {
    myOpen = false;
  }
  virtual bool isOpen() {
    return myOpen;
  }
  virtual int getWidth() {
    return myWidth;
  }
  virtual int getHeight() {
    return myHeight;
  }
  virtual unsigned char *getData() {
    return myData;
  }
  virtual const char *getConfigSectionName() {
    return myName.c_str();
  }
  virtual const char *getVideoSize() {
    return "default";
  }
  virtual int getCaptureTimeSubtrahendMsecs() {
    // todo
    return 0;
  }
  virtual bool getCaptureTime(ArTime *t) {
    // todo
    return false;
  }
  virtual std::list<std::string> getCameraParameterNames() {
    return std::list<std::string>();
  }   
  virtual unsigned int getCameraParameterValue(const std::string& param) {
    return 0;
  }
  virtual ArPriority::Priority getMaxConfigParamPriority() {
    return ArPriority::NORMAL;
  }
 };

#endif
 
