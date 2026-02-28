#pragma once

#include <string>
#include <unordered_map>
#include <vector>


struct AnimationClip
{
    std::vector<int> frames;
    float fps = 12.0f; // frames per second
    bool loop = true;
};

class Animator
{
public:
    void addClip(const std::string& name, AnimationClip clip);// adds or replaces a named clip

    bool play(const std::string& name, bool restart = true);  // starts playing the clip by name


    void playRuntime(AnimationClip clip, bool restart = true); // plays a one-off "runtime" clip with no name stored

    void stop();

    void update(float deltaTime);

    // Returns the current frame index, 0 if nothing playing
    int getFrame() const;

    bool isPlaying() const { return mCurrent != nullptr; }
    bool isFinished() const { return mFinished; }
    const std::string& currentClipName() const { return mCurrentName; }

private:
    std::unordered_map<std::string, AnimationClip> mClips;

    AnimationClip mRuntimeClip; // Stored runtime clip

    const AnimationClip* mCurrent = nullptr;
    std::string mCurrentName;

    int mFramePos = 0;
    float mAccum = 0.0f;
    bool mFinished = false;
};
