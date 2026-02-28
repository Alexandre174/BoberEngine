#include "Animation.h"

#include <algorithm>

namespace
{
    float SafeFrameDuration(float fps)
    {
        if (fps <= 0.0f) return 0.0f;
        return 1.0f / fps;
    }
}

void Animator::addClip(const std::string& name, AnimationClip clip)
{
    mClips[name] = std::move(clip);
}

bool Animator::play(const std::string& name, bool restart)
{
    auto it = mClips.find(name);
    if (it == mClips.end())
        return false;

    if (!restart && mCurrent == &it->second)
        return true;

    mCurrent = &it->second;
    mCurrentName = name;
    mFramePos = 0;
    mAccum = 0.0f;
    mFinished = false;
    return true;
}

void Animator::playRuntime(AnimationClip clip, bool restart)
{
    if (!restart && mCurrent == &mRuntimeClip)
        return;

    mRuntimeClip = std::move(clip);
    mCurrent = &mRuntimeClip;
    mCurrentName.clear();
    mFramePos = 0;
    mAccum = 0.0f;
    mFinished = false;
}

void Animator::stop()
{
    mCurrent = nullptr;
    mCurrentName.clear();
    mFramePos = 0;
    mAccum = 0.0f;
    mFinished = false;
}

void Animator::update(float deltaTime)
{
    if (!mCurrent)
        return;

    const auto& frames = mCurrent->frames;
    if (frames.empty())
        return;

    if (mFinished)
        return;

    const float frameDuration = SafeFrameDuration(mCurrent->fps);
    if (frameDuration <= 0.0f)
        return;

    mAccum += std::max(0.0f, deltaTime);

    // Advance one or more frames if dt is large.
    while (mAccum >= frameDuration)
    {
        mAccum -= frameDuration;
        ++mFramePos;

        if (mFramePos >= static_cast<int>(frames.size()))
        {
            if (mCurrent->loop)
            {
                mFramePos = 0;
            }
            else
            {
                mFramePos = static_cast<int>(frames.size()) - 1;
                mFinished = true;
                break;
            }
        }
    }
}

int Animator::getFrame() const
{
    if (!mCurrent || mCurrent->frames.empty())
        return 0;

    int idx = mFramePos;
    if (idx < 0) idx = 0;
    const int maxIdx = static_cast<int>(mCurrent->frames.size()) - 1;
    if (idx > maxIdx) idx = maxIdx;
    return mCurrent->frames[idx];
}
