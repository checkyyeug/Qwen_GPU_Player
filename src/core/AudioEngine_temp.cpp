bool AudioEngine::IsPaused() const {
    if (!pImpl->initialized) {
        return false;
    }

    return pImpl->isPaused.load();
}

#endif // ENABLE_FLAC