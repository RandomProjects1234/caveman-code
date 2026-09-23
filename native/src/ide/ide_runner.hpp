#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <optional>
#include <string>
#include <thread>

#include "draw.hpp"
#include "interpreter.hpp"

namespace og {

struct RunOutcome {
    enum class Kind { Finished, Stopped, Error };
    Kind kind = Kind::Finished;
    std::string message;
};

class RunManager {
public:
    using OutputFn = std::function<void(const std::string&, const std::string&)>;
    using DoneFn = std::function<void(const RunOutcome&)>;

    RunManager(OutputFn on_output, DoneFn on_done);

    bool start(const std::string& source, const std::string& filename, cmc::DrawSurface* draw);
    void stop();

    bool running() const { return running_; }
    std::optional<std::pair<std::string, int>> pending_input();
    void clear_pending_input();
    void answer_input(const std::string& value);

private:
    std::string ask(const std::string& prompt, int line);
    void sleep_for(double seconds);

    OutputFn on_output_;
    DoneFn on_done_;

    std::thread thread_;
    std::atomic<bool> running_{false};
    std::atomic<bool> stopping_{false};

    std::mutex mutex_;
    std::condition_variable cv_;
    bool has_pending_input_ = false;
    std::string pending_prompt_;
    int pending_line_ = -1;
    bool has_answer_ = false;
    std::string answer_;
};

}