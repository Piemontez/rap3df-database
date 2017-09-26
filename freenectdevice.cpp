#include <libfreenect.hpp>

#include "freenectdevice.h"

Freenect::Freenect freenect;

FreenectDevice::FreenectDevice(freenect_context *_ctx, int _index)
        : Freenect::FreenectDevice(_ctx, _index),
          m_buffer_video(freenect_find_video_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_VIDEO_RGB).bytes),
          m_buffer_depth(freenect_find_depth_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_DEPTH_REGISTERED).bytes / 2),
          m_new_rgb_frame(false), m_new_depth_frame(false)
{
    setDepthFormat(FREENECT_DEPTH_REGISTERED);
}

// Do not call directly, even in child
void FreenectDevice::VideoCallback(void *_rgb, uint32_t timestamp)
{
    Mutex::ScopedLock lock(m_rgb_mutex);
    uint8_t* rgb = static_cast<uint8_t*>(_rgb);
    copy(rgb, rgb+getVideoBufferSize(), m_buffer_video.begin());
    m_new_rgb_frame = true;
}

// Do not call directly, even in child
void FreenectDevice::DepthCallback(void *_depth, uint32_t timestamp)
{
    Mutex::ScopedLock lock(m_depth_mutex);
    uint16_t* depth = static_cast<uint16_t*>(_depth);
    copy(depth, depth+getDepthBufferSize()/2, m_buffer_depth.begin());
    m_new_depth_frame = true;
}

bool FreenectDevice::getRGB(std::vector<uint8_t> &buffer)
{
    Mutex::ScopedLock lock(m_rgb_mutex);

    if (!m_new_rgb_frame)
        return false;

    buffer.swap(m_buffer_video);
    m_new_rgb_frame = false;

    return true;
}

bool FreenectDevice::getDepth(std::vector<uint16_t> &buffer)
{
    Mutex::ScopedLock lock(m_depth_mutex);

    if (!m_new_depth_frame)
        return false;

    buffer.swap(m_buffer_depth);
    m_new_depth_frame = false;

    return true;
}

FreenectDevice *FreenectDevice::createDevice()
{
    return &freenect.createDevice<FreenectDevice>(0);
}

