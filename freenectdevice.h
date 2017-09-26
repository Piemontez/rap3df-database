#ifndef FRENECT_DEVICE_H
#define FRENECT_DEVICE_H

#include <pthread.h>
#include <libfreenect.hpp>
#include <vector>

class Mutex
{
public:
    Mutex()
    {
        pthread_mutex_init(&m_mutex, NULL);
    }

    void lock()
    {
        pthread_mutex_lock(&m_mutex);
    }

    void unlock()
    {
        pthread_mutex_unlock(&m_mutex);
    }

    class ScopedLock
    {
    public:
        ScopedLock(Mutex &mutex) : _mutex(mutex)
        {
            _mutex.lock();
        }

        ~ScopedLock()
        {
            _mutex.unlock();
        }

    private:
        Mutex &_mutex;
    };

private:
    pthread_mutex_t m_mutex;
};


class FreenectDevice : public Freenect::FreenectDevice
{
public:
    FreenectDevice(freenect_context *_ctx, int _index);

    // Do not call directly, even in child
    void VideoCallback(void *_rgb, uint32_t timestamp);
    // Do not call directly, even in child
    void DepthCallback(void *_depth, uint32_t timestamp);

    bool getRGB(std::vector<uint8_t> &buffer);
    bool getDepth(std::vector<uint16_t> &buffer);

    static FreenectDevice* createDevice();
private:
    Mutex m_rgb_mutex;
    Mutex m_depth_mutex;
    std::vector<uint8_t> m_buffer_video;
    std::vector<uint16_t> m_buffer_depth;
    bool m_new_rgb_frame;
    bool m_new_depth_frame;
};

#endif // CAMERA_H
