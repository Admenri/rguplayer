#include "player.hpp"

#include "utils.hpp"
#include "video_player.hpp"

namespace uvpx {

Player::Config s_defaultConfig;

Player::Player(const Config& cfg) {
  m_videoPlayer = new VideoPlayer(cfg);
}

Player::~Player() {
  if (m_videoPlayer)
    delete m_videoPlayer;
}

Player::LoadResult Player::load(SDL_IOStream* io,
                                int audioTrack,
                                bool preloadFile) {
  return m_videoPlayer->load(io, audioTrack, preloadFile);
}

bool Player::update(float dt) {
  return m_videoPlayer->update(dt);
}

Player::VideoInfo* Player::info() const {
  return m_videoPlayer->info();
}

Frame* Player::lockRead() {
  return m_videoPlayer->frameBuffer()->lockRead();
}

void Player::unlockRead() {
  m_videoPlayer->frameBuffer()->unlockRead();
}

void Player::setOnAudioData(OnAudioDataDecoded func, void* userPtr) {
  m_videoPlayer->setOnAudioData(func, userPtr);
}

void Player::setOnVideoFinished(OnVideoFinished func, void* userPtr) {
  m_videoPlayer->setOnVideoFinished(func, userPtr);
}

double Player::playTime() {
  return m_videoPlayer->playTime();
}

float Player::duration() {
  return m_videoPlayer->duration();
}

void Player::play() {
  m_videoPlayer->play();
}

void Player::pause() {
  m_videoPlayer->pause();
}

void Player::stop() {
  m_videoPlayer->stop();
}

bool Player::isStopped() {
  return m_videoPlayer->isStopped();
}

bool Player::isPaused() {
  return m_videoPlayer->isPaused();
}

bool Player::isPlaying() {
  return m_videoPlayer->isPlaying();
}

bool Player::isFinished() {
  return m_videoPlayer->isFinished();
}

bool Player::readStats(Statistics* dst) {
  return m_videoPlayer->readStats(dst);
}

const Player::Config& Player::defaultConfig() {
  return s_defaultConfig;
}

void Player::setDebugLog(DebugLogFuncPtr func) {
  uvpx::setDebugLog(func);
}

}  // namespace uvpx